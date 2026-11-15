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
#include <termcap.h>
#include "chat.h"
#include <sys/types.h>
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include "hanlib.h"

#include "edit.h"		/* added by skji.  (for proto. of editline();
				 * ) */

int             chatroom;
int             chatline;
int             echatwin;
extern char     page_requestor[];
extern char     BoardName[];
extern int      talkrequest;

/* +2 lines added by skji, to support titles of chatting rooms */
char            chatroom_title[CHATROOM_CNT][100];
/* #define CHATROOM_TITLE 	".CHATROOM-TITLE" */

/* + by bugman enable change max chatid len */
#define CHATIDLEN 10
int addColon(char * chatid)
{
	int             i;
	for (i = strlen(chatid); i < CHATIDLEN; i++) {
		chatid[i] = ' ';
	}
	chatid[i++] = ' ';
	chatid[i++] = ':';
	chatid[i++] = ' ';
	chatid[i++] = '\0';
	return 0;
}
/*************/
int fixchatid(char * chatid)
{
	int             i;
	while (*chatid != '\0' && *chatid != '\n') {
		if (strchr(BADCIDCHARS, *chatid))
			*chatid = '_';
		chatid++;
	}
	return 0;
}

int currenttime(void)
{
	time_t          t;
	struct tm      *tm;
	t = time(0);
	tm = localtime((time_t *) & t);
	return (tm->tm_hour * 60 + tm->tm_min);
}
int askLimit(void)
{
	char            buf[50];
	int             tt;
	getdata(6, 0, "시간제한은? (분단위, 0은 무제한) [0] ", buf, 4, DOECHO, NULL);
	if (buf[0] == '\0')
		return 0;
	tt = atoi(buf);
	if (tt <= 0)
		return 0;
	return tt + currenttime();
}

