#ifndef STRUCTURES_H
#define STRUCTURES_H
#include<QtGlobal>
#include "IRS.h"
#include <QtCore>

int const NO_OF_MSG_IN_QUEUE = 3000;
int const MAX_MSG_SIZE = 8000;
int const MAX_SENT_MSG_DATA_SIZE = 2000;




#pragma pack(push, 1)

typedef struct __attribute__ ((packed))
{
    CSCI_ID source_id=0;
    CSCI_ID destination_id=0;
    MESSAGE_ID message_id=0;
    MESSAGE_LENGTH message_len=0;
    PRECEDENCE precedence;
    WORKSTATION_INDEX ws_index;
    SUCOMT_INDEX sucomt_index;
    PACKET_SEQ_NO packet_seq_no=0;
    NO_OF_PACKETS no_of_packets=0;

}STRUCT_MESSAGE_HEADER;

// FOR TESTING only

typedef struct
{
    STRUCT_MESSAGE_HEADER msg_header;
    char *data;

}STRUCT_MESSAGE;



// --

typedef struct _DCCC_DB_CSC_HEALTH_MSG_
{
    STRUCT_MESSAGE_HEADER struct_msg_header;
    STRING_50             csc_id;
    HEALTH_STATUS         health_status;

    _DCCC_DB_CSC_HEALTH_MSG_()
    {
        SMEMCPY(&csc_id,"",sizeof(csc_id));
        health_status = UNKNOWN_VALUE;
    }

}DCCC_DB_CSC_HEALTH_MSG;



typedef struct __attribute__ ((packed))
{
    STRUCT_MESSAGE_HEADER   msg_header;
    char                    my_buf[MAX_MSG_SIZE];
}STRUCT_MQBUF;


typedef struct __attribute__ ((packed))
{
    double latatitude;
    double longitude;
    double height;
    double dir;

}STRUCT_LOCATION;

typedef struct __attribute__ ((packed))
{
    UINT_8 type;
    UINT_8 sub_type;
    UINT_8 classification;
    UINT_8 strength;
    UINT_8 act_type;
    UINT_8 act_sub_type;
    UINT_8 act_classification;

}STRUCT_TRACK_ATTRIBUTES;

typedef struct __attribute__ ((packed))
{
    TRACK_SOURCE source;
    STRING_50 source_id;

}STRUCT_TRACK_SOURCE;

typedef struct __attribute__ ((packed))
{
    STRING_50 symbol_name;

}STRUCT_TRACK_SYMBOL;

typedef struct __attribute__ ((packed))
{
    UINT_8 day;
    UINT_8 month;
    UINT_16 year;
}STRUCT_DATE;

typedef struct __attribute__ ((packed))
{
    UINT_8 hour;
    UINT_8 minute;
    UINT_16 second;

}STRUCT_TIME;


typedef struct __attribute__ ((packed))
{
    STRUCT_DATE date;
    STRUCT_TIME time;

}STRUCT_DATE_TIME;


typedef struct __attribute__ ((packed))
{
    STRUCT_MESSAGE_HEADER   msg_header;
    UINT_8 track_id;
    STRING_100 track_name;
    STRUCT_LOCATION track_loc;
    IDENTITY track_identity;
    STRUCT_TRACK_ATTRIBUTES track_attributes;
    SYSTEM_TRACK_TYPE sys_track_type;
    UINT_8 no_of_sources;
    QVector<STRUCT_TRACK_SOURCE> track_sources;
    STRUCT_TRACK_SYMBOL track_symbol;
    STRUCT_DATE_TIME track_report_time;
    STRING_100 track_remarks;

}STRUCT_TRACK;




typedef struct __attribute__ ((packed))
{
    STRUCT_MESSAGE_HEADER   msg_header;
    UINT_16 no_of_tracks;
    QVector<STRUCT_TRACK> tracks;
}MAIN_LITE_TRACK_MSG;





#pragma pack(pop)
#endif // STRUCTURES_H
