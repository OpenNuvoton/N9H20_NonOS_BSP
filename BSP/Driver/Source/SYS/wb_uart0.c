/**************************************************************************//**
 * @file     wb_uart0.c
 * @version  V3.00
 * @brief    N9H20 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   wb_uart0.c (high speed uart)
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The UART related function of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
*
* REMARK
*   None
 **************************************************************************/
#include <string.h>
#include <stdio.h>
#include "wblib.h"

#ifdef  __HW_SIM__
#undef  REG_UART_THR
#define REG_UART_THR    (0xFFF04400)        // TUBE ON
#endif

static PFN_SYS_UART_CALLBACK (pfnUartIntHandlerTable)[2][UART_INT_TRANEMPTY+1]= {0};

#define vaStart(list, param) list = (INT8*)((INT)&param + sizeof(param))
#define vaArg(list, type) ((type *)(list += sizeof(type)))[-1]

/* Global variables */
static BOOL volatile _sys_bIsUARTInitial = FALSE;
static BOOL volatile _sys_bIsUseUARTInt = TRUE;
static UINT32 _sys_uUARTClockRate = EXTERNAL_CRYSTAL_CLOCK;
//UINT32 UART_BA = UART0_BA;

#define RX_ARRAY_NUM 100
static UINT8 uart_rx[RX_ARRAY_NUM] = {0};

VOID _PutChar_f0(UINT8 ucCh);
static UINT32 volatile rx_cnt = 0;

#define sysTxBufReadNextOne()   (((_sys_uUartTxHead+1)==UART_BUFFSIZE)? (UINT32)NULL: _sys_uUartTxHead+1)
#define sysTxBufWriteNextOne()  (((_sys_uUartTxTail+1)==UART_BUFFSIZE)? (UINT32)NULL: _sys_uUartTxTail+1)
#define UART_BUFFSIZE   256
static UINT8 _sys_ucUartTxBuf[UART_BUFFSIZE];
static UINT32 volatile _sys_uUartTxHead, _sys_uUartTxTail;
static volatile PVOID  _sys_pvOldUartVect;
static UINT32 u32UartPort =0x0; /* Default High Speed UART */


static INT32 i32UsedPort = 0;
static void sysUartPort0(UINT32 u32Port)
{
    u32UartPort = (u32Port & 0x1)*0x100;
    if(u32Port==0)
    {
        //High Speed UART
        outp32(REG_CLKDIV3, (inp32(REG_CLKDIV3) & (~0xFF)));
        outp32(REG_GPDFUN, (inp32(REG_GPDFUN) & ~(MF_GPD2 | MF_GPD1)) | (0x5<<2) );
        outp32(REG_APBCLK, inp32(REG_APBCLK) | UART0_CKE);
    }
    else if(u32Port==1)
    {
        //Nornal Speed UART
        outp32(REG_CLKDIV3, (inp32(REG_CLKDIV3) & (~0xFF00)));
        outp32(REG_GPAFUN, inp32(REG_GPAFUN) | (MF_GPA11 | MF_GPA10));
        outp32(REG_APBCLK, inp32(REG_APBCLK) | UART1_CKE);

    }
    i32UsedPort = u32Port;
}
#if 1
static void sysUart0Installcallback(UINT32 u32IntType,
                                    PFN_SYS_UART_CALLBACK pfnCallback)
{
    if(u32IntType>2)
        return;
    if(u32IntType == UART_INT_RDA)
    {
        pfnUartIntHandlerTable[i32UsedPort][UART_INT_RDA] = (PFN_SYS_UART_CALLBACK)(pfnCallback);
    }
    else if(u32IntType == UART_INT_RDTO)
    {
        pfnUartIntHandlerTable[i32UsedPort][UART_INT_RDTO] = (PFN_SYS_UART_CALLBACK)(pfnCallback);
    }
    else if(u32IntType == UART_INT_TRANEMPTY)
    {
        //Transfer empty.
        pfnUartIntHandlerTable[i32UsedPort][UART_INT_TRANEMPTY] = (PFN_SYS_UART_CALLBACK)(pfnCallback);
    }
}

