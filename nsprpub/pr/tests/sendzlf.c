/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape Portable Runtime (NSPR).
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * Test: sendzlf.c 
 *
 * Description: send a zero-length file with PR_SendFile and
 * PR_TransmitFile.
 */

#define ZERO_LEN_FILE_NAME "zerolen.tmp"
#define HEADER_STR "Header"
#define HEADER_LEN 6 /* length of HEADER_STR, not counting the null byte */
#define TRAILER_STR "Trailer"
#define TRAILER_LEN 7 /* length of TRAILER_STR, not counting the null byte */

#include "nspr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ClientThread(void *arg)
{
    PRFileDesc *sock;
    PRNetAddr addr;
    PRUint16 port = (PRUint16) arg;
    char buf[1024];
    char *bufPtr;
    PRInt32 nbytes;
    PRInt32 ntotal;
    PRInt32 nexpected;

    sock = PR_NewTCPSocket();
    if (NULL == sock) {
        fprintf(stderr, "PR_NewTCPSocket failed\n");
        exit(1);
    }
    if (PR_InitializeNetAddr(PR_IpAddrLoopback, port, &addr) == PR_FAILURE) {
        fprintf(stderr, "PR_InitializeNetAddr failed\n");
        exit(1);
    }
    if (PR_Connect(sock, &addr, PR_INTERVAL_NO_TIMEOUT) == PR_FAILURE) {
        fprintf(stderr, "PR_Connect failed\n");
        exit(1);
    }
    ntotal = 0;
    bufPtr = buf;
    while ((nbytes = PR_Read(sock, bufPtr, sizeof(buf)-ntotal)) > 0) {
        ntotal += nbytes;
        bufPtr += nbytes;
    }
    if (-1 == nbytes) {
        fprintf(stderr, "PR_Read failed\n");
        exit(1);
    }
    nexpected = HEADER_LEN+TRAILER_LEN+TRAILER_LEN+HEADER_LEN+HEADER_LEN;
    if (ntotal != nexpected) {
        fprintf(stderr, "total bytes read should be %d but is %d\n",
                nexpected, ntotal);
        exit(1);
    }
    if (memcmp(buf, HEADER_STR TRAILER_STR TRAILER_STR HEADER_STR HEADER_STR,
            nexpected) != 0) {
        fprintf(stderr, "wrong data is received\n");
        exit(1);
    }
    if (PR_Close(sock) == PR_FAILURE) {
        fprintf(stderr, "PR_Close failed\n");
        exit(1);
    }
}

