/* Modified 2026-11-16. */
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PUTCURS   move(4+locmem->crs_line-locmem->top_line,0);prints(">");
#define RMVCURS   move(4+locmem->crs_line-locmem->top_line,0);prints(" ");

struct keeploc {
	char           *key;
	int             top_line;
	int             crs_line;
	struct keeploc *next;
};

struct keeploc * getkeep(char * s, int def_topline, int def_cursline)
{
	static struct keeploc *keeplist = NULL;
	struct keeploc *p;

	for (p = keeplist; p != NULL; p = p->next) {
		if (!strcmp(s, p->key)) {
			if (p->crs_line < 1)
				p->crs_line = 1;	/* DAMMIT! - rrr */
			return p;
		}
	}
	p = (struct keeploc *) malloc(sizeof(*p));
	p->key = (char *) malloc(strlen(s) + 1);
	strcpy(p->key, s);
	p->top_line = def_topline;
	p->crs_line = def_cursline;
	p->next = keeplist;
	keeplist = p;
	return p;
}

int fixkeep(char * s, int first, int last)
{
	struct keeploc *k;
	k = getkeep(s, 1, 1);
	if (k->crs_line >= first) {
		k->crs_line = (first == 1 ? 1 : first - 1);
		k->top_line = (first < 11 ? 1 : first - 10);
	}
	return 0;
}

char            dirbuf[4096];
struct fileheader *files = NULL;

enum SEARCH_DIREC {
	SEARCH_NONE, SEARCH_FORWARD, SEARCH_BACKWARD
};

#define FIND_CANCELED 1004
#define FIND_FAIL 1318
#define FIND_SUCCESS 7941

char            rot[] = {'-', '\\', '|', '/'};

char * genSearchString(char * s)
{
	static char           ibuf[100];
	char *pc=ibuf;

	for (; *s; s++){
		if ( isalpha(*s)){
			*pc = toupper(*s);
			pc++;
		} else if ( *s != ' ' ){
			*pc = *s;
			pc++;
		}
	}
	*pc = '\0';
	return ibuf;
}

