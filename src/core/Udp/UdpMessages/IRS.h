#ifndef IRS_H
#define IRS_H

#define AES_BLOCK_SIZE 16
#define SMEMCPY memcpy

// Data Types


typedef char                    INT_8;
typedef unsigned char           UINT_8;
typedef short                   INT_16;
typedef unsigned short          UINT_16;
typedef int                     INT_32;
typedef unsigned int            UINT_32;
typedef float                   REAL_32;
typedef double                  REAL_64;
typedef long long               INT_64;
typedef unsigned long long      UINT_64;

typedef char STRING_100[100];
typedef char STRING_200[200];
typedef char STRING_50[50] ;

// --

// UNKNOWN_VALUE

#define UNKNOWN_VALUE 0

// --

// CSCI ID
typedef UINT_16 CSCI_ID;

#define CSCI_ID_WCM 1
#define CSCI_ID_DB 2
#define CSCI_ID_DFE 3
#define CSCI_ID_DSS 4
#define CSCI_ID_WVM 5
#define CSCI_ID_DCCC 6
#define CSCI_ID_VMS 10

// --

// MESSAGE_ID

typedef UINT_16 MESSAGE_ID;

// --

// MESSAGE_LENGTH (THIS DATA ELEMENT INDICATES THE LENGTH OF THE MESSAGE EXCLUDING THE LENGTH OF MESSAGE HEADER.)

typedef UINT_16 MESSAGE_LENGTH;

// --

// PRECEDENCE

typedef UINT_8 PRECEDENCE;

#define PRECEDENCE_NONE 0
#define PRECEDENCE_FLASH 1
#define PRECEDENCE_EMERGENCY 2
#define PRECEDENCE_OPERATIONAL_IMMEDIATE 3
#define PRECEDENCE_PRIORITY 4
#define PRECEDENCE_ROUTINE 5
#define PRECEDENCE_DEFFERED 6

// --

// WORKSTATION_INDEX

typedef UINT_8 WORKSTATION_INDEX;

// --


// SUCOMT_INDEX

typedef UINT_8 SUCOMT_INDEX;

// --

// PACKET_SEQ_NO

typedef UINT_16 PACKET_SEQ_NO;

// --


// NO_OF_PACKETS

typedef UINT_16 NO_OF_PACKETS;

// --

// HEALTH_STATUS

typedef UINT_8 HEALTH_STATUS;

#define HEALTH_STATUS_RED 1
#define HEALTH_STATUS_GREEN 2



typedef UINT_8 IDENTITY;

#define HOSTILE 1
#define FRIENDLY 2

typedef UINT_8 SYSTEM_TRACK_TYPE;

#define SYSTEM1 1
#define SYSTEM2 2
#define FUSED 2

typedef UINT_8 TRACK_SOURCE;

#define SOURCE1 1
#define SOURCE2 2





// --


#endif // IRS_H
