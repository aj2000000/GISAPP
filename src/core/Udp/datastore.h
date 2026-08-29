#ifndef DATASTORE_H
#define DATASTORE_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QList>
#include <QByteArray>
#include <QMutex>
#include "IRS.h"

#include "encryptwithaes256.h"



class DataStore : public QObject
{
    Q_OBJECT

    // Class constructor

    DataStore();//private constructor
    DataStore(DataStore const& copy);//stop Compiler generating methods
    //of copy object & not Implemented
    DataStore& operator=(DataStore const& copy);

    // --

public:

    static DataStore& getInstance(){
        //the only instance
        //guaranteed to be lazy initialized

        //guaranteed will be destroyed correctely
        static DataStore instance;
        return instance;
    }

    int client_side_udp_recv_port=8540;
    QString client_side_server_ip="127.0.0.1";

    int server_side_send_udp_port=8541;
    QString server_side_send_udp_ip="127.0.0.1";

    void readConfigurationFiles();
    QString configPath = "";

    QMutex serverMutex;
    QList<QByteArray> recvByteArrayList;
    EncryptWithAes256 encryptionObj;

private:





signals:

};

#endif // DATASTORE_H