static VOID sysHUartISR()
{

    UINT32 volatile regIIR,i;
    UINT32 volatile regIER, u32EnableInt;
    //sysprintf("UART ISR\n");
    regIIR = inpb(REG_UART_ISR + u32UartPort);
    regIER =  inpb(REG_UART_IER + u32UartPort);
    u32EnableInt = regIER & regIIR;
    if (u32EnableInt & THRE_IF)
    {
        // buffer empty
        if (_sys_uUartTxHead == _sys_uUartTxTail)
        {
            //Disable interrupt if no any request!
            outp32((REG_UART_IER+u32UartPort), inp32(REG_UART_IER+u32UartPort) & (~THRE_IEN));

            if(pfnUartIntHandlerTable[i32UsedPort][UART_INT_TRANEMPTY] !=0)
                pfnUartIntHandlerTable[i32UsedPort][UART_INT_TRANEMPTY](&(uart_rx[0]), 0); /* Don't care transfer parameter */
        }
        else
        {
            //Transmit data
            for (i=0; i<8; i++)
            {
#ifdef __HW_SIM__

#else
                outpb(REG_UART_THR+u32UartPort, _sys_ucUartTxBuf[_sys_uUartTxHead]);
#endif
                _sys_uUartTxHead = sysTxBufReadNextOne();
                if (_sys_uUartTxHead == _sys_uUartTxTail) // buffer empty
                    break;
            }
        }
    }
    if(u32EnableInt & RDA_IF)
    {
        UINT32 u32Count;
        u32Count = (inpw(REG_UART_FSR+u32UartPort) & Rx_Pointer) >> 8;
#if 0
        sysprintf("\nRx data available %d bytes\n",u32Count);
#endif
        for(i=0; i<u32Count; i++)
        {
            if ( rx_cnt == RX_ARRAY_NUM )
            {
                rx_cnt = 0;
            }
            uart_rx[rx_cnt] = (inpb(REG_UART_RBR+u32UartPort));
#if 0
            //debug _PutChar_f(uart_rx[rx_cnt + i]);
#endif
            rx_cnt++;
        }
#if 0
        sysprintf("\nRx array index %d\n",rx_cnt);
#else
        //callback to uplayer(bufptr, len);

        if(pfnUartIntHandlerTable[i32UsedPort][UART_INT_RDA] !=0)
            pfnUartIntHandlerTable[i32UsedPort][UART_INT_RDA](&(uart_rx[0]), u32Count);
        rx_cnt = 0;
#endif

    }
    if(u32EnableInt & Tout_IF)//½ÓÊÕ×´Ì¬·ÖÎö
    {
        UINT32 u32Count;
        u32Count = (inpw(REG_UART_FSR+u32UartPort) & Rx_Pointer) >> 8;
#if 0
        sysprintf("\nRx data time out %d bytes\n",u32Count);
#endif
        for(i=0; i<u32Count; i++)
        {
            if ( rx_cnt == RX_ARRAY_NUM )
            {
                rx_cnt = 0;
            }
            uart_rx[rx_cnt ] = (inpb(REG_UART_RBR+u32UartPort));
#if 0
            _PutChar_f(uart_rx[rx_cnt + i]);
#endif
            rx_cnt++;

        }
#if 0
        sysprintf("\nRx array index %d\n",rx_cnt);
#else
        //callback to uplayer(bufptr, len);
        if(pfnUartIntHandlerTable[i32UsedPort][UART_INT_RDTO] !=0)
            pfnUartIntHandlerTable[i32UsedPort][UART_INT_RDTO](&(uart_rx[0]), u32Count);
        rx_cnt = 0;
#endif
    }
    //sysprintf("\n");
}

