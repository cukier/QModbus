#include "qserial.h"
#include "qmodbus.h"

#include <QSerialPort>
#include <QTimer>
#include <QVector>
#include <QThread>

#include <iostream>
#include <iomanip>

QSerial::QSerial(QObject *parent, const QString &port)
    : QObject{parent}
    , serialPort(new QSerialPort(this))
    , timer(new QTimer(this))
{
    serialPort->setPortName(port);
    serialPort->setBaudRate(QSerialPort::Baud19200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);

    connect(serialPort, &QSerialPort::readyRead,
            this, &QSerial::readData);
    connect(timer, &QTimer::timeout,
            this, &QSerial::onTimeOut);
}

QSerial::~QSerial()
{
    if (serialPort->isOpen())
        serialPort->close();

    delete serialPort;
    delete timer;
}

void QSerial::readWords(const quint16 &id, const quint16 &address, const quint16 &len)
{
    QVector<quint16> payload;

    currentRole = ReadWords;
    lastId = id;
    lastAddress = address;
    lastData = len;
    payload.append(len);
    writeData(RequestReadWords, id, address, payload);
}

void QSerial::writeWord(const quint16 &id, const quint16 &address, const quint16 &data)
{
    QVector<quint16> val;

    currentRole = WriteWord;
    lastId = id;
    lastAddress = address;
    lastData = data;
    val.append(data);
    writeData(RequestWriteWords, id, address, val);
}

void QSerial::readReal(const quint16 &id, const quint16 &address, const quint16 &len)
{
    quint16 addr = address * 2;
    QVector<quint16> payload;

    currentRole = ReadReal;
    lastId = id;
    lastAddress = address;
    lastData = len;
    payload.append(len);
    writeData(RequestReadReals, id, addr, payload);
}

void QSerial::writeReal(const quint16 &id, const quint16 &address, const float &val)
{
    quint16 addr = address * 2;
    QVector<quint16> payload = QModbus::getWordFromReal(val);

    currentRole = WriteReal;
    lastId = id;
    lastAddress = address;
    lastVal = val;
    writeData(RequestWriteMultipleWords, id, addr, payload);
}

void QSerial::setTries(const int &novoTries)
{
    if (novoTries != tries) {
        tries = novoTries;
        triesTotal = novoTries;
    }
}

void QSerial::readData()
{
    data.append(serialPort->readAll());

    if (data.length() == responseSize) {
        const auto decoded = QModbus::decodeMessage(data);

        timer->stop();
        std::cout << "recebido: ";

        for (const auto &i: qAsConst(data)) {
            std::cout << "<<0x"
                      << std::hex
                      << std::setw(2)
                      << std::setfill('0')
                      << static_cast<quint16>(i & 0xFF)
                      << ">>";
        }

        std::cout << "\n\rdado: ";

        if (request == RequestReadReals) {
            int len = decoded.length() / 2;

            for (int i = 0; i < len; ++i) {
                QVector<quint16> aux;

                aux.append(decoded.at(i));
                aux.append(decoded.at(i + 1));

                std::cout << std::fixed
                          << std::setprecision(2)
                          << std::setfill('0')
                          << QModbus::getReal(aux)
                          << " ";
            }
        } else {
            for (const auto &i: qAsConst(decoded)) {
                std::cout << "0x"
                          << std::hex
                          << std::setw(4)
                          << std::setfill('0')
                          << static_cast<quint16>(i & 0xFFFF)
                          << " ";
            }
        }

        std::cout << "\n";
        std::cout.flush();
        serialPort->close();

        if (tries < 2)
            std::exit(0);
        else {
            --tries;
            std::cout << "Reenviando tentativa "
                      << triesTotal - tries + 1
                      << " de "
                      << triesTotal
                      << std::endl;
            std::cout.flush();

            switch (currentRole) {
            case ReadWords: {
                readWords(lastId, lastAddress, lastData);
            }
                break;

            case WriteWord: {
                writeWord(lastId, lastAddress, lastData);
            }
                break;

            case ReadReal: {
                readReal(lastId, lastAddress, lastData);
            }
                break;

            case WriteReal: {
                readReal(lastId, lastAddress, lastVal);
            }
                break;

            default: {
                std::cerr << "Comando invalido erro "
                          << currentRole
                          << std::endl;
                std::cerr.flush();
                std::exit(-2);
            }
            }
        }
    } else if (data.length() == 5) {
        timer->stop();
        std::cout << "recebido: ";

        for (const auto &i: qAsConst(data)) {
            std::cout << "<<0x"
                      << std::hex
                      << std::setw(2)
                      << std::setfill('0')
                      << static_cast<quint16>(i & 0xFF)
                      << ">>";
        }

        std::cout << "\n";
        std::cout.flush();
        serialPort->close();
        std::exit(-1);
    }
}

