#include "udpcommunication.h"
#include "datastore.h"

void UDPCommunication::run()
{

    initUdpCommunication();
}

UDPCommunication::UDPCommunication(quint16 port, QObject *parent)
{
    Q_UNUSED(parent);
    _recv_port = port;

    recv_socket = new QUdpSocket;
    recv_socket->moveToThread(this);


}

void UDPCommunication::initUdpCommunication()
{
    if(recv_socket->bind(QHostAddress::AnyIPv4,_recv_port, QUdpSocket::DontShareAddress)==false)
    {

        //Problem in binding
        qDebug()<<QString("UDP Socket :- Unable to bind at port %1").arg(_recv_port);
        ::exit(1);

    }
    qDebug()<<QString("UDP Socket :- Successfully bind at port %1").arg(_recv_port);
    readPendingDatagrams();


}


void UDPCommunication::readPendingDatagrams()
{
    while(1)
    {
        if(!recv_socket)
        {
            return;
        }

        while (recv_socket->hasPendingDatagrams())
        {

            QHostAddress sender;
            quint16 senderPort;
            qint64  length;

            length = recv_socket->readDatagram((char *)&mqbuf_obj_broadcast, MAX_MSG_SIZE, &sender, &senderPort);
            if(length>0)
            {
                if((ProcessUDPMessageThread::getInstance().msg_counter_broadcast < (unsigned int)NO_OF_MSG_IN_QUEUE))
                {
                    mutex_broadcast.lock();
                    SMEMCPY((char *)&(ProcessUDPMessageThread::getInstance().receive_list_broadcast[ProcessUDPMessageThread::getInstance().msg_counter_broadcast]), (char *)&(mqbuf_obj_broadcast), MAX_MSG_SIZE);
                    ProcessUDPMessageThread::getInstance().msg_counter_broadcast++;
                    mutex_broadcast.unlock();
                }
                else
                {
                    // =============== Packet Discarded============
                }
            }
            else
            {
                // invalid length message
            }

        }

    }

}

void UDPSendData::run()
{
    while (1) {
        if (DataStore::getInstance().recvByteArrayList.size() > 0) {
            DataStore::getInstance().serverMutex.lock();
            QList<QByteArray> receivedDataList = DataStore::getInstance().recvByteArrayList;
            DataStore::getInstance().recvByteArrayList.clear();
            DataStore::getInstance().serverMutex.unlock();

            for (const QByteArray& encryptedData : receivedDataList) {
                QByteArray decryptedData = DataStore::getInstance().encryptionObj.aes256Decrypt(encryptedData);


                STRUCT_MESSAGE_HEADER msg_header;
                SMEMCPY(&msg_header,decryptedData.data(),sizeof(msg_header));

                sendData(DataStore::getInstance().server_side_send_udp_ip,DataStore::getInstance().server_side_send_udp_port,decryptedData);
            }
        }
        QThread::msleep(10);
    }
}



UDPSendData::UDPSendData()
{
    send_socket = new QUdpSocket;
    send_socket->moveToThread(this);
}

void UDPSendData::sendData(QString ip, quint16 port, QByteArray send_bytes)
{
    qint64 no_of_bytes_send = send_socket->writeDatagram(send_bytes,QHostAddress(ip),port);
    qDebug()<<QString("UDP data send to ip= %1, port= %2 ... Total no of bytes sent %3").arg(ip).arg(port).arg(no_of_bytes_send);
    QThread::usleep(10);

}

void UDPSendData::sendDataInMultiPacket(QString ip, quint16 port, QByteArray send_bytes)
{

        quint64 length = send_bytes.size();
        int size_of_header=sizeof(STRUCT_MESSAGE_HEADER);

        quint64 max_msg_buff_size=MAX_SENT_MSG_DATA_SIZE-size_of_header;

        STRUCT_MESSAGE_HEADER msg_header;
        SMEMCPY(&msg_header,send_bytes.data(),sizeof(STRUCT_MESSAGE_HEADER));

        int no_of_pkt=qCeil((length-size_of_header)/(qreal)max_msg_buff_size);
        int pkt_seq_no=1;

        msg_header.no_of_packets=no_of_pkt;

        quint64 offset=size_of_header;
        quint64 remaing_buff_size=length-size_of_header;
        while(remaing_buff_size>0)
        {
            msg_header.packet_seq_no=pkt_seq_no;
            if(remaing_buff_size>max_msg_buff_size)
            {
                msg_header.message_len=max_msg_buff_size;

                char buffer[MAX_SENT_MSG_DATA_SIZE];
                SMEMCPY(buffer,&msg_header,size_of_header);
                SMEMCPY(buffer+size_of_header,send_bytes.data()+offset,max_msg_buff_size);

                sendData(ip,port,QByteArray(buffer,msg_header.message_len+size_of_header));
                offset = offset + msg_header.message_len;
                remaing_buff_size=remaing_buff_size-max_msg_buff_size;
            }
            else
            {
                char buffer[MAX_SENT_MSG_DATA_SIZE];
                msg_header.message_len=remaing_buff_size;

                SMEMCPY(buffer,&msg_header,size_of_header);
                SMEMCPY(buffer+size_of_header,send_bytes.data()+offset,remaing_buff_size);

                sendData(ip,port,QByteArray(buffer,msg_header.message_len+size_of_header));

                remaing_buff_size=0;
            }

            QThread::msleep(1);
            qDebug()<<QString("Sending Data to IP: %1, Port: %2 ,Pkt seq no: %3 of Total Packets : %4 ").arg(ip).arg(port).arg(pkt_seq_no).arg(no_of_pkt);
            pkt_seq_no++;
        }
}

void UDPSendData::deleteLater()
{
    send_socket->deleteLater();
}