int             limitTime = 0;
int t_chat(void)
{
	char            chatid[STRLEN];
	struct hostent *h;
	char            hostname[STRLEN];
	struct sockaddr_in sin;
	char           *colon;
	int             a, n;
	int             currchar;
	int             chatport;
	int             newmail = 0;
	char            chatstr[100];
	char            inbuf[100];
	int             pline;
	extern int      dumb_term;
	int             page_pending = NA;
	time_t          last_check = 0, now;
	char            tempbuf[100];

	inbuf[0] = '\0';
	switch (chatroom) {
	case 1:
		report("Chat-1");
		break;
	case 2:
		report("Chat-2");
		break;
	case 3:
		report("Chat-3");
		break;
	case 4:
		report("Chat-4");
		break;
		/*-
			case 5:
			    report("Chat-5");
			    break;
			case 6:
			    report("Chat-6");
			    break;
		*/
	default:
		/* chatroom = 1; */
		report("Chat-1");
		break;
	}

	chatline = 0;
	currchar = 0;
	getdata(4, 0, "대화명을 쓰세요 : ", chatid, CHATIDLEN + 1, DOECHO, NULL);
	if (!strcmp(chatid, "off")) {
		clear();
		return 0;
	}
	if (chatid[0] == '\0')
		strcpy(chatid, currentuser.userid);
	chatid[CHATIDLEN] = '\0';
	fixchatid(chatid);
	strcpy(uinfo.chatid, chatid);
	addColon(chatid);
	strcpy(inbuf, chatid);
	gethostname(hostname, STRLEN);
	if (!(h = gethostbyname(hostname))) {
		perror("gethostbyname");
		return -1;
	}
	memset(&sin, 0, sizeof sin);
	sin.sin_family = h->h_addrtype;
	memmove(&sin.sin_addr, h->h_addr, h->h_length);
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
	sin.sin_port = chatport;
	a = socket(sin.sin_family, SOCK_STREAM, 0);
	/* 채팅 대몬이 떠 있는가를 체크 */
	if ((connect(a, (struct sockaddr *)&sin, sizeof sin))) {
		move(2, 0);
		edit_chatroom_title(chatroom);
		limitTime = askLimit();
		prints("** 방을 깨끗이 쓸고 닦고 있습니다 (쓱삭쓱삭)");
		refresh();
		close(a);
		sprintf(chatstr, "./bin/bbs.chatd %d", chatroom);
		system(chatstr);
		sleep(2);
		a = socket(sin.sin_family, SOCK_STREAM, 0);
		if ((connect(a, (struct sockaddr *)&sin, sizeof sin))) {
			perror("connect failed");
			return -1;
		}
	} else {
		limitTime = askLimit();
	}

	init_screen(tgetnum("li"), tgetnum("co"));	/* by wings!!! */

	switch (chatroom) {
	case 1:
		uinfo.mode = CHAT1;
		break;
	case 2:
		uinfo.mode = CHAT2;
		break;
	case 3:
		uinfo.mode = CHAT3;
		break;
	case 4:
		uinfo.mode = CHAT4;
		break;
	}
	uinfo.in_chat = YEA;

	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	if (uinfo.invisible) {
#ifdef PERMS
		if (HAS_PERM(PERM_SYSOP))
#else
		if (currentuser.userlevel == 255)
#endif
			sprintf(inbuf, "/sysopin %s", uinfo.chatid);
		else
			sprintf(inbuf, "/cloakin %s", uinfo.chatid);
	} else {
		int             tuid;
		/*
		 * if ((tuid=getuser(uinfo.chatid)) == -1) {
		 */
		sprintf(inbuf, "  ###   '%s'님이 방문을 열고 들어오셨습니다   ###", uinfo.chatid);
		/*-
			} else {
			    struct userec uec;
			    substitute_record(PASSFILE, &uec, sizeof(struct userec), tuid);
			    sprintf(inbuf, "  -=* %s(%s)님이 들어오셨습니다 *=-",
				uinfo.chatid, uec.username);
			}
		*/
	}
	write(a, inbuf, strlen(inbuf) + 1);
	clear();
	sprintf(inbuf, "%s 왁자지껄방 %d번방 -- '/도움말'을 하시면 기능설명서 ", BoardName, chatroom);
	printchatline(inbuf);
	printchatline("  ");
	strcpy(inbuf, chatid);
	pline = t_lines - 1;
	echatwin = t_lines - 2;
	move(echatwin, 0);
	prints("--------------------------------------------------------------------------------");
	move(pline, 0);
	if (!dumb_term)
		prints("%s", inbuf);
	currchar = strlen(inbuf);
	add_io(a, 0);
	while (YEA) {
		int             ch = igetch();
		if (talkrequest)
			page_pending = YEA;
		if (page_pending)
			page_pending = servicepage(0);
		if (!newmail && chkmail()) {
			newmail = 1;
			move(echatwin, 0);
			prints("---*새 편지 도착*-----------------------------------------------------------");
		}
		if (ch == I_OTHERDATA) {
			static int      cnt = 0;
			int             cc;
			static char     buf[512];

			if (limitTime != 0) {
				if (limitTime <= currenttime()) {
					printchatline("  ** 시간이 다 되었습니다. 이제 나갑니다. **");
					sleep(2);
					sprintf(inbuf, "  -= '%s'님이 시간이 다 되서 쫓겨났습니다 =-", uinfo.chatid);
					write(a, inbuf, strlen(inbuf) + 1);
					break;
				}
			}
			if ((cc = read(a, buf + cnt, sizeof buf)) > 0) {
				int             processed = 0;

				while (cc > 0) {
					register int    i;
					for (i = processed; buf[i] != '\0' && i != sizeof buf; i++)
						 /*- NULL STATEMENT */ ;
					if (i == sizeof buf && buf[i] != '\0') {
						bcopy(buf + processed, buf, processed - sizeof(buf));
						cnt = processed - sizeof(buf);
						break;
					}
					cnt = 0;
					if (*(buf + processed) == '/')
						printinfoline(buf + processed + 1);
					else
						printchatline(buf + processed);
					i++;
					cc -= (i - processed);
					processed = i;
					if (i == sizeof buf)
						break;
				}
			}
			/*- trap for chat daemon death */
			else {
				sprintf(buf, "CHAT FAILURE: read returned %d, errno = %d\n", cc, errno);
				report(buf);
				break;
			}
			move(pline, currchar);
			continue;
		}
#ifdef BIT8
		if (isprint2(ch)) {
#else
		if (isprint(ch)) {
#endif
			if (currchar == 78) {
				bell();
				continue;
			}
			inbuf[currchar] = ch;
			currchar++;
			inbuf[currchar] = '\0';
			move(pline, currchar - 1);
			prints("%c", ch);
			continue;
		}
		if (ch == '\n' || ch == '\r') {	/* editing line chatting room  */
			char            buf[STRLEN];
			int             i, j;

			if (dumb_term)
				prints("\n");
			for (i = CHATIDLEN + 3, j = 0; i < STRLEN; i++) {
				if (inbuf[i] == '\0') {
					buf[j] = '\0';
					break;
				}
				if (inbuf[i] != ' ')
					buf[j++] = inbuf[i];
			}
			if (buf[0] == '\0')
				continue;
			if (!strcmp(buf, "Goodbye!") || !strcmp(buf, "안녕히!")) {
				/*-                  report("Chat user typed Goodbye!") ;*/
				if (uinfo.invisible) {
#ifdef PERMS
					if (HAS_PERM(PERM_SYSOP))
#else
					if (currentuser.userlevel == 255)
#endif
						sprintf(inbuf, "/sysopout %s", uinfo.chatid);
					else
						sprintf(inbuf, "/cloakout %s", uinfo.chatid);
				} else {
					sprintf(inbuf,
						"  -= '%s'님이 안녕히 계시라는 말을 남기고 나가셨습니다 =-", uinfo.chatid);
				}
				write(a, inbuf, strlen(inbuf) + 1);
				break;
			}
			if (inbuf[CHATIDLEN + 3] == '/' || inbuf[CHATIDLEN + 3] == '-') {
				char            goodbye_msg[80];
				int             action;

				goodbye_msg[0] = '\0';
				action = dochatcommand(&inbuf[CHATIDLEN + 4], a, goodbye_msg);
				if (action == 1) {
					strcpy(chatid, uinfo.chatid);
					chatid[CHATIDLEN] = '\0';
					addColon(chatid);
				} else if (action == -1) {
					sprintf(buf, "CHAT FAILURE: write returned -1, errno = %d\n", errno);
					report(buf);
					break;
				} else if (action == -2) {	/*- wjlee */
					if (uinfo.invisible) {
#ifdef PERMS
						if (HAS_PERM(PERM_SYSOP))
#else
						if (currentuser.userlevel == 255)
#endif
							sprintf(inbuf, "/sysopout %s", uinfo.chatid);
					} else
					 /*- chopin goodbye_msg */ if (goodbye_msg[0] == '\0')
						sprintf(inbuf,
							"  -= '%s'님이 방문 닫고 나가셨습니다 =-"
							,uinfo.chatid);
					else
						sprintf(inbuf,
							"  -= '%s'님의 마지막 한마디 \"%s\" =-"
						,uinfo.chatid, goodbye_msg);

					write(a, inbuf, strlen(inbuf) + 1);
					break;
				}
			} else if (write(a, inbuf, strlen(inbuf) + 1) == -1) {
				sprintf(buf, "CHAT FAILURE: write returned -1, errno = %d\n", errno);
				report(buf);
				break;
			}
			move(pline, 0);
			strcpy(inbuf, chatid);
			currchar = strlen(inbuf);
			if (!dumb_term)
				prints("%s", inbuf);
			clrtoeol();
			move(pline, currchar);
			continue;
		}
		/* 채팅방에서의 편집키들: + by skji */
		if (ch == CTRL('U')) {
			/* kill to begin of line */
			currchar = CHATIDLEN + 3;
			inbuf[currchar] = 0;
			move(echatwin + 1, currchar);
			clrtoeol();
		} else if (ch == CTRL('K')) {
			int             i = currchar;
			for (; i > CHATIDLEN + 3 && !isspace(inbuf[i]); i--);
			;
			/* skip word */
			for (; i >= CHATIDLEN + 3 && isspace(inbuf[i]); i--);	/* skip white */
			currchar = i + 1;
			inbuf[currchar] = 0;
			move(echatwin + 1, currchar);
			clrtoeol();
		} else
			/* 편집키 여기까지 + skji */

		if (ch == CTRL('H') || ch == '\177') {
			if (currchar == CHATIDLEN + 3) {
				bell();
				continue;
			}
			currchar--;
			move(pline, currchar);
			if (dumb_term)
				ochar(CTRL('H'));
			else
				prints(" ");
			move(pline, currchar);
			inbuf[currchar] = '\0';
			continue;
		}
		if (ch == CTRL('C') || ch == CTRL('D')) {
			if (uinfo.invisible) {
#ifdef PERMS
				if (HAS_PERM(PERM_SYSOP))
#else
				if (currentuser.userlevel == 255)
#endif
					sprintf(inbuf, "/sysopout %s", uinfo.chatid);
				else
					sprintf(inbuf, "/cloakout %s", uinfo.chatid);
			} else
				sprintf(inbuf, "  -= '%s'님이 방문도 안닫고 나가셨습니다 =-", uinfo.chatid);
			write(a, inbuf, strlen(inbuf) + 1);
			break;
		}
	}
	add_io(0, 0);
	close(a);
	uinfo.in_chat = NA;
	uinfo.mode = TMENU;
	uinfo.chatid[0] = '\0';
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

char * advance(char * ptr)
{
	while (*ptr != ' ' && *ptr != '\n' && *ptr != '\t') {
		if (*ptr == '\0')
			return ptr;
		else
			ptr++;
	}
	while (*ptr == ' ' || *ptr == '\n' || *ptr == '\t')
		ptr++;
	*(ptr - 1) = '\0';
	return ptr;
}

int printchatline(char * str)
{
	char *foo;
	extern int      msgLine;
	move(chatline++, 0);
	clrtoeol();
	if (foo = strrchr(str, '\n'))
		*foo = '\0';
	prints("%s\n", str);

	if (chatline >= echatwin)
		chatline = 0;
	move(chatline, 0);

	msgLine = echatwin;

	clrtoeol();
	if (!dumb_term) {
		char            sbuf[80];
		time_t          tt = time(0);
		if (limitTime != 0)
			sprintf(sbuf, " [%s]*%d분전*-->", time_string_short_no_newline(tt), limitTime - currenttime());

		else
			sprintf(sbuf, " [%s] -->", time_string_short_no_newline(tt));


		standout();
		prints(sbuf);
		standend();
	}
	return 0;
}

int printprivmsg(char * to_ptr)
{
	char           *fromptr = advance(to_ptr);
	char           *msgptr = advance(fromptr);
	if (!ci_strncmp(to_ptr, currentuser.userid, sizeof(currentuser.userid))) {
		char            buf[STRLEN];
		sprintf(buf, "<< %s님이 속삭입니다--> %s", fromptr, msgptr);
		standout();
		printchatline(buf);
		standend();
	}
	return 0;
}

int printinfoline(char * cmd)
{
	char           *nextarg;
	char            buf[STRLEN];
	nextarg = advance(cmd);
#ifdef PERMS
	if ((!strcmp(cmd, "sysopin") && HAS_PERM(PERM_SYSOP)) ||
	    (!strcmp(cmd, "cloakin") && HAS_PERM(PERM_SEECLOAK)))
#else
	if ((!strcmp(cmd, "sysopin") && currentuser.userlevel == 255) ||
	    (!strcmp(cmd, "cloakin") && currentuser.userlevel >= SEE_CLOAK))
#endif
	{
		sprintf(buf, "  ** 알림: %s%s 마법의 옷을 입은 채 들어옵니다.", nextarg, josa(nextarg, "가"));
		printchatline(buf);
	}
#ifdef PERMS
	else if ((!strcmp(cmd, "sysopout") && HAS_PERM(PERM_SYSOP)) ||
		 (!strcmp(cmd, "cloakout") && HAS_PERM(PERM_SEECLOAK)))
#else
	else if ((!strcmp(cmd, "sysopout") && currentuser.userlevel == 255) ||
	   (!strcmp(cmd, "cloakout") && currentuser.userlevel >= SEE_CLOAK))
#endif
	{
		sprintf(buf, "  ** 알림: %s%s 마법의 옷을 입은 채 나갔습니다.", nextarg, josa(nextarg, "가"));
		printchatline(buf);
	} else if (!strcmp(cmd, "to"))
		printprivmsg(nextarg);
#ifdef PERMS
	else if (!strcmp(cmd, "nick") || (!strcmp(cmd, "inick") && HAS_PERM(PERM_SEECLOAK)) ||
		 (!strcmp(cmd, "snick") && HAS_PERM(PERM_SYSOP))) {
#else
	else if (!strcmp(cmd, "nick") || (!strcmp(cmd, "inick") && currentuser.userlevel >= SEE_CLOAK) ||
		 (!strcmp(cmd, "snick") && currentuser.userlevel == 255)) {
#endif
		sprintf(buf, "  ** 알림: '%s'님이 대화명을 '%s'%s 바꾸었습니다.", nextarg,
			advance(nextarg), josa(nextarg, "로"));
		printchatline(buf);
	}
	return 0;
}

int printchatent(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;
	static char     uline[80];
	char            pline[30];
	static int      cnt;
	int             mymode;
	switch (chatroom) {
	case 1:
		mymode = CHAT1;
		break;
	case 2:
		mymode = CHAT2;
		break;
	case 3:
		mymode = CHAT3;
		break;
	case 4:
		mymode = CHAT4;
		break;
	}
	if (!uentp) {
		if (cnt)
			printchatline(uline);
		memset(uline, 0, 80);
		cnt = 0;
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->mode != mymode)
		return 0;
#ifdef PERMS
	if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
		return 0;
#else
	if (currentuser.userlevel < SEE_CLOAK && uentp->invisible)
		return 0;
#endif
	if (kill(uentp->pid, 0) == -1)
		return 0;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	sprintf(pline, "%-10s %c%-12s", uentp->chatid, uentp->invisible ? '#' : ' ',
		utmp.userid);
	if (cnt < 2)
		strcat(pline, "   ");
	strcat(uline, pline);
	if (++cnt == 3) {
		cnt = 0;
		printchatline(uline);
		memset(uline, 0, 80);
	}
	return 0;
}

int dowho(void)
{
	int             mymode;
	char            buf[80];
	printchatline(" ");
	printchatline("  *** 이방에 있는 사람들 ***");
	sprintf(buf, "%-10s  %-12s   %-10s  %-12s   %-10s  %-12s",
		"대화명", "아이디", "대화명", "아이디", "대화명", "아이디");
	printchatline(buf);
	sprintf(buf, "%-10s  %-12s   %-10s  %-12s   %-10s  %-12s",
		"------", "------", "------", "------", "------", "------");
	printchatline(buf);
	if (apply_record(ULIST, printchatent, sizeof(struct user_info)) == -1) {
		printchatline("(아무도 없음)");
		return 0;
	}
	printchatent(NULL);
	return 0;
}

int printuserent(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;
	static char     uline[80];
	char            pline[30];
	static int      cnt;
	if (!uentp) {
		if (cnt)
			printchatline(uline);
		memset(uline, 0, 80);
		cnt = 0;
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
#ifdef PERMS
	if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
		return 0;
#else
	if (currentuser.userlevel < SEE_CLOAK && uentp->invisible)
		return 0;
#endif
	if (kill(uentp->pid, 0) == -1)
		return 0;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	sprintf(pline, "%-12s %c%-10s", utmp.userid, uentp->invisible ? '#' : ' ',
		modestring(uentp->mode, uentp->destuid, 0, NULL, uentp->mode == BBSNET ? uentp->chatid : NULL));
	if (cnt < 2)
		strcat(pline, "   ");
	strcat(uline, pline);
	if (++cnt == 3) {
		cnt = 0;
		printchatline(uline);
		memset(uline, 0, 80);
	}
	return 0;
}

int dousers(void)
{
	int             mymode;
	char            buf[80];
	printchatline(" ");
	printchatline("  *** 지금 말려있는 사람들 ***");
	sprintf(buf, "%-12s  %-10s   %-12s  %-10s   %-12s  %-10s",
		"아이디", "상태", "아이디", "상태", "아이디", "상태");
	printchatline(buf);
	sprintf(buf, "%-12s  %-10s   %-12s  %-10s   %-12s  %-10s",
		"------", "----", "------", "----", "------", "----");
	printchatline(buf);
	if (apply_record(ULIST, printuserent, sizeof(struct user_info)) == -1) {
		printchatline("(아무도 없음)");
		return 0;
	}
	printuserent(NULL);
	return 0;
}

#ifdef REALINFO
int             chat_real_names = 0;
#endif

int printlongent(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;
	char            buf[80];
	char           *field2;
#ifdef REALINFO
	if (chat_real_names)
		field2 = "별 명";
	else
#endif
		field2 = "본 명";
	if (uentp == NULL) {
		printchatline(" ");
		printchatline("  *** 지금 말려 있는 사람들 ***");
		sprintf(buf, "아이디       %s                  어디서           P %s 상태", field2,
#ifdef PERMS
			HAS_PERM(PERM_SEECLOAK) ? "C" : " ");
#else
			currentuser.userlevel >= SEE_CLOAK ? "C" : " ");
#endif
		printchatline(buf);
		sprintf(buf, "-------      ---------              ----             - %s ----",
#ifdef PERMS
			HAS_PERM(PERM_SEECLOAK) ? "-" : " ");
#else
			currentuser.userlevel >= SEE_CLOAK ? "-" : " ");
#endif
		printchatline(buf);
		return (0);
	}
	if (!uentp->active || !uentp->pid)
		return 0;
#ifdef PERMS
	if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
#else
	if (currentuser.userlevel < SEE_CLOAK && uentp->invisible)
#endif
		return 0;
	if (kill(uentp->pid, 0) == -1)
		return 0;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	utmp.username[21] = '\0';
#ifdef REALINFO
	utmp.realname[21] = '\0';
#endif
	sprintf(buf, "%-12s %-22s %-16s %c %1s %-9s", utmp.userid,
#ifdef REALINFO
		(chat_real_names ? utmp.realname : utmp.username),
#else
		utmp.username,
#endif
		uentp->from, pagerchar(currentuser.userid, utmp.userid, uentp->pager), uentp->invisible ? "#" : "",
		modestring(uentp->mode, uentp->destuid, 1, uentp->in_chat ? uentp->chatid : NULL, uentp->mode == BBSNET ? uentp->chatid : NULL));
	printchatline(buf);
	return 0;
}

/* added by skji */
/* ULIST파일을 오픈함, ULIST는 현재 로그인 한 사용자의 리스트 */
int count_chatters(int * chatters, char ** chatters_list)
{
	int             fd;
	char            tempbuf[500];
	char            namelist[CHATROOM_CNT][200];
	if ((fd = open(ULIST, O_RDONLY, 0)) != -1) {
		struct user_info *uin;
		char            buf[500];
		int             size, i;
		for (i = 0; i < CHATROOM_CNT; i++) {
			chatters[i] = 0;
			strcpy(namelist[i], "아이디:");
		}
		size = sizeof(struct user_info);
		while (read(fd, buf, size) == size) {
			uin = (struct user_info *) buf;
			switch (uin->mode) {
			case CHAT1:
				chatters[0]++;
				if (chatters_list) {
					strcat(namelist[0], uin->chatid);
					strcat(namelist[0], "/");
				}
				break;
			case CHAT2:
				chatters[1]++;
				if (chatters_list) {
					strcat(namelist[1], uin->chatid);
					strcat(namelist[1], "/");
				}
				break;
			case CHAT3:
				chatters[2]++;
				if (chatters_list) {
					strcat(namelist[2], uin->chatid);
					strcat(namelist[2], "/");
				}
				break;
			case CHAT4:
				chatters[3]++;
				if (chatters_list) {
					strcat(namelist[3], uin->chatid);
					strcat(namelist[3], "/");
				}
				break;
			}
		}

		if (chatters_list) {
			for (i = 0; i < CHATROOM_CNT; i++) {
				chatters_list[i] = (char *) strdup(namelist[i]);
			}
		}
		close(fd);
	}
	return 0;
}
int dotitle(void)
{
	int             chatters[CHATROOM_CNT], i, j;
	char            readbuf[500], tempbuf[500];
	char           *chatters_idlist[CHATROOM_CNT];
	FILE           *inf;
	count_chatters(chatters, chatters_idlist);
	for (i = 0; i < CHATROOM_CNT; i++) {
		if (chatters[i] == 0)
			change_chatroom_title(i + 1, "^_^");
	}
	printchatline("  -=* 왁자지껄방 제목 *=-");
	if ((inf = fopen(CHATROOM_TITLE, "r"))) {
		for (i = 0; i < CHATROOM_CNT; i++) {
			if (fgets(readbuf, 100, inf)) {
				int             j;
				for (j = 0; j < strlen(readbuf); j++) {
					if (readbuf[j] == '\n' || readbuf[j] == '\r') {
						readbuf[j] = 0;
						break;
					}
				}
				if (chatters[i] == 0) {
					sprintf(tempbuf, "방[%d]:", i + 1);
				} else {
					sprintf(tempbuf, "방[%d]: (%d명) \"%s\" [%s]",
						i + 1,
						chatters[i],
						readbuf,
						chatters_idlist[i]);
				}
			} else
				sprintf(tempbuf, "방[%d]: (%d명) ^_^", i, chatters[i]);
			tempbuf[80] = 0;
			printchatline(tempbuf);
			free(chatters_idlist[i]);
		}
		fclose(inf);
	}
	return 0;
}

int dolong(void)
{
	printlongent(NULL);
	if (apply_record(ULIST, printlongent, sizeof(struct user_info)) == -1)
		printchatline("(아무도 없음)");
	return 0;
}

#ifdef REALINFO
int doreal(void)
{
        if (!HAS_PERM(PERM_POST)) {
                printchatline("  ** 알림: 본명을 볼 수 있는 권한이 없습니다.");
                return 0;
        }

	chat_real_names = 1;
	dolong();
	chat_real_names = 0;
	return 0;
}
#endif

char           *msgto;
char           *umsgto;

int chatlookup(void *record_data)
{
    struct user_info * uin = record_data;

	static int      matchcnt, exact;
	static char     fullid[10], userid[15];
	struct userec   utmp;
	int             umatch, cmatch;
	if (!uin) {
		int             final = (matchcnt > 1 ? -1 : matchcnt);
		msgto = fullid;
		umsgto = userid;
		matchcnt = exact = 0;
		return final;
	}
	if (uin->mode != uinfo.mode)
		return 0;
	if (!uin->active || kill(uin->pid, 0) == -1)
		return 0;
#ifdef PERMS
	if (uin->invisible && !HAS_PERM(PERM_SEECLOAK))
		return 0;
#else
	if (uin->invisible && currentuser.userlevel < SEE_CLOAK)
		return 0;
#endif
	get_record(PASSFILE, &utmp, sizeof(utmp), uin->uid);
	cmatch = !ci_strncmp(msgto, uin->chatid, strlen(msgto));
	umatch = !ci_strncmp(msgto, utmp.userid, strlen(msgto));
	if (umatch || cmatch) {
		int             isexact = (cmatch ? (strlen(msgto) == strlen(uin->chatid)) :
				    (strlen(msgto) == strlen(utmp.userid)));
		if (matchcnt > 0) {
			if (exact && isexact) {
				matchcnt = exact = 0;
				return QUIT;
			} else if (exact && !isexact)
				return 0;
			else if (!exact && isexact)
				matchcnt = 0;
		}
		matchcnt++;
		if (isexact)
			exact++;
		strcpy(fullid, uin->chatid);
		strcpy(userid, utmp.userid);
		return 0;
	}
	return 0;
}

int doback(void)
{
        FILE           *fp;
        char            buf[100];
        struct msgRec   msgBuf;
        int             i;

        sprintf(buf, "msgtmp/%s", currentuser.userid);
        fp = fopen(buf, "rb");
        if (fp == NULL) {
                printchatline(" ** 받은 쪽지가 없습니다.");
                return 0;
        }
	printchatline(" ** 지난 쪽지 보기.");
        fread(&msgBuf, sizeof(msgBuf), 1, fp);
        fclose(fp);
        for (i=msgBuf.index; i>=0; i--){
/*
                printchatline(sprintf(buf, "%s", msgBuf.msg[i]));
*/
                printchatline(msgBuf.msg[i]);
        }
        for (i=MSGNUM-1; i>msgBuf.index; i--){
                printchatline(msgBuf.msg[i]);
        }

	return 0;
}
int dofly(char * id)
{
	char           *invitee;
	char           *text;
	int             tuid;
	struct user_info uin;

	if (!HAS_PERM(PERM_POST)) {
		printchatline("  ** 알림: 쪽지를 보낼수 있는 권한이 없습니다.");
		return 0;
	}
	/* user list를 만든다.  */
	creat_list();

	/* id를 따로 떼어 낸다. */
	while (*id == ' ' || *id == '\t' || *id == '\n')
		id++;

	/* id가 NULL이면... */
	if (*id == '\0') {
		printchatline("  ** 실수: 누구에게 보내죠?");
		return 0;
	}
	text = advance(id);
	invitee = id;
	if (*text == '\0') {
		printchatline("  ** 실수: 내용을 쓰세요.");
		return 0;
	}
	if (invitee[0] == '-' ) invitee = getLastMessageSender();
	if (invitee[0] == '=' ) invitee = getLastMessageReceiver();
	/* id가 valid하지 않으면... */
	if ( invitee[0] == '\0'|| !(tuid = getuser(invitee)) || tuid == usernum) {
		printchatline("  ** 아이디가 이상하군요 **");
		return 0;
	}
	/*
	 * tuid에 해당하는 데이터를 uin에 채우고, 몇번째 레코드인가 돌려준다. 첫번째
	 * 을 돌려줄 것임
	 */

	search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);

	/* mode check. */
	/*-bugman: 뭘하고 있든지 쪽지는 잘 날아감..
	if (uin.mode == ULDL || uin.mode == IRCCHAT ||
	    uin.mode == BBSNET || uin.mode == FOURM) {
		printchatline("  ** 상대방이 다른 일로 바쁩니다 **");
		return;
	}
	*/
	{
		FILE           *fp;
		char            buf[100];

		sprintf(buf, ">> %s님의 쪽지: %s", currentuser.userid, text);
		writeFlyingMessage(invitee, buf);
	}
	if (uin.refuse_call){
		char buf[80];
		sprintf(buf, "  ** %s님은 지금 쪽지를 받을 수 없습니다. **", invitee);
		printchatline(buf);
		return 0;
	}else if (!uin.active || (kill(uin.pid, SIGUSR2) == -1)) {
		printchatline("  ** 상대방이 지금 BBS안에 없습니다. **");
		return 0;
	}
	else{
		char            buf[80];
		sprintf(buf, "  ** %s님에게 쪽지를 보냈습니다. **", invitee);
		printchatline(buf);
	}
	return 0;
}
int doinvite(char * id)
{
	char           *invitee;
	int             tuid;
	struct user_info uin;

	/* user list를 만든다.  */
	creat_list();

	/* id를 따로 떼어 낸다. */
	while (*id == ' ' || *id == '\t' || *id == '\n')
		id++;

	/* id가 NULL이면... */
	if (*id == '\0') {
		printchatline("  ** 실수: 누구를 초대할까요?");
		return 0;
	}
	invitee = id;

	/* id가 valid하지 않으면... */
	if (!(tuid = getuser(invitee)) || tuid == usernum) {
		printchatline("  ** 아이디가 이상하군요. **");
		return 0;
	}
	/*
	 * tuid에 해당하는 데이터를 uin에 채우고, 몇번째 레코드인가 돌려준다. 첫번째을 돌려줄 것임
	 */

	search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);

	/* mode check. */
	/*- bugman: 어떤 상황이든 삐삐는 잘 날아감.
	if (uin.mode == ULDL || uin.mode == IRCCHAT ||
	    uin.mode == BBSNET || uin.mode == FOURM) {
		printchatline("  ** 상대방이 다른 일로 바쁩니다. **");
		return;
	}
	*/
#ifdef PERMS
	if (!HAS_PERM(PERM_SYSOP)) {
#else
	if (currentuser.userlevel < 255) {
#endif
		if (uin.pager == NA && !can_override(NULL, tuid, currentuser.userid)) {
			printchatline("  ** 상대방의 삐삐가 꺼져 있습니다. **");
			return 0;
		}
	}
	{
		char 		buf[100];

	 /* -bugman :          v 요 I자는 다른걸로 바꾸면 안됨 */
		sprintf(buf, "I) %s님이 %d번 이야기방에서 초대를 하고 있습니다.", currentuser.userid, chatroom);
		writeFlyingMessage(invitee, buf);
	}
	if (uin.refuse_call){
		char buf[80];
		sprintf(buf, "  ** %s님은 지금 초청장을 받을 수 없습니다. **", invitee);
		printchatline(buf);
		return 0;
	} else if (!uin.active || (kill(uin.pid, SIGUSR2) == -1)) {
		printchatline("  ** 상대방이 지금 BBS에 없습니다. **");
		return 0;
	} {
		char            buf[80];
		sprintf(buf, "  ** %s님에게 초청장을 보냈습니다. **", id);
		printchatline(buf);
	}
    return 0;
}

int domsg(char * id, int fd)
{
	char           *text;
	char            buf2[STRLEN], buf[STRLEN];
	int             verdict, retval;
	while (*id == ' ' || *id == '\t' || *id == '\n')
		id++;
	if (*id == '\0') {
		printchatline("  ** 실수: 누구에게 전할지를 말하지 않았군요.");
		return 0;
	}
	text = advance(id);
	msgto = id;
	if (*text == '\0') {
		printchatline("  ** 실수: 전할 내용이 없어요.");
		return 0;
	}
	if (apply_record(ULIST, chatlookup, sizeof(struct user_info)) == QUIT)
		verdict = -1;
	else
		verdict = chatlookup(NULL);
	switch (verdict) {
	case 0:
		printchatline("  ** 실수: 그 이름을 가진 사람은 이방에 없어요.");
		break;
	case -1:
		printchatline("  ** 실수: 애매한 이름이예요. 더 자세히 말해요!");
		break;
	default:
		sprintf(buf, "/to %s %s %s", umsgto, uinfo.chatid, text);
		buf[STRLEN - 1] = '\0';
		retval = write(fd, buf, strlen(buf) + 1);
		sprintf(buf2, ">> %s님에게 '%s'라고 속삭입니다.", msgto, text);
		printchatline(buf2);
	}
	msgto = NULL;
	if (retval > -1)
		retval = 0;
	return retval;
}
int chat_query(char * id, int fd)
{
	char            inbuf[STRLEN * 2], *newline;
	FILE           *planfile;
	char            buf2[150], buf[STRLEN];
	int             verdict, retval, i;
	while (*id == ' ' || *id == '\t' || *id == '\n')
		id++;
	if (*id == '\0') {
		printchatline("  ** 실수: 누가 궁금하세요?");
		return 0;
	}
	msgto = id;

	if (apply_record(ULIST, chatlookup, sizeof(struct user_info)) == QUIT)
		verdict = -1;
	else
		verdict = chatlookup(NULL);
	switch (verdict) {
	case 0:
		if ((verdict = getuser(id)) == 0)
			printchatline("0** 실수: 없는 이름입니다.");
		break;
	case 1:
		if ((verdict = getuser(umsgto)) == 0)
			printchatline("1** 실수: 없는 이름입니다.");
		break;
	case -1:
		printchatline("  ** 실수: 애매한 이름이예요. 더 자세히 말해요!");
		verdict = 0;
	}
	if (verdict == 0)
		return 0;
#ifdef PERMS
	if ((PERM_SYSOP) ? lookupuser.userlevel & PERM_SYSOP : 1)
#else
	if (lookupuser.userlevel == 255)
#endif
	{
		sprintf(buf, "/to %s %s %s%s", lookupuser.userid, "보디가드", uinfo.chatid, "님이 댁을 궁금해 하고 있습니다.");
		buf[STRLEN - 1] = '\0';
		retval = write(fd, buf, strlen(buf) + 1);
	}
	printchatline(" ");
	sprintf(buf2, "* %s (%s), 방문 %d번, 올린글 %d개.\n", lookupuser.userid,
	    lookupuser.username, lookupuser.numlogins, lookupuser.numposts);
	printchatline(buf2);
/*
	strcpy(genbuf, ctime(&(lookupuser.lastlogin)));
	if (newline = index(genbuf, '\n'))
		*newline = '\0';
*/
	sprintf(buf2, "* 마지막 방문 %s (들어온곳 : %s)\n",
		time_string_no_newline(lookupuser.lastlogin),
		(lookupuser.lasthost[0] == '\0' ? "모름" : lookupuser.lasthost));
	printchatline(buf2);
#if defined(REALINFO) && defined(QUERY_REALNAMES)
#ifdef PERMS
	if (HAS_PERM(PERM_PAGE))
#endif
	{
		sprintf(buf2, "* 본  명: %s \n", lookupuser.realname);
		printchatline(buf2);
		sprintf(buf2, "* 소  속: %s\n", lookupuser.address);
		printchatline(buf2);
	}
#endif
/* 재민이가 지웠다. 챗방에서 plan 안나오게
	sprintf(genbuf, "plans/%s", lookupuser.userid);
	if ((planfile = fopen(genbuf, "r")) == NULL)
		printchatline("허탕치셨습니다\n");
	else {
		printchatline("소개말:\n\n");
		for (i = 1; i <= MAXQUERYLINES; i++) {
			if (fgets(inbuf, sizeof(inbuf), planfile))
				printchatline(inbuf);
			else
				break;
		}
		fclose(planfile);
	}
*/
	printchatline(" ");
	msgto = NULL;
	return 0;
}

int donick(char * newname, int fd)
{
	char           *p;
	char            buf[40];
	int             retval;
	while (*newname == ' ' || *newname == '\t' || *newname == '\n')
		newname++;
	if (*newname == '\0') {
		printchatline("  ** 실수: 이름을 정하세요.");
		return 0;
	}
	for (p = newname; *p && *p != ' ' && *p != '\t' && *p != '\n'; p++);
	*p = '\0';
	newname[CHATIDLEN] = '\0';
	fixchatid(newname);
	if (!uinfo.invisible)
		sprintf(buf, "/nick %s %s", uinfo.chatid, newname);
	else {
#ifdef PERMS
		if (HAS_PERM(PERM_SYSOP))
#else
		if (currentuser.userlevel == 255)
#endif
			sprintf(buf, "/snick %s %s", uinfo.chatid, newname);
		else
			sprintf(buf, "/inick %s %s", uinfo.chatid, newname);
	}
	retval = write(fd, buf, strlen(buf) + 1);
	strcpy(uinfo.chatid, newname);
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	if (retval > -1)
		retval = 1;	/*- cause t_chat to update chatid */
	return retval;
}

int dochatcommand(char * cmd, int fd, char * goodbye_msg)
{
	char           *endcmd;
	int             retval = 0, cmdlen;
	while (*cmd == ' ')
		cmd++;
	endcmd = cmd;
	while (*endcmd != ' ' && *endcmd != '\n' && *endcmd)
		endcmd++;
	if (*endcmd == '\0')
		*(endcmd + 1) = '\0';
	else
		*endcmd = '\0';
	cmdlen = strlen(cmd);
	if (!ci_strncmp(cmd, "help", cmdlen) ||
	    !ci_strncmp(cmd, "도움말", cmdlen) ||
	    !ci_strncmp(cmd, "ㄷ", cmdlen)) {

		printchatline(" ");
		printchatline("왁자지껄방 도움말: '/' 대신 '-'도 가능합니다. ex) -help");
		printchatline("  /help            [/도움말]     - 지금 보시는 내용 ");
		printchatline("  /who             [/누구]       - 이방에 있는 사람 ");
		printchatline("  /users           [/사람들]     - 있는 사람 ");
		printchatline("  /long            [/자]         - /사람들 자세히 ");
#ifdef REALINFO
		printchatline("  /real            [/이름]       - /본명보기 ");
#endif
		printchatline("  /to ID TEXT      [/귓속말]     - 특정인물에게만 말하기");
		/* <--- by wings */
		printchatline("  /invite ID       [/초대]       - 특정인물을 방에 초대");
		/* by wings ---> */
#ifdef PERMS
		if (HAS_PERM(PERM_BASIC))
#endif
			printchatline("  /pager           [/삐삐]       - 삐삐 끄기/켜기 ");
		printchatline("  /nick NAME       [/별명]     - 대화명 바꾸기 ");
		printchatline("  /me ACTION       [/나(조사)]   - 자기 행동 표현 ");
		printchatline("  /clear           [/지우기]     - 화면 지우기 ");
		printchatline("  /fly ID TEXT     [/날리기]     - 쪽지 날리기 ");
		printchatline("  /title           [/제목]       - 방 제목 보기 ");
#ifdef PERMS
		if (HAS_PERM(PERM_CHATCLOAK) && HAS_PERM(PERM_CLOAK))
#else
		if (currentuser.userlevel >= CLOAK)
#endif
			printchatline("  /cloak           [/뿅]         - 귀신 되기 ");
		printchatline("  /query           [/궁금해] ");
		printchatline("  /back            [/전]         - 지난 쪽지보기");
		printchatline("  /quit <인사말>   [/끝 /나가자] - 인사하고 문닫고 나가기");
		printchatline("  /<ctrl-d>                      - 문도 안닫고 나가기 ");
		printchatline(" ");
	} else if (!ci_strncmp(cmd, "who", cmdlen) ||
		   !ci_strncmp(cmd, "st", cmdlen) ||
		   !ci_strncmp(cmd, "누구", cmdlen) ||
		   !ci_strncmp(cmd, "ㄴ", cmdlen))
		dowho();
	else if (!ci_strncmp(cmd, "users", cmdlen) ||
		 !ci_strncmp(cmd, "사람들", cmdlen) ||
		 !ci_strncmp(cmd, "ㅅ", cmdlen))
		dousers();
	/* added by skji */
	else if (!ci_strncmp(cmd, "title", cmdlen) ||
		 !ci_strncmp(cmd, "제목", cmdlen))
		dotitle();
	/* 여기까지 */

	else if (!ci_strncmp(cmd, "long", cmdlen) ||
		 !ci_strncmp(cmd, "자", cmdlen) ||
		 !ci_strncmp(cmd, "ㄹ", cmdlen))
		dolong();
#ifdef REALINFO
	else if (!ci_strncmp(cmd, "real", cmdlen) ||
		 !ci_strncmp(cmd, "이름", cmdlen) ||
		 !ci_strncmp(cmd, "ㅇ", cmdlen) ||
		 !ci_strncmp(cmd, "ㄱ", cmdlen))
		doreal();
#endif
	/* <--- by wings */
	else if (!ci_strncmp(cmd, "invite", cmdlen) ||
		 !ci_strncmp(cmd, "in", cmdlen) ||
		 !ci_strncmp(cmd, "초대", cmdlen) ||
		 !ci_strncmp(cmd, "ㅊ", cmdlen))
		doinvite(endcmd + 1);
	else if (!ci_strncmp(cmd, "fly", cmdlen) ||
/*		 !ci_strncmp(cmd, "to", cmdlen) ||*/
		 !ci_strncmp(cmd, "f", cmdlen) ||
		 !ci_strncmp(cmd, "날리기", cmdlen) ||
		 !ci_strncmp(cmd, "쪽지", cmdlen) ||
		 !ci_strncmp(cmd, "쪽", cmdlen) ||
		 !ci_strncmp(cmd, "날", cmdlen))
		dofly(endcmd + 1);
	/* by wings ---> */
	else if (!ci_strncmp(cmd, "msg", cmdlen) ||
		 !ci_strncmp(cmd, "to", cmdlen) ||
		 !ci_strncmp(cmd, "ear", cmdlen) ||
		 !ci_strncmp(cmd, "귓속말", cmdlen) ||
		 !ci_strncmp(cmd, "귀속말", cmdlen) ||
		 !ci_strncmp(cmd, "삭", cmdlen) ||
		 !ci_strncmp(cmd, "말", cmdlen))
		retval = domsg(endcmd + 1, fd);
	else if (!ci_strncmp(cmd, "nick", cmdlen) ||
		 !ci_strncmp(cmd, "al", cmdlen) ||
		 !ci_strncmp(cmd, "대화명", cmdlen) ||
		 !ci_strncmp(cmd, "별명", cmdlen))
		retval = donick(endcmd + 1, fd);
	else if (!ci_strncmp(cmd, "pager", cmdlen) || !ci_strncmp(cmd, "삐삐", cmdlen)) {
#ifdef PERMS
		if (HAS_PERM(PERM_PAGE)) {
#endif
			t_pager();
			printchatline(" ");
			if (!uinfo.pager)
				printchatline("  *** 삐삐를 껐습니다(OFF)");
			else
				printchatline("  *** 삐삐를 켰습니다(ON)");
#ifdef PERMS
		} else
			printchatline("  ** 실수: 이상한 명령어입니다.");
#endif
	} else if (!ci_strncmp(cmd, "me", cmdlen) ||
		   (!strncmp(cmd, "나", 2) && strncmp(cmd + 2, "가", 2)) ||
		   (!strncmp(cmd, "내", 2) && !strncmp(cmd + 2, "가", 2))) {
		char            actionbuf[80], *firstlet = endcmd + 1;

		while (*firstlet == ' ' || *firstlet == '\n' || *firstlet == '\t')
			firstlet++;
		if (*firstlet != '\0') {
			sprintf(actionbuf, "    -*( %s%s %s )*-", uinfo.chatid, josa(uinfo.chatid, (cmd[2] & 0x80) ? cmd + 2 : ""), endcmd + 1);
			write(fd, actionbuf, strlen(actionbuf) + 1);
		} else
			printchatline("  ** 실수: 행동을 표현해야 합니다.");
	} else if (!ci_strncmp(cmd, "clear", cmdlen) || !ci_strncmp(cmd, "지우기", cmdlen)) {
		char            bufr[80];
		clear();
		move(echatwin, 0);
		prints("--------------------------------------------------------------------------------");
		chatline = 0;
		sprintf(bufr, "%s 왁자지껄방 %d번방 -- '/도움말'을 하시면 기능설명서", BoardName, chatroom);
		printchatline(bufr);
		printchatline("  ");
		/* by wings ---> */
	} else if (!ci_strncmp(cmd, "msg", cmdlen) ||
/*		   !ci_strncmp(cmd, "to", cmdlen) || */
		   !ci_strncmp(cmd, "귓속말", cmdlen) ||
		   !ci_strncmp(cmd, "귀속말", cmdlen) ||
		   !ci_strncmp(cmd, "삭", cmdlen) ||
		   !ci_strncmp(cmd, "말", cmdlen))
		retval = domsg(endcmd + 1, fd);
	else if (!ci_strncmp(cmd, "nick", cmdlen) ||
		 !ci_strncmp(cmd, "별명", cmdlen))
		retval = donick(endcmd + 1, fd);
	else if (!ci_strncmp(cmd, "pager", cmdlen) || !ci_strncmp(cmd, "삐삐", cmdlen)) {
#ifdef PERMS
		if (HAS_PERM(PERM_PAGE)) {
#endif
			t_pager();
			printchatline(" ");
			if (!uinfo.pager)
				printchatline("  *** 삐삐를 껐습니다(OFF)");
			else
				printchatline("  *** 삐삐를 켰습니다(ON)");
#ifdef PERMS
		} else
			printchatline("  ** 실수: 이상한 명령어입니다.");
#endif
	} else if (!ci_strncmp(cmd, "me", cmdlen) ||
		   (!strncmp(cmd, "나", 2) && strncmp(cmd + 2, "가", 2)) ||
		   (!strncmp(cmd, "내", 2) && !strncmp(cmd + 2, "가", 2))) {
		char            actionbuf[80], *firstlet = endcmd + 1;

		while (*firstlet == ' ' || *firstlet == '\n' || *firstlet == '\t')
			firstlet++;
		if (*firstlet != '\0') {
			sprintf(actionbuf, "    -*( %s%s %s )*-", uinfo.chatid, josa(uinfo.chatid, (cmd[2] & 0x80) ? cmd + 2 : ""), endcmd + 1);
			write(fd, actionbuf, strlen(actionbuf) + 1);
		} else
			printchatline("  ** 실수: 행동을 표현해야 합니다.");
	} else if (!ci_strncmp(cmd, "clear", cmdlen) || !ci_strncmp(cmd, "지우기", cmdlen)) {
		char            bufr[80];
		clear();
		move(echatwin, 0);
		prints("--------------------------------------------------------------------------------");
		chatline = 0;
		sprintf(bufr, "%s 왁자지껄방 %d번방 -- '/도움말'을 하시면 기능설명서", BoardName, chatroom);
		printchatline(bufr);
		printchatline("  ");
	} else if (!ci_strncmp(cmd, "date", cmdlen) || !ci_strncmp(cmd, "시간", cmdlen)) {
		char            bufr[80];
		time_t          thetime;
		time(&thetime);
		/*
		 * sprintf(bufr, "  *** 지금 시간은 : %s", ctime(&thetime));
		 */
		sprintf(bufr, "  *** 지금 시간은 : %s", time_string(thetime));
		printchatline(bufr);
	} else if (!ci_strncmp(cmd, "cloak", cmdlen) || !ci_strncmp(cmd, "뿅", cmdlen)) {
#ifdef PERMS
		if (HAS_PERM(PERM_CHATCLOAK) && HAS_PERM(PERM_CLOAK)) {
#else
		if (currentuser.userlevel >= CLOAK) {
#endif
			x_cloak();
			if (!uinfo.invisible)
				printchatline("  *** 마법의 외투를 벗었습니다");
			else
				printchatline("  *** 마법의 외투를 입었습니다");
		} else
			printchatline("  ** 실수: 이상한 명령어여..");
	} else if (!ci_strncmp(cmd, "quit", cmdlen) ||
		   !ci_strncmp(cmd, "끝", cmdlen) ||
		   !ci_strncmp(cmd, "ㅂ", cmdlen) ||
		   !ci_strncmp(cmd, "나가자", cmdlen)) {
		strcpy(goodbye_msg, (char *) (endcmd + 1));	/*- chopin */
		return -2;
	} else if (!ci_strncmp(cmd, "query", cmdlen) ||
		   !ci_strncmp(cmd, "pf", cmdlen) ||
		   !ci_strncmp(cmd, "궁금해", cmdlen)) {
		retval = chat_query(endcmd + 1, fd);
	} else if (!ci_strncmp(cmd, "전", cmdlen) ||
		   !ci_strncmp(cmd, "back", cmdlen)) {
		doback();
	} else
		printchatline("  ** 실수: 아니 그런 명령어도 있었나?");
	return retval;
}

int ent_chat1(void)
{
	int             savecloak = uinfo.invisible;
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = NA;
	chatroom = 1;
	t_chat();
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = savecloak;
	return 0;
}

int ent_chat2(void)
{
	int             savecloak = uinfo.invisible;
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = NA;
	chatroom = 2;
	t_chat();
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = savecloak;
	return 0;
}

#ifdef CHAT3_CLOAKED
int ent_chat3(void)
{
	int             savecloak = uinfo.invisible;
	uinfo.invisible = YEA;
	chatroom = 3;
	t_chat();
	uinfo.invisible = savecloak;
}
#else

int ent_chat3(void)
{
	int             savecloak = uinfo.invisible;
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = NA;
	chatroom = 3;
	t_chat();
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = savecloak;
	return 0;
}
#endif				/*- CHAT3_CLOAKED */

#ifdef CHAT4_CLOAKED
int ent_chat4(void)
{
	int             savecloak = uinfo.invisible;
	uinfo.invisible = YEA;
	chatroom = 6;
	t_chat();
	uinfo.invisible = savecloak;
}
#else
int ent_chat4(void)
{
	int             savecloak = uinfo.invisible;
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = NA;
	chatroom = 4;
	t_chat();
#ifdef PERMS
	if (!HAS_PERM(PERM_CHATCLOAK))
#else
	if (currentuser.userlevel < 255)
#endif
		uinfo.invisible = savecloak;
	return 0;
}
#endif				/*- CHAT4_CLOAKED */
