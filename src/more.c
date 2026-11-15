/* Modified 2026-11-16. */
/*
 * Pirate Bulletin Board System Copyright (C) 1990, Edward Luke,
 * lush@Athena.EE.MsState.EDU Eagles Bulletin Board System Copyright (C)
 * 1992, Raymond Rocker, rocker@rock.b11.ingr.com Guy Vega,
 * gtvega@seabass.st.usm.edu Dominic Tynes, dbtynes@seabass.st.usm.edu
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 1, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA.
 */

#include "bbs.h"
#include "more.h"
#include <sys/types.h>
#ifndef XINU
#include <sys/stat.h>
#endif

int readln2(FILE *fp, char *buf, int len)
{
	int             i;
	int             ch;
	int             bytes;
	i = 0;
	bytes = 0;
	buf[0] = '\0';		/* by bugman */
	while ((ch = getc(fp)) != EOF) {
		bytes++;
		if (ch == '\n') {
			buf[i] = '\0';
			return bytes;
		}
#ifdef BIT8
		if (isprint2(ch))
#else
		if (isprint(ch))
#endif

			buf[i++] = ch;
		if (i == len) {
			buf[i] = '\0';
			return bytes;
		}
	}
	if (i == 0)
		return 0;
	else {
		buf[i] = '\0';
		return bytes;
	}
}

int readln(FILE * fp, char * buf)
{
	return readln2(fp, buf, 80);
}
#define BACK_BYTE 1500		/* 뒤로 가는 바이트수 */
int more(char * filename, int promptend, int type)
{
	FILE           *fp;
	char            ch;
	int             i;
	int             viewed, pos;
	long            tsize;
	struct stat     st;
	int             numbytes;
	char            buf[81];
	extern int      t_lines;

	int             byte_count, page_byte;	/* ㅤchopin */
	int             line;

	if ((fp = fopen(filename, "r")) == NULL) {
		if (HAS_PERM(PERM_SYSOP))
			perror(filename);
		return -1;
	}
	if (fstat(fileno(fp), &st)) {
		if (HAS_PERM(PERM_SYSOP))
			perror("fstat");
		return -1;
	}
	tsize = st.st_size;
	clear();
	i = 0;
	pos = 0;
	viewed = 0;
	if (type == MORE_IS_LOGIN) {
		if (currentuser.lasthost[0] != '\0') {
			prints("마지막으로 들어온 때는 %s에서 %s", currentuser.lasthost,
			       time_string(currentuser.lastlogin));
			pos++;
		}
#ifdef VOTE
		if (HAS_PERM(PERM_VOTE) && !stat("vote/control", &st) &&
		    !(int) (currentuser.flags[0] & VOTE_FLAG)) {
			prints("당신은 이번 투표에 참여하지 않았습니다.\n");
			pos++;
		}
#endif
	}
	numbytes = readln(fp, buf);
	page_byte = byte_count = numbytes;
	line = 1;
	do {
		if (!numbytes)
			break;
		if (viewed >= 0)
			prints("%s\n", buf);
		viewed += numbytes;
		pos++;
		i++;
		if (pos == t_lines) {
			scroll();
			pos--;
		}
		numbytes = readln(fp, buf);
		if (!numbytes)
			break;
		byte_count += numbytes;
		page_byte = byte_count;
		if (line < 1) {
			line = 1;
			page_byte = byte_count = numbytes;
		}
		if (line > 24) {
			line = 24;
			page_byte = byte_count = numbytes;
		}
		/*
		 *
		 * if(line>t_lines){line=t_lines;page_byte=byte_count=numbytes;
		 * }
		 */
		if (i == t_lines - 1) {
			move(t_lines - 1, 0);
			if (!dumb_term) {
				standout();
				prints("--다음쪽(Space), 한줄아래(Return,j), 위쪽(b), 끝(q)-- (%d%%)", (viewed * 100) / tsize);
				standend();

			} else
				bell();
			while ((ch = igetch()) != EOF) {
				if (ch == ' ') {
					move(t_lines - 1, 0);
					clrtoeol();
					refresh();
					i = 1;
					line += 24;
					break;
				}
				/*
				 * 실패 if (ch == 'k') { int p,b,count; char
				 * temp[100];
				 *
				 * viewed+=numbytes; if(viewed<(page_byte+80))
				 * b=viewed; else b=(page_byte+80);
				 * fseek(fp,-b,1);
				 * fgets(temp,b-(page_byte+80),fp);
				 * for(p=strlen(temp)-1,count=0;p>=0;p--,count
				 * ++) if(temp[p]=='\n' || temp[i]=='\r')
				 * break; fseek(fp,-count,1);
				 * viewed=viewed-count-numbytes;
				 * move(t_lines-1,0); clrtoeol(); refresh();
				 * i=-2; line--; byte_count-=count; break;
				 *
				 * } */
				if (ch == 'b') {	/* chopin */
					int             back;
					viewed += numbytes;
					if (viewed >= page_byte * 2)
						back = page_byte * 2;
					else
						back = viewed;
					fseek(fp, -back, 1);
					viewed = viewed - back - numbytes;
					move(t_lines - 1, 0);
					clrtoeol();
					refresh();
					i = -1;
					line -= 24;
					byte_count -= back;
					break;
				}
				if (ch == 'q') {
					move(t_lines - 1, 0);
					clrtoeol();
					refresh();
					fclose(fp);
					return 0;
				}
				if (ch == '\r' || ch == 'j') {
					move(t_lines - 1, 0);
					clrtoeol();
					refresh();
					i = t_lines - 2;
					line++;
					break;
				}
				if (ch == CTRL('O')){

					do_fly_on_line(t_lines -1, NA );
				}
				bell();
			}
		}
	} while (numbytes);
	fclose(fp);
	move(t_lines - 1, 0);
	if (promptend)
		pressanykey();
	/*- skji
		pressreturn();
	*/
	return 0;
}

