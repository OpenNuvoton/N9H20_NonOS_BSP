/**************************************************************************//**
 * @file     N9H20_USBH.h
 * @version  V3.00
 * @brief    N9H20 series USB Host driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/ 
#ifndef _USB_H_
#define _USB_H_


/* USB constants */

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE         0       /* for DeviceClass */
#define USB_CLASS_AUDIO                 1
#define USB_CLASS_COMM                  2
#define USB_CLASS_HID                   3
#define USB_CLASS_PHYSICAL              5
#define USB_CLASS_PRINTER               7
#define USB_CLASS_MASS_STORAGE          8
#define USB_CLASS_HUB                   9
#define USB_CLASS_DATA                  10
#define USB_CLASS_APP_SPEC              0xfe
#define USB_CLASS_VENDOR_SPEC           0xff

/*
 * USB types
 */
#define USB_TYPE_STANDARD               (0x00 << 5)
#define USB_TYPE_CLASS                  (0x01 << 5)
#define USB_TYPE_VENDOR                 (0x02 << 5)
#define USB_TYPE_RESERVED               (0x03 << 5)

/*
 * USB recipients
 */
#define USB_RECIP_MASK                  0x1f
#define USB_RECIP_DEVICE                0x00
#define USB_RECIP_INTERFACE             0x01
#define USB_RECIP_ENDPOINT              0x02
#define USB_RECIP_OTHER                 0x03

/*
 * USB directions
 */
#define USB_DIR_OUT                     0
#define USB_DIR_IN                      0x80

/*
 * Descriptor types
 */
#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05

#define USB_DT_HID                      (USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT                   (USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL                 (USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)

/*
 * Descriptor sizes per descriptor type
 */
#define USB_DT_DEVICE_SIZE              18
#define USB_DT_CONFIG_SIZE              9
#define USB_DT_INTERFACE_SIZE           9
#define USB_DT_ENDPOINT_SIZE            7
#define USB_DT_ENDPOINT_AUDIO_SIZE      9       /* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE          7
#define USB_DT_HID_SIZE                 9

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK        0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK           0x80

#define USB_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL       0
#define USB_ENDPOINT_XFER_ISOC          1
#define USB_ENDPOINT_XFER_BULK          2
#define USB_ENDPOINT_XFER_INT           3

/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_UNDEF_0                        0xf0
#define USB_PID_OUT                            0xe1
#define USB_PID_ACK                            0xd2
#define USB_PID_DATA0                          0xc3
#define USB_PID_PING                           0xb4     /* USB 2.0 */
#define USB_PID_SOF                            0xa5
#define USB_PID_NYET                           0x96     /* USB 2.0 */
#define USB_PID_DATA2                          0x87     /* USB 2.0 */
#define USB_PID_SPLIT                          0x78     /* USB 2.0 */
#define USB_PID_IN                             0x69
#define USB_PID_NAK                            0x5a
#define USB_PID_DATA1                          0x4b
#define USB_PID_PREAMBLE                       0x3c     /* Token mode */
#define USB_PID_ERR                            0x3c     /* USB 2.0: handshake mode */
#define USB_PID_SETUP                          0x2d
#define USB_PID_STALL                          0x1e
#define USB_PID_MDATA                          0x0f     /* USB 2.0 */

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS              0x00
#define USB_REQ_CLEAR_FEATURE           0x01
#define USB_REQ_SET_FEATURE             0x03
#define USB_REQ_SET_ADDRESS             0x05
#define USB_REQ_GET_DESCRIPTOR          0x06
#define USB_REQ_SET_DESCRIPTOR          0x07
#define USB_REQ_GET_CONFIGURATION       0x08
#define USB_REQ_SET_CONFIGURATION       0x09
#define USB_REQ_GET_INTERFACE           0x0A
#define USB_REQ_SET_INTERFACE           0x0B
#define USB_REQ_SYNCH_FRAME             0x0C

/*
 * HID requests
 */
#define USB_REQ_GET_REPORT              0x01
#define USB_REQ_GET_IDLE                0x02
#define USB_REQ_GET_PROTOCOL            0x03
#define USB_REQ_SET_REPORT              0x09
#define USB_REQ_SET_IDLE                0x0A
#define USB_REQ_SET_PROTOCOL            0x0B

#if defined(__GNUC__)
typedef struct
{
    UINT8  requesttype;
    UINT8  request;
    UINT16 value;
    UINT16 index;
    UINT16 length;
}__attribute__((packed)) DEV_REQ_T;
#else
typedef struct 
{
    __packed UINT8  requesttype;
    __packed UINT8  request;
    __packed UINT16 value;
    __packed UINT16 index;
    __packed UINT16 length;
} DEV_REQ_T;
#endif
/*
 * USB-status codes:
 * USB_ST* maps to -E* and should go away in the future
 */

