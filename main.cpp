#include <QCoreApplication>
#include <QVector>

#include <iostream>

#include "qserial.h"

#define PORT_ARG            "-p"
#define READWORD_CMD        "-rw"
#define WRITEWORD_CMD       "-ww"
#define READFLOAT_CMD       "-rf"
#define WRITEFLOAT_CMD      "-wf"
#define READBOOL_CMD        "-rb"
#define WRITEBOOL_CMD       "-wb"
#define ID_CMD              "-i"
#define ADDR_CMD            "-a"
#define VAL_CMD             "-v"
#define TRIES               "-t"

int main(int argc, char *argv[])
{
    if (argc > 1) {
        int indexPorta = -1;
        int indexWriteWord = -1;
        int indexReadWord = -1;
        int indexWriteFloat = -1;
        int indexReadFloat = -1;
        int indexWriteBool = -1;
        int indexReadBool = -1;
        int indexID = -1;
        int indexAddr = -1;
        int indexVal = -1;
        int indexTries = -1;
        int id = -1;
        int addr = -1;
        int valWordBool = -1;
        int tries = -1;
        float valReal = -1.0;

        for (int i = 0; i < argc; ++i) {
            std::string str(argv[i]);

            if (!str.compare(PORT_ARG)) {
                indexPorta = i + 1;
            } else if (!str.compare(ID_CMD)) {
                indexID = i + 1;
            } else if (!str.compare(ADDR_CMD)) {
                indexAddr = i + 1;
            } else if (!str.compare(VAL_CMD)) {
                indexVal = i + 1;
            } else if (!str.compare(READWORD_CMD)) {
                indexReadWord = i;
            } else if (!str.compare(WRITEWORD_CMD)) {
                indexWriteWord = i;
            } else if (!str.compare(READFLOAT_CMD)) {
                indexReadFloat = i + 1;
            } else if (!str.compare(WRITEFLOAT_CMD)) {
                indexWriteFloat = i + 1;
            } else if (!str.compare(READBOOL_CMD)) {
                indexReadBool = i;
            } else if (!str.compare(WRITEBOOL_CMD)) {
                indexWriteBool = i;
            } else if (!str.compare(TRIES)) {
                indexTries = i + 1;
            }
        }

        if (indexPorta == -1) {
            std::cout << "Necessario indicar porta serial '-p porta'" << std::endl;
            std::cout.flush();
            return -1;
        }

        if ((indexWriteWord == -1) && (indexReadWord == -1) &&
                (indexWriteFloat == -1) && (indexReadFloat == -1) &&
                (indexWriteBool == -1) && (indexReadBool == -1)) {
            std::cout << "Necessario indicar um comando escrita '-ww' ou leitura '-rw' para as words\n"
                      << "escrita '-wf' ou leitura '-rf' para os numeros reais\n"
                      << "escrita '-wb' ou leitura '-rb' para os booleanos\n";
            std::cout.flush();
            return -1;
        }

        if (indexID == -1) {
            std::cout << "Necessario indicar id do escravo '-i'\n";
            std::cout.flush();
            return -1;
        } else if (indexAddr == -1) {
            std::cout << "Necessario indicar endereco do escravo '-a'\n";
            std::cout.flush();
            return -1;
        } else if ((indexVal == -1) && (valReal == -1.0)) {
            std::cout << "Necessario indicar o valor a ser escrito/lido do escravo '-v'\n";
            std::cout.flush();
            return -1;
        }

        try {
            id = std::stoi(argv[indexID]);
            addr = std::stoi(argv[indexAddr]);

            if (indexTries != -1)
                tries = std::stoi(argv[indexTries]);

            if ((indexWriteFloat != -1) || (indexReadFloat != -1)) {
                valReal = std::stof(argv[indexVal]);
            } else {
                valWordBool = std::stoi(argv[indexVal]);
            }
        } catch (std::invalid_argument const&) {
            std::cout << "valores de id addr e val inconsistentes\n";
            std::cout.flush();
            return -1;
        }

        if (indexReadWord != -1) {
            QCoreApplication a(argc, argv);
            QSerial s(&a, argv[indexPorta]);

            if (tries != -1)
                s.setTries(tries);

            s.readWords(quint16(id), quint16(addr), quint16(valWordBool));
            return a.exec();
        } else if (indexWriteWord != -1) {
            QCoreApplication a(argc, argv);
            QSerial s(&a, argv[indexPorta]);

            if (tries != -1)
                s.setTries(tries);

            s.writeWord(quint16(id), quint16(addr), quint16(valWordBool));
            return a.exec();
        } else if (indexReadFloat != -1) {
            QCoreApplication a(argc, argv);
            QSerial s(&a, argv[indexPorta]);

            if (tries != -1)
                s.setTries(tries);

            s.readReal(quint16(id), quint16(addr), quint16(valReal));
            return a.exec();
        } else if (indexWriteFloat != -1) {
            QCoreApplication a(argc, argv);
            QSerial s(&a, argv[indexPorta]);

            if (tries != -1)
                s.setTries(tries);

            s.writeReal(quint16(id), quint16(addr), static_cast<float>(valReal));
            return a.exec();
        } else if (indexReadBool != -1) {
            //todo: ler bool
        } else if (indexWriteBool != -1) {
            //todo: escrever bool
        }

        std::cout << "nao era para ter chego aqui!!!\n";
        std::cout.flush();
        return -1;
    } else {
        std::cout << "falta de argumentos" << std::endl;
        std::cout.flush();
    }

    return -1;
}
