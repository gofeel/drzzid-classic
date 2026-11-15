/* Modified 2026-11-16. */
#include <termcap.h>
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

#include "bbs.h"
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#define OBUFSIZE  (4096)
#define IBUFSIZE  (256)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

extern int      dumb_term;

char            outbuf[OBUFSIZE];
int             obufsize = 0;

char            inbuf[IBUFSIZE];
int             ibufsize = 0;
int             icurrchar = 0;

static int      i_mode = INPUT_ACTIVE;

void hit_alarm_clock(int dummy)
{
#ifdef PERMS
	if (HAS_PERM(PERM_NOTIMEOUT))
#else
	if (currentuser.userlevel == 255)
#endif
		return;
	if (i_mode == INPUT_IDLE) {
		clear();
		fprintf(stderr, "%d분 동안 아무것도 누르지 않으면 끝나요. 안녕히 !!\n", IDLE_TIMEOUT / 60);
		kill(getpid(), SIGHUP);
	}
	i_mode = INPUT_IDLE;
	signal(SIGALRM, hit_alarm_clock);
	alarm(IDLE_TIMEOUT);
}

int init_alarm(void)
{
	signal(SIGALRM, hit_alarm_clock);
	alarm(IDLE_TIMEOUT);
	return 0;
}

int oflush(void)
{
	if (obufsize)
		write(1, outbuf, obufsize);
	obufsize = 0;
	return 0;
}

int output(char * s, int len)
{
	/*- Invalid if len >= OBUFSIZE */

	if (obufsize + len > OBUFSIZE) {	/*- doin a oflush */
		write(1, outbuf, obufsize);
		obufsize = 0;
	}
	memmove(outbuf + obufsize, s, len);
	obufsize += len;
	return 0;
}

int ochar(int c)
{
	if (obufsize > OBUFSIZE - 1) {	/*- doin a oflush */
		write(1, outbuf, obufsize);
		obufsize = 0;
	}
	outbuf[obufsize++] = c;
	return 0;
}

int             i_newfd = 0;
struct timeval  i_to, i_totmp, *i_top = NULL;
bbs_action flushf = NULL;

int add_io(int fd, int timeout)
{
	i_newfd = fd;
	if (timeout) {
		i_to.tv_sec = timeout;
		i_to.tv_usec = 0;
		i_totmp = i_to;
		i_top = &i_totmp;
	} else
		i_top = NULL;
	return 0;
}

int add_flush(bbs_action flushfunc)
{
	flushf = flushfunc;
	return 0;
}

int num_in_buf(void)
{
	return icurrchar - ibufsize;
}

/*
 * void talkChk(){ extern int talkrequest; extern int chatrequest; if
 * (talkrequest == YEA){ talkrequest = NA; print_talk_request(); } if
 * (chatrequest == YEA){ chatrequest = NA; print_chat_request(); } }
 */