void QSerial::onTimeOut()
{
    if (serialPort->isOpen())
        serialPort->close();

    std::cout << "Sem resposta do dispositivo\n";
    std::cout.flush();

    if (tries < 2)
        std::exit(-3);
    else {
        --tries;
        std::cout << "Reenviando tentativa "
                  << triesTotal - tries + 1
                  << " de "
                  << triesTotal
                  << std::endl;
        std::cout.flush();

        switch (currentRole) {
        case ReadWords: {
            readWords(lastId, lastAddress, lastData);
        }
            break;

        case WriteWord: {
            writeWord(lastId, lastAddress, lastData);
        }
            break;

        case ReadReal: {
            readReal(lastId, lastAddress, lastData);
        }
            break;

        case WriteReal: {
            readReal(lastId, lastAddress, lastVal);
        }
            break;

        default: {
            std::cerr << "Comando invalido erro "
                      << currentRole
                      << std::endl;
            std::cerr.flush();
            std::exit(-2);
        }
        }
    }
}

void QSerial::writeData(const RequestType reuestType, const quint16 &id, const quint16 &addr, const QVector<quint16> &val)
{
    QByteArray req;

    if (!serialPort) {
        std::cout << "error no serial\n";
        std::cout.flush();
        std::exit(-1);
    }

    if ((reuestType == RequestReadWords) || (reuestType == RequestReadBit)
            || (reuestType == RequestReadToWriteBit)) {
        req.append(QModbus::makeReadRequest(id, addr, val.at(0)));
        responseSize = (val.at(0) * 2) + 5;
    } else if (reuestType == RequestReadReals) {
        req.append(QModbus::makeReadRequest(id, addr, val.at(0) * 2));
        responseSize = (val.at(0) * 4) + 5;
    } else if ((reuestType == RequestWriteWords) || (reuestType == RequestWriteBit)) {
        req.append(QModbus::makeWriteSingleRequest(id, addr, val.at(0)));
        responseSize = 8;
    } else if (reuestType == RequestWriteMultipleWords) {
        req.append(QModbus::makeWriteMultipleRequest(id, addr, val));
        responseSize = 8;
    }

    if (req.length()) {
        if (serialPort->open(QIODevice::ReadWrite)) {
            serialPort->setDataTerminalReady(false);
            QThread::sleep(1);
            data.clear();
            request = reuestType;
            startAddress = addr;
            serialPort->write(req);
            std::cout << "enviando: ";

            for (const auto &i: qAsConst(req)) {
                std::cout << "<<0x"
                          << std::hex
                          << std::setw(2)
                          << std::setfill('0')
                          << static_cast<quint16>(i & 0xFF)
                          << ">>";
            }

            std::cout << "\n";
            std::cout.flush();
            timer->start(3000);
        } else {
            std::cout << "Problemas ao abrir a porta " << serialPort->portName().toStdString() << '\n';
            std::cout.flush();
            std::exit(-2);
        }
    }
}