#define USB_ST_NOERROR          0
#define USB_ST_CRC              (-EILSEQ)
#define USB_ST_BITSTUFF         (-EPROTO)
#define USB_ST_NORESPONSE       (-ETIMEDOUT)                    /* device not responding/handshaking */
#define USB_ST_DATAOVERRUN      (-EOVERFLOW)
#define USB_ST_DATAUNDERRUN     (-EREMOTEIO)
#define USB_ST_BUFFEROVERRUN    (-ECOMM)
#define USB_ST_BUFFERUNDERRUN   (-ENOSR)
#define USB_ST_INTERNALERROR    (-EPROTO)                       /* unknown error */
#define USB_ST_SHORT_PACKET     (-EREMOTEIO)
#define USB_ST_PARTIAL_ERROR    (-EXDEV)                        /* ISO transfer only partially completed */
#define USB_ST_URB_KILLED       (-ENOENT)                       /* URB canceled by user */
#define USB_ST_URB_PENDING       (-EINPROGRESS)
#define USB_ST_REMOVED          (-ENODEV)                       /* device not existing or removed */
#define USB_ST_TIMEOUT          (-ETIMEDOUT)                    /* communication timed out, also in urb->status**/
#define USB_ST_NOTSUPPORTED     (-ENOSYS)                       
#define USB_ST_BANDWIDTH_ERROR  (-ENOSPC)                       /* too much bandwidth used */
#define USB_ST_URB_INVALID_ERROR  (-EINVAL)                     /* invalid value/transfer type */
#define USB_ST_STALL            (-EPIPE)                        /* pipe stalled, also in urb->status*/

/*
 * USB device number allocation bitmap. There's one bitmap
 * per USB tree.
 */
#define USB_MAXBUS              64

struct usb_devmap 
{
    UINT8 devicemap[16];
};

struct usb_busmap 
{
    UINT8 busmap[USB_MAXBUS / 8];
};


typedef struct list_head 
{
	struct list_head *next, *prev;
} USB_LIST_T;


/*
 * This is a USB device descriptor.
 *
 * USB device information
 */

/* Everything but the endpoint maximums are aribtrary */
#define USB_MAXCONFIG           8
#define USB_ALTSETTINGALLOC     16
#define USB_MAXALTSETTING       128  /* Hard limit */
#define USB_MAXINTERFACES       32
#define USB_MAXENDPOINTS        32

/*
 * declaring data structures presented before their definition met
 */
struct usb_device;
struct urb;

/* All standard descriptors have these 2 fields in common */
#if defined(__GNUC__)
typedef struct usb_descriptor_header 
{
    UINT8  bLength;
    UINT8  bDescriptorType;
} __attribute__((packed)) USB_DESC_HDR_T;  
#else
typedef struct usb_descriptor_header 
{
    __packed UINT8  bLength;
    __packed UINT8  bDescriptorType;     
} USB_DESC_HDR_T;
#endif

/* Device descriptor */
#if defined(__GNUC__)
typedef struct usb_device_descriptor 
{
    UINT8  bLength;
    UINT8  bDescriptorType;
    UINT16 bcdUSB;
    UINT8  bDeviceClass;
    UINT8  bDeviceSubClass;
    UINT8  bDeviceProtocol;
    UINT8  bMaxPacketSize0;
    UINT16 idVendor;
    UINT16 idProduct;
    UINT16 bcdDevice;
    UINT8  iManufacturer;
    UINT8  iProduct;
    UINT8  iSerialNumber;
    UINT8  bNumConfigurations; 
} __attribute__((packed)) USB_DEV_DESC_T;
#else  
typedef struct usb_device_descriptor 
{
    __packed UINT8  bLength;
    __packed UINT8  bDescriptorType;
    __packed UINT16 bcdUSB;
    __packed UINT8  bDeviceClass;
    __packed UINT8  bDeviceSubClass;
    __packed UINT8  bDeviceProtocol;
    __packed UINT8  bMaxPacketSize0;
    __packed UINT16 idVendor;
    __packed UINT16 idProduct;
    __packed UINT16 bcdDevice;
    __packed UINT8  iManufacturer;
    __packed UINT8  iProduct;
    __packed UINT8  iSerialNumber;
    __packed UINT8  bNumConfigurations;     
} USB_DEV_DESC_T;
#endif

/* Endpoint descriptor */
#if defined(__GNUC__)
typedef struct usb_endpoint_descriptor 
{
    UINT8  bLength;
    UINT8  bDescriptorType;
    UINT8  bEndpointAddress;
    UINT8  bmAttributes;
    UINT16 wMaxPacketSize;
    UINT8  bInterval;
    UINT8  bRefresh;
    UINT8  bSynchAddress;

    UINT8   *extra;                    /* Extra descriptors */
    INT     extralen;
} __attribute__((packed)) USB_EP_DESC_T;
#else   
typedef struct usb_endpoint_descriptor 
{
    __packed UINT8  bLength;
    __packed UINT8  bDescriptorType;
    __packed UINT8  bEndpointAddress;
    __packed UINT8  bmAttributes;
    __packed UINT16 wMaxPacketSize;
    __packed UINT8  bInterval;
    __packed UINT8  bRefresh;
    __packed UINT8  bSynchAddress;

    UINT8   *extra;                    /* Extra descriptors */
    INT     extralen;
} USB_EP_DESC_T;
#endif

