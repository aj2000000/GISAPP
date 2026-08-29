#ifndef ENCRYPTWITHAES256_H
#define ENCRYPTWITHAES256_H


#include <QDebug>
#include <QByteArray>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "IRS.h"
#include <QElapsedTimer>
class EncryptWithAes256
{
    QByteArray key;
    QByteArray iv;
    QElapsedTimer timer;
public:
    EncryptWithAes256();
    QByteArray aes256Encrypt(const QByteArray &plainText);
    QByteArray aes256Decrypt(const QByteArray &cipherText);
};

#endif // ENCRYPTWITHAES256_H