#else
VOID sysHUartISR()
{
    UINT32 volatile regIIR, i;
    regIIR = inpb(REG_UART_ISR+u32UartPort*0x100);

    if (regIIR & THRE_IF)
    {
        // buffer empty
        if (_sys_uUartTxHead == _sys_uUartTxTail)
        {
            //Disable interrupt if no any request!
            outpb((REG_UART_IER+u32UartPort), inp32(REG_UART_IER+u32UartPort) & (~THRE_IEN));
        }
        else
        {
            //Transmit data
            for (i=0; i<8; i++)
            {
#ifdef __HW_SIM__

#else
                outpb(REG_UART_THR+u32UartPort, _sys_ucUartTxBuf[_sys_uUartTxHead]);
#endif
                _sys_uUartTxHead = sysTxBufReadNextOne();
                if (_sys_uUartTxHead == _sys_uUartTxTail)   // buffer empty
                    break;
            }
        }
    }
}
#endif
static VOID sysSetBaudRate(UINT32 uBaudRate)
{
    UINT32 _mBaudValue;

    /* First, compute the baudrate divisor. */
#if 0
    // mode 0
    _mBaudValue = (_sys_uUARTClockRate / (uBaudRate * 16));
    if ((_sys_uUARTClockRate % (uBaudRate * 16)) > ((uBaudRate * 16) / 2))
        _mBaudValue++;
    _mBaudValue -= 2;
    outpw(REG_UART_BAUD+u32UartPort, _mBaudValue);
#else
    // mode 3
    _mBaudValue = (_sys_uUARTClockRate / uBaudRate)-2;
    outpw(REG_UART_BAUD+u32UartPort,  ((0x30<<24)| _mBaudValue));
#endif
}

static void sysUart0EnableInt(INT32 eIntType)
{
    if(eIntType==UART_INT_RDA)
        outp32(REG_UART_IER+u32UartPort, inp32(REG_UART_IER+u32UartPort) |RDA_IEN);
    else if (eIntType==UART_INT_RDTO)
        outp32(REG_UART_IER+u32UartPort, inp32(REG_UART_IER+u32UartPort) |RTO_IEN |Time_out_EN);
    else if (eIntType==UART_INT_NONE)
        outp32(REG_UART_IER+u32UartPort, 0x0);
}
INT32 sysInitializeUART0(WB_UART_T *uart)
{
    /* Enable UART multi-function pins*/
    //outpw(REG_PINFUN, inpw(REG_PINFUN) | 0x80);
    //outpw(REG_GPAFUN, inpw(REG_GPAFUN) | 0x00F00000); //Normal UART pin function
    static BOOL bIsResetFIFO = FALSE;
    /* Check the supplied parity */
    if ((uart->uiParity != WB_PARITY_NONE) &&
            (uart->uiParity != WB_PARITY_EVEN) &&
            (uart->uiParity != WB_PARITY_ODD))

        /* The supplied parity is not valid */
        return WB_INVALID_PARITY;

    /* Check the supplied number of data bits */
    else if ((uart->uiDataBits != WB_DATA_BITS_5) &&
             (uart->uiDataBits != WB_DATA_BITS_6) &&
             (uart->uiDataBits != WB_DATA_BITS_7) &&
             (uart->uiDataBits != WB_DATA_BITS_8))

        /* The supplied data bits value is not valid */
        return WB_INVALID_DATA_BITS;

    /* Check the supplied number of stop bits */
    else if ((uart->uiStopBits != WB_STOP_BITS_1) &&
             (uart->uiStopBits != WB_STOP_BITS_2))

        /* The supplied stop bits value is not valid */
        return WB_INVALID_STOP_BITS;

    /* Verify the baud rate is within acceptable range */
    else if (uart->uiBaudrate < 1200)
        /* The baud rate is out of range */
        return WB_INVALID_BAUD;

    /* Reset the TX/RX FIFOs */
    if(bIsResetFIFO==FALSE)
    {
        outpw(REG_UART_FCR+u32UartPort, 0x07);
        bIsResetFIFO=TRUE;
    }
    /* Setup reference clock */
    _sys_uUARTClockRate = uart->uiFreq;

    /* Setup baud rate */
    sysSetBaudRate(uart->uiBaudrate);

    /* Set the modem control register. Set DTR, RTS to output to LOW,
    and set INT output pin to normal operating mode */
    //outpb(UART_MCR, (WB_DTR_Low | WB_RTS_Low | WB_MODEM_En));

    /* Setup parity, data bits, and stop bits */
    outpw(REG_UART_LCR+u32UartPort,(uart->uiParity | uart->uiDataBits | uart->uiStopBits));

    /* Timeout if more than ??? bits xfer time */
    outpw(REG_UART_TOR+u32UartPort, 0x80+0x20);

    /* Setup Fifo trigger level and enable FIFO */
    outpw(REG_UART_FCR+u32UartPort, (uart->uiRxTriggerLevel << 4) | 0x01);

    /* Enable HUART interrupt Only (Receive Data Available Interrupt & RX Time out Interrupt) */
#if 0
#if 1 //Enable RX data time out 
    outp32(REG_UART_IER+u32UartPort,inp32(REG_UART_IER+u32UartPort) |RDA_IEN |RTO_IEN |Time_out_EN);
#else
    outp32(REG_UART_IER+u32UartPort,inp32(REG_UART_IER+u32UartPort) |RDA_IEN);
#endif
#endif

    /* Timeout if more than ??? bits xfer time */
    outpw(REG_UART_TOR+u32UartPort, 0x7F);
#if 0
    // hook UART interrupt service routine
    if (u32UartPort)
    {
        //==1 NORMAL UART
        _sys_uUartTxHead = _sys_uUartTxTail = (UINT32)NULL;
        _sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_UART, (PVOID)sysUartISR);
        sysEnableInterrupt(IRQ_UART);
    }
    else
    {
        //==0 High SPEED
        _sys_uUartTxHead = _sys_uUartTxTail = (UINT32)NULL;
        _sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_HUART, (PVOID)sysHUartISR);
        sysEnableInterrupt(IRQ_HUART);
    }