static void ServerThread(void *arg)
{
    PRFileDesc *listenSock = (PRFileDesc *) arg;
    PRFileDesc *acceptSock;
    PRFileDesc *file;
    PRSendFileData sfd;
    char header[1024], trailer[1024];
    PRInt32 nbytes;

    /* Create a zero-length file */
    file = PR_Open(ZERO_LEN_FILE_NAME,
            PR_CREATE_FILE|PR_TRUNCATE|PR_RDWR, 0666);
    if (NULL == file) {
        fprintf(stderr, "PR_Open failed\n");
        exit(1);
    }
    sfd.fd = file;
    sfd.file_offset = 0;
    sfd.file_nbytes = 0;
    memcpy(header, HEADER_STR, HEADER_LEN);
    memcpy(trailer, TRAILER_STR, TRAILER_LEN);
    sfd.header = header;
    sfd.hlen = HEADER_LEN;
    sfd.trailer = trailer;
    sfd.tlen = TRAILER_LEN;
    acceptSock = PR_Accept(listenSock, NULL, PR_INTERVAL_NO_TIMEOUT);
    if (NULL == acceptSock) {
        fprintf(stderr, "PR_Accept failed\n");
        exit(1);
    }
    /* Send both header and trailer */
    nbytes = PR_SendFile(acceptSock, &sfd, PR_TRANSMITFILE_KEEP_OPEN,
            PR_INTERVAL_NO_TIMEOUT);
    if (HEADER_LEN+TRAILER_LEN != nbytes) {
        fprintf(stderr, "PR_SendFile should return %d but returned %d\n",
                HEADER_LEN+TRAILER_LEN, nbytes);
        exit(1);
    }
    /* Trailer only, no header */
    sfd.hlen = 0;
    nbytes = PR_SendFile(acceptSock, &sfd, PR_TRANSMITFILE_KEEP_OPEN,
            PR_INTERVAL_NO_TIMEOUT);
    if (TRAILER_LEN != nbytes) {
        fprintf(stderr, "PR_SendFile should return %d but returned %d\n",
                TRAILER_LEN, nbytes);
        exit(1);
    }
    /* Header only, no trailer */
    sfd.hlen = HEADER_LEN;
    sfd.tlen = 0;
    nbytes = PR_SendFile(acceptSock, &sfd, PR_TRANSMITFILE_KEEP_OPEN,
            PR_INTERVAL_NO_TIMEOUT);
    if (HEADER_LEN != nbytes) {
        fprintf(stderr, "PR_SendFile should return %d but returned %d\n",
                HEADER_LEN, nbytes);
        exit(1);
    }
    /* Try PR_TransmitFile */
    nbytes = PR_TransmitFile(acceptSock, file, header, HEADER_LEN,
            PR_TRANSMITFILE_KEEP_OPEN, PR_INTERVAL_NO_TIMEOUT);
    if (HEADER_LEN != nbytes) {
        fprintf(stderr, "PR_TransmitFile should return %d but returned %d\n",
                HEADER_LEN, nbytes);
        exit(1);
    }
    if (PR_Close(acceptSock) == PR_FAILURE) {
        fprintf(stderr, "PR_Close failed\n");
        exit(1);
    }
    if (PR_Close(file) == PR_FAILURE) {
        fprintf(stderr, "PR_Close failed\n");
        exit(1);
    }
    if (PR_Delete(ZERO_LEN_FILE_NAME) == PR_FAILURE) {
        fprintf(stderr, "PR_Delete failed\n");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    PRFileDesc *listenSock;
    PRThread *clientThread;
    PRThread *serverThread;
    PRNetAddr addr;
    PRThreadScope scope = PR_GLOBAL_THREAD;

    listenSock = PR_NewTCPSocket();
    if (NULL == listenSock) {
        fprintf(stderr, "PR_NewTCPSocket failed\n");
        exit(1);
    }
    if (PR_InitializeNetAddr(PR_IpAddrAny, 0, &addr) == PR_FAILURE) {
        fprintf(stderr, "PR_InitializeNetAddr failed\n");
        exit(1);
    }
    if (PR_Bind(listenSock, &addr) == PR_FAILURE) {
        fprintf(stderr, "PR_Bind failed\n");
        exit(1);
    }
    /* Find out what port number we are bound to. */
    if (PR_GetSockName(listenSock, &addr) == PR_FAILURE) {
        fprintf(stderr, "PR_GetSockName failed\n");
        exit(1);
    }
    if (PR_Listen(listenSock, 5) == PR_FAILURE) {
        fprintf(stderr, "PR_Listen failed\n");
        exit(1);
    }

    clientThread = PR_CreateThread(PR_USER_THREAD,
            ClientThread, (void *) PR_ntohs(PR_NetAddrInetPort(&addr)),
            PR_PRIORITY_NORMAL, scope, PR_JOINABLE_THREAD, 0);
    if (NULL == clientThread) {
        fprintf(stderr, "PR_CreateThread failed\n");
        exit(1);
    }
    serverThread = PR_CreateThread(PR_USER_THREAD,
            ServerThread, listenSock,
            PR_PRIORITY_NORMAL, scope, PR_JOINABLE_THREAD, 0);
    if (NULL == serverThread) {
        fprintf(stderr, "PR_CreateThread failed\n");
        exit(1);
    }
    if (PR_JoinThread(clientThread) == PR_FAILURE) {
        fprintf(stderr, "PR_JoinThread failed\n");
        exit(1);
    }
    if (PR_JoinThread(serverThread) == PR_FAILURE) {
        fprintf(stderr, "PR_JoinThread failed\n");
        exit(1);
    }
    if (PR_Close(listenSock) == PR_FAILURE) {
        fprintf(stderr, "PR_Close failed\n");
        exit(1);
    }
    printf("PASS\n");
    return 0;
}
