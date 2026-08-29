#include "processudpmessagethread.h"

ProcessUDPMessageThread::ProcessUDPMessageThread()
{

    timer = new QTimer;

    connect(timer,SIGNAL(timeout()),this,SLOT(timeoutslot()));
    timer->start(1000);
}
void ProcessUDPMessageThread::timeoutslot()
{
    ProcessBroadcastMessages();
}

void ProcessUDPMessageThread::processMessage(STRUCT_MESSAGE_HEADER msg_header, char *my_buf, quint64 total_buffer_len)
{
    if(total_buffer_len>=MAX_MSG_SIZE){ return;}
    char buffer[MAX_MSG_SIZE];
    int offset=0;

    SMEMCPY(buffer,&msg_header,sizeof(msg_header));
    offset+=sizeof(msg_header);

    SMEMCPY(buffer+offset,my_buf,total_buffer_len);
    offset+=total_buffer_len;

    QByteArray message(buffer,offset);

    emit sendUdpDataToMediatorSignal(message);

}



void ProcessUDPMessageThread::ProcessBroadcastMessages()
{

    if(msg_counter_broadcast > 0)
    {
        Total_messages_broadcast = 0;

        mutex_broadcast.lock();
        {
            for(quint32 i = 0; i< msg_counter_broadcast; i++)
            {
                mqbuf_list_broadcast[i] = receive_list_broadcast[i];
                Total_messages_broadcast++;
            }
            msg_counter_broadcast = 0;
        }
        mutex_broadcast.unlock();

        STRUCT_MESSAGE_HEADER msg_header;

        for(quint32 i(0); i < Total_messages_broadcast; i++)
        {
            msg_header = mqbuf_list_broadcast[i].msg_header;

            if(msg_header.no_of_packets==1)
            {
                //TODO remove any entry from the multi list
                processMessage(mqbuf_list_broadcast[i].msg_header,mqbuf_list_broadcast[i].my_buf,msg_header.message_len); //msg_header.msg_len
            }
            else
            {
                processMessage(mqbuf_list_broadcast[i].msg_header,mqbuf_list_broadcast[i].my_buf,msg_header.message_len); //msg_header.msg_len
            }
        }
    }

}

void ProcessUDPMessageThread::addToMultiBufferMap(STRUCT_MQBUF msg)
{

    QString msg_id_cmb=QString("%1_%2").arg(msg.msg_header.source_id).arg(msg.msg_header.message_id);
    if(multibuffer_map.contains(msg_id_cmb)) //already some packet present
    {
        QVector<STRUCT_MQBUF> *msg_list=multibuffer_map.value(msg_id_cmb);

        int pkt_seq_no=msg_list->last().msg_header.packet_seq_no;
        if(msg.msg_header.packet_seq_no == (pkt_seq_no+1)) //if in sequence
        {
            msg_list->append(msg);

            if(msg.msg_header.no_of_packets==msg.msg_header.packet_seq_no) //if last message of sequence
            {
                quint64 total_buf_len=0;
                for(int i=0;i<msg_list->size();i++)
                {
                    total_buf_len+=msg_list->at(i).msg_header.message_len;
                }
                char *msg_buffer=new char[total_buf_len];

                int buf_index=0;
                for(int i=0;i<msg_list->size();i++)
                {
                    SMEMCPY(msg_buffer+buf_index,msg_list->at(i).my_buf,msg_list->at(i).msg_header.message_len);

                    buf_index+=msg_list->at(i).msg_header.message_len;
                }
                processMessage(msg.msg_header,msg_buffer,total_buf_len); //total_buf_len

                delete[] msg_buffer;
                delete msg_list;
                multibuffer_map.remove(msg_id_cmb);
            }
        }
        else
        {
            // out of sequence
            qWarning()<<"UDP Socket :- MuliPacket, packet received out of sequence order";

            if(msg.msg_header.packet_seq_no==1) //the first message , discard previous msg keep the current
            {
                //discard the previous messages
                msg_list->clear();
                msg_list->append(msg);
            }
            else                            //not he first message , discard previous and current msg
            {
                //discard all message including this one
                delete msg_list;
                multibuffer_map.remove(msg_id_cmb);
            }
        }
    }
    else
    {
        if(msg.msg_header.packet_seq_no==1) //if it is the first message
        {
            QVector<STRUCT_MQBUF> *msg_list=new QVector<STRUCT_MQBUF>;
            msg_list->append(msg);

            multibuffer_map.insert(msg_id_cmb,msg_list);
        }
        else
        {
            qWarning()<<"UDP Socket :- MuliPacket, packet received out of sequence order";
        }
    }
}