#else
    _sys_uUartTxHead = _sys_uUartTxTail = (UINT32)NULL;
    _sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_HUART, (PVOID)sysHUartISR);
    sysEnableInterrupt(IRQ_HUART);
#endif
    _sys_bIsUARTInitial = TRUE;

    return Successful;
}


VOID _PutChar_f0(UINT8 ucCh)
{
    if (_sys_bIsUseUARTInt == TRUE)
    {
        while(sysTxBufWriteNextOne() == _sys_uUartTxHead) ; // buffer full

        _sys_ucUartTxBuf[_sys_uUartTxTail] = ucCh;
        _sys_uUartTxTail = sysTxBufWriteNextOne();

        if (ucCh == '\n')
        {
            while(sysTxBufWriteNextOne() == _sys_uUartTxHead) ; // buffer full

            _sys_ucUartTxBuf[_sys_uUartTxTail] = '\r';
            _sys_uUartTxTail = sysTxBufWriteNextOne();
        }

        if (!(inpw(REG_UART_IER+u32UartPort) & 0x02))
            outpw(REG_UART_IER+u32UartPort, inpw(REG_UART_IER+u32UartPort) | THRE_IEN);
    }
    else
    {
        /* Wait until the transmitter buffer is empty */
        while (!(inpw(REG_UART_FSR+u32UartPort) & 0x400000));
        /* Transmit the character */
        outpb(REG_UART_THR+u32UartPort, ucCh);

        if (ucCh == '\n')
        {
            /* Wait until the transmitter buffer is empty */
            while (!(inpw(REG_UART_FSR+u32UartPort) & 0x400000));
            outpb(REG_UART_THR+u32UartPort, '\r');
        }
    }
}


VOID sysPutString0(INT8 *string)
{
    while (*string != '\0')
    {
        _PutChar_f0(*string);
        string++;
    }
}


static VOID sysPutRepChar(INT8 c, INT count)
{
    while (count--)
        _PutChar_f0(c);
}


static VOID sysPutStringReverse0(INT8 *s, INT index)
{
    while ((index--) > 0)
        _PutChar_f0(s[index]);
}


