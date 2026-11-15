/* Modified 2026-11-16. */
/*-    Pirate Bulletin Board System
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
#include "edit.h"
#include <time.h>

struct textline *firstline = NULL;
struct textline *lastline = NULL;

struct textline *currline = NULL;
int             currpnt = 0;

struct textline *top_of_win = NULL;
int             curr_window_line;
int             redraw_everything;

int IAmEditing=0;

void indigestion(int i)
{
	fprintf(stderr, "SERIOUS INTERNAL INDIGESTION CLASS %d\n", i);
}

struct textline * back_line(struct textline * pos, int num)
{
	while (num-- > 0)
		if (pos && pos->prev)
			pos = pos->prev;
	return pos;
}

struct textline * forward_line(struct textline * pos, int num)
{
	while (num-- > 0)
		if (pos && pos->next)
			pos = pos->next;
	return pos;
}

int getlineno(void)
{
	int             cnt = 0;
	struct textline *p = currline;

	while (p != top_of_win) {
		if (p == NULL)
			break;
		cnt++;
		p = p->prev;
	}
	return cnt;
}
char * killsp(char * s)
{
	while (*s == ' ')
		s++;
	return s;
}

struct textline * alloc_line(void)
{

	struct textline *p;

	p = (struct textline *) malloc(sizeof(*p));
	if (p == NULL) {
		indigestion(13);
		abort_bbs(0);
	}
	p->next = NULL;
	p->prev = NULL;
	p->data[0] = '\0';
	p->len = 0;
	return p;
}

/*-
  Appends p after line in list.  keeps up with last line as well.
 */

void append(struct textline * p, struct textline * line)
{
	p->next = line->next;
	if (line->next)
		line->next->prev = p;
	else
		lastline = p;
	line->next = p;
	p->prev = line;
}

/*-
  delete_line deletes 'line' from the list and maintains the lastline, and
  firstline pointers.
 */

void delete_line(struct textline * line)
{
	if (!line->next && !line->prev) {
		line->data[0] = '\0';
		line->len = 0;
		return;
	}
	if (line->next)
		line->next->prev = line->prev;
	else
		lastline = line->prev;
	if (line->prev)
		line->prev->next = line->next;
	else
		firstline = line->next;
	free(line);
}

/*-
  split splits 'line' right before the character pos
 */

void split(struct textline * line, int pos)
{
	struct textline *p = alloc_line();

	if (pos > line->len) {
		free(p);
		return;
	}
	p->len = line->len - pos;
	line->len = pos;
	strcpy(p->data, (line->data + pos));
	*(line->data + pos) = '\0';
	append(p, line);
	if (line == currline && pos <= currpnt) {
		currline = p;
		currpnt -= pos;
		curr_window_line++;
	}
	redraw_everything = YEA;
}

/*-
  join connects 'line' and the next line.  It returns true if:

  1) lines were joined and one was deleted
  2) lines could not be joined
  3) next line is empty

  returns false if:

  1) Some of the joined line wrapped
 */

int join(struct textline *line)
{
	register int    ovfl;

	if (!line->next)
		return YEA;
	if (*killsp(line->next->data) == '\0')
		return YEA;
	ovfl = line->len + line->next->len - WRAPMARGIN;
	if (ovfl < 0) {
		strcat(line->data, line->next->data);
		line->len += line->next->len;
		delete_line(line->next);
		return YEA;
	} else {
		register char  *s;
		register struct textline *p = line->next;

		s = p->data + p->len - ovfl - 1;
		while (s != p->data && *s == ' ')
			s--;
		while (s != p->data && *s != ' ')
			s--;
		if (s == p->data)
			return YEA;
		split(p, (s - p->data) + 1);
		if (line->len + p->len >= WRAPMARGIN) {
			indigestion(0);
			return YEA;
		}
		join(line);
		p = line->next;
		if (p->len >= 1 && p->len + 1 < WRAPMARGIN) {
			if (p->data[p->len - 1] != ' ') {
				strcat(p->data, " ");
				p->len++;
			}
		}
		return NA;
	}
}

void insert_char(int ch)
{
	register int    i;
	register char  *s;
	register struct textline *p = currline;
	int             wordwrap = YEA;

	if (currpnt > p->len) {
		indigestion(1);
		return;
	}
	for (i = p->len; i >= currpnt; i--)
		p->data[i + 1] = p->data[i];
	p->data[currpnt] = ch;
	p->len++;
	currpnt++;
	if (p->len < WRAPMARGIN)
		return;
	s = p->data + (p->len - 1);
	while (s != p->data && *s == ' ')
		s--;
	while (s != p->data && *s != ' ')
		s--;
	if (s == p->data) {
		wordwrap = NA;
		s = p->data + (p->len - 2);
	}
	split(p, (s - p->data) + 1);
	p = p->next;
	if (wordwrap && p->len >= 1) {
		i = p->len;
		if (p->data[i - 1] != ' ') {
			p->data[i] = ' ';
			p->data[i + 1] = '\0';
			p->len++;
		}
	}
	while (!join(p)) {
		p = p->next;
		if (p == NULL) {
			indigestion(2);
			break;
		}
	}
}