/* Interface descriptor */
#if defined(__GNUC__)
typedef struct usb_interface_descriptor 
{
    UINT8  bLength;
    UINT8  bDescriptorType;
    UINT8  bInterfaceNumber;
    UINT8  bAlternateSetting;
    UINT8  bNumEndpoints;
    UINT8  bInterfaceClass;
    UINT8  bInterfaceSubClass;
    UINT8  bInterfaceProtocol;
    UINT8  iInterface;

    USB_EP_DESC_T *endpoint;

    UINT8  *extra;                     /* Extra descriptors */
    INT    extralen;   
} __attribute__((packed)) USB_IF_DESC_T;
#else  
typedef struct usb_interface_descriptor 
{
    __packed UINT8  bLength;
    __packed UINT8  bDescriptorType;
    __packed UINT8  bInterfaceNumber;
    __packed UINT8  bAlternateSetting;
    __packed UINT8  bNumEndpoints;
    __packed UINT8  bInterfaceClass;
    __packed UINT8  bInterfaceSubClass;
    __packed UINT8  bInterfaceProtocol;
    __packed UINT8  iInterface;

    USB_EP_DESC_T *endpoint;

    UINT8  *extra;                     /* Extra descriptors */
    INT    extralen;    
} USB_IF_DESC_T;
#endif 

typedef struct usb_interface 
{
    USB_IF_DESC_T  *altsetting;
    INT   act_altsetting;              /* active alternate setting */
    INT   num_altsetting;              /* number of alternate settings */
    INT   max_altsetting;              /* total memory allocated */
    struct usb_driver  *driver;        /* driver */
    VOID  *private_data;
} USB_IF_T;

/* Configuration descriptor information.. */
#if defined(__GNUC__)
typedef struct usb_config_descriptor 
{
    UINT8   bLength;
    UINT8   bDescriptorType;
    UINT16  wTotalLength;
    UINT8   bNumInterfaces;
    UINT8   bConfigurationValue;
    UINT8   iConfiguration;
    UINT8   bmAttributes;
    UINT8   MaxPower;

    USB_IF_T  *interface;
    UINT8   *extra;                    /* Extra descriptors */
    INT     extralen;   
}__attribute__((packed)) USB_CONFIG_DESC_T;
#else
typedef struct usb_config_descriptor 
{
    __packed UINT8   bLength;
    __packed UINT8   bDescriptorType;
    __packed UINT16  wTotalLength;
    __packed UINT8   bNumInterfaces;
    __packed UINT8   bConfigurationValue;
    __packed UINT8   iConfiguration;
    __packed UINT8   bmAttributes;
    __packed UINT8   MaxPower;

    USB_IF_T  *interface;
    UINT8   *extra;                    /* Extra descriptors */
    INT     extralen;   
} USB_CONFIG_DESC_T;
#endif


/* String descriptor */
#if defined(__GNUC__)
typedef struct usb_string_descriptor
{
    UINT8  bLength;
    UINT8  bDescriptorType;
    UINT16 wData[1];
} __attribute__((packed)) USB_STR_DESC_T;
#else  
typedef struct usb_string_descriptor 
{
    __packed UINT8  bLength;
    __packed UINT8  bDescriptorType;
    __packed UINT16 wData[1];   
} USB_STR_DESC_T;
#endif

typedef void (*PFN_PORT_MS_CALLBACK)(VOID  *);
VOID umass_register_connect(PFN_PORT_MS_CALLBACK pfnCallback);
VOID umass_register_disconnect(PFN_PORT_MS_CALLBACK pfnCallback);

/*
 * Device table entry for "new style" table-driven USB drivers.
 * User mode code can read these tables to choose which modules to load.
 * Declare the table as __devinitdata, and as a MODULE_DEVICE_TABLE.
 *
 * With a device table provide bind() instead of probe().  Then the
 * third bind() parameter will point to a matching entry from this
 * table.  (Null value reserved.)
 * 
 * Terminate the driver's table with an all-zeroes entry.
 * Init the fields you care about; zeroes are not used in comparisons.
 */
#define USB_DEVICE_ID_MATCH_VENDOR              0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT             0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO              0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI              0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS           0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS        0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL        0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS           0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS        0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL        0x0200

