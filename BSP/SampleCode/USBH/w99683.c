/****************************************************************************
 * @file     W99683.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    USBH sample source file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

/*-------------------------------------------------------------------------

unhandled interfaces on deviceWarning! - USB device 2 (vend/prod 0x416/0x9683) is not claimed by any active driver.

  Length              = 18
  DescriptorType      = 01
  USB version         = 1.10
  Vendor:Product      = 0416:9683
  MaxPacketSize0      = 16
  NumConfigurations   = 1
  Device version      = 1.00
  Device Class:SubClass:Protocol = 00:00:00
    Per-interface classes

Configuration:
  bLength             =    9
  bDescriptorType     =   02
  wTotalLength        = 00C2
  bNumInterfaces      =   03
  bConfigurationValue =   01
  iConfiguration      =   00
  bmAttributes        =   80
  MaxPower            =  500mA

  Interface: 0
  Alternate Setting:  0
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   00
    bAlternateSetting   =   00
    bNumEndpoints       =   03
    bInterface Class:SubClass:Protocol =   00:00:00
    iInterface          =   00
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   81 (in)
      bmAttributes        =   02 (Bulk)
      wMaxPacketSize      = 0040
      bInterval           =   01
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   02 (out)
      bmAttributes        =   02 (Bulk)
      wMaxPacketSize      = 0040
      bInterval           =   01
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   83 (in)
      bmAttributes        =   03 (Interrupt)
      wMaxPacketSize      = 0008
      bInterval           =   01

  Interface: 1
  Alternate Setting:  0
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   01
    bAlternateSetting   =   00
    bNumEndpoints       =   01
    bInterface Class:SubClass:Protocol =   00:00:00
    iInterface          =   00
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   84 (in)
      bmAttributes        =   01 (Isochronous)
      wMaxPacketSize      = 0000
      bInterval           =   01
  Alternate Setting:  1
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   01
    bAlternateSetting   =   01
    bNumEndpoints       =   01
    bInterface Class:SubClass:Protocol =   00:00:00
    iInterface          =   00
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   84 (in)
      bmAttributes        =   01 (Isochronous)
      wMaxPacketSize      = 0200
      bInterval           =   01
  Alternate Setting:  2
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   01
    bAlternateSetting   =   02
    bNumEndpoints       =   01
    bInterface Class:SubClass:Protocol =   00:00:00
    iInterface          =   00
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   84 (in)
      bmAttributes        =   01 (Isochronous)
      wMaxPacketSize      = 0300
      bInterval           =   01
  Alternate Setting:  3
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   01
    bAlternateSetting   =   03
    bNumEndpoints       =   01
    bInterface Class:SubClass:Protocol =   00:00:00
    iInterface          =   00
    Endpoint:
      bLength             =    7
      bDescriptorType     =   05
      bEndpointAddress    =   84 (in)
      bmAttributes        =   01 (Isochronous)
      wMaxPacketSize      = 03FE
      bInterval           =   01

  Interface: 2
  Alternate Setting:  0
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   02
    bAlternateSetting   =   00
    bNumEndpoints       =   00
    bInterface Class:SubClass:Protocol =   01:01:00
    iInterface          =   00
  Alternate Setting:  1
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   02
    bAlternateSetting   =   01
    bNumEndpoints       =   00
    bInterface Class:SubClass:Protocol =   01:02:00
    iInterface          =   00
  Alternate Setting:  2
    bLength             =    9
    bDescriptorType     =   04
    bInterfaceNumber    =   02
    bAlternateSetting   =   02
    bNumEndpoints       =   01
    bInterface Class:SubClass:Protocol =   01:02:00
    iInterface          =   00
    Endpoint:
      bLength             =    9 (Audio)
      bDescriptorType     =   05
      bEndpointAddress    =   85 (in)
      bmAttributes        =   01 (Isochronous)
      wMaxPacketSize      = 0010
      bInterval           =   01
      bRefresh            =   00
      bSynchAddress       =   00

-----------------------------------------------------------------------*/

#ifdef ECOS
#include "stdio.h"
#include "string.h"
#include "drv_api.h"
#include "diag.h"
#else
#include <stdio.h>
#include <string.h>
#endif

#include "N9H20.h"
#include "usbvideo.h"
#include "w99683ini.h"
#include "w99683.h"

#ifdef ECOS
#define sysGetTicks(TIMER0)  cyg_current_time()
#endif


#define V4L_BYTES_PER_PIXEL            3     /* Because we produce RGB24 */

#define VIDEOSIZE(x,y)                 (((x) & 0xFFFFL) | (((y) & 0xFFFFL) << 16))
#define VIDEOSIZE_X(vs)                ((vs) & 0xFFFFL)
#define VIDEOSIZE_Y(vs)                (((vs) >> 16) & 0xFFFFL)


static UVD_T  *_W99683_Camera = NULL;
static INT  _W99683_initialized = 0;

static USB_DEV_ID_T  W99683_id_table[] = 
{
    USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT,   /* match_flags */
    W99683_VENDOR_ID, W99683_PRODUCT_ID, 0, 0, 0, 0, 0,
    0,                   /* bInterfaceClass */
    0, 0, 0 
};