void delete_char(void)
{
	register int    i;

	if (currline->len == 0)
		return;
	if (currpnt >= currline->len) {
		indigestion(1);
		return;
	}
#ifdef BIT8
	if (currline->data[currpnt] & 0x80) {
		for (i = currpnt; i && (currline->data[i] & 0x80); i--);
		if (!(currline->data[i] & 0x80))
			i++;
		for (; i + 1 < currpnt; i++, i++);
		currpnt = i;
		for (; i + 1 < currline->len; i++)
			currline->data[i] = currline->data[i + 2];
		currline->len--;
		currline->len--;
	} else
#endif
	{
		for (i = currpnt; i != currline->len; i++)
			currline->data[i] = currline->data[i + 1];
		currline->len--;
	}
}

void vedit_init(void)
{
	struct textline *p = alloc_line();

	firstline = p;
	lastline = p;
	currline = p;
	currpnt = 0;
	top_of_win = p;
	curr_window_line = 0;
	redraw_everything = NA;
}

void read_file(char * filename)
{
	FILE           *fp;
	int             ch;

	if (currline == NULL)
		vedit_init();
	if ((fp = fopen(filename, "r+")) == NULL) {
		if ((fp = fopen(filename, "w+")) != NULL) {
			fclose(fp);
			return;
		}
		indigestion(4);

		prints("\n\n\n");
		prints("파일생성에러\n");
		prints("파일(%s)을 만드는데 실패했습니다.\n", filename);
		prints("비비에스를 종료합니다.\n");
		pressanykey();

		IAmEditing = NA;
		abort_bbs(0);
	}
	while ((ch = getc(fp)) != EOF)
#ifdef BIT8
		if (isprint2(ch))
#else
		if (isprint(ch))
#endif
			insert_char(ch);
		else if (ch == '\n')
			split(currline, currpnt);
	fclose(fp);
}

char            save_title[STRLEN];
char            save_filename[4096];
char            Cc_list[STRLEN];
int             in_mail;

void write_header(FILE * fp, int type)
{
	char            uid[20];
	char            uname[40];
	extern long     ti;
	char            tempbuf[100];

	if (type == TEXT_IS_EDIT) return;
	ti = time(0);
	strncpy(uid, (type == TEXT_IS_NONAME) ? "몰라" : currentuser.userid, 20);
	uid[19] = '\0';
#if defined(REALINFO) && defined(MAIL_REALNAMES)
	strncpy(uname, (type == TEXT_IS_NONAME) ? "모른다니까!!" : currentuser.realname, 40);
#else
	strncpy(uname, (type == TEXT_IS_NONAME) ? "모른다니까!!" : currentuser.username, 40);
#endif
	uid[39] = '\0';
	save_title[STRLEN - 10] = '\0';
	sprintf(tempbuf, "%s (%s)", uid, uname);
	if (type == TEXT_IS_MAIL) {
		fprintf(fp, "보낸이: %s\n", tempbuf);
	} else if (type != TEXT_IS_NOTICE) {
		sprintf(tempbuf, "%s (%s)", uid, uname);
		fprintf(fp, "쓴사람: %-32s 글상자: <%s>\n", tempbuf, currboard);
	}
	if (type == TEXT_IS_NOTICE){
		fprintf(fp, "제  목: %s\n", save_title);
		fprintf(fp, "날  짜: %s", time_string(ti));
	}  else {
		if (type == TEXT_IS_NONAME){
			fprintf(fp, "날  짜: %s\n", date_string_no_newline(ti));
		} else {
			fprintf(fp, "날  짜: %s", time_string(ti));
		}
		fprintf(fp, "제  목: %s\n", save_title);
	}
	/*
	 * Cc_list가 널스트링이면 게시판의 글, Cc_list가 널스트링이 아니면 편지
	 *
	 */
	if (Cc_list[0]) {
		fprintf(fp, "참  조: %s\n", Cc_list);
		Cc_list[0] = 0;
	}
	fprintf(fp, "\n");
}

