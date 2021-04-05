/*****************************************************************************
 Copyright(c) 2009 FCI Inc. All Rights Reserved

 File name : bbm.c

 Description : API of dmb baseband module

 History :
 ----------------------------------------------------------------------
 2009/08/29     jason        initial
*******************************************************************************/

#ifndef __ISDBT_H__
#define __ISDBT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/list.h>

#include "fci_types.h"
#include "fci_ringbuffer.h"

#define CTL_TYPE 0
#define TS_TYPE 1

#define ISDBT_IOC_MAGIC    'I'
//#define IOCTL_MAXNR        18
#define IOCTL_MAXNR        44

typedef struct
{
    unsigned short     lock;    /*baseband lock state                     (1: Lock, 0:Unlock)*/
    unsigned short    cn;        /*Signal Level (C/N)                     (0 ~ 28)*/
    unsigned int        ber;     /*Bit Error rate                         (0 ~ 100000)*/
    unsigned int        per;      /*Packet Error rate                      (0 ~ 100000)*/
    unsigned short    agc;      /*Auto Gain control                      (0 ~ 255)*/
    int                 rssi;      /*Received Signal Strength Indication      (0 ~ -99)*/
    unsigned short    ErrTSP;
    unsigned short    TotalTSP;
    unsigned int        Num;
    unsigned int        Exp;
    unsigned int        mode;
} IOCTL_ISDBT_SIGNAL_INFO;

#define IOCTL_ISDBT_POWER_ON            _IO(ISDBT_IOC_MAGIC,10)
#define IOCTL_ISDBT_POWER_OFF            _IO(ISDBT_IOC_MAGIC, 11)
#define IOCTL_ISDBT_SCAN_FREQ            _IOW(ISDBT_IOC_MAGIC,12, unsigned int)
#define IOCTL_ISDBT_SET_FREQ            _IOW(ISDBT_IOC_MAGIC,13, unsigned int)
#define IOCTL_ISDBT_GET_LOCK_STATUS        _IOR(ISDBT_IOC_MAGIC,14, unsigned int)
#define IOCTL_ISDBT_GET_SIGNAL_INFO        _IOR(ISDBT_IOC_MAGIC,15, IOCTL_ISDBT_SIGNAL_INFO)
#define IOCTL_ISDBT_START_TS            _IO(ISDBT_IOC_MAGIC,16)
#define IOCTL_ISDBT_STOP_TS                _IO(ISDBT_IOC_MAGIC,17)

#define LGE_BROADCAST_DMB_IOCTL_MAGIC 'I'

#define LGE_BROADCAST_DMB_IOCTL_ON \
    _IO(LGE_BROADCAST_DMB_IOCTL_MAGIC, 30)

#define LGE_BROADCAST_DMB_IOCTL_OFF \
    _IO(LGE_BROADCAST_DMB_IOCTL_MAGIC, 31)

#define LGE_BROADCAST_DMB_IOCTL_OPEN \
    _IOW(LGE_BROADCAST_DMB_IOCTL_MAGIC, 32, struct broadcast_dmb_init_info)

#define LGE_BROADCAST_DMB_IOCTL_CLOSE \
    _IO(LGE_BROADCAST_DMB_IOCTL_MAGIC, 33)
#define LGE_BROADCAST_DMB_IOCTL_SET_CH \
    _IOW(LGE_BROADCAST_DMB_IOCTL_MAGIC, 35, struct broadcast_dmb_set_ch_info)

#define LGE_BROADCAST_DMB_IOCTL_RESYNC \
    _IOW(LGE_BROADCAST_DMB_IOCTL_MAGIC, 36, int)

#define LGE_BROADCAST_DMB_IOCTL_DETECT_SYNC \
    _IOR(LGE_BROADCAST_DMB_IOCTL_MAGIC, 37, struct broadcast_dmb_sync_info)

#define LGE_BROADCAST_DMB_IOCTL_GET_SIG_INFO \
    _IOWR(LGE_BROADCAST_DMB_IOCTL_MAGIC, 38, struct broadcast_dmb_control_info)

#define LGE_BROADCAST_DMB_IOCTL_GET_CH_INFO \
    _IOR(LGE_BROADCAST_DMB_IOCTL_MAGIC, 39, struct broadcast_dmb_ch_info)

#define LGE_BROADCAST_DMB_IOCTL_RESET_CH \
    _IO(LGE_BROADCAST_DMB_IOCTL_MAGIC, 40)

#define LGE_BROADCAST_DMB_IOCTL_USER_STOP \
    _IOW(LGE_BROADCAST_DMB_IOCTL_MAGIC, 41, int)