UINT8  SensorWAddr[] = 
{
      0,
      SA7111_WRITE,
      OV7620_WRITE,
      0,
      0,
      0,
      0,
      OV8610_WRITE,
      0,
      HP2020_WRITE,
      0,
      0,
      0,
      M20027_WRITE
};


/*
 *  remaining - remaining number of frame pieces
 *
 *  suspend -  NU_SUSPEND: suspend if no frame piece available
 *             NU_NO_SUSPEND: do not suspend
 *
 *  RETURN -   0:   success
 *             -1:  no video frame available
 */

UINT32	_QueuedSize = 0;


INT  W99683_HasImageQueued()
{
    if (!_W99683_Camera || !_W99683_Camera->dev) 
        return 0;
    
    if (!_W99683_Camera->streaming)
        return 0;

    if (_W99683_Camera->vbq_head != _W99683_Camera->vbq_tail)
        return 1;
    return 0;
}



INT  W99683_GetFramePiece(UINT8 **pbuf, INT *length)
{
    USB_VID_BUF_T  *vbuf;
//     INT  t0;

    if (!_W99683_Camera || !_W99683_Camera->dev) 
        return -1;
    
    if (!_W99683_Camera->streaming)
        return -1;
    
    while (1)
    {
//         t0 = sysGetTicks(TIMER0);
        while ((volatile USB_VID_BUF_T  *)_W99683_Camera->vbq_head == (volatile USB_VID_BUF_T  *)_W99683_Camera->vbq_tail)
        {
        }

        vbuf = _W99683_Camera->vbq_head;
    
        /* find the next non-zero length iso packet */
        while (vbuf->packet_idx < FRAMES_PER_DESC)
        {
            if (vbuf->iso_packet[vbuf->packet_idx].actual_length > 0)
                break;
            vbuf->packet_idx++;
        }

        /* whether to dequeue */ 
        if (vbuf->packet_idx >= FRAMES_PER_DESC)
        {
            _W99683_Camera->vbq_head = _W99683_Camera->vbq_head->next;
            _W99683_Camera->vbq_cnt--;
            USB_free(vbuf->iso_buf);
            USB_free(vbuf);
            _QueuedSize -= FRAMES_PER_DESC * _W99683_Camera->iso_packet_len;
            continue;
        }
    
        /* Have find an available iso packet */
        *pbuf = vbuf->iso_buf + vbuf->iso_packet[vbuf->packet_idx].offset;
        *length = vbuf->iso_packet[vbuf->packet_idx].actual_length;
        vbuf->packet_idx++;
        //sysprintf("W99683_GetFramePiece - size:%d idx:%d\n", *length, vbuf->packet_idx);
        break;
    }
    return 0;
}



static VOID  W99683Cam_DropFrameQueue()
{
    USB_VID_BUF_T  *vbuf;
    
    while (_W99683_Camera->vbq_head != _W99683_Camera->vbq_tail)
    {
        USB_free(vbuf->iso_buf);
        vbuf = _W99683_Camera->vbq_head->next;
        USB_free(_W99683_Camera->vbq_head);
        _W99683_Camera->vbq_head = vbuf;
        _W99683_Camera->vbq_cnt--;
    }
    return;
}


/*
 *  Video Isochronous in pipe callback routine
 */
