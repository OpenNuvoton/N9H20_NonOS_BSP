/****************************************************************************
 * @file     usbvideo.h
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    USB device driver header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef usbvideo_h
#define usbvideo_h


/* Camera capabilities (maximum) */
#define FRAMES_PER_DESC                8


typedef UINT32   VID_SIZE_T;


#define USBVIDEO_NUMFRAMES      2       /* How many frames we work with */
#define USBVIDEO_NUMSBUF        2       /* How many URBs linked in a ring */

/* This structure represents one Isoc request - URB and buffer */
typedef struct 
{
    UINT8   *data;
    URB_T   *urb;
} USB_VID_STREAM_BUF_T;


#define FRAME_QUEUE_MAX_NUMBER          4096

typedef struct vid_buf
{
    INT      packet_idx;
    struct vid_buf  *next;
    UINT8    *iso_buf;
    ISO_PACKET_DESCRIPTOR_T  iso_packet[FRAMES_PER_DESC];
} USB_VID_BUF_T;


typedef struct 
{
    USB_DRIVER_T  usbdrv;              /* Interface to the USB stack */
    CHAR    drvName[80];               /* Driver name */
    USB_DEV_T  *dev;
    UINT8   iface;                     /* Video interface number */
    UINT8   video_endp;
    UINT8   ifaceAltActive;
    UINT8   ifaceAltInactive;          /* Alt settings */
    UINT32  flags;                     /* FLAGS_USBVIDEO_xxx */
    INT     user;                      /* user count for exclusive use */

    VID_SIZE_T  videosize;             /* Current setting */
    VID_SIZE_T  canvas;                /* This is the width,height of the V4L canvas */
    INT     max_frame_size;            /* Bytes in one video frame */

    INT     uvd_used;                  /* Is this structure in use? */
    INT     streaming;                 /* Are we streaming Isochronous? */

    INT     iso_packet_len;            /* Videomode-dependent, saves bus bandwidth */

    USB_VID_STREAM_BUF_T  sbuf[USBVIDEO_NUMSBUF];
        
    /* usb Iso frame queue */
    //NU_SEMAPHORE    vbq_semaphore;
    USB_VID_BUF_T  *vbq_head;
    USB_VID_BUF_T  *vbq_tail;
    UINT32  vbq_cnt;
} UVD_T;


/* YCHuang, backdoor routine */
INT   W99683_GetFramePiece(UINT8 **pbuf, INT *length);

INT   W99683Cam_StartDataPump(VOID);
VOID  W99683Cam_StopDataPump(VOID);
INT   W99683Cam_IsStreaming(VOID);
INT   W99683Cam_IsConnected(VOID);
INT   W99683Cam_Open(VOID);
INT   W99683Cam_Init(VOID);
VOID  W99683Cam_Exit(VOID);

#endif /* usbvideo_h */
