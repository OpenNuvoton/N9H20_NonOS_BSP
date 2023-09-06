/**************************************************************************//**
 * @file     processIni.c
 * @brief    NandWriter source code for NandWriter.ini file process
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
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

#define INI_BUF_SIZE    512
char iniBuf[INI_BUF_SIZE];
int  buffer_current = 0, buffer_end = 0;    // position to buffer iniBuf

INI_INFO_T Ini_Writer;

extern char *rtrim(char *str);

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
        //--- parse INI file in buffer iniBuf[] that read in at previous fsReadFile().
        while(buffer_current < buffer_end)
        {
            if (iniBuf[buffer_current] == 0x0D)
            {
                // DOS   use 0x0D 0x0A as end of line;
                // Linux use 0x0A as end of line;
                // To support both DOS and Linux, we treat 0x0A as real end of line and ignore 0x0D.
                buffer_current++;
                continue;
            }
            else if (iniBuf[buffer_current] == 0x0A)   // Found end of line
            {
                Cmd[i_cmd] = 0;     // end of string
                buffer_current++;
                rtrim(Cmd);
                return Successful;
            }
            else
            {
                Cmd[i_cmd] = iniBuf[buffer_current];
                buffer_current++;
                i_cmd++;
            }
        }

        //--- buffer iniBuf[] is empty here. Try to read more data from file to buffer iniBuf[].

        // no more data to read since previous fsReadFile() cannot read buffer full
        if ((buffer_end < INI_BUF_SIZE) && (buffer_end > 0))
        {
            if (i_cmd > 0)
            {
                // return last line of INI file that without end of line
                Cmd[i_cmd] = 0;     // end of string
                rtrim(Cmd);
                return Successful;
            }
            else
            {
                Cmd[i_cmd] = 0;     // end of string to clear Cmd
                rtrim(Cmd);
                return ERR_FILE_EOF;
            }
        }
        else
        {
            status = fsReadFile(FileHandle, (UINT8 *)iniBuf, INI_BUF_SIZE, &nReadLen);
            if (status < 0)
            {
                Cmd[i_cmd] = 0;     // end of string to clear Cmd
                rtrim(Cmd);
                return status;      // error or end of file
            }
            buffer_current = 0;
            buffer_end = nReadLen;
        }
    }   // end of while (1)
}


/*-----------------------------------------------------------------------------
 * To parse INI file and store configuration to global variable Ini_Writer.
 * Return:
 *      Successful  : OK
 *      < 0 : error code of NVTFAT
 *---------------------------------------------------------------------------*/
int ProcessINI(char *fileName)
{
    CHAR szNvtFullName[512], suNvtFullName[512], Cmd[256];
    int FileHandle, status;

    //--- initial default value
    Ini_Writer.NandLoader[0] = 0;
    Ini_Writer.Logo[0] = 0;
    Ini_Writer.NVTLoader[0] = 0;
    Ini_Writer.SystemReservedMegaByte = 4;
    Ini_Writer.NAND1_1_SIZE = 16;
    Ini_Writer.NAND1_1_FAT = FAT_MODE_FILE;
    Ini_Writer.NAND1_2_FAT = FAT_MODE_FILE;

    //--- open INI file
    strcpy(szNvtFullName, fileName);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    FileHandle = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
    if (FileHandle < 0)
    {
        return FileHandle;  // open file error
    }

    //--- parse INI file
    do {
        status = readLine(FileHandle, Cmd);
        if (status < 0)     // read file error. Coulde be end of file.
            break;
NextMark2:
        if (strcmp(Cmd, "[NandLoader File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    if (strlen(Cmd) > 511)
                    {
                        sysprintf("ERROR: NandLoader File Name too long !! It must < 512 !!\n");
                        return -1;
                    }
                    strcpy(Ini_Writer.NandLoader, Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[Logo File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    if (strlen(Cmd) > 511)
                    {
                        sysprintf("ERROR: Logo File Name too long !! It must < 512 !!\n");
                        return -1;
                    }
                    strcpy(Ini_Writer.Logo, Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[NVTLoader File Name]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    if (strlen(Cmd) > 511)
                    {
                        sysprintf("ERROR: NVTLoader File Name too long !! It must < 512 !!\n");
                        return -1;
                    }
                    strcpy(Ini_Writer.NVTLoader, Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[System Reserved MegaB]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    Ini_Writer.SystemReservedMegaByte = atoi(Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[NAND1-1 DISK SIZE]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    Ini_Writer.NAND1_1_SIZE = atoi(Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[NAND1-1 FAT FILE]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    Ini_Writer.NAND1_1_FAT = atoi(Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[NAND1-2 FAT FILE]") == 0)
        {
            do {
                status = readLine(FileHandle, Cmd);
                if (status < 0)
                    break;          // use default value since error code from NVTFAT. Coulde be end of file.
                else if (Cmd[0] == 0)
                    continue;       // skip empty line
                else if ((Cmd[0] == '/') && (Cmd[1] == '/'))
                    continue;       // skip comment line
                else if (Cmd[0] == '[')
                    goto NextMark2; // use default value since no assign value before next keyword
                else
                {
                    Ini_Writer.NAND1_2_FAT = atoi(Cmd);
                    break;
                }
            } while (1);
        }
    } while (status >= 0);  // keep parsing INI file

    //--- show final configuration
    dbgprintf("Process %s file ...\n", fileName);
    dbgprintf("  NnandLoader = %s\n",Ini_Writer.NandLoader);
    dbgprintf("  Logo = %s\n",Ini_Writer.Logo);
    dbgprintf("  NvtLoader = %s\n",Ini_Writer.NVTLoader);
    dbgprintf("  SystemReservedMegaByte = %d\n",Ini_Writer.SystemReservedMegaByte);
    dbgprintf("  NAND1_1_SIZE = %d\n",Ini_Writer.NAND1_1_SIZE);
    dbgprintf("  NAND1_1_FAT = %d\n",Ini_Writer.NAND1_1_FAT);
    dbgprintf("  NAND1_2_FAT = %d\n",Ini_Writer.NAND1_2_FAT);

    return Successful;
}