static VOID sysPutNumber(INT value, INT radix, INT width, INT8 fill)
{
    INT8    buffer[40];
    INT     bi = 0;
    UINT32  uvalue;
    UINT16  digit;
    UINT16  left = FALSE;
    UINT16  negative = FALSE;

    if (fill == 0)
        fill = ' ';

    if (width < 0)
    {
        width = -width;
        left = TRUE;
    }

    if (width < 0 || width > 80)
        width = 0;

    if (radix < 0)
    {
        radix = -radix;
        if (value < 0)
        {
            negative = TRUE;
            value = -value;
        }
    }

    uvalue = value;

    do
    {
        if (radix != 16)
        {
            digit = uvalue % radix;
            uvalue = uvalue / radix;
        }
        else
        {
            digit = uvalue & 0xf;
            uvalue = uvalue >> 4;
        }
        buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
        bi++;

        if (uvalue != 0)
        {
            if ((radix == 10)
                    && ((bi == 3) || (bi == 7) || (bi == 11) || (bi == 15)))
            {
                buffer[bi++] = ',';
            }
        }
    }
    while (uvalue != 0);

    if (negative)
    {
        buffer[bi] = '-';
        bi += 1;
    }

    if (width <= bi)
        sysPutStringReverse0(buffer, bi);
    else
    {
        width -= bi;
        if (!left)
            sysPutRepChar(fill, width);
        sysPutStringReverse0(buffer, bi);
        if (left)
            sysPutRepChar(fill, width);
    }
}


static INT8 *FormatItem(INT8 *f, INT a)
{
    INT8   c;
    INT    fieldwidth = 0;
    INT    leftjust = FALSE;
    INT    radix = 0;
    INT8   fill = ' ';

    if (*f == '0')
        fill = '0';

    while ((c = *f++) != 0)
    {
        if (c >= '0' && c <= '9')
        {
            fieldwidth = (fieldwidth * 10) + (c - '0');
        }
        else
            switch (c)
            {
            case '\000':
                return (--f);
            case '%':
                _PutChar_f0('%');
                return (f);
            case '-':
                leftjust = TRUE;
                break;
            case 'c':
            {
                if (leftjust)
                    _PutChar_f0(a & 0x7f);

                if (fieldwidth > 0)
                    sysPutRepChar(fill, fieldwidth - 1);

                if (!leftjust)
                    _PutChar_f0(a & 0x7f);
                return (f);
            }
            case 's':
            {
                if (leftjust)
                    sysPutString0((PINT8)a);

                if (fieldwidth > strlen((PINT8)a))
                    sysPutRepChar(fill, fieldwidth - strlen((PINT8)a));

                if (!leftjust)
                    sysPutString0((PINT8)a);
                return (f);
            }
            case 'd':
            case 'i':
                radix = -10;
                break;
            case 'u':
                radix = 10;
                break;
            case 'x':
                radix = 16;
                break;
            case 'X':
                radix = 16;
                break;
            case 'o':
                radix = 8;
                break;
            default:
                radix = 3;
                break;      /* unknown switch! */
            }
        if (radix)
            break;
    }

    if (leftjust)
        fieldwidth = -fieldwidth;

    sysPutNumber(a, radix, fieldwidth, fill);

    return (f);
}


VOID sysPrintf0(PINT8 pcStr,...)
{
    WB_UART_T uart;
    INT8  *argP;

    _sys_bIsUseUARTInt = TRUE;
    if (!_sys_bIsUARTInitial)
    {
        uart.uart_no = WB_UART_0;
        uart.uiFreq = sysGetExternalClock()*1000;
        uart.uiBaudrate = 115200;
        uart.uiDataBits = WB_DATA_BITS_8;
        uart.uiStopBits = WB_STOP_BITS_1;
        uart.uiParity = WB_PARITY_NONE;
        uart.uiRxTriggerLevel = LEVEL_1_BYTE;
        sysInitializeUART0(&uart);
    }

    vaStart(argP, pcStr);       /* point at the end of the format string */
    while (*pcStr)
    {
        /* this works because args are all ints */
        if (*pcStr == '%')
            pcStr = FormatItem(pcStr + 1, vaArg(argP, INT));
        else
            _PutChar_f0(*pcStr++);
    }
}