static VOID usbvideo_IsocIrq(URB_T *urb)
{
    USB_VID_BUF_T  *vbuf, *dummy_vbuf;
    USB_DEV_T  *dev = _W99683_Camera->dev;
    //INT  index;
    INT  status;
    INT  j, k;
    UINT8  *newBuffer;
    static INT  printCount=0;

    /* We don't want to do anything if we are about to be removed! */
    if (!_W99683_Camera || !_W99683_Camera->dev) 
        return;

    if (!_W99683_Camera->streaming) 
    {
        sysprintf("Not streaming, but interrupt!\n");
        return;
    }

    //sysprintf("Iso in - URB:%x SF=%d, EC=%d, L=%d.\n", urb, urb->start_frame, urb->error_count, urb->actual_length);
        
    if (urb->actual_length <= 0)
    {
        //sysprintf("usbvideo_IsocIrq - actual_length incorrect: %d\n", urb->actual_length);
        goto urb_done_with;
    }
        
    newBuffer = USB_malloc(FRAMES_PER_DESC * _W99683_Camera->iso_packet_len, 4);
    if (newBuffer == NULL) 
    {
        sysprintf("usbvideo_IsocIrq - memory not enough!\n");
        return;
    }

    dummy_vbuf = (USB_VID_BUF_T *)USB_malloc(sizeof(USB_VID_BUF_T), 4);
    if (dummy_vbuf == NULL)
    {
        sysprintf("usbvideo_IsocIrq - Not enough system memory! %d\n", sizeof(USB_VID_BUF_T));
        USB_free(newBuffer);
        return;
    }
    vbuf = _W99683_Camera->vbq_tail;
    vbuf->next = dummy_vbuf;
    
    memcpy((CHAR *)&vbuf->iso_packet[0], (CHAR *)urb->iso_frame_desc, sizeof(ISO_PACKET_DESCRIPTOR_T) * FRAMES_PER_DESC);
    vbuf->iso_buf = urb->transfer_buffer;
    vbuf->packet_idx = 0;

    /* advanced the tail pointer, thus enqueue the new video buffer */
    _W99683_Camera->vbq_cnt++;
    _W99683_Camera->vbq_tail = dummy_vbuf;
    _QueuedSize += FRAMES_PER_DESC * _W99683_Camera->iso_packet_len;

    /* 
     * The old transfer buffer has been delivered to frame buffer queue. 
     * Here we give a new transfer buffer to isochronous urb.
     */
    urb->transfer_buffer = newBuffer;
    _W99683_Camera->sbuf[0].data = newBuffer;

    urb_done_with:
    urb->dev = dev;
    urb->context = _W99683_Camera;
    urb->pipe = usb_rcvisocpipe(dev, _W99683_Camera->video_endp);
    urb->transfer_flags = USB_ISO_ASAP;
    urb->transfer_buffer = _W99683_Camera->sbuf[0].data;
    urb->complete = usbvideo_IsocIrq;
    urb->interval = 2;
    urb->number_of_packets = FRAMES_PER_DESC;
    urb->transfer_buffer_length = _W99683_Camera->iso_packet_len * FRAMES_PER_DESC;
    for (j=k=0; j < FRAMES_PER_DESC; j++, k += _W99683_Camera->iso_packet_len) 
    {
        urb->iso_frame_desc[j].status = 0;
        urb->iso_frame_desc[j].actual_length = 0;
        urb->iso_frame_desc[j].offset = k;
        urb->iso_frame_desc[j].length = _W99683_Camera->iso_packet_len;
    }

    /* Submit URB */
    status = USB_SubmitUrb(urb);
    if (status)
        sysprintf("usbvideo_IsocIrq: USB_SubmitUrb ret %d\n", status);

    if (printCount++ % 500 == 0)
       sysprintf("FQL:%d, free:%x\n", _W99683_Camera->vbq_cnt, USB_available_memory());

    return;
}


INT  W99683Cam_IsStreaming()
{
    return _W99683_Camera->streaming;
}


INT  W99683Cam_StartDataPump()
{
    static const CHAR proc[] = "W99683Cam_StartDataPump";
    USB_DEV_T  *dev = _W99683_Camera->dev;
    URB_T  *urb;
    INT  errFlag;
    INT  j, k;

    sysprintf("W99683Cam_StartDataPump...\n");

    if (!_W99683_Camera || !_W99683_Camera->dev) 
    {
        sysprintf("%s: Camera is not operational\n", proc);
        return -1; //-EFAULT;
    }

    /* Alternate interface 1 is the biggest frame size */
    errFlag = USB_SetInterface(dev, _W99683_Camera->iface, _W99683_Camera->ifaceAltActive);
    if (errFlag < 0) 
    {
        sysprintf("%s: usb_set_interface error\n", proc);
        return -1; /* -EBUSY; */
    }
    
    /* W99683VideoStart(); */
    urb = _W99683_Camera->sbuf[0].urb;
    urb->dev = dev;
    urb->context = _W99683_Camera;
    urb->pipe = usb_rcvisocpipe(dev, _W99683_Camera->video_endp);
    urb->transfer_flags = USB_ISO_ASAP;
    urb->transfer_buffer = _W99683_Camera->sbuf[0].data;
    urb->complete = usbvideo_IsocIrq;
    urb->interval = 2;
    urb->number_of_packets = FRAMES_PER_DESC;
    urb->transfer_buffer_length = _W99683_Camera->iso_packet_len * FRAMES_PER_DESC;

    for (j=k=0; j < FRAMES_PER_DESC; j++, k += _W99683_Camera->iso_packet_len) 
    {
        urb->iso_frame_desc[j].offset = k;
        urb->iso_frame_desc[j].length = _W99683_Camera->iso_packet_len;
    }

    /* Submit URB */
    errFlag = USB_SubmitUrb(_W99683_Camera->sbuf[0].urb);
    if (errFlag)
        sysprintf("%s: usb_submit_isoc ret %d\n", proc, errFlag);
    sysprintf("W99683 has submit all urbs...\n");
    _W99683_Camera->streaming = 1;
    return 0;
}


/*
 * W99683Cam_StopDataPump()
 *
 * This procedure stops streaming and deallocates URBs. Then it
 * activates zero-bandwidth alt. setting of the video interface.
 */
VOID  W99683Cam_StopDataPump()
{
    static const  CHAR proc[] = "W99683Cam_StopDataPump";
    INT    status;

    if ((_W99683_Camera == NULL) || (_W99683_Camera->dev == NULL))
    {
        sysprintf("%s - driver or hardware not present!\n");
        return;
    }

    /* Unschedule all of the iso td's */
    status = USB_UnlinkUrb(_W99683_Camera->sbuf[0].urb);
    if (status < 0)
            sysprintf("%s: USB_UnlinkUrb() error %d.\n", proc, status);

    sysprintf("W99683Cam_StopDataPump called!\n");
    _W99683_Camera->streaming = 0;

    W99683Cam_DropFrameQueue();

    /* W99683VideoStop(); */
}
/*
 * W99683 USB Video Camera driver
 */
