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

#define	INI_BUF_SIZE 	512
char iniBuf[INI_BUF_SIZE];
int  buffer_current = 0, buffer_end = 0;    // position to buffer iniBuf

INI_INFO_T Ini_Writer;


/*-----------------------------------------------------------------------------
 * To read one string line from file FileHandle and save it to Cmd.
 * Return:
 *      Successful  : OK
 *      < 0 : error code of NVTFAT, include ERR_FILE_EOF
 *---------------------------------------------------------------------------*/
static int readLine (int FileHandle, char *Cmd)
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
                return Successful;
            }
            else
            {
                Cmd[i_cmd] = 0;     // end of string to clear Cmd
                return ERR_FILE_EOF;
            }
        }
        else
        {
            status = fsReadFile(FileHandle, (UINT8 *)iniBuf, INI_BUF_SIZE, &nReadLen);
            if (status < 0)
            {
                Cmd[i_cmd] = 0;     // end of string to clear Cmd
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
    CHAR szNvtFullName[64], suNvtFullName[512], Cmd[256];
    int FileHandle, status;

    //--- initial default value
    Ini_Writer.SDLoader[0] = 0;
    Ini_Writer.Logo[0] = 0;
    Ini_Writer.NVTLoader[0] = 0;
    Ini_Writer.SystemReservedMegaByte = 4;
    Ini_Writer.SD1_1_SIZE = 16;
    Ini_Writer.SD1_1_FAT = 1;
    Ini_Writer.SD1_2_FAT = 1;
    Ini_Writer.Target_SD_Port = 1;

    //--- open INI file
    strcpy(szNvtFullName, fileName);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    FileHandle = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
    if (FileHandle < 0)
    {
        return FileHandle;  // open file error
    }

    buffer_current = 0;
    buffer_end = 0;         // position to buffer iniBuf

    //--- parse INI file
    do {
        status = readLine(FileHandle, Cmd);
        if (status < 0)     // read file error. Coulde be end of file.
            break;
NextMark2:
        if (strcmp(Cmd, "[SDLoader File Name]") == 0)
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
                    strcpy(Ini_Writer.SDLoader, Cmd);
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

        else if (strcmp(Cmd, "[SD1-1 DISK SIZE]") == 0)
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
                    Ini_Writer.SD1_1_SIZE = atoi(Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[SD1-1 FAT FILE]") == 0)
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
                    Ini_Writer.SD1_1_FAT = atoi(Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[SD1-2 FAT FILE]") == 0)
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
                    Ini_Writer.SD1_2_FAT = atoi(Cmd);
                    break;
                }
            } while (1);
        }

        else if (strcmp(Cmd, "[Target SD Port]") == 0)
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
                    Ini_Writer.Target_SD_Port = atoi(Cmd);
                    break;
                }
            } while (1);
        }
    } while (status >= 0);  // keep parsing INI file

    //--- show final configuration
	dbgprintf("Process SDWriter.ini file...\n");
	dbgprintf(" SDLoader = %s\n",Ini_Writer.SDLoader);
	dbgprintf(" Logo = %s\n",Ini_Writer.Logo);
	dbgprintf(" NvtLoader = %s\n",Ini_Writer.NVTLoader);
	dbgprintf(" SystemReservedMegaByte = %d\n",Ini_Writer.SystemReservedMegaByte);
	dbgprintf(" SD1_1_SIZE = %d\n",Ini_Writer.SD1_1_SIZE);
	dbgprintf(" SD1_1_FAT = %d\n",Ini_Writer.SD1_1_FAT);
	dbgprintf(" SD1_2_FAT = %d\n",Ini_Writer.SD1_2_FAT);
	dbgprintf(" Target_SD_Port = %d\n",Ini_Writer.Target_SD_Port);

    fsCloseFile(FileHandle);
    return Successful;
}