#define USB_DEVICE_ID_MATCH_DEVICE              (USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)
#define USB_DEVICE_ID_MATCH_DEV_RANGE           (USB_DEVICE_ID_MATCH_DEV_LO | USB_DEVICE_ID_MATCH_DEV_HI)
#define USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION  (USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_DEV_RANGE)
#define USB_DEVICE_ID_MATCH_DEV_INFO \
        (USB_DEVICE_ID_MATCH_DEV_CLASS | USB_DEVICE_ID_MATCH_DEV_SUBCLASS | USB_DEVICE_ID_MATCH_DEV_PROTOCOL)
#define USB_DEVICE_ID_MATCH_INT_INFO \
        (USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS | USB_DEVICE_ID_MATCH_INT_PROTOCOL)

/* Some useful macros */
#define USB_DEVICE(vend,prod) \
        { USB_DEVICE_ID_MATCH_DEVICE, vend, prod, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

#define USB_DEVICE_VER(vend,prod,lo,hi) \
        { USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION, vend, prod, lo, hi, 0, 0, 0, 0, 0, 0, 0 }

#define USB_DEVICE_INFO(cl,sc,pr) \
        { USB_DEVICE_ID_MATCH_DEV_INFO, 0, 0, 0, 0, cl, sc, pr, 0, 0, 0, 0 }

#define USB_INTERFACE_INFO(cl,sc,pr) \
        { USB_DEVICE_ID_MATCH_INT_INFO, 0, 0, 0, 0, 0, 0, 0, cl, sc, pr, 0 }

typedef struct usb_device_id 
{
    /* This bitmask is used to determine which of the following fields
     * are to be used for matching.
     */
    UINT16  match_flags;

    /*
     * vendor/product codes are checked, if vendor is nonzero
     * Range is for device revision (bcdDevice), inclusive;
     * zero values here mean range isn't considered
     */
    UINT16  idVendor;
    UINT16  idProduct;
    UINT16  bcdDevice_lo; 
    UINT16	bcdDevice_hi;

    /*
     * if device class != 0, these can be match criteria;
     * but only if this bDeviceClass value is nonzero
     */
    UINT8   bDeviceClass;
    UINT8   bDeviceSubClass;
    UINT8   bDeviceProtocol;

    /*
     * if interface class != 0, these can be match criteria;
     * but only if this bInterfaceClass value is nonzero
     */
    UINT8   bInterfaceClass;
    UINT8   bInterfaceSubClass;
    UINT8   bInterfaceProtocol;

    /*
     * for driver's use; not involved in driver matching.
     */
    UINT32  driver_info;
} USB_DEV_ID_T;

typedef struct usb_driver 
{
    const CHAR  *name;

    VOID  *(*probe)(struct usb_device *dev, UINT32 intf, const USB_DEV_ID_T *id);
    VOID  (*disconnect)(struct usb_device *, VOID *);
    USB_LIST_T  driver_list;
    /* ioctl -- userspace apps can talk to drivers through usbdevfs */
    INT  (*ioctl)(struct usb_device *dev, UINT32 code, VOID *buf);
    /* 
     * support for "new-style" USB hotplugging
     * binding policy can be driven from user mode too
     */
    const USB_DEV_ID_T  *id_table;
    /* suspend before the bus suspends;
     * disconnect or resume when the bus resumes */
    VOID (*suspend)(struct usb_device *dev);
    VOID (*resume)(struct usb_device *dev);
} USB_DRIVER_T;
       
       
        
/*----------------------------------------------------------------------------* 
 * New USB Structures                                                         *
 *----------------------------------------------------------------------------*/
/*
 * urb->transfer_flags:
 */
#define USB_DISABLE_SPD   0x0001
#define URB_SHORT_NOT_OK  USB_DISABLE_SPD
#define USB_ISO_ASAP      0x0002
#define USB_ASYNC_UNLINK  0x0008
#define USB_QUEUE_BULK    0x0010
#define USB_NO_FSBR       0x0020
#define USB_ZERO_PACKET   0x0040  /* Finish bulk OUTs always with zero length packet */
#define URB_NO_INTERRUPT  0x0080  /* HINT: no non-error interrupt needed */

#define USB_TIMEOUT_KILLED 0x1000 /* only set by HCD! */

typedef struct
{
    UINT32  offset;
    UINT32  length;                    /* expected length */
    UINT32  actual_length;
    UINT32  status;
} ISO_PACKET_DESCRIPTOR_T, *PISO_PACKET_DESCRIPTOR_T;

typedef struct urb
{
    VOID    *hcpriv;                   /* private data for host controller */
    USB_LIST_T  urb_list;              /* list pointer to all active urbs */
    struct urb  *next;                 /* pointer to next URB  */
    struct usb_device  *dev;           /* pointer to associated USB device */
    UINT32  pipe;                      /* pipe information */
    INT     status;                    /* returned status */
    UINT32  transfer_flags;            /* USB_DISABLE_SPD | USB_ISO_ASAP | etc. */
    VOID    *transfer_buffer;          /* associated data buffer */
    //VOID  *transfer_dma;
    INT     transfer_buffer_length;    /* data buffer length */
    INT     actual_length;             /* actual data buffer length */
    INT     bandwidth;                 /* bandwidth for this transfer request (INT or ISO) */
    UINT8   *setup_packet;             /* setup packet (control only) */
    //UINT8	*setup_dma;

    INT     start_frame;               /* start frame (iso/irq only) */
    INT     number_of_packets;         /* number of packets in this request (iso) */
    INT     interval;                  /* polling interval (irq only) */
    INT     error_count;               /* number of errors in this transfer (iso only) */
    INT     timeout;                   /* timeout (in jiffies) */
    VOID    *context;
#ifdef NUCLEUS
    NU_TASK *task;
    NU_EVENT_GROUP events;
#endif    
    VOID (*complete)(struct urb *);
    ISO_PACKET_DESCRIPTOR_T  iso_frame_desc[32];
} URB_T, *PURB_T;


#define FILL_CONTROL_URB(a,aa,b,c,d,e,f,g) \
    do {\
        (a)->dev=aa;\
        (a)->pipe=b;\
        (a)->setup_packet=c;\
        (a)->transfer_buffer=d;\
        (a)->transfer_buffer_length=e;\
        (a)->complete=f;\
        (a)->context=g;\
    } while (0)


#define FILL_BULK_URB(a,aa,b,c,d,e,f) \
    do {\
        (a)->dev=aa;\
        (a)->pipe=b;\
        (a)->transfer_buffer=c;\
        (a)->transfer_buffer_length=d;\
        (a)->complete=e;\
        (a)->context=f;\
    } while (0)

    
#define FILL_INT_URB(a,aa,b,c,d,e,f,g) \
    do {\
        (a)->dev=aa;\
        (a)->pipe=b;\
        (a)->transfer_buffer=c;\
        (a)->transfer_buffer_length=d;\
        (a)->complete=e;\
        (a)->context=f;\
        (a)->interval=g;\
        (a)->start_frame=-1;\
    } while (0)


#define FILL_CONTROL_URB_TO(a,aa,b,c,d,e,f,g,h) \
    do {\
        (a)->dev=aa;\
        (a)->pipe=b;\
        (a)->setup_packet=c;\
        (a)->transfer_buffer=d;\
        (a)->transfer_buffer_length=e;\
        (a)->complete=f;\
        (a)->context=g;\
        (a)->timeout=h;\
    } while (0)


#define FILL_BULK_URB_TO(a,aa,b,c,d,e,f,g) \
    do {\
        (a)->dev=aa;\
        (a)->pipe=b;\
        (a)->transfer_buffer=c;\
        (a)->transfer_buffer_length=d;\
        (a)->complete=e;\
        (a)->context=f;\
        (a)->timeout=g;\
    } while (0)

    


/* -------------------------------------------------------------------------- */

typedef struct usb_operations 
{
    INT (*allocate)(struct usb_device *);
    INT (*deallocate)(struct usb_device *);
    INT (*get_frame_number) (struct usb_device *usb_dev);
    INT (*submit_urb)(PURB_T purb);
    INT (*unlink_urb)(PURB_T purb);
} USB_OP_T;

/*
 * Allocated per bus we have
 */
typedef struct usb_bus 
{
    INT    busnum;                      /* Bus number (in order of reg) */
    struct usb_devmap  devmap;          /* Device map */
    USB_OP_T  *op;                      /* Operations (specific to the HC) */
    struct usb_device  *root_hub;       /* Root hub */
    USB_LIST_T  bus_list;
    VOID   *hcpriv;                     /* Host Controller private data */

    INT bandwidth_allocated;            /* on this Host Controller; */
                                        /* applies to Int. and Isoc. pipes; */
                                        /* measured in microseconds/frame; */
                                        /* range is 0..900, where 900 = */
                                        /* 90% of a 1-millisecond frame */
    INT    bandwidth_int_reqs;          /* number of Interrupt requesters */
    INT    bandwidth_isoc_reqs;         /* number of Isoc. requesters */
} USB_BUS_T;

#define USB_MAXCHILDREN         (16)    /* This is arbitrary */


typedef struct usb_device 
{
    INT     devnum;                     /* Device number on USB bus */
    INT     slow;                       /* Slow device? */
    enum 
    {
        USB_SPEED_UNKNOWN = 0,          /* enumerating */
        USB_SPEED_LOW, USB_SPEED_FULL,  /* usb 1.1 */
        USB_SPEED_HIGH                  /* usb 2.0 */
    } speed;

    struct usb_tt	*tt;                  /* low/full speed dev, highspeed hub */
    int     ttport;                     /* device port on that tt hub */

    INT     refcnt;                     /* atomic_t, Reference count */

    UINT32  toggle[2];                  /* one bit for each endpoint ([0] = IN, [1] = OUT) */
    UINT32  halted[2];                  /* endpoint halts; one bit per endpoint # & direction; */
                                        /* [0] = IN, [1] = OUT */
    INT     epmaxpacketin[16];          /* INput endpoint specific maximums */
    INT     epmaxpacketout[16];         /* OUTput endpoint specific maximums */

    struct usb_device  *parent;
    INT     hub_port;
    USB_BUS_T  *bus;                    /* Bus we're part of */

    USB_DEV_DESC_T  descriptor;         /* Descriptor */
    USB_CONFIG_DESC_T *config;          /* All of the configs */
    USB_CONFIG_DESC_T *actconfig;       /* the active configuration */

    CHAR    **rawdescriptors;           /* Raw descriptors for each config */

    INT     have_langid;                /* whether string_langid is valid yet */
    INT     string_langid;              /* language ID for strings */
  
    VOID    *hcpriv;                   /* Host Controller private data */
        
    /*
     * Child devices - these can be either new devices
     * (if this is a hub device), or different instances
     * of this same device.
     *
     * Each instance needs its own set of data structures.
     */

    INT     maxchild;                  /* Number of ports if hub */
    struct usb_device  *children[USB_MAXCHILDREN];
} USB_DEV_T;


/*
 * Calling this entity a "pipe" is glorifying it. A USB pipe
 * is something embarrassingly simple: it basically consists
 * of the following information:
 *  - device number (7 bits)
 *  - endpoint number (4 bits)
 *  - current Data0/1 state (1 bit)
 *  - direction (1 bit)
 *  - speed (1 bit)
 *  - max packet size (2 bits: 8, 16, 32 or 64) [Historical; now gone.]
 *  - pipe type (2 bits: control, interrupt, bulk, isochronous)
 *
 * That's 18 bits. Really. Nothing more. And the USB people have
 * documented these eighteen bits as some kind of glorious
 * virtual data structure.
 *
 * Let's not fall in that trap. We'll just encode it as a simple
 * UINT32. The encoding is:
 *
 *  - max size:         bits 0-1        (00 = 8, 01 = 16, 10 = 32, 11 = 64) [Historical; now gone.]
 *  - direction:        bit 7           (0 = Host-to-Device [Out], 1 = Device-to-Host [In])
 *  - device:           bits 8-14
 *  - endpoint:         bits 15-18
 *  - Data0/1:          bit 19
 *  - speed:            bit 26          (0 = Full, 1 = Low Speed)
 *  - pipe type:        bits 30-31      (00 = isochronous, 01 = interrupt, 10 = control, 11 = bulk)
 *
 * Why? Because it's arbitrary, and whatever encoding we select is really
 * up to us. This one happens to share a lot of bit positions with the UHCI
 * specification, so that much of the uhci driver can just mask the bits
 * appropriately.
 */

#define PIPE_ISOCHRONOUS                0
#define PIPE_INTERRUPT                  1
#define PIPE_CONTROL                    2
#define PIPE_BULK                       3

#define HOST_LIKE_PORT0           0
#define HOST_LIKE_PORT1           1
#define HOST_NORMAL_PORT0_ONLY    2
#define HOST_NORMAL_TWO_PORT      3

#define usb_maxpacket(dev, pipe, out)   (out \
                                ? (dev)->epmaxpacketout[usb_pipeendpoint(pipe)] \
                                : (dev)->epmaxpacketin [usb_pipeendpoint(pipe)] )