#ifndef MAKEVE
int addsignature(FILE * fp)
{
	FILE           *sigfile;
	int             i;
	char            inbuf[STRLEN * 3];
	char            fname[STRLEN];
	sprintf(fname, "signatures/%s", currentuser.userid);
	if ((sigfile = fopen(fname, "r")) == NULL)
		return 0;
	for (i = 1; i <= MAXSIGLINES; i++) {
		if (fgets(inbuf, sizeof(inbuf), sigfile))
			fputs(inbuf, fp);
		else
			break;
	}
	fclose(sigfile);
	return 0;
}
#endif

#define KEEP_EDITING -2

int vedit_from_file(char * srcfile, char * dstfile, int saveheader, int type)
{
	FILE           *fp;
	FILE           *fpSrc;
	char            buf[80];
	int             readlen = 0;

	if ((fp = fopen(dstfile, "w")) == NULL) {
		indigestion(5);
		abort_bbs(0);
	}
	if (saveheader)
		write_header(fp, type);

	fpSrc = fopen(srcfile, "r");
	if (fpSrc == NULL) {
		fclose(fp);
		unlink(dstfile);
		return -1;
	}
	while ((readlen = fread(buf, 1, 80, fpSrc)) > 0) {
		fwrite(buf, 1, readlen, fp);
	}
	fclose(fpSrc);
#ifndef MAKEVE
        if (!(currentuser.flags[0] & SIG_FLAG))
                if (type == TEXT_IS_POST) {
                        addsignature(fp);
                } else if (type == TEXT_IS_MAIL) {
                        char            fbuf[80];
                        struct stat     statb;
                        sprintf(fbuf, "signatures/%s", currentuser.userid);
/*
                        if (HAS_PERM(PERM_SYSOP)) {
                                int             res = stat(fbuf, &statb);
                                prints("fn:%s size:%d res:%d\n", fbuf, statb.st_size, res);
                                pressanykey();
                        }
*/
                        if (stat(fbuf, &statb) == 0 && statb.st_size > 0) {
                                getdata(1, 0, "서명을 붙일까요? (Y/N)[Y] ? ", fbuf, 4, DOECHO, NULL);
                                if (fbuf[0] != 'n' && fbuf[0] != 'N')
                                        addsignature(fp);
                } else if (type == TEXT_IS_NONAME || type == TEXT_IS_EDIT){
                         /* Do Nothing */
		}
        }
#endif
	fclose(fp);
	return 0;
}

int write_file(char * filename, int saveheader, int type)
{
	FILE           *fp;
	struct textline *p = firstline;
	char            abort[6];
	int             aborted = 0;
	getdata(0, 0, "(S)간직함 (A)관둠 (E)고침? [S] ", abort, 6, DOECHO, NULL);
	if (abort[0] == 'a' || abort[0] == 'A') {
		struct stat     stbuf;
		prints("\n글을 간직하지 않았습니다.\n");
		refresh();
		sleep(1);
		if (stat(filename, &stbuf) || stbuf.st_size == 0)
			unlink(filename);
		aborted = -1;
	} else if (abort[0] == 'e' || abort[0] == 'E')
		return KEEP_EDITING;
	firstline = NULL;
	if (!aborted) {
		if ((fp = fopen(filename, "w")) == NULL) {
			indigestion(5);
			abort_bbs(0);
		}
		if (saveheader) {
			write_header(fp, type);
		}
	}
	while (p != NULL) {
		struct textline *v = p->next;
		if (!aborted)
			if (p->next != NULL || p->data[0] != '\0')
				fprintf(fp, "%s\n", p->data);
		free(p);
		p = v;
	}
#ifndef MAKEVE
	if (!aborted && !(currentuser.flags[0] & SIG_FLAG))
		if (type == TEXT_IS_POST) {
			addsignature(fp);
		} else if (type == TEXT_IS_MAIL) {
			char            fbuf[80];
			struct stat     statb;
			sprintf(fbuf, "signatures/%s", currentuser.userid);
/*
			if (HAS_PERM(PERM_SYSOP)) {
				int             res = stat(fbuf, &statb);
				prints("fn:%s size:%d res:%d\n", fbuf, statb.st_size, res);
				pressanykey();
			}
*/
			if (stat(fbuf, &statb) == 0 && statb.st_size > 0) {
				getdata(1, 0, "서명을 붙일까요? (Y/N)[Y] ? ", fbuf, 4, DOECHO, NULL);
				if (fbuf[0] != 'n' && fbuf[0] != 'N')
					addsignature(fp);
		} else if (type == TEXT_IS_NONAME || type == TEXT_IS_EDIT) {
			 /* Do Nothing */
		}
	}
#endif
	if (!aborted)
		fclose(fp);
	currline = NULL;
	lastline = NULL;
	firstline = NULL;
	return aborted;
}

void display_buffer(void)
{
	struct textline *p;
	int    i;

	for (p = top_of_win, i = 0; i < 24; i++) {
		move(i, 0);
		if (p) {
			prints("%s", p->data);
			p = p->next;
		} else
			prints("~");
		clrtoeol();
	}
	return;
}

