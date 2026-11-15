/* Modified 2026-11-16. */
#include <stdlib.h>
#include <string.h>
/*-
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "chat.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>

#include "permissions.h"	/*- added by belami: PERM_SYSOP */

#define MAXPORTS 32
#define YEA       1
#define NA        0

int             chatroom;

#ifdef SYSV
#include <sys/ipc.h>
#include <sys/sem.h>

#define LOCK_EX         2	/*- exclusive lock */
#define LOCK_UN         8	/*- unlock */

#define SEM_KEY 9237
int             semid;

int get_semaphore(void)
{
	if ((semid = semget(SEM_KEY, 1, 600)) == -1) {
		if ((semid = semget(SEM_KEY, 1, 600 | IPC_CREAT)) == -1)
			return -1;
		semctl(semid, 0, SETVAL, 1);
	}
	return 0;
}

int flock(int fd, int op)
{
	/*- one semaphore for all shared files -> fd is unused */
	struct sembuf   sops;
	sops.sem_num = 0;
	sops.sem_flg = SEM_UNDO;
	switch (op) {
	case LOCK_EX:
		sops.sem_op = -1;
		break;
	case LOCK_UN:
		sops.sem_op = 1;
		break;
	default:
		return -1;
	}
	semop(semid, &sops, 1);
	return 0;
}

#endif

static int report(char * s)
{
	static int      disable = NA;
	int             fd;
	if (disable)
		return 0;
	if ((fd = open("trace.chatd", O_WRONLY, 0664)) != -1) {
		char            buf[160];
		flock(fd, LOCK_EX);
		lseek(fd, 0, L_XTND);
		sprintf(buf, "Room %d: %s\n", chatroom, s);
		write(fd, buf, strlen(buf));
		flock(fd, LOCK_UN);
		close(fd);
		return 0;
	}
	disable = YEA;
	return 0;
}

int main(int ac, char ** av)
{
	int             sock, msgsock, length, chatport;
	struct sockaddr_in server;
	int             numports;
	int             portfds[MAXPORTS];
	char            buf[80];

	numports = 0;
	if (ac != 2) {
		fprintf(stderr, "Invalid arguments\n");
		exit(-1);
	}
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	chatroom = atoi(av[1]);
	switch (chatroom) {
	case 4:
		chatport = CHATPORT4;
		break;
	case 1:
		chatport = CHATPORT1;
		break;
	case 2:
		chatport = CHATPORT2;
		break;
	case 3:
		chatport = CHATPORT3;
		break;
	}
#ifndef DEBUG
	if (fork())
		exit(0);
	{
		int             s, ndescriptors = getdtablesize();
		for (s = 0; s < ndescriptors; s++);
		(void) close(s);
	}
	(void) open("/", O_RDONLY);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
#ifdef SYSV
	setsid();
#else
	{
		/*- chopin
		    int             tt = open("/dev/tty", O_RDWR);
	        */
		int             tt = open("dev/tty", O_RDWR);
		if (tt > 0) {
			ioctl(tt, TIOCNOTTY, (char *) 0);
			(void) close(tt);
		}
	}
#endif				/*- SYSV */
#endif
	report("starting up");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		report("socket() failed: exiting");
		return -1;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = chatport;
	if (bind(sock, (struct sockaddr *) & server, sizeof server) < 0) {
		report("bind() failed: exiting");
		return -1;
	}
	length = sizeof server;
	if (getsockname(sock, (struct sockaddr *) & server, &length) < 0) {
		report("getsockname() failed: exiting");
		return -1;
	}
	listen(sock, 5);
	while (YEA) {
		fd_set          readfds;
		int             sr;
		int             i;

		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		for (i = 0; i < numports; i++)
			FD_SET(portfds[i], &readfds);
		if ((sr = select(getdtablesize(), &readfds, NULL, NULL, NULL)) < 0) {
			report("select() failed: exiting");
			exit(-1);
		}
		if (sr == 0)
			continue;
		if (FD_ISSET(sock, &readfds)) {
			int             s;

			s = accept(sock, (struct sockaddr *) 0, (int *) 0);
			if (s == -1) {
				report("accept() failed: continuing");
				continue;
			}
			portfds[numports] = s;
			numports++;
			sprintf(buf, "entry: numports is now %d", numports);
			report(buf);
			if (sr == 1)
				continue;
		}
		for (i = 0; i < numports; i++)
			if (FD_ISSET(portfds[i], &readfds)) {
				char            buf[80];
				int             cc;

				cc = read(portfds[i], buf, 80);
				if (cc == 0) {
					int             j;

					change_chatroom_title(chatroom, "");
					/*
					 * fprintf(stderr,"\r\n\n방 [%d]�
					 * \n",  chatroom); fflush(stderr);
					 */
					sleep(1);

					close(portfds[i]);
					for (j = i; j < numports - 1; j++)
						memmove(&portfds[j], &portfds[j + 1], sizeof(int));
					numports--;
					i--;
					sprintf(buf, "exit: numports is now %d", numports);
					report(buf);
					if (numports == 0) {
						report("normal termination");
						exit(0);	/*- last person closed connection */
					}
					continue;
				}
				while (YEA) {
					int             j;
					for (j = 0; j < numports; j++)
						write(portfds[j], buf, cc);
					if (buf[cc - 1] == '\0')
						break;	/*- all complete messages end in newline */
					cc = read(portfds[i], buf, 80);
					if (cc <= 0)
						break;
				}
			}
	}
	return 0;
}