#if defined (__GNUC__)
UINT8  _dma_data_pool[4096] __attribute__((aligned (32)));
#else
__align(32) UINT8  _dma_data_pool[4096];
#endif
UINT8  *_dma_data;

static  INT W99683Cam_read_register(UINT16 index, UINT8 *regb)
{
    INT  i;

    i = USB_SendControlMessage(_W99683_Camera->dev, usb_rcvctrlpipe(_W99683_Camera->dev, 0),
                        W99683_RegisterReadRequest, 
                        USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                        0, index, _dma_data, 1, 3000);
    if (i < 0)
        sysprintf("W99683Cam_read_register error! %d\n", i);
    *regb = _dma_data[0];
    return i;
}


static INT  W99683Cam_write_register(UINT16 index, UINT8 value)
{
    INT   i;

    _dma_data[0] = value;
    i = USB_SendControlMessage(_W99683_Camera->dev, usb_sndctrlpipe(_W99683_Camera->dev, 0),
                        W99683_RegisterWriteRequest,
                        USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                        0, index, _dma_data, 1, 10000);
    if (i < 0)
        sysprintf("W99683Cam_write_registers error! %d\n", i);
    return i;
}

static INT  W99683Cam_I2C_read(UINT16 slave_addr, UINT16 index, UINT8 *regb)
{
    INT  i;

    i = USB_SendControlMessage(_W99683_Camera->dev, usb_rcvctrlpipe(_W99683_Camera->dev, 0),
                        W99683_I2CReadRequest, 
                        USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                        slave_addr, index, _dma_data, 1, 600);
    if (i < 0)
        sysprintf("W99683Cam_I2C_read error! %d\n", i);
    *regb = _dma_data[0];
    return i;
}


static INT  W99683Cam_I2C_write(UINT16 slave_addr, UINT16 index, UINT8 value)
{
    INT  i;

    _dma_data[0] = value;
    i = USB_SendControlMessage(_W99683_Camera->dev, usb_sndctrlpipe(_W99683_Camera->dev, 0),
                        W99683_I2CWriteRequest,
                        USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                        slave_addr, index, _dma_data, 1, 2000);
    if (i < 0)
        sysprintf("W99683Cam_I2C_write error! %d\n", i);
    return i;
}


static VOID  W99683Cam_set_quantization_table(INT TableSelect)
{
    INT     i;
    UINT8   QEntry;
    INT     indexL;
//     INT     indexH;

    /* Luminance quantization table */
    for (i=0; i<0x40; i++)
    {
        indexL = RevZigzag(i);
//         indexH = RevZigzag(i+1);
        QEntry = InitQuantizationTable[TableSelect][indexL + 0x0005];
        W99683Cam_write_register(LumQTable + i , QEntry);
    }

    /* Chrominance quantization table */
    for (i=0; i<0x40; i++)
    {
        indexL = RevZigzag(i);
//         indexH = RevZigzag(i+1);
        QEntry = InitQuantizationTable[TableSelect][indexL + 0x004A];
        W99683Cam_write_register(ChrQTable + i , QEntry);
    }
}


static INT  W99683Cam_check_sensor(INT *capc1)
{
    UINT8   *regb;
    INT     i;
    
    regb = (UINT8 *)USB_malloc(1, 4);

    for (i = 1; i < sizeof(SensorWAddr); i++)
    {
        switch(i)
        {
            case 1:
                W99683Cam_I2C_write(SA7111_WRITE, 0x00, 0x00); 
                W99683Cam_read_register(SerialBusCR, regb);
                W99683Cam_write_register(SerialBusCR, *regb & 0xfe);
                W99683Cam_I2C_read(SA7111_READ, 0x00, regb);
                *capc1 = VPRE_CAPTURE_TVDECODE_PLANE;
                break;
            case 2:
                W99683Cam_I2C_write(OV7620_WRITE, 0x12, 0x80);  /* Reset OMNI7620 */
                W99683Cam_read_register(SerialBusCR , regb);
                W99683Cam_write_register(SerialBusCR , *regb & 0xfe);
                W99683Cam_I2C_read(OV7620_READ, 0x12, regb);
                *capc1 = VPRE_CAPTURE_SENSOR_PLANE;
                break;
            case 7:
                W99683Cam_I2C_write(OV8610_WRITE, 0x12, 0x80);  /* Reset OMNI8610 */
                W99683Cam_read_register(SerialBusCR , regb);
                W99683Cam_write_register(SerialBusCR , *regb & 0xfe);
                W99683Cam_I2C_read(OV8610_WRITE, 0x12, regb);
                *capc1 = VPRE_CAPTURE_SENSOR_PLANE;
                break;
            case 9:
                W99683Cam_I2C_write(HP2020_WRITE, 0x38, 0x04);
                W99683Cam_read_register(SerialBusCR , regb);
                W99683Cam_write_register(SerialBusCR , *regb & 0xfe);
                W99683Cam_I2C_read(HP2020_WRITE, 0x38, regb);
                *capc1 = VPRE_CAPTURE_SENSOR_PLANE;
                break;
            case 13:
                W99683Cam_I2C_write(M20027_WRITE, 0x65, 0x00);
                W99683Cam_read_register(SerialBusCR , regb);
                W99683Cam_write_register(SerialBusCR , *regb & 0xfe);
                W99683Cam_I2C_read(M20027_WRITE, 0x65, regb);
                *capc1 = VPRE_CAPTURE_SENSOR_PLANE;
                break;
            default:
                W99683Cam_read_register(SerialBusCR , regb);
                W99683Cam_write_register(SerialBusCR , *regb & 0xfe);
                *capc1 = VPRE_CAPTURE_SENSOR_PLANE;
                break;
        }
        *regb = 0;
        W99683Cam_read_register(SerialBusCR , regb);
        if ((*regb & 0x01) == 1)
        {
            USB_free(regb);
            return i;
        }
    }
    *capc1 = VPRE_CAPTURE_SENSOR_PLANE;
    USB_free(regb);
    return 2;  /* default */
}


