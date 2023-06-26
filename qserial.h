#ifndef QSERIAL_H
#define QSERIAL_H

#include <QObject>

class QSerialPort;
class QTimer;

class QSerial : public QObject
{
    Q_OBJECT

public:
    enum RequestType {
        RequestASCII,
        RequestReadWords,
        RequestReadReals,
        RequestReadBit,
        RequestWriteWords,
        RequestWriteMultipleWords,
        RequestReadToWriteBit,
        RequestWriteBit
    };
    Q_ENUM(RequestType)

    enum RoleType {
        NoRole,
        ReadWords,
        WriteWord,
        ReadReal,
        WriteReal,
        ReadBool,
        WriteBool
    };
    Q_ENUM(RoleType)

    explicit QSerial(QObject *parent = nullptr, const QString &port = "");
    ~QSerial();

    void readWords(const quint16 &id, const quint16 &address, const quint16 &len);
    void writeWord(const quint16 &id, const quint16 &address, const quint16 &data);

    void readReal(const quint16 &id, const quint16 &address, const quint16 &len);
    void writeReal(const quint16 &id, const quint16 &address, const float &val);

    void readBool(const quint16 &id, const quint16 &address, const quint16 &len);
    void writeBool(const quint16 &id, const quint16 &address, const quint16 &len);

    void setTries(const int &novoTries);

signals:

private slots:
    void readData();
    void onTimeOut();

private:
    QSerialPort *serialPort;
    QTimer *timer;

    QByteArray data;
    RequestType request;
    RoleType currentRole = NoRole;
    int responseSize;
    int startAddress;
    int tries = -1;
    int triesTotal = -1;
    quint16 lastId;
    quint16 lastAddress;
    quint16 lastData;
    float lastVal;

    void writeData(const RequestType reuestType, const quint16 &id, const quint16 &addr,
                   const QVector<quint16> &val);
};

#endif // QSERIAL_H