int lmore(char * filename, int promptend, int type)
{
	FILE           *fp;
	struct stat     st;
	int             numbytes;
	char            buf[162];

	reset_tty();
	if ((fp = fopen(filename, "r")) == NULL) {
		if (HAS_PERM(PERM_SYSOP))
			perror(filename);
		restore_tty();
		return -1;
	}
	if (fstat(fileno(fp), &st)) {
		if (HAS_PERM(PERM_SYSOP))
			perror("fstat");
		restore_tty();
		return -1;
	}
	if (type == MORE_IS_LOGIN) {
		if (currentuser.lasthost[0] != '\0') {
			printf("마지막으로 들어온 때는 %s에서 %s\n", currentuser.lasthost,
			time_string_short_no_newline(currentuser.lastlogin));
		}
#ifdef VOTE
		if (HAS_PERM(PERM_VOTE) && !stat("vote/control", &st) &&
		    !(int) (currentuser.flags[0] & VOTE_FLAG)) {
			printf("당신은 이번 투표에 참여하지 않았습니다.\n");
		}
#endif
	}
	do {
		numbytes = readln2(fp, buf, 160);
		if (!numbytes)
			break;
		if (!numbytes)
			break;
		printf("%s\n", buf);
	} while (numbytes);
	fclose(fp);
	restore_tty();
	move(t_lines - 1, 0);
	if (promptend) {
		pressreturn();
	}
	return 0;
}

void cat(char * filename)
{

	FILE           *fp;
	struct stat     st;
	int             numbytes;
	char            buf[162];

	if ((fp = fopen(filename, "r")) == NULL) {
		if (HAS_PERM(PERM_SYSOP))
			perror(filename);
		return;
	}
	if (fstat(fileno(fp), &st)) {
		if (HAS_PERM(PERM_SYSOP))
			perror("fstat");
		return;
	}
	do {
		numbytes = readln2(fp, buf, 160);
		if (!numbytes)
			break;
		printf("%s\r\n", buf);
	} while (numbytes);
	fclose(fp);
}
