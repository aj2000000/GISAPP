#ifndef UDPCOMMUNICATION_H
#define UDPCOMMUNICATION_H

#include <QObject>
#include <QUdpSocket>
#include <QDebug>
#include <Structures.h>
#include <QMutex>
#include <QTimer>
#include <QtMath>
#include <QThread>
#include "processudpmessagethread.h"


class DataStore;
class UDPCommunication : public QThread
{
    Q_OBJECT

public:
    void run() override;
    explicit UDPCommunication(quint16 port,QObject *parent = nullptr);
    void initUdpCommunication();





private slots:
    void readPendingDatagrams();


private:
    quint16 _recv_port;

    QUdpSocket *recv_socket;


    STRUCT_MQBUF   mqbuf_obj_broadcast;
    QMutex         mutex_broadcast;




};


class UDPSendData : public QThread{
    QUdpSocket *send_socket;
    QList<QByteArray> sendByteArrayList;
public:
    void run() override;
    UDPSendData();
    void sendData(QString ip,quint16 port, QByteArray send_bytes);
    void sendDataInMultiPacket(QString ip,quint16 port, QByteArray send_bytes);
    void deleteLater();
};

#endif // UDPCOMMUNICATION_H