int i_read(char * dirent, bbs_action dotitle, bbs_entry_formatter doentry, struct one_key * rcmdlist)
{
	char            lbuf[11];
	int             lbc;
	struct keeploc *locmem;
	int             screen_len = SCREEN_SIZE - 4;
	int             num_entries;
	int             ch;
	int             i;
	int             last_line;
	int             mode = DONOTHING;
	int             ssize = sizeof(struct fileheader);
	static char     searchStr[100] = {'\0',};
	int             searchDirect = SEARCH_NONE;

	/*
	 * searchStr[0] = '\0';
	 */

	if (!files)
		files = (struct fileheader *) calloc(screen_len, sizeof(*files));
	strcpy(dirbuf, direct);
	move(0, 0);
	(*dotitle) ();
	clrtobot();
	last_line = get_num_records(dirbuf, ssize);
	if (last_line == 0) {
		prints("아무 내용도 없습니다.\n");
		pressreturn();
		move(0, 0);
		clrtoeol();
		clear();
		return 0;
	}
	locmem = getkeep(dirbuf, (last_line - screen_len + 1 < 1) ?
			 1 : last_line - screen_len + 1, last_line);
	if (locmem->top_line > last_line) {
		locmem->crs_line = last_line;
		locmem->top_line = last_line - 10;
		if (locmem->top_line < 1)
			locmem->top_line = 1;
	} else if (locmem->crs_line > last_line)
		locmem->crs_line = last_line;
	num_entries = get_records(dirbuf, files, ssize,
				  locmem->top_line, screen_len);
	move(4, 0);
	for (i = 0; i < num_entries; i++) {
		char           *t = (*doentry) (locmem->top_line + i, &files[i]);
		if (strlen(t) >= 79) {
			t[78] = '^';
			t[79] = '\0';
		}
		prints("%s\n", t);
	}
	PUTCURS;
	lbc = 0;
	while ((ch = igetch()) != EOF) {
		extern int      talkrequest;
NO_INPUT:
		if (talkrequest) {
			talkreply();
			mode = FULLUPDATE;
			goto endofloop;
		}
		if (isdigit(ch)) {
			if (lbc < 9)
				lbuf[lbc++] = ch;
			goto endofloop;
		}
		if (ch != '\n' && ch != '\r')
			lbc = 0;
		switch (ch) {
			int             val;
			int             cnt;
		case 'q':
		case 'e':
			mode = DOQUIT;
			break;
			/* by bugman : 글을 읽을 때 제목이나 올린이 찾기기능 */
			/*
			 * by bugman : /는 현재 위치에서 아래로, ?는
			 *
			 */
			/* by bugman : .은 마지막 검색을 반복한다. */
		case '/':
		case '?':
		case '.':
		case '>':
		case '<':
			switch (ch) {
			case '/':
				searchDirect = SEARCH_FORWARD;
				searchStr[0] = '\0';
				break;
			case '?':
				searchDirect = SEARCH_BACKWARD;
				searchStr[0] = '\0';
				break;
			case '>':
				searchDirect = SEARCH_FORWARD;
				break;
			case '<':
				searchDirect = SEARCH_BACKWARD;
				break;
			}
			if (searchDirect == SEARCH_NONE)
				break;
			else {
				char            buf[100];
				struct fileheader fh;
				int             finded = FIND_FAIL;
				int             startingPos = 0;
				int             endingPos = 0;
				int             inc = 1;
				fd_set		rfds;
				struct timeval tv;

				if (searchStr[0] == '\0') {
					getdata(2, 0, "찾을 단어 : ", buf, 60, DOECHO, NULL);
					if (buf[0] == '\0') {
						PUTCURS;
						break;
					}
					strcpy(searchStr, buf);
				} else {
					strcpy(buf, searchStr);
				}
				strcpy(buf, genSearchString(buf));
				move(2, 0);
				prints("  >> 찾는중 : %s >>", buf);
				refresh();

				switch (searchDirect) {
				case SEARCH_FORWARD:
					for (i = locmem->crs_line + 1, cnt = 0; i <= last_line && finded == FIND_FAIL; i++, cnt++) {
						char           *t;
						get_record(dirbuf, &fh, ssize, i);
						t = (*doentry) (i, &fh);
						if (cnt % 100 == 0 || 1) {
							move(2, 0);
							prints("%c", rot[(cnt / 200) % 4]);
							refresh();
						}
						if (strstr(genSearchString(t), buf) != NULL) {	/* FIND */
							finded = FIND_SUCCESS;
							sprintf(lbuf, "%d", i);
							lbc = strlen(lbuf);
						}

						FD_ZERO(&rfds);
						FD_SET(0, &rfds);
						tv.tv_sec = tv.tv_usec = 0;
						if (select(1, &rfds, NULL, NULL, &tv) > 0){
							if (FD_ISSET(0, &rfds)) {
								char c;
								read(0, &c, 1);
								if (c == 0x1B || c == 'Q' || c == 'q') finded = FIND_CANCELED;
							}
						}
					}
					break;
				case SEARCH_BACKWARD:
					for (i = locmem->crs_line - 1, cnt = 0; i >= 1 && finded == FIND_FAIL; i--, cnt++) {
						char           *t;
						get_record(dirbuf, &fh, ssize, i);
						t = (*doentry) (i, &fh);
						if (cnt % 100 == 0 || 1) {
							move(2, 0);
							prints("%c", rot[(cnt / 200) % 4]);
							refresh();
						}
						if (strstr(genSearchString(t), buf) != NULL) {	/* FIND */
							finded = FIND_SUCCESS;
							sprintf(lbuf, "%d", i);
							lbc = strlen(lbuf);
						}

						FD_ZERO(&rfds);
						FD_SET(0, &rfds);
						tv.tv_sec = tv.tv_usec = 0;
						if (select(1, &rfds, NULL, NULL, &tv) > 0){
							if (FD_ISSET(0, &rfds)) {
								char c;
								read(0, &c, 1);
								if (c == 0x1B || c == 'Q' || c == 'q' ) finded = FIND_CANCELED;
							}
						}
					}
					break;
				}

				if (finded == FIND_FAIL) {
					mode = PARTUPDATE;
					move(2, 0);
					prints("** 못 찾았습니다!!!!");
					clrtoeol();
					break;
				} else if (finded == FIND_CANCELED) {
					mode = PARTUPDATE;
					move(2, 0);
					prints("** 찾기를 중단했습니다.!!!!");
					clrtoeol();
					break;
				} else {
					move(2, 0);
					prints("** 찾았습니다!!!!");
					clrtoeol();
					/* continuted to next case '\n' statements */
				}
			}
		case '\n':
		case '\r':
			/*- edited by chopin
			  for unwilling reading by mytyping
			        	    if(lbc == 0) goto wjlee_read ;
			*/
			if (lbc == 0)
				break;
			lbuf[lbc] = '\0';
			val = atoi(lbuf);
			lbc = 0;
			if (val > last_line)
				val = last_line;
			if (val <= 0)
				val = 1;
			if (val >= locmem->top_line &&
			    val < locmem->top_line + screen_len) {
				RMVCURS;
				locmem->crs_line = val;
				PUTCURS;
				continue;
			}
			locmem->top_line = val - 10;
			if (locmem->top_line <= 0)
				locmem->top_line = 1;
			locmem->crs_line = val;
			mode = PARTUPDATE;
			goto endofloop;
		case 'p':
		case 'k':
			if (locmem->crs_line == locmem->top_line) {
				if (locmem->crs_line == 1) {
					bell();
					break;
				}
				locmem->top_line -= screen_len - 2;
				if (locmem->top_line <= 0)
					locmem->top_line = 1;
				locmem->crs_line--;
				mode = PARTUPDATE;
				break;
			}
			RMVCURS;
			locmem->crs_line--;
			PUTCURS;
			break;
		case CTRL('O'):
			do_fly_on_line(2, NA);
			mode = PARTUPDATE;
			break;
		case CTRL('L'):
			redoscr();
			break;
		case 'n':
		case 'j':
			if (locmem->crs_line == last_line) {
				bell();
				break;
			}
			if (locmem->crs_line + 1 == locmem->top_line + screen_len) {
				locmem->top_line += screen_len - 2;
				locmem->crs_line++;
				mode = PARTUPDATE;
				break;
			}
			RMVCURS;
			locmem->crs_line++;
			PUTCURS;
			break;
		case 'N':
		case 'J':
			if (locmem->top_line + screen_len - 1 >= last_line) {
				bell();
				break;
			}
			locmem->top_line += screen_len - 1;
			locmem->crs_line = locmem->top_line;
			mode = PARTUPDATE;
			break;
		case 'P':
		case 'K':
			if (locmem->top_line == 1) {
				bell();
				break;
			}
			locmem->top_line -= screen_len - 1;
			if (locmem->top_line <= 0)
				locmem->top_line = 1;
			locmem->crs_line = locmem->top_line;
			mode = PARTUPDATE;
			break;
		case '$':
			if (last_line < locmem->top_line + screen_len) {
				RMVCURS;
				locmem->crs_line = last_line;
				PUTCURS;
				break;
			}
			locmem->top_line = last_line - screen_len + 1;
			if (locmem->top_line <= 0)
				locmem->top_line = 1;
			locmem->crs_line = last_line;
			mode = PARTUPDATE;
			break;
		default:
			for (i = 0; rcmdlist[i].fptr != NULL; i++) {
				if (rcmdlist[i].key == ch) {
					mode = (*(rcmdlist[i].fptr)) (locmem->crs_line, &files[locmem->crs_line - locmem->top_line], dirbuf);
					break;
				}
			}
			if (rcmdlist[i].fptr == NULL)
				bell();
			/* chopin */
			mode = NEWDIRECT;
			break;
		}

endofloop:
		switch (mode) {
		case FULLUPDATE:
			clear();
		case NEWDIRECT:
			move(0, 0);
			(*dotitle) ();
			clrtobot();
		case PARTUPDATE:
			last_line = get_num_records(dirbuf, ssize);
			if (mode == NEWDIRECT)
				locmem = getkeep(dirbuf, (last_line - screen_len < 1) ?
				 1 : last_line - screen_len + 1, last_line);

			if (last_line == 0) {
				prints("글이 없습니다.\n");
				num_entries = 0;
			} else
				num_entries = get_records(dirbuf, files, ssize,
					      locmem->top_line, screen_len);
			if (locmem->crs_line > last_line)
				locmem->crs_line = last_line;
			move(4, 0);
			for (i = 0; i < num_entries; i++) {
				char           *t;
				t = (*doentry) (locmem->top_line + i, &files[i]);
				if (strlen(t) >= 79) {
					t[78] = '>';
					t[79] = '\0';
				}
				prints("%s\n", t);
			}
			clrtobot();
			PUTCURS;
			break;
		default:
			break;
		}
		if (mode == DOQUIT)
			break;
		mode = DONOTHING;
		if (num_entries == 0)
			break;
		/*
		 * 마지막 글이 아니고 위의 키가 눌리면 한칸
		 *
		 *
		 */
		if (locmem->crs_line != last_line)
			if (ch == 'v' || ch == 'V' || ch == ' ' || ch == 'r' || ch == 'm' || ch == CTRL('T')) {
				ch = 'j';
				goto NO_INPUT;
			}
	}
	clear();
	return 0;
}