#define usb_packetid(pipe)      (((pipe) & USB_DIR_IN) ? USB_PID_IN : USB_PID_OUT)

#define usb_pipeout(pipe)       ((((pipe) >> 7) & 1) ^ 1)
#define usb_pipein(pipe)        (((pipe) >> 7) & 1)
#define usb_pipedevice(pipe)    (((pipe) >> 8) & 0x7f)
#define usb_pipe_endpdev(pipe)  (((pipe) >> 8) & 0x7ff)
#define usb_pipeendpoint(pipe)  (((pipe) >> 15) & 0xf)
#define usb_pipedata(pipe)      (((pipe) >> 19) & 1)
#define usb_pipeslow(pipe)      (((pipe) >> 26) & 1)
#define usb_pipetype(pipe)      (((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)      (usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)       (usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)   (usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)      (usb_pipetype((pipe)) == PIPE_BULK)

#define PIPE_DEVEP_MASK         0x0007ff00

/* The D0/D1 toggle bits */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> ep) & 1)
#define usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << ep))
#define usb_settoggle(dev, ep, out, bit) ((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << ep)) | ((bit) << ep))

/* Endpoint halt control/status */
#define usb_endpoint_out(ep_dir)        (((ep_dir >> 7) & 1) ^ 1)
#define usb_endpoint_halt(dev, ep, out) ((dev)->halted[out] |= (1 << (ep)))
#define usb_endpoint_running(dev, ep, out) ((dev)->halted[out] &= ~(1 << (ep)))
#define usb_endpoint_halted(dev, ep, out) ((dev)->halted[out] & (1 << (ep)))

