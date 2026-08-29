#ifndef PROCESSUDPMESSAGETHREAD_H
#define PROCESSUDPMESSAGETHREAD_H
#include "datastore.h"
#include "Structures.h"
#include <QTimer>
#include <QUdpSocket>
class ProcessUDPMessageThread: public QObject
{
    Q_OBJECT
    QTimer *timer;


    ProcessUDPMessageThread(); //private constructor
    ProcessUDPMessageThread(ProcessUDPMessageThread const& copy);//stop Compiler generating methods
    //of copy object & not Implemented
    ProcessUDPMessageThread& operator=(ProcessUDPMessageThread const& copy);
public:
    static ProcessUDPMessageThread& getInstance(){
        //the only instance
        //guaranteed to be lazy initialized

        //guaranteed will be destroyed correctely
        static ProcessUDPMessageThread instance;
        return instance;
    }

    STRUCT_MQBUF mqbuf_list_broadcast[NO_OF_MSG_IN_QUEUE];
    quint32        msg_counter_broadcast;
    STRUCT_MQBUF   mqbuf_obj_broadcast;
    QMutex         mutex_broadcast;
    STRUCT_MQBUF   receive_list_broadcast[NO_OF_MSG_IN_QUEUE];


signals:
    void sendUdpDataToMediatorSignal(QByteArray);

public slots:
    void ProcessBroadcastMessages();
    void timeoutslot();

private:
    quint32 Total_messages_broadcast;

    //for multi
    QMap<QString,QVector<STRUCT_MQBUF>*> multibuffer_map;
    void addToMultiBufferMap(STRUCT_MQBUF);
    //--for multi


    void processMessage(STRUCT_MESSAGE_HEADER msg_header,char *my_buf,quint64 total_buffer_len);


};

#endif // PROCESSUDPMESSAGETHREAD_H
