#include "qmodbus.h"

#include <QVector>

#define l_byte(X)				(X & 0xFF)
#define h_byte(X)				(((X & 0xFF00) >> 8) & 0xFF)

QModbus::QModbus(QObject *parent)
    : QObject{parent}
{

}

quint16 QModbus::ModRTU_CRC(const QByteArray &nData)
{
    quint8 nTemp;
    quint16 wCRCWord = 0xFFFF;

    for (auto const &i : qAsConst(nData)) {
        nTemp = i ^ wCRCWord;
        wCRCWord >>= 8;
        wCRCWord ^= wCRCTable[nTemp];
    }

    return wCRCWord;
}

QByteArray QModbus::makeReadRequest(const quint16 slvAddr, const quint16 &addr, const quint16 &qtd)
{
    QByteArray ret;

    ret.append(quint8(slvAddr));
    ret.append(ReadHoldingRegisters);
    ret.append(h_byte(addr));
    ret.append(l_byte(addr));
    ret.append(h_byte(qtd));
    ret.append(l_byte(qtd));
    const quint16 crc = ModRTU_CRC(ret);
    ret.append(l_byte(crc));
    ret.append(h_byte(crc));

    return ret;
}

QByteArray QModbus::makeWriteSingleRequest(const quint16 slvAddr, const quint16 &addr, const quint16 &data)
{
    QByteArray ret;

    ret.append(quint8(slvAddr));
    ret.append(WriteSingleRegister);
    ret.append(h_byte(addr));
    ret.append(l_byte(addr));
    ret.append(h_byte(data));
    ret.append(l_byte(data));
    const quint16 crc = ModRTU_CRC(ret);
    ret.append(l_byte(crc));
    ret.append(h_byte(crc));

    return ret;
}

QByteArray QModbus::makeWriteMultipleRequest(const quint16 slvAddr, const quint16 &addr, const QVector<quint16> &data)
{
    QByteArray ret;
    quint16 qtd = quint16(data.length());

    ret.append(quint8(slvAddr));
    ret.append(WriteMultipleRegisters);
    ret.append(h_byte(addr));
    ret.append(l_byte(addr));
    ret.append(h_byte(qtd));
    ret.append(l_byte(qtd));
    ret.append(qtd * 2);

    for (auto const &i : qAsConst(data)) {
        ret.append(h_byte(i));
        ret.append(l_byte(i));
    }

    const quint16 crc = ModRTU_CRC(ret);
    ret.append(l_byte(crc));
    ret.append(h_byte(crc));

    return ret;
}

QList<quint16> QModbus::decodeMessage(const QByteArray &msg)
{
    const quint8 sizeMsg = quint8(msg.at(2));
    const QByteArray auxArr = msg.mid(3, sizeMsg);
    QList<quint16> ret;

    for (int i = 0; i < auxArr.length(); i += 2) {
        quint16 aux = 0;
        aux = (auxArr.at(i) << 8) & 0xFF00;
        aux |= (auxArr.at(i + 1) & 0xFF);
        ret.append(aux);
    }

    return ret;
}

bool QModbus::getBit(const QVector<quint16> &mem, const quint16 &bit)
{
    quint16 _word = quint16(bit / 16);
    quint16 _bit = bit - (_word * 16);
    quint16 _mask = (1 << _bit);
    bool ret = mem[_word] & _mask;
    return ret;
}

bool QModbus::getBit(const quint16 &mem, const quint16 &bit)
{
    quint16 _mask = (1 << bit);
    bool ret = mem & _mask;

    return ret;
}

quint16 QModbus::setBit(QVector<quint16> &mem, const quint16 &bit, const bool &value)
{
    quint16 _word = quint16(bit / 16);
    quint16 _bit = bit - (_word * 16);
    quint16 _mask = (1 << _bit);

    if (value) {
        mem[_word] |= _mask;
    } else {
        mem[_word] &= ~(_mask);
    }

    return _word;
}

quint16 QModbus::setBit(quint16 &mem, const quint16 &bit, const bool &value)
{
    quint16 ret;
    quint16 _mask = (1 << bit);

    if (value) {
        ret = mem | _mask;
    } else {
        ret = mem & ~(_mask);
    }

    return ret;
}

double QModbus::getReal(const QVector<quint16> &mem, const quint16 &addr)
{
    union rtof toF;
    quint32 aux = 0;
    quint16 auxAddr = addr * 2;
    float ret = 0.0f;

    aux = (mem.at(auxAddr + 1) << 16) & 0xFFFF0000;
    aux |= mem.at(auxAddr) & 0xFFFF;
    toF.r = aux;
    ret = toF.f;

    return ret;
}

double QModbus::getReal(const QVector<quint16> &val)
{
    if (val.length() != 2)
        return -1.0f;

    union rtof toF;
    quint32 aux = 0;

    aux = (val.at(1) << 16) & 0xFFFF0000;
    aux |= val.at(0) & 0xFFFF;

    toF.r = aux;

    return toF.f;
}

QVector<quint16> QModbus::getWordFromReal(const float real)
{
    union rtof toF;
    QVector<quint16> ret;

    toF.f = real;
    ret.append(toF.r & 0xFFFF);
    ret.append((toF.r & 0xFFFF0000) >> 16);

    return ret;
}