VOID sysprintf0(PINT8 pcStr,...)
{
    WB_UART_T uart;
    INT8  *argP;

    _sys_bIsUseUARTInt = FALSE;
    if (!_sys_bIsUARTInitial)
    {
        //Default use external clock 12MHz as source clock.
        sysUartPort0(0);
        uart.uart_no = WB_UART_0;
        uart.uiFreq = sysGetExternalClock()*1000;
        uart.uiBaudrate = 115200;
        uart.uiDataBits = WB_DATA_BITS_8;
        uart.uiStopBits = WB_STOP_BITS_1;
        uart.uiParity = WB_PARITY_NONE;
        uart.uiRxTriggerLevel = LEVEL_1_BYTE;
        sysInitializeUART(&uart);
    }

    vaStart(argP, pcStr);       /* point at the end of the format string */
    while (*pcStr)
    {
        /* this works because args are all ints */
        if (*pcStr == '%')
            pcStr = FormatItem(pcStr + 1, vaArg(argP, INT));
        else
            _PutChar_f0(*pcStr++);
    }
}


static INT8 sysGetChar0(void)
{
    while (1)
    {
        if (inpw(REG_UART_ISR+u32UartPort) & 0x01)
            return (inpb(REG_UART_RBR+u32UartPort));
    }
}

static INT8 sysGetChar0_NoBlocking(void)
{
//  while (1)
    {
        if (inpw(REG_UART_ISR+u32UartPort) & 0x01)
            return (inpb(REG_UART_RBR+u32UartPort));
        else
            return (0xFF);
    }
}

static VOID sysPutChar0(UINT8 ucCh)
{
    /* Wait until the transmitter buffer is empty */
    while (!(inpw(REG_UART_FSR+u32UartPort) & 0x400000));
    /* Transmit the character */
    outpb(REG_UART_THR+u32UartPort, ucCh);
}

static void sysUartTransfer0(char* pu8buf, UINT32 u32Len)
{
    do
    {
        if( (inp32(REG_UART_FSR+u32UartPort) & Tx_Full) ==0 )
        {
            outpb(REG_UART_THR+u32UartPort, *pu8buf++);
            u32Len = u32Len-1;
        }
    }
    while(u32Len!=0) ;
}

static INT32 UartTransferInt(char* pu8buf, UINT32 u32Len)
{
    UINT32 i, u32TranLen = 0;
    while(sysTxBufWriteNextOne() == _sys_uUartTxHead) ; // buffer full

    for(i = 0; i < u32Len; i = i+1 )
    {
        _sys_ucUartTxBuf[_sys_uUartTxTail] = *pu8buf++;
        _sys_uUartTxTail = sysTxBufWriteNextOne();
        u32TranLen = u32TranLen + 1;
    }

    if (!(inpw(REG_UART_IER+u32UartPort) & THRE_IEN))
        outpw(REG_UART_IER+u32UartPort, inpw(REG_UART_IER+u32UartPort) | THRE_IEN);
    return u32TranLen;
}
//=========================================================================
static void UartPort(UINT32 u32Port)
{
    sysUartPort0(u32Port);
}
static void UartInstallcallback(UINT32 u32IntType,
                                PFN_SYS_UART_CALLBACK pfnCallback)
{
    sysUart0Installcallback(u32IntType, pfnCallback);
}
static INT32 UartInitialize(WB_UART_T *uart)
{
    INT32 i32ErrCode;
    i32ErrCode = sysInitializeUART0(uart);
    return i32ErrCode;
}
static  void UartEnableInt(INT32 eIntType)
{
    sysUart0EnableInt(eIntType);
}

static void UartTransfer(char* pu8buf, UINT32 u32Len)
{
    sysUartTransfer0( pu8buf, u32Len);
}
static VOID UartPutChar(UINT8 ucCh)
{
    sysPutChar0(ucCh);
}
static INT8 UartGetChar(void)
{
    INT8 i8Data;
    i8Data = sysGetChar0();
    return i8Data;
}
static INT8 UartGetChar_NoBlocking(void)
{
    INT8 i8Data;
    i8Data = sysGetChar0_NoBlocking();
    return i8Data;
}

#if 0
static void Uartprintf(PINT8 pcStr,...)
{
    sysprintf(pcStr,...);
}

static void sysPrintf(PINT8 pcStr,...)
{

}
#endif
UARTDEV_T nvt_uart0 =
{
    UartPort,
    UartInstallcallback,
    UartInitialize,
    UartEnableInt,
    UartTransfer,
    UartPutChar,
    UartGetChar,
    UartTransferInt,
    UartGetChar_NoBlocking,
#if 0
    Uartprintf,
    UartPrintf,
#endif
};