#define ESCAPE  (01)
#define VTKEYS  (02)
#define NORMAL  (00)
#define PCKEYS  (03)		/*- wjlee */

int do_fly_on_line(int msgLine, int promptend)
{
        char text[100];
        char uident[30];
        char buf[200];
        int     tuid;
	int ox, oy;
        struct user_info uin;

	if (!HAS_PERM(PERM_POST)) return 0;
	getyx(&oy, &ox);
        getdata(msgLine, 0, "상대방의 아이디를 입력하세요 : ", uident, 14, DOECHO, NULL);
        if (uident[0] == '\0') {
                move(msgLine, 0);
                prints(splitStr(" ** 관두었습니다. "));
		if (promptend) igetch();
		move(oy, ox);
                return 1;
        }

	if (uident[0] == '-') strcpy(uident, getLastMessageSender());
	if (uident[0] == '=') strcpy(uident, getLastMessageReceiver());
        if ( uident[0] == '\0' || !(tuid = getuser(uident)) || tuid == usernum) {
                move(msgLine, 0);
                prints(splitStr(" ** 아이디가 이상합니다. "));
		if (promptend) igetch();
		move(oy, ox);
                return  1;
        }
        search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);
	if (uin.refuse_call){
		char buf[80];
		sprintf(buf, " ** %s님은 지금 쪽지를 받을 수 없습니다.", uident);
                move(msgLine, 0);
                prints(splitStr(buf));
		if (promptend) igetch();
		move(oy, ox);
                return 1;
	}
        getdata(msgLine, 0, "쪽지내용 : ", text, 60, DOECHO, NULL);
        if (text[0] == '\0') {
                move(msgLine, 0);
                prints(splitStr(" ** 관두었습니다.  "));
		if (promptend) igetch();
		move(oy, ox);
                return 1;
        }
        sprintf(buf, ">> %s님의 쪽지: %s", currentuser.userid, text);
        writeFlyingMessage(uident, buf);
        if (!uin.active || (kill(uin.pid, SIGUSR2) == -1)) {
                move(msgLine, 0);
                prints(splitStr(" ** 상대방이 지금 BBS안에 없습니다. "));
		if (promptend) igetch();
		move(oy, ox);
                return 1;
        }
        sprintf(buf, "- %s님에게 쪽지를 보냈습니다. ", uident);
        move(msgLine, 0);
        prints(splitStr(buf));
	if (promptend) igetch();
	move(oy, ox);
        return 1;
}

int back_during_edit(void)
{
	FILE           *fp;
	char            buf[100];
	struct msgRec 	msgBuf;
	int		i;

	sprintf(buf, "msgtmp/%s", currentuser.userid);
	fp = fopen(buf, "rb");
	move(0, 0);
	prints("* 지난 쪽지 보기 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	if (fp == NULL) {
		prints("  - 아무 쪽지도 없습니다. >>");
		igetch();
		return 0;
	}
	fread(&msgBuf, sizeof(msgBuf), 1, fp);
	fclose(fp);
	clrtobot();
	prints("\n");
	for (i=msgBuf.index; i>=0; i--){
		prints("^^ %s ^^;\n", time_string_no_newline(msgBuf.time[i]));
		prints("%s\n\n", msgBuf.msg[i]);
	}
	for (i=MSGNUM-1; i>msgBuf.index; i--){
		prints("^^ %s ^^;\n", time_string_no_newline(msgBuf.time[i]));
		prints("%s\n\n", msgBuf.msg[i]);
	}
	igetch();
	return 0;
}