static __inline UINT32 __create_pipe(USB_DEV_T *dev, UINT32 endpoint)
{
        return (dev->devnum << 8) | (endpoint << 15) | (dev->slow << 26);
}

static __inline UINT32 __default_pipe(USB_DEV_T *dev)
{
        return (dev->slow << 26);
}

/* Create various pipes... */
#define usb_sndctrlpipe(dev,endpoint)   (0x80000000 | __create_pipe(dev,endpoint))
#define usb_rcvctrlpipe(dev,endpoint)   (0x80000000 | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndisocpipe(dev,endpoint)   (0x00000000 | __create_pipe(dev,endpoint))
#define usb_rcvisocpipe(dev,endpoint)   (0x00000000 | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev,endpoint)   (0xC0000000 | __create_pipe(dev,endpoint))
#define usb_rcvbulkpipe(dev,endpoint)   (0xC0000000 | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev,endpoint)    (0x40000000 | __create_pipe(dev,endpoint))
#define usb_rcvintpipe(dev,endpoint)    (0x40000000 | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_snddefctrl(dev)             (0x80000000 | __default_pipe(dev))
#define usb_rcvdefctrl(dev)             (0x80000000 | __default_pipe(dev) | USB_DIR_IN)

/*
 * Some USB bandwidth allocation constants.
 */
