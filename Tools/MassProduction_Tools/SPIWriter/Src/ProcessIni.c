/****************************************************************************
 * @file     ProcessIni.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SpiWriter source file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "writer.h"

#if 1
#define dbgprintf sysprintf
#else
#define dbgprintf(...)
#endif

#define  INI_BUF_SIZE  512
char iniBuf[INI_BUF_SIZE];
int  buffer_current = 0, buffer_end = 0;    /* position to buffer iniBuf */

INI_INFO_T Ini_Writer;
UINT32 u32GpioPort_Start = 0, u32GpioPort_Pass = 0, u32GpioPort_Fail = 0;
UINT32 u32GpioPin_Start = 0, u32GpioPin_Pass = 0, u32GpioPin_Fail = 0;
UINT32 u32GpioLevel_Start = 0, u32GpioLevel_Pass = 0, u32GpioLevel_Fail = 0;
UINT32 u32ImageCount = 0;
UINT32 u32UserImageCount = 0;
/*-----------------------------------------------------------------------------
 * To read one string line from file FileHandle and save it to Cmd.
 * Return:
 *      Successful  : OK
 *      < 0 : error code of NVTFAT, include ERR_FILE_EOF
 *---------------------------------------------------------------------------*/
int readLine (int FileHandle, char *Cmd)
{
    int status, nReadLen, i_cmd;

    i_cmd = 0;

    while (1)
    {
        /* parse INI file in buffer iniBuf[] that read in at previous fsReadFile() */
        while(buffer_current < buffer_end)
        {
            if (iniBuf[buffer_current] == 0x0D)
            {
                /* DOS   use 0x0D 0x0A as end of line;
                   Linux use 0x0A as end of line;
                 To support both DOS and Linux, we treat 0x0A as real end of line and ignore 0x0D.
                */
                buffer_current++;
                continue;
            }
            else if (iniBuf[buffer_current] == 0x0A)   /* Found end of line */
            {
                Cmd[i_cmd] = 0;     /* end of string */
                buffer_current++;
                return Successful;
            }
            else
            {
                Cmd[i_cmd] = iniBuf[buffer_current];
                buffer_current++;
                i_cmd++;
            }
        }

        /*
           buffer iniBuf[] is empty here. Try to read more data from file to buffer iniBuf[].
           no more data to read since previous fsReadFile() cannot read buffer full
         */
        if ((buffer_end < INI_BUF_SIZE) && (buffer_end > 0))
        {
            if (i_cmd > 0)
            {
                /* return last line of INI file that without end of line */
                Cmd[i_cmd] = 0;     /* end of string */
                return Successful;
            }
            else
            {
                Cmd[i_cmd] = 0;     /* end of string to clear Cmd */
                return ERR_FILE_EOF;
            }
        }
        else
        {
            status = fsReadFile(FileHandle, (UINT8 *)iniBuf, INI_BUF_SIZE, &nReadLen);
            if (status < 0)
            {
                Cmd[i_cmd] = 0;     /* end of string to clear Cmd */
                return status;      /* error or end of file */
            }
            buffer_current = 0;
            buffer_end = nReadLen;
        }
    }   /* end of while (1) */
}

int Convert(char *Data)
{
    int i = 0,j = 0;
    int value = 0;
    for(i=strlen(Data) -1 ;i>=0;i--)
    {
        if(Data[i] >= '0' && Data[i] <='9')
            value = value + (Data[i] - '0') * (1 << (4 * j) );
        else if(Data[i] >= 'A' && Data[i] <='F')
            value = value + (Data[i] - 'A' + 10) * (1 << (4 * j) );
        else if(Data[i] >= 'a' && Data[i] <='f')
            value = value + (Data[i] - 'a' + 10) * (1 << (4 * j) );
        j++;
    }
    return value;
}
/*-----------------------------------------------------------------------------
 * To parse INI file and store configuration to global variable Ini_Writer.
 * Return:
 *      Successful  : OK
 *      < 0 : error code of NVTFAT
 *---------------------------------------------------------------------------*/
