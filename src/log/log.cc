/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "log.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <string>
#include <mutex>
using namespace std;

#include "log_text.h"
#include "main/analyzer.h"
#include "snort.h"

#define DEFAULT_DAEMON_ALERT_FILE  "alert"

/* Input is packet and an nine-byte (including NULL) character array.  Results
 * are put into the character array.
 */
void CreateTCPFlagString(Packet * p, char *flagBuffer)
{
    /* parse TCP flags */
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_RES1) ? '1' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_RES2) ? '2' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_URG)  ? 'U' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_ACK)  ? 'A' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_PUSH) ? 'P' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_RST)  ? 'R' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_SYN)  ? 'S' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_FIN)  ? 'F' : '*');
    *flagBuffer = '\0';

}

/****************************************************************************
 *
 * Function: OpenAlertFile(char *)
 *
 * Purpose: Set up the file pointer/file for alerting
 *
 * Arguments: filearg => the filename to open
 *
 * Returns: file handle
 *
 ***************************************************************************/
FILE *OpenAlertFile(const char *filearg)
{
    FILE *file;

    if ( !filearg )
        filearg = "alert.txt";

    std::string name;
    const char* filename = get_instance_file(name, filearg);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Opening alert file: %s\n", filename););

    if((file = fopen(filename, "a")) == NULL)
    {
        FatalError("OpenAlertFile() => fopen() alert file %s: %s\n",
                   filename, get_error(errno));
    }
    setvbuf(file, (char *) NULL, _IOLBF, (size_t) 0);

    return file;
}

/****************************************************************************
 *
 * Function: RollAlertFile(char *)
 *
 * Purpose: rename existing alert file with by appending time to name
 *
 * Arguments: filearg => the filename to rename (same as for OpenAlertFile())
 *
 * Returns: 0=success, else errno
 *
 ***************************************************************************/
int RollAlertFile(const char *filearg)
{
    char newname[STD_BUF+1];
    time_t now = time(NULL);

    if ( !filearg )
        filearg = "alert.txt";

    std::string name;
    get_instance_file(name, filearg);
    const char* oldname = name.c_str();

    SnortSnprintf(newname, sizeof(newname)-1, "%s.%lu", oldname, (unsigned long)now);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Rolling alert file: %s\n", newname););

    if ( rename(oldname, newname) )
    {
        FatalError("RollAlertFile() => rename(%s, %s) = %s\n",
                   oldname, newname, get_error(errno));
    }
    return errno;
}

//--------------------------------------------------------------------
// default logger stuff
//--------------------------------------------------------------------

static mutex log_mutex;

static TextLog* text_log = NULL;

void OpenLogger()
{
    text_log = TextLog_Init("stdout", 300*1024);
}

void CloseLogger()
{
    TextLog_Term(text_log);
}

void LogIPPkt(int type, Packet* p)
{
    log_mutex.lock();
    LogIPPkt(text_log, type, p);
    TextLog_Flush(text_log);
    log_mutex.unlock();
}

void snort_print(Packet* p)
{
    if (p->iph != NULL)
    {
        LogIPPkt(text_log, GET_IPH_PROTO((p)), p);
    }
#ifndef NO_NON_ETHER_DECODER
    else if (p->ah != NULL)
    {
        log_mutex.lock();
        LogArpHeader(text_log, p);
        TextLog_Flush(text_log);
        log_mutex.unlock();
    }
#if 0
    else if (p->eplh != NULL)
    {
        LogEapolPkt(text_log, p);
    }
    else if (p->wifih && ScOutputWifiMgmt())
    {
        LogWifiPkt(text_log, p);
    }
#endif
#endif
}

void LogNetData(const uint8_t* data, const int len, Packet* p)
{
    log_mutex.lock();
    LogNetData(text_log, data, len, p);
    TextLog_Flush(text_log);
    log_mutex.unlock();
}