#define BW_HOST_DELAY                1000L      /* nanoseconds */
#define BW_HUB_LS_SETUP              333L       /* nanoseconds */
                                                /* 4 full-speed bit times (est.) */

#define FRAME_TIME_BITS              12000L     /* frame = 1 millisecond */
#define FRAME_TIME_MAX_BITS_ALLOC    (90L * FRAME_TIME_BITS / 100L)
#define FRAME_TIME_USECS             1000L
#define FRAME_TIME_MAX_USECS_ALLOC   (90L * FRAME_TIME_USECS / 100L)

/* 
 * With integer truncation, trying not to use worst-case bit-stuffing
 * of (7/6 * 8 * bytecount) = 9.33 * bytecount 
 * bytecount = data payload byte count 
 */
#define BitTime(bytecount)           (7 * 8 * bytecount / 6)  

/* convert & round nanoseconds to microseconds */
#define NS_TO_US(ns)                 ((ns + 500L) / 1000L)


extern USB_LIST_T  usb_driver_list;
extern USB_LIST_T  usb_bus_list;
                        


/*-------------------------------------------------------------------------
 *   Exported Functions
 *-------------------------------------------------------------------------*/

/* USB System */
extern INT  InitUsbSystem(VOID);
extern INT  DeInitUsbSystem(VOID);
extern INT  UsbInitializeOHCI(VOID);
extern INT  UsbInitializeHCD(VOID);
extern INT  UsbReleaseHCD(VOID);
#ifdef ECOS
extern void  Hub_CheckIrqEvent(cyg_addrword_t);
#else
extern VOID  Hub_CheckIrqEvent(VOID);
#endif

/* USB Bus */
extern USB_BUS_T  *USB_AllocateBus(USB_OP_T *);
extern VOID USB_FreeBus(USB_BUS_T *);
extern VOID USB_RegisterBus(USB_BUS_T *);
extern VOID usb_deregister_bus(USB_BUS_T *);

/* USB Device Driver */
extern INT  USB_RegisterDriver(USB_DRIVER_T *);
extern VOID USB_DeregisterDriver(USB_DRIVER_T *);
extern VOID USB_ScanDevices(VOID);
extern VOID USB_DriverClaimInterface(USB_DRIVER_T *driver, USB_IF_T *iface, VOID* priv);
extern INT  USB_InterfaceClaimed(USB_IF_T *iface);
extern VOID USB_DriverReleaseInterface(USB_DRIVER_T *driver, USB_IF_T *iface);
extern const  USB_DEV_ID_T *usb_match_id(USB_DEV_T *dev, USB_IF_T *interface, const USB_DEV_ID_T *id);

/* USB Device */
extern USB_DEV_T  *USB_AllocateDevice(USB_DEV_T *parent, USB_BUS_T *);
extern VOID USB_FreeDevice(USB_DEV_T *);
extern VOID USB_ConnectDevice(USB_DEV_T *dev);
extern VOID USB_DisconnectDevice(USB_DEV_T **);
extern INT  USB_SettleNewDevice(USB_DEV_T *dev);
extern INT  USB_ResetDevice(USB_DEV_T *dev);
extern VOID USB_IncreaseDeviceUser(USB_DEV_T *);
#define USB_DecreaseDeviceUser  USB_FreeDevice

/* Bandwidth */
extern INT  USB_CheckBandwidth(USB_DEV_T *dev, URB_T *urb);
extern VOID USB_ClaimBandwidth(USB_DEV_T *dev, URB_T *urb, INT bustime, INT isoc);
extern VOID USB_ReleaseBandwidth(USB_DEV_T *dev, URB_T *urb, INT isoc);