int ProcessINI(char *fileName)
{
    CHAR szNvtFullName[64], suNvtFullName[512], Cmd[512],tmp[512],filename[512],address[256],block[256];
    int FileHandle, status, i;

    /* initial default value */
    Ini_Writer.SpiLoader.FileName[0] = 0;
    Ini_Writer.RawData.FileName[0] = 0;
    Ini_Writer.Logo.FileName[0] = 0;
    Ini_Writer.Execute.FileName[0] = 0;
    Ini_Writer.ChipErase = 0;
    Ini_Writer.FourByteAddressMode = 0;
    Ini_Writer.DataVerify = 0;
    Ini_Writer.SoundPlay = 0;
    Ini_Writer.SoundPlayVolume = 0x1F;
    Ini_Writer.SoundPlayStart.FileName[0] = 0;
    Ini_Writer.SoundPlayFail.FileName[0] = 0;
    Ini_Writer.SoundPlayPass.FileName[0] = 0;
    for(i=0;i<22;i++)
    {
        Ini_Writer.UserImage[i].FileName[0] = 0;
        Ini_Writer.UserImage[i].address = 0;
    }
    /* open INI file */
    strcpy(szNvtFullName, fileName);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    FileHandle = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
    if (FileHandle < 0)
    {
        return FileHandle;  /* open file error */
    }

    dbgprintf("**Process SpiWriter.ini file**\n");
    /* parse INI file */
    do {
        status = readLine(FileHandle, Cmd);
        if (status < 0)     /* read file error. Coulde be end of file */
            break;
NextMark2:
        if (strcmp(Cmd, "[SpiLoader File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    sscanf (Cmd,"%[^,], 0x%s",filename, address);
                    strcpy(Ini_Writer.SpiLoader.FileName, filename);
                    Ini_Writer.SpiLoader.address = Convert(address);
                    u32ImageCount++;
                    break;
                }
            } while (1);
        }
        if (strcmp(Cmd, "[Raw Data Mode File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    sscanf (Cmd,"%[^,]",filename);
                    strcpy(Ini_Writer.RawData.FileName, filename);
                    u32ImageCount++;    
                    sysprintf("Use Raw Data Mode - %s\n",Ini_Writer.RawData.FileName);
                    break;
                }
            } while (1);
        }
        else if (strcmp(Cmd, "[Logo File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto  NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    sscanf (Cmd,"%[^,], 0x%[^,], 0x%[^,]",filename, block, address);
                    strcpy(Ini_Writer.Logo.FileName, filename);
                    Ini_Writer.Logo.StartBlock = Convert(block);    
                    Ini_Writer.Logo.address = Convert(address);             
                    u32ImageCount++;
                    break;
                }
            } while (1);
        }
        else if (strcmp(Cmd, "[Execute File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    sscanf (Cmd,"%[^,], 0x%[^,], 0x%[^,]",filename, block, address);
                    strcpy(Ini_Writer.Execute.FileName, filename);
                    Ini_Writer.Execute.StartBlock = Convert(block);
                    Ini_Writer.Execute.address = Convert(address);
                    u32ImageCount++;
                    break;
                }
            } while (1);
        }
        else if (strstr(Cmd,"[User Image File"))
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    sscanf (Cmd,"%[^,], 0x%s",filename, block);
                    strcpy(Ini_Writer.UserImage[u32UserImageCount].FileName, filename);
                    Ini_Writer.UserImage[u32UserImageCount].StartBlock = Convert(block);
                    u32ImageCount++;
                    u32UserImageCount++;
                    break;
                }
            } while (1);
        }
        else if (strcmp(Cmd, "[Data Verify]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    Ini_Writer.DataVerify = atoi(Cmd);
                    sysprintf(" Data Verify is ");
                    Ini_Writer.DataVerify?sysprintf("enable\n"): sysprintf("disable\n");          
                    break;
                }
            } while (1);
        }  
        else if (strcmp(Cmd, "[Sound Play]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    Ini_Writer.SoundPlay = atoi(Cmd);
                    sysprintf(" Sound Play is ");
                    Ini_Writer.SoundPlay?sysprintf("enable\n"): sysprintf("disable\n");          
                    break;
                }
            } while (1);
        }      
        else if (strcmp(Cmd, "[Sound Play Volume]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    Ini_Writer.SoundPlayVolume = atoi(Cmd);
                    if(Ini_Writer.SoundPlayVolume > 0x3F)
                        Ini_Writer.SoundPlayVolume = 0x3F;
                    sysprintf(" Sound Play Volume is %d\n",Ini_Writer.SoundPlayVolume);
                    break;
                }
            } while (1);
        }
        else if (strcmp(Cmd, "[Chip Erase]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       // skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    Ini_Writer.ChipErase = atoi(Cmd);
                    sysprintf(" Chip Erase is ");
                    Ini_Writer.ChipErase?sysprintf("enable\n"): sysprintf("disable\n");          
                    break;
                }
            } while (1);
        } 
        else if (strcmp(Cmd, "[4-Byte Address Mode Enable]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       // skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    Ini_Writer.FourByteAddressMode = atoi(Cmd);
                    sysprintf(" 4-Byte Address Mode is ");
                    Ini_Writer.FourByteAddressMode?sysprintf("enable\n"): sysprintf("disable\n");          
                    break;
                }
            } while (1);
        }               
        else if (strstr(Cmd, "[Flow Status Sound File Name "))
        {
            strcpy(tmp,Cmd);
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    if(strstr(tmp, "Start"))
                        strcpy(Ini_Writer.SoundPlayStart.FileName, Cmd);
                    else if(strstr(tmp, "Pass"))
                        strcpy(Ini_Writer.SoundPlayPass.FileName, Cmd);
                    else if(strstr(tmp, "Fail"))
                        strcpy(Ini_Writer.SoundPlayFail.FileName, Cmd);
                    break;
                }
            } while (1);
        }         
        else if (strcmp(Cmd, "[Write Root]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    Ini_Writer.RootKey = atoi(Cmd);
                    if(Ini_Writer.RootKey >= 1 && Ini_Writer.RootKey <= 4)
                        sysprintf(" Root key index is %d\n",Ini_Writer.RootKey);
                    else if(Ini_Writer.RootKey != 0)
                        sysprintf(" Wrong Root key index %d\n",Ini_Writer.RootKey);      
                    break;
                }
            } while (1);
        }
        else if (strstr(Cmd, "[GPIO Flow Status - "))
        {
            strcpy(tmp,Cmd);
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          /* use default value since error code from NVTFAT. Coulde be end of file. */
                else if (Cmd[0] == 0)
                    continue;       /* skip empty line */
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       /* skip comment line */
                else if (Cmd[0] == '[')
                    goto NextMark2; /* use default value since no assign value before next keyword */
                else
                {
                    char* gpio;
                    int level;
                    int offset;
                    int pin;
                    char str [20];
                    gpio = strstr(Cmd,"GP");
                    if(gpio)
                    {
                        sscanf (Cmd,"GP%c%d, %d",str,&pin,&level);

                        if(pin >= 0 && pin <= 15 && level < 2)
                        {
                            if(gpio[2] >= 'A' && gpio[2] <= 'D')
                            {
                                offset = gpio[2] - 'A';
                                if(strstr(tmp,"Start"))
                                {
                                    u32GpioPin_Start = 1 << pin;
                                    u32GpioLevel_Start = level;
                                    u32GpioPort_Start = REG_GPIOA_DOUT + offset * 0x10;
                                    sysprintf(" [GPIO Flow Status - Start] - GP%c%d, Active Level %d\n",gpio[2],pin,level);
                                }
                                else if(strstr(tmp,"Pass"))
                                {
                                    u32GpioPin_Pass = 1 << pin;
                                    u32GpioLevel_Pass = level;
                                    u32GpioPort_Pass = REG_GPIOA_DOUT + offset * 0x10;
                                    sysprintf(" [GPIO Flow Status - Pass] - GP%c%d, Active Level %d\n",gpio[2],pin,level);
                                }
                                else if(strstr(tmp,"Fail"))
                                {
                                    u32GpioPin_Fail = 1 << pin;
                                    u32GpioLevel_Fail = level;
                                    u32GpioPort_Fail = REG_GPIOA_DOUT + offset * 0x10;
                                    sysprintf(" [GPIO Flow Status - Fail] - GP%c%d, Active Level %d\n",gpio[2],pin,level);
                                }
                                else
                                    break;

                                if(level)
                                    outp32(REG_GPIOA_DOUT + offset * 0x10, inp32(REG_GPIOA_DOUT + offset * 0x10) & ~(1 << pin));
                                else
                                    outp32(REG_GPIOA_DOUT + offset * 0x10, inp32(REG_GPIOA_DOUT + offset * 0x10) | (1 << pin));
                                outp32(REG_GPIOA_OMD + offset * 0x10, inp32(REG_GPIOA_OMD + offset * 0x10) | (1 << pin));
                                outp32(REG_GPAFUN + offset * 0x04, inp32(REG_GPAFUN+ offset * 0x04) & ~(0x03 << (2 * pin)));
                            }
                        }
                    }
                    break;
                }
            } while (1);
        }
    } while (status >= 0);   /* keep parsing INI file */

    fsCloseFile(FileHandle);
    /* show final configuration */
    dbgprintf(" SpiLoader = %s, address 0x%X\n",Ini_Writer.SpiLoader.FileName,Ini_Writer.SpiLoader.address);
    dbgprintf(" Logo = %s, start block 0x%X, address 0x%X\n",Ini_Writer.Logo.FileName,Ini_Writer.Logo.StartBlock, Ini_Writer.Logo.address);
    dbgprintf(" Execute = %s, start block 0x%X, address 0x%X\n",Ini_Writer.Execute.FileName, Ini_Writer.Execute.StartBlock, Ini_Writer.Execute.address);

    if(u32UserImageCount != 0)
    {
        sysprintf(" User Image Count is %d\n",u32UserImageCount);
        for(i=0;i<u32UserImageCount;i++)
            sysprintf("   User image name:%s, start block 0x%X\n",Ini_Writer.UserImage[i].FileName,Ini_Writer.UserImage[i].StartBlock);
    }
    if (strlen(Ini_Writer.SoundPlayStart.FileName) !=0)
        sysprintf("SoundPlayStart File name is %s\n",Ini_Writer.SoundPlayStart.FileName);
    if (strlen(Ini_Writer.SoundPlayFail.FileName) !=0)
        sysprintf("SoundPlayFail File name is %s\n",Ini_Writer.SoundPlayFail.FileName);
    if (strlen(Ini_Writer.SoundPlayPass.FileName) !=0)
        sysprintf("SoundPlayPass File name is %s\n",Ini_Writer.SoundPlayPass.FileName);

    if(u32ImageCount > 21)
        return Fail;

    return Successful;
}