#define LGE_BROADCAST_DMB_IOCTL_GET_DMB_DATA \
    _IOW(LGE_BROADCAST_DMB_IOCTL_MAGIC, 42, struct broadcast_dmb_data_info)

#define LGE_BROADCAST_DMB_IOCTL_SELECT_ANTENNA \
    _IOW(LGE_BROADCAST_DMB_IOCTL_MAGIC, 43, int)


struct broadcast_dmb_set_ch_info
{
    unsigned int    mode;
    unsigned int    rf_band;
    unsigned int    channel;
    unsigned int    subchannel;
    unsigned int    segment;
};
struct broadcast_dmb_init_info
{
    unsigned int rf_band;
    unsigned int segment;
};

typedef struct
{
    int lock;    /*baseband lock state                     (1: Lock, 0:Unlock)*/
    int cn;        /*Signal Level (C/N)                     (0 ~ 28)*/
    int ber;     /*Bit Error rate                         (0 ~ 100000)*/
    int per;      /*Packet Error rate                      (0 ~ 100000)*/
    int agc;      /*Auto Gain control                      (0 ~ 255)*/
    int rssi;      /*Received Signal Strength Indication      (0 ~ -99)*/
    int ErrTSP;
    int TotalTSP;
    int antenna_level;
    int Num;
    int Exp;
    int mode;
}oneseg_sig_info;

#if 1
typedef struct
{
    int cn;
    int ber_a;
    int per_a;
    int layerinfo_a;
    int tmccinfo;
    int receive_status;
    int rssi;
    int scan_status;
    int sysinfo;
    int total_tsp_a;

    int ber_b;
    int per_b;
    int layerinfo_b;
    int total_tsp_b;

    int ber_c;
    int per_c;
    int layerinfo_c;
    int total_tsp_c;

    int antenna_level_fullseg;
    int antenna_level_oneseg;
    int agc;
    int ber_1seg;
    int per_1seg;
    int total_tsp_1seg;
    int err_tsp_1seg;
    int ber_fullseg;
    int per_fullseg;
    int total_tsp_fullseg;
    int err_tsp_fullseg;
    int oneseg_to_fullseg_value;
    int fullseg_to_oneseg_value;
}mmb_sig_info;
#else
typedef struct
{
    int cn;
    int ber;
    int per;
    int layerinfo;
    int tmccinfo;
    int receive_status;
    int rssi;
    int scan_status;
    int sysinfo;
    int TotalTSP;
}mmb_sig_info;
#endif

struct broadcast_dmb_sync_info
{
    unsigned int sync_status;
    unsigned int sync_ext_status;
};

struct broadcast_dmb_sig_info
{
    union
    {
        oneseg_sig_info oneseg_info;
        mmb_sig_info mmb_info;
    }info;
};

struct broadcast_dmb_ch_info
{
    unsigned int    ch_buf_addr;
    unsigned int    buf_len;
};

struct broadcast_dmb_data_info
{
    unsigned int    data_buf_addr;
    unsigned int    data_buf_size;
    unsigned int    copied_size;
    unsigned int    packet_cnt;
};

typedef struct
{
    unsigned short    reserved;
    unsigned char    subch_id;
    unsigned short    size;
    unsigned char    data_type:7;
    unsigned char    ack_bit:1;
} DMB_BB_HEADER_TYPE;

struct broadcast_dmb_cmd_info
{
    unsigned int service_type;
    unsigned int cmd;
    unsigned int layer;
    unsigned int mode;
    unsigned int fullseg_oneseg_flag;
    unsigned int over;
};

struct broadcast_dmb_control_info
{
    struct broadcast_dmb_cmd_info cmd_info;
    struct broadcast_dmb_sig_info sig_info;
};

typedef enum
{
    DMB_BB_DATA_TS,
    DMB_BB_DATA_DAB,
    DMB_BB_DATA_PACK,
    DMB_BB_DATA_FIC,
    DMB_BB_DATA_FIDC
} DMB_BB_DATA_TYPE;

enum
{
    DMB_OP_CMD_NORMAL = 0x00,
    DMB_OP_CMD_SCAN,
    DMB_OP_CMD_CHANGE_MODE
};

typedef enum {
    ISDBT_POWERON       = 0,
    ISDBT_POWEROFF        = 1,
    ISDBT_DATAREAD        = 2
} ISDBT_MODE;
typedef struct {
    HANDLE                *hInit;
    struct list_head        hList;
    struct fci_ringbuffer        RingBuffer;
    u8                *buf;
    u8                isdbttype;
} ISDBT_OPEN_INFO_T;

typedef struct {
    struct list_head        hHead;
} ISDBT_INIT_INFO_T;

#ifdef __cplusplus
}
#endif

#endif // __ISDBT_H__