/* USB Hub */
extern INT  USB_InitHubDriver(VOID);
extern VOID USB_RemoveHubDriver(VOID);
extern INT  USB_RootHubString(INT id, INT serial, CHAR *type, UINT8 *data, INT len);

/* URB */
extern URB_T  *USB_AllocateUrb(INT iso_packets);
extern VOID USB_FreeUrb(URB_T *urb);
extern INT  USB_SubmitUrb(URB_T *urb);
extern INT  USB_UnlinkUrb(URB_T *urb);

/* Blocking Mode Transfer */
extern INT  USB_SendControlMessage(USB_DEV_T *dev, UINT32 pipe, UINT8 request, UINT8 requesttype, UINT16 value, UINT16 index, VOID *data, UINT16 size, INT timeout);
extern INT  USB_SendBulkMessage(USB_DEV_T *usb_dev, UINT32 pipe, VOID *data, INT len, INT *actual_length, INT timeout);

/* Standard Device Request Commands */
extern INT  USB_GetDescriptor(USB_DEV_T *dev, UINT8 desctype, UINT8 descindex, VOID *buf, INT size);
extern INT  USB_GetClassDescriptor(USB_DEV_T *dev, INT ifnum, UINT8 desctype, UINT8 descindex, VOID *buf, INT size);
extern INT  USB_GetDeviceDescriptor(USB_DEV_T *dev);
extern INT  USB_GetExtraDescriptor(CHAR *buffer, UINT32 size, UINT8 type, VOID **ptr);
extern INT  USB_GetStringDescriptor(USB_DEV_T *dev, UINT16 langid, UINT8 index, VOID *buf, INT size);
extern INT  USB_SetAddress(USB_DEV_T *dev);
extern INT  USB_SetInterface(USB_DEV_T *dev, INT ifnum, INT alternate);
extern INT  USB_GetConfiguration(USB_DEV_T *dev);
extern INT  USB_SetConfiguration(USB_DEV_T *dev, INT configuration);
extern INT  USB_GetStatus(USB_DEV_T *dev, INT type, INT target, VOID *data);

/* HID Class Request Commands */
extern INT	USB_GetProtocol(USB_DEV_T *dev, INT ifnum);
extern INT	USB_SetProtocol(USB_DEV_T *dev, INT ifnum, INT protocol);
extern INT	USB_GetReport(USB_DEV_T *dev, INT ifnum, UINT8 type, UINT8 id, VOID *buf, INT size);
extern INT  USB_SetReport(USB_DEV_T *dev, INT ifnum, UINT8 type, UINT8 id, VOID *buf, INT size);
extern INT  USB_SetIdle(USB_DEV_T *dev, INT ifnum, INT duration, INT report_id);

/* Init USB mass storage class driver */
extern INT  UMAS_InitUmasDriver(VOID);
extern VOID  UMAS_RemoveUmasDriver(VOID);
/* Miscellaneous */
extern INT  USB_GetCurrentFrameNumber(USB_DEV_T *usb_dev);
extern VOID USB_DestroyConfiguration(USB_DEV_T *dev);
extern USB_IF_T	 *USB_GetInterfaceData(USB_DEV_T *dev, INT ifnum);
extern USB_EP_DESC_T  *USB_GetEndpointDescriptor(USB_DEV_T *dev, INT epnum);
extern INT	USB_ClearHalt(USB_DEV_T *dev, INT pipe);
extern VOID	USB_SetMaximumPacketSize(USB_DEV_T *dev);
extern INT	USB_TranslateString(USB_DEV_T *dev, INT index, CHAR *buf, INT size);
extern VOID  	USB_WaitMiliseconds(UINT32 msec);

/* Debugging helpers.. */
extern VOID	USB_ShowDeviceDescriptor(USB_DEV_DESC_T *);
extern VOID	USB_ShowConfigurationDescriptor(USB_CONFIG_DESC_T *);
extern VOID	USB_ShowInterfaceDescriptor(USB_IF_DESC_T *);
extern VOID	USB_ShowEndpointDescriptor(USB_EP_DESC_T *);
extern VOID	USB_ShowDevice(USB_DEV_T *);
extern VOID	USB_ShowUsbString(USB_DEV_T *dev, CHAR *id, INT index);
extern VOID	USB_DumpUrb(URB_T *purb);

extern VOID	*USB_malloc(INT wanted_size, INT boundary);
extern VOID	USB_free(VOID *);
extern INT 	USB_available_memory(VOID);
extern INT 	USB_allocated_memory(VOID);

/* USB multiple pin function */	
extern VOID USB_PortInit(UINT32 u32PortType);
extern VOID USB_PortDisable(BOOL bIsDisPort0, BOOL bIsDisPort1);

#endif  /* _USB_H_ */