/*
 * Return negative code on failure, 0 on success.
 */
static INT  W99683Cam_SetupOnOpen()
{
    INT    CrpStartX, CrpStartY, CrpEndX, CrpEndY;
    INT    setup_ok = 0;    /* Success by default */
    INT    sensor_type, i, CapC1;
    UINT8  *regb;

    regb = (UINT8 *)USB_malloc(1, 4);

    CrpStartX = 2;
    CrpStartY = 4;
    CrpEndX = 646;
    CrpEndY = 488;

    sysprintf("enter W99683Cam_SetupOnOpen() ...\n");
    /* Send init sequence only once, it's large! */
    if (!_W99683_initialized) 
    {
        sensor_type = W99683Cam_check_sensor(&CapC1);
        sysprintf("Sensor type: %d (9 for HP2020)\n", sensor_type);
        
        /* CR0002->CR0003 */
        InitRegValue[EngResetCR]        = (UINT8) ALL_NORMAL;
        InitRegValue[EngClockCR]        = (UINT8) ALL_CLOCK;

        /* CR011c->CR0123 */
        InitRegValue[CropStartXLoR]     = (UINT8) (CrpStartX);
        InitRegValue[CropStartXHiR]     = (UINT8) (CrpStartX>>8);
        InitRegValue[CropStartYLoR]     = (UINT8) (CrpStartY);
        InitRegValue[CropStartYHiR]     = (UINT8) (CrpStartY>>8);
        InitRegValue[CropWidthLoR]      = (UINT8) (CrpEndX-CrpStartX);
        InitRegValue[CropWidthHiR]      = (UINT8) ((CrpEndX-CrpStartX)>>8);
        InitRegValue[CropHeightLoR]     = (UINT8) (CrpEndY-CrpStartY);
        InitRegValue[CropHeightHiR]     = (UINT8) ((CrpEndY-CrpStartY)>>8);

        /* CR0200->CR020a */
        InitRegValue[CapC1R]            = (UINT8) CapC1;
        InitRegValue[CapC2R]            = (UINT8) 0x10; 

        /* CR0220->CR0223 */
        InitRegValue[CapHoriScaleMR]    = 1;   /* (UINT8) CapHDnM; */
        InitRegValue[CapHoriScaleNR]    = 1;   /* (UINT8) CapHDnN; */
        InitRegValue[CapVertScaleMR]    = 1;   /* (UINT8) CapVDnM; */
        InitRegValue[CapVertScaleNR]    = 1;   /* (UINT8) CapVDnN; */

        /* CR0280->CR0282 */
        InitRegValue[JPGSelectCR]       = 0xB0;
        InitRegValue[JPGHeaderCR]       = 0xF0;
        InitRegValue[JPGUpScaleCR]      = 0;   /* (UINT8) VertUpscale; */ 

        /* CR0287 */
        InitRegValue[JPGDnScaleCR]      = 0;   /* (UINT8) DownScale; */

        /* CR028b->CR028e */
        InitRegValue[JPGHeightLoR]      = (UINT8) 480;      /* (OutHeight); */
        InitRegValue[JPGHeightHiR]      = (UINT8) (480> 8); /* ((OutHeight)>>8); */
        InitRegValue[JPGWidthLoR]       = (UINT8) 640;      /* (OutWidth); */
        InitRegValue[JPGWidthHiR]       = (UINT8) (640>>8); /* (OutWidth>>8); */

        /* CR0293->CR0294 */
        InitRegValue[JPGRSTLoR]         = 0x00;
        InitRegValue[JPGRSTHiR]         = 0x00;

        /* CR02ac->CR02b7 */
        InitRegValue[JPGYStideLoR]      = (UINT8) 320;      /* (CapWidth/2); */
        InitRegValue[JPGYStideHiR]      = (UINT8) (320>>8); /* ((CapWidth/2)>>8); */
        InitRegValue[JPGUStideLoR]      = (UINT8) 160;      /* (CapWidth/4); */
        InitRegValue[JPGUStideHiR]      = (UINT8) (160>>8); /* ((CapWidth/4)>>8); */
        InitRegValue[JPGVStideLoR]      = (UINT8) 160;      /* (CapWidth/4); */
        InitRegValue[JPGVStideHiR]      = (UINT8) (160>>8); /* ((CapWidth/4)>>8); */
        InitRegValue[JPGStrm0StartLoR]  = (UINT8) 0;
        InitRegValue[JPGStrm0StartMiR]  = (UINT8) 0;
        InitRegValue[JPGStrm0StartHiR]  = (UINT8) 0x28;
        InitRegValue[JPGStrm1StartLoR]  = (UINT8) 0;
        InitRegValue[JPGStrm1StartMiR]  = (UINT8) 0;
        InitRegValue[JPGStrm1StartHiR]  = (UINT8) 0x38;
     
        /* CR0650->CR0652 */
        InitRegValue[UsbVISOStartLoR]   = (UINT8) InitRegValue[JPGStrm0StartLoR];
        InitRegValue[UsbVISOStartMiR]   = (UINT8) InitRegValue[JPGStrm0StartMiR];
        InitRegValue[UsbVISOStartHiR]   = (UINT8) InitRegValue[JPGStrm0StartHiR];

        /* Disable VPRE/JPEG interrupts */
        setup_ok |= W99683Cam_read_register(GINTCR, regb);
        setup_ok |= W99683Cam_write_register(GINTCR, *regb & VPRE_DISABLE & JPEG_DISABLE);
       
        /* Set engine reset and clock  */
        for (i = EngResetCR; i <= EngClockCR; i++)
            W99683Cam_write_register(i, InitRegValue[i]);    

        /* Disable DISPLAY/VPRE/JPEG/MEM engines and Enable DSP engine */
        setup_ok |= W99683Cam_read_register(EngOPCR, regb);
        setup_ok |= W99683Cam_write_register(EngOPCR, (*regb & JPEG_DISABLE & VPRE_DISABLE & DISPLAY_DISABLE & MEM_DISABLE) | DSP_ENABLE);
       
        /* Clear double buffer */
        setup_ok |= W99683Cam_read_register(DualBufferCR, regb);
        setup_ok |= W99683Cam_write_register(DualBufferCR, *regb & DB_CLEAR);

        if (sensor_type != 2)
        {
            for (i = CropStartXLoR; i <= CropHeightHiR; i++)
                setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);
        }

        for (i = CapC1R; i <= CapC2R; i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        for (i = CapHoriScaleMR; i <= CapVertScaleNR; i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        for (i = JPGSelectCR; i <= JPGUpScaleCR; i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        setup_ok |= W99683Cam_write_register(JPGDnScaleCR, InitRegValue[JPGDnScaleCR]);

        for (i = JPGHeightLoR; i <= JPGWidthHiR; i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        for (i = JPGRSTLoR; i <= JPGRSTHiR; i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        for (i = JPGY0StartLoR; i <= JPGV0StartHiR; i++)
        {
            setup_ok |= W99683Cam_read_register(i-(JPGY0StartLoR-CapY0StartLoR), regb);
            setup_ok |= W99683Cam_write_register(i, *regb);
        } 

        for (i = JPGY1StartLoR; i <= JPGV1StartHiR; i++)
        {
            setup_ok |= W99683Cam_read_register(i-(JPGY1StartLoR-CapY1StartLoR), regb);
            setup_ok |= W99683Cam_write_register(i, *regb);
        } 

        for (i = JPGYStideLoR; i <= JPGStrm1StartHiR; i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        /* disable video ISO */
        setup_ok |= W99683Cam_read_register(UsbVISOCR, regb);
        setup_ok |= W99683Cam_write_register(UsbVISOCR, *regb & VISO_DISABLE);

        for (i = UsbVISOStartLoR; i <= UsbVISOStartHiR ;i++)
             setup_ok |= W99683Cam_write_register(i, InitRegValue[i]);

        //W99683Cam_alternateSetting(3); 

        W99683Cam_set_quantization_table(3);
        
#if 1   /* YCHuang, add to fix registers */
        setup_ok |= W99683Cam_write_register(0x73, 0xBE);   /* audio frame buffer starting address low */
        setup_ok |= W99683Cam_write_register(0x106, 0x24);  /* Sensor DSP Horizontal Active Output Control Low Register */
        setup_ok |= W99683Cam_write_register(0x125, 0);     /* Sensor DSP Color Balance Register-2 */
        setup_ok |= W99683Cam_write_register(0x127, 7);     /* Sensor DSP Color Balance Register-4 */
#endif        

        /*----------------------------------------------------------------*/
        /*        Start Capture                                           */
        /*----------------------------------------------------------------*/

        /* Set start VPRE Engine */
        setup_ok |= W99683Cam_read_register(EngOPCR, regb);
        setup_ok |= W99683Cam_write_register(EngOPCR, *regb | VPRE_ENABLE);

        /* Clear jpeg decode status interrupt */
        setup_ok |= W99683Cam_read_register(JPGINTCSR, regb);
        setup_ok |= W99683Cam_write_register(JPGINTCSR, (*regb & JPGINT_ENCODE_DISABLE) | JPGINT_ENCODE_CLEAR);
       
        /* Set jpeg to still mode and double buffer */
        setup_ok |= W99683Cam_read_register(JPGSelectCR, regb);
        setup_ok |= W99683Cam_write_register(JPGSelectCR, *regb | ENCODE_DB);
       
        /* Set start VPRE Engine */
        setup_ok |= W99683Cam_read_register(EngOPCR, regb);
        setup_ok |= W99683Cam_write_register(EngOPCR, *regb | JPEG_ENABLE);
       
        /* Set video capture , jpeg and ISO transfer double buffer */
        setup_ok |= W99683Cam_read_register(DualBufferCR, regb);
        setup_ok |= W99683Cam_write_register(DualBufferCR, *regb | DB_ENABLE);

        /* Set continuous mode video ISO transfer size */
        setup_ok |= W99683Cam_write_register(UsbVISOSizeLoR, CONTINUOUT_MODE_SIZE_LO);
        setup_ok |= W99683Cam_write_register(UsbVISOSizeMiR, CONTINUOUT_MODE_SIZE_MI);
        setup_ok |= W99683Cam_write_register(UsbVISOSizeHiR, CONTINUOUT_MODE_SIZE_HI);

        /* Set continuous mode and no extra data */ 
        setup_ok |= W99683Cam_read_register(UsbISOOPCR, regb);
        setup_ok |= W99683Cam_write_register(UsbISOOPCR, (*regb & VISO_NON_EXTRA) | VISO_CONTINUOUS);

        /* Set start MEM Engines (real start transfer) */
        setup_ok |= W99683Cam_read_register(EngOPCR, regb);
        setup_ok |= W99683Cam_write_register(EngOPCR, *regb | MEM_ENABLE);

        /* Set start video iso transfer */
        setup_ok |= W99683Cam_read_register(UsbVISOCR, regb);
        setup_ok |= W99683Cam_write_register(UsbVISOCR, *regb | VISO_ENABLE);

        _W99683_initialized = (setup_ok > 0);
    }

    return setup_ok;
}


/*
 * W99683Cam_Open()
 *
 * The procedure then allocates buffers needed for video processing and
 * start W99683 video camera.
 */
INT  W99683Cam_Open()
{
    static CHAR  proc[] = "W99683Cam_Open";

    _W99683_Camera->sbuf[0].urb = USB_AllocateUrb(FRAMES_PER_DESC);
    if (_W99683_Camera->sbuf[0].urb == NULL) 
    {
        sysprintf("%s - USB_AllocateUrb(%d.) failed.\n", proc, FRAMES_PER_DESC);
        goto open_x;
    }
    
    _W99683_Camera->sbuf[0].data = USB_malloc(FRAMES_PER_DESC * _W99683_Camera->iso_packet_len, 4);
    if (_W99683_Camera->sbuf[0].data == NULL) 
    {
        sysprintf("%s - memory not enough!\n", proc, FRAMES_PER_DESC);
        goto open_x;
    }
 
    USB_WaitMiliseconds(1000);
    
    if (W99683Cam_SetupOnOpen() < 0)
    {
        sysprintf("W99683Cam_SetupOnOpen failed\n");
        goto open_x;
    }

    USB_WaitMiliseconds(1000);

    if (W99683Cam_StartDataPump() < 0)
    {
        sysprintf("W99683Cam_StartDataPump failed\n");
        goto open_x;
    }
    return 0;

open_x:
    sysprintf("Disconnect W99683 device!\n");
    USB_FreeUrb(_W99683_Camera->sbuf[0].urb);
    USB_free(_W99683_Camera->sbuf[0].data);    
    W99683Cam_DropFrameQueue();
    USB_free(_W99683_Camera);
    return -1;
}


/*
 * W99683Cam_Disconnect()
 *
 * This procedure stops all driver activity. Deallocation of
 * the interface-private structure (pointed by 'ptr') is done now
 * (if we don't have any open files) or later, when those files
 * are closed. After that driver should be removable.
 */
static VOID  W99683Cam_Disconnect(USB_DEV_T *dev, VOID *ptr)
{
    static const CHAR proc[] = "W99683Cam_Disconnect";

    if ((dev == NULL) || (_W99683_Camera == NULL)) 
    {
       sysprintf("%s($%p,$%p): Illegal call.\n", proc, dev, ptr);
       return;
    }
    
    sysprintf("W99683Cam_Disconnect called!\n");

    /* At this time we ask to cancel outstanding URBs */
    W99683Cam_StopDataPump();
    
    W99683Cam_DropFrameQueue();

    USB_FreeUrb(_W99683_Camera->sbuf[0].urb);
    USB_free(_W99683_Camera->sbuf[0].data);

    USB_DecreaseDeviceUser(_W99683_Camera->dev);

    USB_free(_W99683_Camera);
    _W99683_Camera = NULL;

    sysprintf("USB camera disconnected.\n");
}


INT  W99683Cam_IsConnected()
{
    if (_W99683_Camera == NULL)
        return 0;
    else 
        return 1;
}


/*
 * W99683Cam_Probe()
 * This procedure queries device descriptor and accepts the interface
 * if it looks like our camera.
 */
static VOID  *W99683Cam_Probe(USB_DEV_T *dev, UINT32 ifnum,
                          const struct usb_device_id *devid)
{
    static CHAR  proc[] = "W99683Cam_Probe";
    const USB_IF_DESC_T  *interface;
    const USB_EP_DESC_T  *endpoint;
    INT     actInterface=-1, inactInterface=-1, maxPS=0;
    UINT8   video_ep = 0;
    USB_VID_BUF_T  *vbuf;

    if (ifnum != 1) 
        return NULL;

    /* Is it an W99683 video camera? */
    if ((dev->descriptor.idVendor != W99683_VENDOR_ID) ||
        (dev->descriptor.idProduct != W99683_PRODUCT_ID))
         return NULL;

    sysprintf("%s, %x %x\n", proc, dev->descriptor.idVendor, dev->descriptor.idProduct);
    sysprintf("W99683 video camera found (rev. 0x%04x)\n", dev->descriptor.bcdDevice);

    inactInterface = 0;
    actInterface = 1;  /* 512 */
    interface = &dev->actconfig->interface[ifnum].altsetting[actInterface];
    endpoint = &interface->endpoint[0];
    video_ep = endpoint->bEndpointAddress;
    maxPS = endpoint->wMaxPacketSize;

    /* alocate W99683 control handle */
    _W99683_Camera = (UVD_T *)USB_malloc(sizeof(UVD_T), 4);
    if (_W99683_Camera == NULL) 
    {
        sysprintf("W99683Cam_Init - failed to allocate memory!\n");
        return NULL;
    }
    memset((UINT8 *)_W99683_Camera, 0, sizeof(UVD_T));

    _W99683_Camera->dev = dev;
    _W99683_Camera->iface = ifnum;
    _W99683_Camera->ifaceAltInactive = inactInterface;
    _W99683_Camera->ifaceAltActive = actInterface;
    _W99683_Camera->video_endp = video_ep;
    _W99683_Camera->iso_packet_len = maxPS;
    _W99683_Camera->canvas = VIDEOSIZE(640, 480);  /* FIXME */
    _W99683_Camera->videosize = _W99683_Camera->canvas;  /* w99683_size_to_videosize(size);*/

    /* allocate a dummy item for frame queue */
    vbuf = (USB_VID_BUF_T *)USB_malloc(sizeof(USB_VID_BUF_T), 4);
    if (vbuf == NULL)
    {
        sysprintf("%s - Not enough system memory!\n", proc);
        USB_free(_W99683_Camera);
        return NULL;
    }
    
    /* initialize frame queue */
    _W99683_Camera->vbq_head = vbuf;
    _W99683_Camera->vbq_tail = vbuf;
    _W99683_Camera->vbq_cnt = 0;
     
    _W99683_initialized = 0;

    _W99683_Camera->max_frame_size = VIDEOSIZE_X(_W99683_Camera->canvas) *
                                     VIDEOSIZE_Y(_W99683_Camera->canvas) * V4L_BYTES_PER_PIXEL;

    sysprintf("%s: iface=%d. endpoint=$%02x\n", proc, _W99683_Camera->iface, _W99683_Camera->video_endp);
    sysprintf("Default: Interface 1, Altsetting %d, endpoint 0x%x, maxPS:0x%x\n", actInterface, video_ep, maxPS);
 
    USB_IncreaseDeviceUser(dev);
    
    return _W99683_Camera;
}


static USB_DRIVER_T  W99683_driver = 
{
    "W99683",
    W99683Cam_Probe,
    W99683Cam_Disconnect,
    {NULL,NULL},               /* driver_list */ 
    // {0},                    /* semaphore */
    NULL,                      /* ioctl */
    W99683_id_table,
    NULL,                      /* suspend */
    NULL                       /* resume */
};

/*
 * W99683Cam_Init()
 *
 * This code is run to initialize the driver.
 */
INT  W99683Cam_Init(VOID)
{
    _dma_data = (UINT8 *)((UINT32)_dma_data_pool | 0x80000000);

    if (USB_RegisterDriver(&W99683_driver) < 0)
    {
        sysprintf("Failed to register W99683 driver!!\n");
        return -1;
    }

    return 0;
}


/*
 * W99683Cam_Exit()
 * Procedure frees all usbvideo and user data structures. 
 */
VOID W99683Cam_Exit(VOID)
{
    if (_W99683_Camera == NULL) 
        return;

    USB_DeregisterDriver(&W99683_driver);
}

