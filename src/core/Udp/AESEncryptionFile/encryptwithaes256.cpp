#include "encryptwithaes256.h"


EncryptWithAes256::EncryptWithAes256()
{
    key = QByteArray::fromHex("001122334455667fcd99aabbccddeeff00112233445566778899aa78aafb5c"); // 32 bytes
    iv  = QByteArray::fromHex("e90203040aab0708090a0b0c0d0e0f10"); // 16 bytes
}
QByteArray EncryptWithAes256:: aes256Encrypt(const QByteArray &plainText) {

    QByteArray encrypted;
    encrypted.resize(plainText.size() + AES_BLOCK_SIZE);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len;
    int ciphertext_len;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), reinterpret_cast<const unsigned char*>(iv.data()));
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encrypted.data()), &len, reinterpret_cast<const unsigned char*>(plainText.data()), plainText.size());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encrypted.data()) + len, &len);
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(ciphertext_len);





    return encrypted;
}


QByteArray EncryptWithAes256:: aes256Decrypt(const QByteArray &cipherText) {


    QByteArray decrypted;
    decrypted.resize(cipherText.size());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len;
    int plaintext_len;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), reinterpret_cast<const unsigned char*>(iv.data()));
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted.data()), &len, reinterpret_cast<const unsigned char*>(cipherText.data()), cipherText.size());
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted.data()) + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    decrypted.resize(plaintext_len);
    return decrypted;
}