int vedit(char * filename, int saveheader, int type)
{
	int             ch;
	int             foo;
	int             lastcharindent = -1;
	int             mode = NORMAL;
	int             firstkey;
	extern	int	msgLine;

	msgLine = t_lines -1;
	IAmEditing = YEA;

	/*-
    	call_helvis(filename, saveheader);
    	return;
    	*/
	read_file(filename);
	top_of_win = firstline;
	currline = firstline;
	curr_window_line = 0;
	currpnt = 0;
	clear();
	display_buffer();
	move(t_lines - 1, 20);
	standout();
	prints(" Ctrl-Z를 누르면 도움말이 나옵니다. ");
	standend();
	/*
	 * clrtoeol(); move(1, 0); clrtoeol();
	 */
	move(curr_window_line, currpnt);
	/*- chopin
        redraw_everything = YEA;
        */
	firstkey = 0;		/* + chopin */
	while ((ch = igetch()) != EOF) {
		if (ch == '\n' && firstkey == 0) {
			firstkey = 10;
			redraw_everything = YEA;	/* chopin */
			/*
			 * prints("~\n~");
			 */
		}
#ifdef BIT8
		if (ch & 0x80)
			mode = NORMAL;
		else if (ch == 27) {	/*- wjlee */
			mode = ESCAPE;
			if ((ch = igetch()) == EOF)
				break;
		} else if (ch == 0) {
			mode = PCKEYS;
			if ((ch = igetch()) == EOF)
				break;
		}
#else
		if (ch & 0x80 && mode == NORMAL) {
			mode = ESCAPE;
			ch = ch & 0x7f;
		}
#endif
		switch (mode) {
		case ESCAPE:
			switch (ch) {
			case '[':
				mode = VTKEYS;
				break;
			case '>':
				top_of_win = back_line(lastline, 23);
				currline = lastline;
				curr_window_line = getlineno();
				currpnt = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			case '<':
				top_of_win = firstline;
				currline = top_of_win;
				currpnt = 0;
				curr_window_line = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			case 'v':
			case 'V':
				top_of_win = back_line(top_of_win, 22);
				currline = top_of_win;
				currpnt = 0;
				curr_window_line = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			default:
				bell();
				mode = NORMAL;
				break;
			}
			break;
		case PCKEYS:
			switch (ch) {
			case 0x48:
				ch = CTRL('P');
				break;
			case 0x50:
				ch = CTRL('N');
				break;
			case 0x4d:
				ch = CTRL('F');
				break;
			case 0x4b:
				ch = CTRL('B');
				break;
			case 0x51:
				ch = CTRL('V');
				break;
			case 0x4f:
				top_of_win = back_line(lastline, 23);
				currline = lastline;
				curr_window_line = getlineno();
				currpnt = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			case 0x47:
				top_of_win = firstline;
				currline = top_of_win;
				currpnt = 0;
				curr_window_line = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			case 0x49:
				top_of_win = back_line(top_of_win, 22);
				currline = top_of_win;
				currpnt = 0;
				curr_window_line = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			default:
				bell();
				mode = NORMAL;
				break;
			}
			if (mode == NORMAL)
				break;
			else
				goto normal;
		case VTKEYS:
			switch (ch) {
			case 'A':
				ch = CTRL('P');
				break;
			case 'B':
				ch = CTRL('N');
				break;
			case 'C':
				ch = CTRL('F');
				break;
			case 'D':
				ch = CTRL('B');
				break;
			case '5':
				if ((ch = igetch()) != 0x7e)
					break;
				top_of_win = back_line(top_of_win, 22);
				currline = top_of_win;
				currpnt = 0;
				curr_window_line = 0;
				redraw_everything = YEA;
				mode = NORMAL;
				break;
			case '6':
				if ((ch = igetch()) != 0x7e)
					break;
				ch = CTRL('V');
				break;
			default:
				bell();
				mode = NORMAL;
				break;
			}
			if (mode == NORMAL)
				break;
	normal:
			mode = NORMAL;
		default:
#ifdef BIT8
			if (isprint2(ch)) {
#else
			if (isprint(ch)) {
#endif
				insert_char(ch);
				lastcharindent = -1;
			} else
				switch (ch) {
				case '\011':	/* 탭 */
					{
						int             wjlee;
						for (wjlee = 0; wjlee < 8; wjlee++) {
							insert_char(' ');
							lastcharindent = -1;
						}
					}
					break;
				case CTRL('P'):
					if (lastcharindent == -1)
						lastcharindent = currpnt;
					if (!currline->prev) {
						bell();
						break;
					}
					curr_window_line--;
					currline = currline->prev;
					currpnt = (currline->len > lastcharindent) ?
						lastcharindent : currline->len;
					break;
				case CTRL('N'):
					if (lastcharindent == -1)
						lastcharindent = currpnt;
					if (!currline->next) {
						bell();
						break;
					}
					currline = currline->next;
					curr_window_line++;
					currpnt = (currline->len > lastcharindent) ?
						lastcharindent : currline->len;
					break;
				default:
					lastcharindent = -1;
					switch (ch) {
					case CTRL('F'):
						if (currline->len == currpnt) {
							if (!currline->next) {
								bell();
								break;
							}
							currpnt = 0;
							curr_window_line++;
							currline = currline->next;
							break;
						} else {
							if (currline->data[currpnt] & 0x80)
								currpnt++;
							currpnt++;
						}
						break;
					case CTRL('B'):
						if (currpnt == 0) {
							if (!currline->prev) {
								bell();
								break;
							}
							curr_window_line--;
							currline = currline->prev;
							currpnt = currline->len;
							break;
						} else {
							currpnt--;
							if (currline->data[currpnt] & 0x80)
								currpnt--;
						}
						break;
					case CTRL('G'):
						clear();
						redraw_everything = YEA;
						break;
					case CTRL('Z'):
						vedit_help();
						break;
					case CTRL('V'):
						{
							struct textline *temp =
							forward_line(top_of_win, 22);
							if (temp == lastline) {
								bell();
								move(23, 0);
								prints(" -- 파일의 끝입니다 -- ");
								move(curr_window_line, currpnt);
								break;
							}
							top_of_win = temp;
						}
						currline = top_of_win;
						curr_window_line = 0;
						currpnt = 0;
						redraw_everything = YEA;
						break;
					case CTRL('A'):
						currpnt = 0;
						break;
					case CTRL('E'):
						currpnt = currline->len;
						break;
					case CTRL('X'):
					case CTRL('W'):
						clear();
						foo = write_file(filename, saveheader, type);
						if (foo != KEEP_EDITING) {
							IAmEditing = NA;
							return foo;
						}
					case '\r':
					case '\n':
						split(currline, currpnt);
						break;
					case '\177':
					case CTRL('H'):
						if (currpnt == 0) {
							struct textline *p;

							if (!currline->prev) {
								bell();
								break;
							}
							curr_window_line--;
							currline = currline->prev;
							currpnt = currline->len;
							if (*killsp(currline->next->data) == '\0') {
								delete_line(currline->next);
								redraw_everything = YEA;
								break;
							}
							p = currline;
							while (!join(p)) {
								p = p->next;
								if (p == NULL) {
									indigestion(2);
									IAmEditing = NA;
									abort_bbs(0);
								}
							}
							redraw_everything = YEA;
							break;
						}
						currpnt--;
						delete_char();
						break;
					case CTRL('D'):
						if (currline->len == currpnt) {
							struct textline *p = currline;

							while (!join(p)) {
								p = p->next;
								if (p == NULL) {
									indigestion(2);
									IAmEditing = NA;
									abort_bbs(0);
								}
							}
							redraw_everything = YEA;
							break;
						}
						delete_char();
						break;
					case CTRL('K'):
						if (currline->len == 0) {
							struct textline *p = currline->next;

							if (!p) {
								p = currline->prev;
								if (!p) {
									bell();
									break;
								}
								curr_window_line--;
							}
							if (currline == top_of_win)
								top_of_win = p;
							delete_line(currline);
							currline = p;
							redraw_everything = YEA;
							break;
						}
						if (currline->len == currpnt) {
							struct textline *p = currline;

							while (!join(p)) {
								p = p->next;
								if (p == NULL) {
									indigestion(2);
									IAmEditing = NA;
									abort_bbs(0);
								}
							}
							redraw_everything = YEA;
							break;
						}
						currline->len = currpnt;
						currline->data[currpnt] = '\0';
						break;
					case '\033':	/*- ESC */
						mode = ESCAPE;
						break;
					case CTRL('O'):
						{
						int cch;
						move(0,0);
						prints( "쪽지 보내기(Space/Enter/S), 지난 쪽지 보기(B) [S] ");
						cch = igetch();
						switch(cch){
						case ' ': case '\r': case '\n':
						case 's': case 'S':
							do_fly_on_line(0, YEA);
							break;
						case 'b': case 'B':
							back_during_edit();
						}
						redraw_everything = YEA;
						}
						break;
					default:
						bell();
						break;
					}
				}
			break;
		}
		if (curr_window_line == -1) {
			curr_window_line = 0;
			if (!top_of_win->prev) {
				indigestion(6);
				bell();
			} else {
				top_of_win = top_of_win->prev;
				rscroll();
			}
		}
		if (curr_window_line == 24) {
			curr_window_line = 23;
			if (!top_of_win->next) {
				indigestion(7);
				bell();
			} else {
				top_of_win = top_of_win->next;
				scroll();
			}
		}
		move(curr_window_line, 0);
		prints("%s", currline->data);
		clrtoeol();
		if (redraw_everything)
			display_buffer();
		redraw_everything = NA;
		move(curr_window_line, currpnt);
	}
	IAmEditing = NA;
	return 0;
}

char           *helptxt[] = {
	"\01일반 명령:",
	"Ctrl-X  마치기 (Ctrl-W)",
	"Ctrl-L  화면 정리",
	"Ctrl-Z  도움말 화면보기, 지금보는 화면",
	"Ctrl-O  쪽지 보내기/ 지난 쪽지 보기",
	"",
	"\01커서 이동 명령:",
	"Ctrl-F  한칸 앞으로               VT100            PC          XTERM ",
	"Ctrl-B  한칸 뒤로                   화             화            화",
	"Ctrl-P  윗줄로                      살             살            살",
	"Ctrl-N  아랫줄로                    표             표            표 ",
	"Ctrl-V  다음 페이지                               PgDn          PgDn",
	"ESC v   앞 페이지                                 PgUp          PgUp",
	"ESC >   파일의 맨 뒤로                            End   ",
	"ESC <   파일의 맨 앞으로                          Home  ",
	"Ctrl-A  줄의 맨 앞으로                                     ",
	"Ctrl-E  줄의 맨 뒤로                                       ",
	"",
	"\01삭제 명령:",
	"Ctrl-D  한글자 지우기",
	"Ctrl-K  줄의 맨 뒤까지 지우기",
NULL};

int vedit_help(void)
{
	int             i, off = 0, pos = 2;

	clear();
	standout();
	prints("에디터 도움말 ------------------------------------------ 아무글쇠나 누르세요 --");
	standend();
	for (i = 0; helptxt[i]; i++) {
		move(pos, off);
		if (helptxt[i][0] == '\01') {
			standout();
			prints("%s", &helptxt[i][1]);
			standend();
		} else
			prints("%s", helptxt[i]);
		clrtoeol();
		pos++;
	}
	move(23, 0);
	igetch();
	clear();
	redraw_everything = YEA;
	return 0;
}


/* 한 줄 편집 : This function is added by belami */

int editline(int y, int x, char *buf, int limitlen)
{
	int             ch, curpos = 0, i, len, leftchars = 0;
	char            workbuf[500];
	strncpy(workbuf, buf, limitlen);
	workbuf[limitlen] = 0;

	len = strlen(workbuf);
	if (workbuf[len - 1] == '\n')
		workbuf[--len] = 0;
	for (i = len + 1; i < limitlen; i++)
		workbuf[i] = '.';
	curpos = len;
	while (1) {
		move(y, x);
		clrtoeol();
		prints(workbuf);
		move(y, x + curpos);
		if (leftchars > 0)
			leftchars--;
		else
			ch = igetch();
		switch (ch) {
		case CTRL('U'):	/* kill to begin of line */
			leftchars = curpos;
			ch = '\b';
			break;
		case CTRL('K'):	/* kill to end of line */
			leftchars = len - curpos;
			ch = CTRL('D');
			break;
		case CTRL('F'):	/* forward */
			if (curpos < limitlen && curpos < len) {
				if (workbuf[curpos] & 0x80)
					curpos++;	/* hangul */
				curpos++;
			}
			break;
		case CTRL('B'):	/* backward */
			if (curpos > 0) {
				curpos--;
				if (workbuf[curpos] & 0x80)
					curpos--;
			}
			break;
		case CTRL('E'):	/* go to end */
			curpos = len;
			if (curpos > limitlen)
				curpos = limitlen;
			break;
		case CTRL('Z'):	/* help */
			move(t_lines - 1, 0);
			standout();
			prints(" (^U/^K) Kill line  (ESC) Quit  (CR/^X) ");
			prints("Save & Quit  (^B/^F) Left & Right ");
			standend();
			break;
		case CTRL('A'):	/* go to begin */
			curpos = 0;
			break;
		case '\r':
		case '\n':
		case CTRL('X'):	/* save and return */
			strncpy(buf, workbuf, len);
			buf[len] = 0;
			return EDIT_ENTER;
		case 27:	/* no touch and return */
			return EDIT_ESC;
		case '\b':	/* backspace */
			if (curpos > 0) {
				curpos--;
				if (curpos > 0 && (workbuf[curpos] & 0x80))
					curpos--;
			}
			/* NO BREAK! */
		case CTRL('D'):	/* delete char */
			if (workbuf[curpos] & 0x80) {
				strcpy(&workbuf[curpos], &workbuf[curpos + 2]);
				len -= 2;
				if (len < 0)
					len = 0;
				workbuf[limitlen] = 0;
				workbuf[limitlen - 1] = '.';
				workbuf[limitlen - 2] = '.';
			} else {
				strcpy(&workbuf[curpos], &workbuf[curpos + 1]);
				len--;
				if (len < 0)
					len = 0;
				workbuf[limitlen] = 0;
				workbuf[limitlen - 1] = '.';
			}
			break;
		default:	/* insert char */
			if (isprint2(ch) && len < limitlen) {
				for (i = limitlen - 1; i >= curpos; i--) {
					workbuf[i + 1] = workbuf[i];
				}
				workbuf[curpos] = ch;
				workbuf[limitlen] = 0;
				curpos++;
				len++;
			}
		}
	}
}

/*- skji */
int call_helvis(char * filename, int saveheader, int type)
{
	char            cmdbuf[500], headerfname[500], ebuf[100], bodyfname[500];
	FILE           *headerf, *bodyf;
	/* 본문 편집 */
	strcpy(bodyfname, filename);
	sprintf(cmdbuf, "/usr/local/bin/helvis-for-bbs %s", bodyfname);
	system(cmdbuf);
	/* 본문은 bodyfname에 저장되어 있음 */
	move(0, 0);
	clrtobot();
	getone(1, 0, "시그너쳐를 붙일까요 (Y/N)[N] ? ", ebuf, 4, DOECHO, NULL);
	if (ebuf[0] == 'y' || ebuf[0] == 'Y') {
		if ((bodyf = fopen(bodyfname, "r+"))) {
			addsignature(bodyf);
			fclose(bodyf);
			/* 본문에 시그너쳐를 붙임 */
		}
	}
	if (saveheader) {
		sprintf(headerfname, "%s.header", filename);
		if ((headerf = fopen(headerfname, "w"))) {
			char            rbuf[512];
			write_header(headerf, type);
			if ((bodyf = fopen(bodyfname, "r"))) {
				while (fgets(rbuf, 500, bodyf))
					fprintf(headerf, "%s", rbuf);
				fclose(bodyf);
			}
			fclose(headerf);
		} else {
			;	/* 글을 저장할 수 없습니다. */
		}
		unlink(bodyfname);
		rename(headerfname, filename);
	} else {
		;
	}
	return 0;
}

void dumpEditing(void)
{
	struct fileheader newmessage;
	char            target_fullpath[4096];
	char           *ip;
	struct stat     st;
	FILE           *fp;
	int fd;
	struct textline *p = firstline;
	char *userid = currentuser.userid;

	if ( ! IAmEditing) return;

	/* 메일 디렉토리가 없으면 새로 만든다 */
	sprintf(genbuf, "mail/%s", userid);
	if (stat(genbuf, &st) == -1) {
		if (mkdir(genbuf, 0770) == -1) {
			return;
		}
	} else {
		if (!(st.st_mode & S_IFDIR)) {
			return;
		}
	}

	/*- 타겟 파일 이름 고르기 */
	memset(&newmessage, 0, sizeof(newmessage));
	sprintf(target_fullpath, "M.%d.%s.A", time(0), currentuser.userid);
	sprintf(genbuf, "mail/%s/%s", userid, target_fullpath);
	ip = strrchr(target_fullpath, 'A');
	while((fd = open(genbuf, O_CREAT | O_EXCL | O_WRONLY, 0664)) == -1) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(genbuf, "mail/%s/%s", userid, target_fullpath);
	}
	close(fd);

	strcpy(newmessage.filename, target_fullpath);	/* 파일네임만 */
	strcpy(target_fullpath, genbuf);	/* 풀 패스 */

	strncpy(newmessage.owner, "SYSOP", STRLEN);
	strcpy(newmessage.title, "** 쓰다가 날아간 글을 주워왔어요 **");
	{			/*- 타임 세팅 */
		time_t          clk;
		struct tm      *tm;
		clk = time(NULL);
		tm = localtime(&clk);
		newmessage.tm_year = tm->tm_year;
		newmessage.tm_mon = tm->tm_mon;
		newmessage.tm_mday = tm->tm_mday;
	}

	fp = fopen(target_fullpath, "w");
	if (fp != NULL){
		long ti;

		ti = time(0);
		fprintf(fp, "보낸이: SYSOP\n");
		fprintf(fp, "날  짜: %s", time_string(ti));
		fprintf(fp, "제  목: ** 쓰다가 날아간 글을 주워왔어요 **\n\n");
		fprintf(fp, " 안녕하십니까.SYSOP입니다\n\n이글은 프로그램에 BBS 프로그램에 의해\n\n 자동발송 되었습니다\n\n\n");
		if (save_title[0]){
			fprintf(fp, "귀하가 [%s] (이)라는 제목으로 \n", save_title);
		}
 		fprintf(fp, "열심히 쓰시다가 날아간 글 쪼가리를 주워왔습니다.\n\n");
		fprintf(fp, "**************************************** 요기부터 *\n");
		firstline = NULL;
		while (p != NULL) {
			struct textline *v = p->next;
			if (p->next != NULL || p->data[0] != '\0')
					fprintf(fp, "%s\n", p->data);
			free(p);
			p = v;
		}
		fprintf(fp, "* 요기까지 ****************************************\n");
		fprintf(fp, "\n          그럼 더욱 말리시기를-_-..\n");
		fclose(fp);
			/* security 를 위해서 모드를 바꿈 + by bugman */
		chmod(target_fullpath, 0600);

		/* 인덱스에 등록 */
		sprintf(genbuf, "mail/%s%s", userid, DIR);
		if (append_record(genbuf, &newmessage, sizeof(newmessage)) == -1)
			return;
	}
}