int igetch(void)
{
	igetagain:
	if (ibufsize == icurrchar) {
		fd_set          readfds;
		struct timeval  to;
		int             sr;

		to.tv_sec = 0;
		to.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		if (i_newfd)
			FD_SET(i_newfd, &readfds);
		if ((sr = select(getdtablesize(), &readfds, NULL, NULL, &to)) <= 0) {
			if (flushf)
				(*flushf) ();
			if (dumb_term)
				oflush();
			else
				refresh();
			FD_ZERO(&readfds);
			FD_SET(0, &readfds);
			if (i_newfd)
				FD_SET(i_newfd, &readfds);
			if (i_top) i_totmp = i_to;
			while ((sr = select(getdtablesize(), &readfds,
					    NULL, NULL, i_top)) < 0) {
				/*
				 * talkChk();
				 */
				if (errno == EINTR)
					continue;
				else {
					perror("select");
					fprintf(stderr, "abnormal select conditions\n");
					return -1;
				}
				FD_ZERO(&readfds);
				FD_SET(0, &readfds);
				if (i_newfd)
					FD_SET(i_newfd, &readfds);
				if (i_top) i_totmp = i_to;
			}
			if (sr == 0)
				return I_TIMEOUT;
		}
		if (i_newfd && FD_ISSET(i_newfd, &readfds))
			return I_OTHERDATA;
		while ((ibufsize = read(0, inbuf, IBUFSIZE)) <= 0) {
			/*
			 * talkChk();
			 */
			if (ibufsize == 0)
				longjmp(byebye, -1);
			if (ibufsize < 0 && errno != EINTR)
				longjmp(byebye, -1);

		}
		icurrchar = 0;
	}
	i_mode = INPUT_ACTIVE;
	/*
	 * talkChk();
	 */
	switch (inbuf[icurrchar]) {
	case CTRL('L'):
		redoscr();
		icurrchar++;
		goto igetagain;
	default:
		break;
	}
	return inbuf[icurrchar++];
}
/*
 * chopin getdata(line, col, prompt, buf, len, echo, complete) int
 * line, col; char           *prompt, *buf; int             len, echo; int
 * (*complete) (); { int             ch; int             clen = 0; int
 * x, y; extern unsigned char scr_cols;
 *
 * if(echo==NOECHO){ getdataxx(line, col, prompt, buf, len, echo, complete);
 * return; }
 *
 * if (prompt!=NULL) { move(line, col); prints("%s", prompt); refresh(); }
 * editline(line,col,buf,len); prints("\n"); }
 */
int getdata(int line, int col, char *prompt, char *buf, int len, int echo, bbs_action complete)
{
	int             ch;
	int             clen = 0;
	int             x, y;
	extern unsigned char scr_cols;

	if (prompt) {
		move(line, col);
		prints("%s", prompt);
	}
	if (dumb_term) {
		while ((ch = igetch()) != '\r') {
			if (ch == '\n')
				break;
			if (ch == '\177' || ch == CTRL('H')) {

				/*
				 * by bugman if (clen == 0) { bell();
				 * continue; }
				 */
				if (clen <= 0) {
					bell();
					clen = 0;
					continue;
				}
				clen--;
				if (echo) {
					int             k, j;
					for (k = clen, j = 0; k >= 0; k--, j = !(j))
						if ((unsigned) buf[k] < (unsigned) 128)
							break;
					/*
					 * by bugman if (j == 0 ) {
					 */
					if (j == 0 && clen > 1) {	/*- 한글시작이면 두바이트를 지움*/
						ochar(CTRL('H'));
						ochar(' ');
						ochar(CTRL('H'));
						clen--;
					}
					ochar(CTRL('H'));
					ochar(' ');
					ochar(CTRL('H'));
				}
				continue;
			}
#ifdef BIT8
			if (!isprint2(ch)) {
#else
			if (!isprint(ch)) {
#endif
				if (echo)
					bell();
				continue;
			}
			if (clen >= len - 1) {
				if (echo)
					bell();
				continue;
			}
			buf[clen++] = ch;
			if (echo)
				ochar(ch);
		}
		buf[clen] = '\0';
		prints("\n");
		oflush();
		return clen;
	}
	clrtoeol();
	getyx(&y, &x);
	while ((ch = igetch()) != '\r') {
		if (ch == '\n')
			break;
		if (ch == '\177' || ch == CTRL('H')) {
			if (clen <= 0) {
				bell();
				continue;
			}
			clen--;
			if (echo) {
				move(y, x + clen);
				prints(" ");
				move(y, x + clen);
			}
			continue;
		}
#ifdef BIT8
		if (!isprint2(ch)) {
#else
		if (!isprint(ch)) {
#endif
			bell();
			continue;
		}
		if (x + clen >= scr_cols || clen >= len - 1) {
			bell();
			continue;
		}
		buf[clen++] = ch;
		if (echo)
			prints("%c", ch);
	}
	buf[clen] = '\0';
	if (echo)
		prints("\n");
	refresh();
	return clen;
}
/*- returnless new version of getdata() by chopin*/
int getone(int line, int col, char * prompt, char * buf, int len, int echo, bbs_action complete)
{
	if (prompt) {
		move(line, col);
		prints("%s", prompt);
	}
	buf[0] = igetch();
	buf[1] = 0;
	if (echo)
		ochar(buf[0]);
	return 0;
}
