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
#include <unistd.h>
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define M_INT 3			/*- monitor mode update interval */
#define P_INT 20		/*- interval to check for page req. in talk/chat */

#ifdef REALINFO
int             real_user_names = 0;
#endif

char lastMessageReceiver[20] = {0,};

extern char     gb[];

int ishidden(char * user)
{
	int             tuid;
	struct userec   saverec;
	struct user_info uin;
	memmove(&saverec, gb, sizeof(saverec));
	if (!(tuid = getuser(user)))
		return 0;
	search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);
	memmove(gb, &saverec, sizeof(saverec));
	return (uin.invisible);
}

char * modestring(int mode, int towho, int complete, char * chatid, char * action)
{
	static char     modestr[STRLEN];
	struct userec   urec;
	if (mode == BBSNET && action != NULL) {
		if (action[0] != '\0') {
			if (complete)
				sprintf(modestr, "%s", action);
			else
				return ModeType(mode);
			return (modestr);
		}
	}
	if (chatid) {
		if (complete)
			if (mode == TALK) {
				sprintf(modestr, "%s /%s/", ModeType(mode), chatid);
			} else {
				sprintf(modestr, "%s <%s>", ModeType(mode), chatid);
			}
		else
			return (ModeType(mode));
		return (modestr);
	}
	if (mode != TALK && mode != PAGE && mode != QUERY)
		return (ModeType(mode));
	if (get_record(PASSFILE, &urec, sizeof(urec), towho) == -1)
		return (ModeType(mode));
#ifdef PERMS
	if (mode != QUERY && !HAS_PERM(PERM_SEECLOAK) &&
#else
	if (mode != QUERY && currentuser.userlevel < SEE_CLOAK &&
#endif
	    ishidden(urec.userid))
		return (ModeType(TMENU));
	if (complete)
		if (mode == TALK) {
			sprintf(modestr, "%s /%s/", ModeType(mode), urec.userid);
		} else {
			sprintf(modestr, "%s <%s>", ModeType(mode), urec.userid);
		}
	/*
	 * sprintf(modestr, "%s <%s>", ModeType(mode), urec.userid);
	 */
	else
		return (ModeType(mode));
	return (modestr);
}

char pagerchar(char * me, char * them, int pager)
{
	if (pager)
		return 0;
	else if (can_override(them, -1, me))
		return 0;
	else
		return 0;
}

int printcuent(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;
	static int      i;
	char           *field_2;
#ifdef REALINFO
	if (real_user_names)
		field_2 = "본 명";
	else
#endif
		field_2 = "별 명";
	if (uentp == NULL) {
		move(3, 0);
		clrtobot();
		move(3, 0);
		standout();	/*-chopin*/
		prints("%-12s %-20s %-20s %-1s %-1s %-19s\n", "아이디 ", field_2,
#ifdef PERMS
		       "들어온 곳", "P", (HAS_PERM(PERM_SEECLOAK) ? "C" : " "), "상  태");
#else
		       "들어온 곳", "P", currentuser.userlevel >= SEE_CLOAK ? "C" : " ", "상  태");
#endif
		standend();	/*-chopin*/
		i = 3;
		return 0;
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
	if (i == 22) {
		int             ch;
		standout();
		prints("--계속--");
		standend();
		clrtoeol();
		while ((ch = igetch()) != EOF) {
			if (ch == '\n' || ch == '\r' || ch == ' ')
				break;
			bell();
		}
		move(3, 0);
		prints("%-12s %-20s %-20s %-1s %-1s %-19s\n", "아이디", field_2,
#ifdef PERMS
		       "들어온 곳", "P", (HAS_PERM(PERM_SEECLOAK) ? "C" : " "), "상  태");
#else
		       "들어온 곳", "P", currentuser.userlevel >= SEE_CLOAK ? "C" : " ", "상  태");
#endif
		i = 3;
		clrtobot();
	}
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
#ifdef REALINFO
	if (real_user_names)
		prints("%-12s %-20s %-20s %c %1s %-20s\n", utmp.userid,
		       utmp.realname, uentp->from, pagerchar(currentuser.userid, utmp.userid, uentp->pager),
		       (uentp->invisible ? "#" : " "), modestring(uentp->mode, uentp->destuid, 1, (uentp->in_chat ? uentp->chatid : NULL), (uentp->mode == BBSNET ? uentp->chatid : NULL)));
	else
#endif
		prints("%-12s %-20s %-20s %c %1s %-20s\n", utmp.userid,
		       utmp.username, uentp->from, pagerchar(currentuser.userid, utmp.userid, uentp->pager),
		       (uentp->invisible ? "#" : " "), modestring(uentp->mode, uentp->destuid, 1, (uentp->in_chat ? uentp->chatid : NULL), (uentp->mode == BBSNET ? uentp->chatid : NULL)));
	i++;
	return 0;
}

int listcuent(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;

	if (uentp == NULL) {
		CreateNameList();
		return 0;
	}
	if (uentp->uid == usernum)
		return 0;
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->mode == ULDL)
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
	AddNameList(utmp.userid);
	return 0;
}

extern int      bfinger;

int dumb_list(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;

	if (uentp == NULL) {
#ifdef BOARDNAME
		if (bfinger) {
			sprintf(genbuf, "\n[%s]\n\n", BOARDNAME);
			write(1, genbuf, strlen(genbuf));
		}
#endif
		sprintf(genbuf, "%-14s %-25s %-10s %-10s %-16s\n", "아이디 ", "별  명",
			"Term", "상태", "들어온 곳");
		write(1, genbuf, strlen(genbuf));
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (kill(uentp->pid, 0) == -1)
		return 0;
	if (bfinger && uentp->invisible)
		return 0;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	sprintf(genbuf, "%-14s %-25s %-10s %-10s %s\n", utmp.userid, utmp.username
		,utmp.termtype, modestring(uentp->mode, uentp->destuid, 0, NULL, NULL), uentp->from);
	write(1, genbuf, strlen(genbuf));
	return 1;
}

int creat_list(void)
{
	listcuent(NULL);
	/* apply_record()는 record.c 에 있음 - ohno */
	apply_record(ULIST, listcuent, sizeof(struct user_info));
	return 0;
}

int t_users(void)
{
	int             savemode = uinfo.mode;
	uinfo.mode = LUSERS;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	/*-	report("talk user list") ; */
	printcuent(NULL);
	/* apply_record()는 record.c 에 있음 - ohno */
	if (apply_record(ULIST, printcuent, sizeof(struct user_info)) == -1) {
		prints("사용자가 없습니다.\n");
		pressreturn();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		pressanykey();
		return 0;
	}
	clrtobot();
	uinfo.mode = savemode;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pressanykey();
	return 0;
}

#ifdef REALINFO
int t_rusers(void)
{
	real_user_names = 1;
	t_users();
	real_user_names = 0;
	return 0;
}
#endif

int t_pager(void)
{				/*-     Pager Toggle   --gtv     */
	if (uinfo.pager == YEA) {
		if (!uinfo.in_chat) {
			move(2, 0);
			prints("삐삐를 껐습니다(OFF)\n");
			report("pager off");
			uinfo.pager = NA;
			/* clrtobot(); <--- deleted by skji */
		} else
			uinfo.pager = NA;
	} else {
		if (!uinfo.in_chat) {
			move(2, 0);
			prints("삐삐를 켰습니다(ON)\n");
			report("pager on");
			uinfo.pager = YEA;
			/* clrtobot(); <--- delete by skji */
		} else
			uinfo.pager = YEA;
	}
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pressanykey();
	return 0;
}

char * getLastMessageReceiver(void)
{
	return &(lastMessageReceiver[0]);
}
int writeFlyingMessage(char * id, char * msg)
{
	int fd,i;
	char buf[100];
	struct stat stBuf;
	struct msgRec msgBuf;

	strcpy(lastMessageReceiver, id);
        sprintf(buf, "msgtmp/%s", id);
	if (stat(buf, &stBuf) <0 || stBuf.st_size == 0 || stBuf.st_size != sizeof(msgBuf)){
		fd = open(buf, O_CREAT | O_RDWR, 0660);
		memset(&msgBuf, 0, sizeof(msgBuf));
	} else {
        	fd = open(buf, O_RDWR);
		read(fd, &msgBuf, sizeof(msgBuf));
	}
	flock(fd, LOCK_EX);
	msgBuf.index ++;
	msgBuf.index %= MSGNUM;
	for(i=0; msg[i] && i < 79; i++){
		msgBuf.msg[msgBuf.index][i] = msg[i];
	}
	for(;i<79;  i++)
		msgBuf.msg[msgBuf.index][i] = '-';
	msgBuf.msg[msgBuf.index][i] = '\0';
	time ( &(msgBuf.time[msgBuf.index]));
	lseek(fd, 0, SEEK_SET);
	write(fd, &msgBuf, sizeof(msgBuf));
	flock(fd, LOCK_UN);
        close(fd);

	return YEA;
}

int t_fly(void)
{
	char            uident[STRLEN];
	int             tuid, i;
	char            msgBuf[100];
	char            buf[100];
	struct user_info uin;
	int		fd;

	uinfo.mode = FLY;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	move(2, 0);
	prints("<쪽지를 받을 사람을 정해주세요>\n");
	move(1, 0);
	clrtoeol();
	prints("받을사람: ");
	creat_list();
	namecomplete(NULL, uident);
	if (uident[0] == '\0') {
		clear();
		return -1;
	}
	if (!(tuid = getuser(uident)) || tuid == usernum) {
		move(2, 0);
		prints("아이디가 이상합니다.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return -1;
	}
	search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);
/*- 거의 모든 상황에서 쪽지가 잘 날라가는 것같음..
	switch (uin.mode) {
	case ULDL:
	case IRCCHAT:
	case BBSNET:
	case FOURM:
	case SMAIL: case RMAIL: case POSTING: case READING: case MONITOR:
	case EDITSIG: case EDITPLAN: case QUERY: case CNTBRDS:
	case VOTING: case VISIT: case PAGE: case LAUSERS: case NEW:
	case READNEW:
		move(2, 0);
		prints("  ** 상대방이 다른 일로 바쁩니다 **");
		clrtoeol();
		pressanykey();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return;
	}
*/
	if (uin.refuse_call){
		char buf[80];
		move(2, 0);
		sprintf(buf, "%s님은 지금 쪽지를 받을 수 없습니다.\n", uident);
		prints(buf);
		pressreturn();
		move(2, 0);
		clrtoeol();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return -1;
	}

	move(4, 0);
	clrtobot();
	getdata(4, 0, "쪽지내용 : ", msgBuf, 60, DOECHO, NULL);
	if (msgBuf[0] == '\0'){
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return -1;
	}
	sprintf(buf, ">> %s님의 쪽지: %s", currentuser.userid, msgBuf);
	writeFlyingMessage(uident, buf);

	if (!uin.active || (kill(uin.pid, SIGUSR2) == -1)) {
		move(2, 0);
		prints("상대방이 지금 이 곳에 없습니다.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return -1;
	}
	move(6, 0);
	prints("%s님에게 쪽지를 보냈습니다.\n", uident);

	pressreturn();
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int t_back(void)
{
	FILE           *fp;
	char            buf[100];
	struct msgRec 	msgBuf;
	int		i;

	uinfo.mode = BACK;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	sprintf(buf, "msgtmp/%s", currentuser.userid);
	fp = fopen(buf, "rb");
	move(2, 0);
	if (fp == NULL) {
		prints("아무 쪽지도 없습니다.");
		pressanykey();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return 0;
	}
	fread(&msgBuf, sizeof(msgBuf), 1, fp);
	fclose(fp);
	clrtobot();
	prints("\n");
	for (i=msgBuf.index; i>=0; i--){
		standout();
		prints("^^ %s ^^;\n", time_string_no_newline(msgBuf.time[i]));
		standend();
		prints("%s\n\n", msgBuf.msg[i]);
	}
	for (i=MSGNUM-1; i>msgBuf.index; i--){
		standout();
		prints("^^ %s ^^;\n", time_string_no_newline(msgBuf.time[i]));
		standend();
		prints("%s\n\n", msgBuf.msg[i]);
	}
	pressanykey();

	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

char            msgBuf[100];
int shoutent(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;
	char            buf[100];
	FILE           *fp;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);

	sprintf(buf, "!! %s님이 모두에게: %s", currentuser.userid, msgBuf);
	if (uentp->active && !uentp->refuse_call) {
		writeFlyingMessage(utmp.userid, buf);
		kill(uentp->pid, SIGUSR2);
		move(3,0);
		prints("[%12s] 님에게 보냅니다.", utmp.userid);
		refresh();
	}
	return 0;
}
int t_shout(void)
{
	FILE           *fp;
	char            buf[100];

	uinfo.mode = SHOUT;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	move(3, 0);
	prints("모두에게 전달할 쪽지 내용을 적어주세요");
	getdata(2, 0, "쪽지내용:", msgBuf, 60, DOECHO, NULL);
	if (msgBuf[0] == '\0') {
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return 0;
	}
	if (apply_record(ULIST, shoutent, sizeof(struct user_info)) == -1) {
		prints("사용자가 없습니다.\n");
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		pressanykey();
		return 0;
	}
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	return 0;
}

int t_attach(void)
{
        char            uident[STRLEN], inbuf[STRLEN * 2], tempbuf[100];
        int             tuid;
	time_t		tt;
        FILE           *memofp;

        uinfo.mode = MEMO;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
        move(2, 0);
        clrtoeol();
        prints("아이디를 넣으세요.");
        move(1, 0);
        clrtoeol();
        prints("아이디: ");
        u_namelist();
        namecomplete(NULL, uident);
        if (uident[0] == '\0') {
                clear();
                uinfo.mode = TMENU;
                substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
                return 0;
        }
        if (!(tuid = getuser(uident))) {
                move(2, 0);
                prints("아이디가 이상합니다.\n");
                pressreturn();
                move(2, 0);
                clrtoeol();
                uinfo.mode = TMENU;
                substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
                return -1;
        }
	uinfo.destuid = tuid;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
        clrtobot();
	getdata(3, 0, "남길 메모 : ", inbuf,  70, DOECHO, NULL);
	if (inbuf[0] == '\0'){
                clear();
                uinfo.mode = TMENU;
                substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return 0;
	}
	sprintf(tempbuf, "memo/%s", lookupuser.userid);
	memofp = fopen(tempbuf, "at");
	time(&tt);
	fprintf(memofp, "%s 님이 %s에 남긴 메모 : \n", currentuser.userid, time_string_short_no_newline(tt));
	fprintf(memofp, ">> %s\n\n", inbuf);
	fclose(memofp);
        uinfo.mode = TMENU;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int t_wait_user(void)
{
        char            uident[STRLEN], inbuf[STRLEN * 2], tempbuf[100];
        int             tuid;
	time_t		tt;
        FILE           *fp;
	struct user_info uin;

        uinfo.mode = WAIT;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
        move(2, 0);
        clrtoeol();
        prints("누구를 기다릴 래요? ");
        move(1, 0);
        clrtoeol();
        prints("아이디: ");
        u_namelist();
        namecomplete(NULL, uident);
        if (uident[0] == '\0') {
                clear();
                uinfo.mode = TMENU;
                substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
                return 0;
        }
        if (!(tuid = getuser(uident))) {
                move(2, 0);
                prints("아이디가 이상합니다.\n");
                pressreturn();
                move(2, 0);
                clrtoeol();
                uinfo.mode = TMENU;
                substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
                return -1;
        }
	if (search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid)){
		if (uin.active){
                	move(4, 0);
                	prints("%s 님은 현재 사용중 입니다. \n", uident);
			clrtobot();
			pressanykey();
        		uinfo.mode = TMENU;
        		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
			return 0;
		}
	}
	uinfo.destuid = tuid;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
        clrtobot();

	sprintf(tempbuf, "wait_me/%s", lookupuser.userid);
	fp = fopen(tempbuf, "rt");
	if (fp != NULL){
		while(fscanf(fp, "%s", inbuf) == 1){
			if (!strcmp(inbuf, currentuser.userid)){
				fclose(fp);
				move(4, 0);
				prints("이미 %s님을 기다리고 있습니다.", lookupuser.userid);
				pressanykey();
        			uinfo.mode = TMENU;
        			substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
				return 0;
			}
		}
		fclose(fp);
	}
	fp = fopen(tempbuf, "at");
	fprintf(fp, "%s\n", currentuser.userid);
	fclose(fp);
	move(4, 0);
	prints("%s님을 기다립니다.", lookupuser.userid);
	pressanykey();

        uinfo.mode = TMENU;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int t_query(void)
{
	char            uident[STRLEN], inbuf[STRLEN * 2], *newline, tempbuf[100];
	extern char     currmaildir[4096];
	int             tuid, i;
	FILE           *planfile;

	uinfo.mode = QUERY;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	move(2, 0);
	clrtoeol();
	prints("아이디를 넣으세요.");
	move(1, 0);
	clrtoeol();
	prints("아이디: ");
	u_namelist();
	namecomplete(NULL, uident);
	if (uident[0] == '\0') {
		clear();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return 0;
	}
	if (!(tuid = getuser(uident))) {
		move(2, 0);
		prints("아이디가 이상합니다.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return -1;
	}
	uinfo.destuid = tuid;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	prints("-------------------------------------------------------------------------------\n");
	move(4, 0);
	clrtobot();
	sprintf(genbuf, "query %s", lookupuser.userid);
	report(genbuf);
	sprintf(tempbuf, "%s (%s)", lookupuser.userid, lookupuser.username);
	prints("아이디: %-30s", tempbuf);
	prints("이메일: %-30s\n", lookupuser.email);
	prints("방  문: %-30d", lookupuser.numlogins);
	prints("올린글: %-30d\n", lookupuser.numposts);

	/*-
	    prints("%s (%s), 방문 %d번, 올린글 %d개, Email:%s\n",
			lookupuser.userid,
		   	lookupuser.username,
			lookupuser.numlogins,
			lookupuser.numposts,
			lookupuser.email);
	*/
	/*-skji strcpy(genbuf, ctime(&(lookupuser.lastlogin))); */
	strcpy(genbuf, time_string(lookupuser.lastlogin));	/* 한글스트링 */
	/*
	 * by sangil  -  error 교정  strcpy(genbuf, time_string(time(0)));
	 */
	if (newline = strchr(genbuf, '\n'))
		*newline = '\0';
	prints("마지막: %-30s", genbuf);
	prints("어디서: %-30s\n", (lookupuser.lasthost[0] == '\0' ?
				   "" : lookupuser.lasthost));
#if defined(REALINFO) && defined(QUERY_REALNAMES)
#ifdef PERMS
	if (HAS_PERM(PERM_POST))
#endif
		prints("본  명: %-30s", lookupuser.realname);
#endif
	{
		int             saved_ismail;
		long            saved_lasttime;
		extern int      ismail;
		extern long     lasttime;
		/* defined in fastnew.c */
		saved_ismail = ismail;
		saved_lasttime = lasttime;
		sprintf(currmaildir, "mail/%s/%s", lookupuser.userid, DIR);
		lasttime = 0;
		if (chkmail())	/* query할 사람의 메일 플랙을 세팅 */
			prints("새 편지가 있습니다.\n");
		else
			prints("새 편지 없음.\n");
		sprintf(currmaildir, "mail/%s/%s", currentuser.userid, DIR);
		ismail = saved_ismail;
		lasttime = saved_lasttime;
		chkmail();
	}
	/* chkmail(); 나의 메일 플랙을 세팅 */
#if defined(REALINFO) && defined(QUERY_REALNAMES)
#ifdef PERMS
	if (HAS_PERM(PERM_POST))
#endif
		prints("소  속: %-70s\n", lookupuser.address);
#endif
	prints("-------------------------------------------------------------------------------\n");

	sprintf(genbuf, "plans/%s", lookupuser.userid);
	if ((planfile = fopen(genbuf, "r")) == NULL)
		prints("\n\n계획없이 살아 갑니다-_-;\n");
	else {
		prints("\n");
		for (i = 1; i <= MAXQUERYLINES; i++) {
			if (fgets(inbuf, sizeof(inbuf), planfile))
				prints("%s", inbuf);
			else
				break;
		}
		fclose(planfile);
	}
	pressreturn();
	uinfo.destuid = 0;
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int dumb_user_list(void)
{
	dumb_list(NULL);
	/* apply_record()는 record.c에 있음 - ohno */
	if (apply_record(ULIST, dumb_list, sizeof(struct user_info)) == -1) {
		write(1, "사용자가 없습니다.\n", 15);
	}
	if (bfinger)
		write(1, "\n", 1);
	return 0;
}

int do_all_list(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;

	if (uentp == NULL) {
		printf("%-14s %-25s %-10s\n", "아이디", "별  명", "터미널");
		return 0;
	}
	/*-	if(kill(uentp->pid,0) == -1) return 0 ;*/
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	if (utmp.username[0] == '\0')
		return 0;
	/*
	 * printf("%-14s %s", utmp.userid, ctime(&(utmp.lastlogin)));
	 */
	printf("%-14s %s", utmp.userid, time_string(utmp.lastlogin));
	printf("%-14s %-25s %-10s %-9s %-16s %-6s\n", utmp.userid, utmp.username
	       ,utmp.termtype, ModeType(uentp->mode), uentp->from,
	       (uentp->active) ? "ACTIVE" : "");
	return 1;
}

int list_all_users(void)
{
	uinfo.mode = LAUSERS;
	do_all_list(NULL);	/* do_all_list()는 talk.c 에 있음-ohno */
	if (apply_record(ULIST, do_all_list, sizeof(struct user_info)) == -1) {
		printf("사용자가 없습니다.");
	}
	return 0;
}
/*-  begin part added from pbbs 1.8 --gtv */
int count_active(void *record_data)
{
    struct user_info * uentp = record_data;

	struct userec   utmp;
	static int      count;
	if (uentp == NULL) {
		int             c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (kill(uentp->pid, 0) == -1)
		return 0;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
#ifdef PERMS
	if (utmp.userlevel & PERM_MULTILOG)
		return 0;	/*- don't count multiloggers */
#else
	if (utmp.userlevel >= MULTILOGOK)
		return 0;	/*- don't count multiloggers */
#endif
	count++;
	return 1;
}

int num_active_users(void)
{
	count_active(NULL);
	/* count_active()는 talk.c(여기)에 있음 - ohno */
	apply_record(ULIST, count_active, sizeof(struct user_info));
	return count_active(NULL);
}				/*-  end part added from pbbs 1.8 --gtv */

int tooManyUsers(void)
{
	return (num_active_users() > MAX_SUIT_ACTIVE);
}
/*
#include <sys/time.h>
*/
int t_cmpuids(intptr_t uid, const void *record_data)
{
    const struct user_info * up = record_data;

	return (uid == up->uid);
}

int t_talk(void)
{
	char            uident[STRLEN];
	int             tuid;
	struct user_info uin;

	move(2, 0);
	prints("<이야기 할 상대를 고르세요>\n");
	move(1, 0);
	clrtoeol();
	prints("1:1 상대: ");
	creat_list();
	namecomplete(NULL, uident);
	if (uident[0] == '\0') {
		clear();
		return 0;
	}
	if (!(tuid = getuser(uident)) || tuid == usernum) {
		move(2, 0);
		prints("아이디가 이상합니다.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}
	/*- tuid에 해당하는 데이터를 uin에 채우고, 몇번째 레코드인가
            돌려준다.  첫번째 레코드였다면, 1을 돌려줄 것임 */
	search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);
	if (uin.refuse_call){
		move(2, 0);
		prints("상대방이 talk 신청을 받을 수 없는 상태입니다.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}
	/*-  check if pager on/off       --gtv */
#ifdef PERMS
	if (!HAS_PERM(PERM_SYSOP)) {
#else
	if (currentuser.userlevel < 255) {
#endif
		if (uin.pager == NA && !can_override(NULL, tuid, currentuser.userid)) {
			move(2, 0);
			prints("상대방의 삐삐가 꺼져 있습니다.\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
	}
	if (uin.mode == ULDL || uin.mode == IRCCHAT ||
	    uin.mode == BBSNET || uin.mode == FOURM) {
		move(2, 0);
		prints("상대방이 다른일로 엄청나게(!) 바쁘시다는군요.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}
	if (!uin.active || (kill(uin.pid, 0) == -1)) {
		move(2, 0);
		prints("상대방이 지금 없습니다.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	} else {
		int             sock, msgsock, length;
		struct sockaddr_in server;
		char            c;
		char            buf[512];

		sprintf(buf, "Talk to '%s'", uident);
		report(buf);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) {
			perror("opening stream socket\n");
			return -1;
		}
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = 0;
		if (bind(sock, (struct sockaddr *) & server, sizeof server) < 0) {
			perror("binding stream socket");
			return -1;
		}
		length = sizeof server;
		if (getsockname(sock, (struct sockaddr *) & server, &length) < 0) {
			perror("getting socket name");
			return -1;
		}
		uinfo.sockactive = YEA;
		uinfo.sockaddr = server.sin_port;
		uinfo.destuid = tuid;
		uinfo.mode = PAGE;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		clear();
		prints("%s님을 부르고 있습니다...\n끝내려면 ctrl-D를 누르세요.\n",
		       uident);
		kill(uin.pid, SIGUSR1);
		listen(sock, 1);
		add_io(sock, 20);
		while (YEA) {
			int             ch;
			ch = igetch();
			if (ch == I_TIMEOUT) {
				move(0, 0);
				prints("다시 부르고 있습니다..\n");
				bell();
				if (kill(uin.pid, SIGUSR1) == -1) {
					move(0, 0);
					prints("상대방이 나갔습니다.\n\n\n");
					pressreturn();
					uinfo.mode = TMENU;
					substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
					return -1;
				}
				continue;
			}
			if (ch == I_OTHERDATA)
				break;
			if (ch == '\004') {
				add_io(0, 0);
				close(sock);
				uinfo.sockactive = NA;
				uinfo.destuid = 0;
				uinfo.mode = TMENU;
				substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
				clear();
				return 0;
			}
		}

		msgsock = accept(sock, (struct sockaddr *) 0, (int *) 0);
		if (msgsock == -1) {
			perror("accept");
			uinfo.mode = TMENU;
			substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
			return -1;
		}
		add_io(0, 0);
		close(sock);
		uinfo.sockactive = NA;
		/*-		uinfo.destuid = 0 ;*/
		read(msgsock, &c, sizeof c);
		if (c == 'y')
			do_talk(msgsock);
		else {
			clear();
			prints("상대방이 다른일로 엄청나게(!) 바쁘시다는군요..\n");
			pressreturn();
		}
		close(msgsock);
		clear();
		uinfo.destuid = 0;
		uinfo.mode = TMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	}
	return 0;
}

extern int      talkrequest;
struct user_info ui;
char            page_requestor[STRLEN];

int cmpunums(intptr_t unum, const void *record_data)
{
    const struct user_info * up = record_data;

	if (!up->active)
		return 0;
	return (unum == up->destuid);
}

int searchuserlist(int unum)
{
	return search_record(ULIST, &ui, sizeof(ui), cmpunums, unum);
}

int setpagerequest(void)
{
	int             tuid;
	struct userec   au;
	tuid = searchuserlist(usernum);
	if (tuid == 0)
		return 1;
	if (!ui.sockactive)
		return 1;
	get_record(PASSFILE, &au, sizeof(au), ui.uid);
	uinfo.destuid = ui.uid;
	sprintf(page_requestor, "%s (%s)", au.userid, au.username);
	return 0;
}

int servicepage(int arg)
{
	static time_t   last_check;
	time_t          now;
	char            buf[STRLEN];
	int             tuid = searchuserlist(usernum);
	if (tuid == 0 || !ui.sockactive)
		talkrequest = NA;
	if (!talkrequest) {
		if (page_requestor[0]) {
			switch (uinfo.mode) {
			case TALK:
				move(arg, 0);
				prints("-[삐삐진동]------------------------------------------------");
				break;
			default:	/*- a chat mode */
				sprintf(buf, "** 알림: %s에 의한 호출이 중단되었습니다.",
					page_requestor);
				printchatline(buf);
			}
			memset(page_requestor, 0, STRLEN);
			last_check = 0;
		}
		return NA;
	} else {
		now = time(0);
		if (now - last_check > P_INT) {
			last_check = now;
			if (!page_requestor[0] && setpagerequest())
				return NA;
			else
				switch (uinfo.mode) {
				case TALK:
					move(arg, 0);
					prints("--- ** 알림: %s님이 당신에게 삐삐를 칩니다.",
					       page_requestor);
					break;
				default:	/*- chat */
					sprintf(buf, "** 알림: %s님이 당신에게 삐삐를 칩니다.",
						page_requestor);
					printchatline(buf);
				}
		}
	}
	return YEA;
}

int talkreply(void)
{
	int             a;
	struct hostent *h;
	char            buf[512];
	char            hostname[STRLEN];
	struct sockaddr_in sin;

	talkrequest = NA;
	if (setpagerequest()) {
		if (HAS_PERM(PERM_SYSOP)) {	/* 시험, Talk Request */
			move(0, 76);
			standout();
			prints("TALK");
			standend();
			bell();
		}
		return 0;
	}
	clear();
	prints("%s님과 대담하시겠습니까.", page_requestor);
	memset(page_requestor, 0, sizeof(page_requestor));
	getdata(1, 0, "(Y/N)[Y] ? ", buf, STRLEN, DOECHO, NULL);
	gethostname(hostname, STRLEN);
	if (!(h = gethostbyname(hostname))) {
		perror("gethostbyname");
		return -1;
	}
	memset(&sin, 0, sizeof sin);
	sin.sin_family = h->h_addrtype;
	memmove(&sin.sin_addr, h->h_addr, h->h_length);
	sin.sin_port = ui.sockaddr;
	a = socket(sin.sin_family, SOCK_STREAM, 0);
	if ((connect(a, (struct sockaddr *)&sin, sizeof sin))) {
		perror("connect failed");
		return -1;
	}
	if (buf[0] != 'n' && buf[0] != 'N')
		buf[0] = 'y';
	write(a, buf, 1);
	if (buf[0] != 'y') {
		close(a);
		report("page refused");
		clear();
		return 0;
	}
	report("page accepted");
	do_talk(a);
	close(a);
	clear();
	return 0;
}

char * splitStr(char * src)
{
	static char ret[80];
	int i;

	for(i=0; i<79 && src[i]; i++)
		ret[i] = src[i];
	for(;i<79;i++)
		ret[i] = '-';
	ret[i] = '\0';
	return ret;
}

int dotalkent(struct user_info * uentp, char * buf)
{
	struct userec   utmp;
	char            mch;
	if (!uentp->active || !uentp->pid)
		return -1;
#ifdef PERMS
	if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
#else
	if (currentuser.userlevel < SEE_CLOAK && uentp->invisible)
#endif
		return -1;
	if (kill(uentp->pid, 0) == -1)
		return -1;
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	switch (uentp->mode) {
	case ULDL:
		mch = 'U';
		break;
	case TALK:
		mch = 'T';
		break;
	case CHAT1:
	case CHAT2:
	case CHAT3:
	case CHAT4:
		mch = 'C';
		break;
	case IRCCHAT:
		mch = 'I';
		break;
	case FOURM:
		mch = '4';
		break;
	case BBSNET:
		mch = 'B';
		break;
	case READNEW:
	case READING:
		mch = 'R';
		break;
	case POSTING:
		mch = 'P';
		break;
	case SMAIL:
	case RMAIL:
	case MAIL:
		mch = 'M';
		break;
	default:
		mch = '-';
	}
	sprintf(buf, "%s%s(%c), ", uentp->invisible ? "*" : "", utmp.userid, mch);
	return 0;
}

extern int      t_columns;

int dotalkuserlist(int sline, int eline, int * curln, int * curcol, char * wordbuf, int * wordbuflen)
{
	char           *s = "\n*** 현재 접속자 ***\n";
	char            bigbuf[STRLEN];
	char            littlebuf[20];
	int             fd, savecolumns, pos = 0;
	struct user_info uent;
	savecolumns = (t_columns > STRLEN ? t_columns : 0);
	bigbuf[0] = '\0';
	if ((fd = open(ULIST, O_RDONLY, 0)) == -1)
		return -1;
	do_talk_string(sline, eline, curln, curcol, wordbuf, wordbuflen, s);
	while (read(fd, &uent, sizeof(uent)) == sizeof(uent)) {
		if (dotalkent(&uent, littlebuf) == -1)
			continue;
		if (pos + strlen(littlebuf) >= t_columns) {
			do_talk_string(sline, eline, curln, curcol, wordbuf, wordbuflen, bigbuf);
			bigbuf[0] = '\0';
			pos = 0;
		}
		strcat(bigbuf, littlebuf);
		pos += strlen(littlebuf);
	}
	if (pos > 0) {
		bigbuf[pos - 2] = '\n';
		bigbuf[pos - 1] = '\0';
		do_talk_string(sline, eline, curln, curcol, wordbuf, wordbuflen, bigbuf);
	}
	close(fd);
	if (savecolumns)
		t_columns = savecolumns;
	return 0;
}

int do_talk_string(int sline, int eline, int * curln, int * curcol, char * wordbuf, int * wordbuflen, char * s)
{
	while (*s) {
		do_talk_char(sline, eline, curln, curcol, wordbuf, wordbuflen, *s);
		s++;
	}
	return 0;
}

int do_talk_char(int sline, int eline, int * curln, int * curcol, char * wordbuf, int * wordbuflen, int ch)
{
	extern int      dumb_term;
#ifdef BIT8
	if (isprint2(ch)) {
#else
	if (isprint(ch)) {
#endif
		if (*curcol != 79) {
			wordbuf[(*wordbuflen)++] = ch;
			if (ch == ' ')
				*wordbuflen = 0;
			move(*curln, (*curcol)++);
			prints("%c", ch);
			return 0;
		}
		if (ch == ' ' || *wordbuflen >= 78) {
			(*curln)++;
			*curcol = 0;
			if (*curln > eline)
				*curln = sline;
			if ((*curln) != eline) {
				move((*curln) + 1, 0);
				clrtoeol();
			}
			move(*curln, *curcol);
			clrtoeol();
			*wordbuflen = 0;
			return 0;
		}
		move(*curln, (*curcol) - *wordbuflen);
		clrtoeol();
		(*curln)++;
		*curcol = 0;
		if (*curln > eline)
			*curln = sline;
		if ((*curln) != eline) {
			move((*curln) + 1, 0);
			clrtoeol();
		}
		move(*curln, *curcol);
		clrtoeol();
		wordbuf[*wordbuflen] = '\0';
		if (dumb_term)
			prints("\n");
		prints("%s%c", wordbuf, ch);
		*curcol = (*wordbuflen) + 1;
		*wordbuflen = 0;
		return 0;
	}
	switch (ch) {
	case CTRL('H'):
	case '\177':
		if (dumb_term)
			ochar(CTRL('H'));
		if (*curcol == 0) {
			if (sline == 0)
				bell();
			return 0;
		}
		(*curcol)--;
		move(*curln, *curcol);
		if (!dumb_term)
			prints(" ");
		move(*curln, *curcol);
		if (*wordbuflen)
			(*wordbuflen)--;
		return 0;
	case CTRL('M'):
	case CTRL('J'):
		if (dumb_term)
			prints("\n");
		(*curln)++;
		*curcol = 0;
		if (*curln > eline)
			*curln = sline;
		if ((*curln) != eline) {
			move((*curln) + 1, 0);
			clrtoeol();
		}
		move(*curln, *curcol);
		clrtoeol();
		*wordbuflen = 0;
		return 0;
	case CTRL('G'):
		bell();
		return 0;
	default:
		break;
	}
	return 0;
}

char            talkobuf[80];
int             talkobuflen;
int             talkflushfd;

int talkflush(void)
{
	if (talkobuflen)
		write(talkflushfd, talkobuf, talkobuflen);
	talkobuflen = 0;
	return 0;
}
static int             myln, mycol, myfirstln, mylastln;
static int             itsln, itscol, itsfirstln, itslastln;
static char            itswordbuf[80], mywordbuf[80];
static int             itswordbuflen, mywordbuflen;
static int 	talk_fly_fd;

/* Bugman: Talk도중에 쪽지 보내기 : io.c 의 getdata를 긁어다가 수정  */
int getdata_during_talk_fly(int line, int col, char * prompt, char * buf, int len, int echo, bbs_action complete)
{
	int ch;
        int             clen = 0;
        int             x, y;
        extern unsigned char scr_cols;

        if (prompt) {
                move(line, col);
                prints("%s", prompt);
        }

        clrtoeol();
        getyx(&y, &x);
        while ((ch = igetch()) != '\r') {
                if (ch == '\n')
                        break;
        	if (ch == I_OTHERDATA) {
                	char            data[80];
                	int             datac;
			int i;
			int xx, yy;

                	datac = read(talk_fly_fd, data, 80);
			if (datac >0){
				getyx(&yy, &xx);
                		for (i = 0; i < datac; i++){
                        		do_talk_char(itsfirstln, itslastln, &itsln, &itscol, itswordbuf,
                                		&itswordbuflen, data[i]);
				}
				move(yy, xx);
			} else {
				return -1;
			}
			continue;
	}

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
int do_talk_fly(void)
{ /* return value -> 1: continue, -1: quit talk */
        char text[100];
        char uident[30];
        char buf[200];
        int     tuid;
        struct user_info uin;
        extern int msgLine;

	int ret;

        ret = getdata_during_talk_fly(msgLine, 0, "상대방의 아이디를 입력하세요 : ", uident, 14, DOECHO, NULL);
	if (ret <0) return -1;
        if (uident[0] == '\0') {
                move(msgLine, 0);
                prints(splitStr(" ** 관두었습니다. "));
                return 1;
        }
	if (uident[0] == '-') strcpy(uident, getLastMessageSender());
	if (uident[0] == '=') strcpy(uident, getLastMessageReceiver());

        if (uident[0] == '\0' || !(tuid = getuser(uident)) || tuid == usernum) {
                move(msgLine, 0);
                prints(splitStr(" ** 아이디가 이상합니다. "));
                return  1;
        }
        search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid);
        if (uin.refuse_call){
                move(msgLine, 0);
                prints(splitStr(" ** 상대방이 지금 쪽지를 받을 수 없습니다. "));
                return 1;
        }
        ret = getdata_during_talk_fly(msgLine, 0, "쪽지내용 : ", text, 60, DOECHO, NULL);
	if (ret <0) return -1;
	if (text[0] == '\0') {
                move(msgLine, 0);
                prints(splitStr(" ** 관두었습니다.  "));
		return 1;
	}
        sprintf(buf, ">> %s님의 쪽지: %s", currentuser.userid, text);
        writeFlyingMessage(uident, buf);
        if (!uin.active || (kill(uin.pid, SIGUSR2) == -1)) {
                move(msgLine, 0);
                prints(splitStr(" ** 상대방이 지금 없습니다. "));
                return 1;
        }
	sprintf(buf, "- %s님에게 쪽지를 보냈습니다. ", uident);
	move(msgLine, 0);
        prints(splitStr(buf));
	return 1;
}

#define TALK_STR(s) \
	do_talk_string(myfirstln, mylastln, &myln, &mycol, mywordbuf, &mywordbuflen, s)

int do_talk_query(void)
{
        char uident[30];
        int     tuid;
	char buf[100];
        extern int msgLine;
	int ret;

        ret = getdata_during_talk_fly(msgLine, 0, "누가 궁금하세요 : ", uident, 14,DOECHO, NULL);
	if (ret <0)  return -1;
        if (uident[0] == '\0') {
                move(msgLine, 0);
                prints(splitStr(" ** 관두었습니다. "));
                return 1;
        }

        if (!(tuid = getuser(uident)) ) {
                move(msgLine, 0);
                prints(splitStr(" ** 아이디가 이상합니다. "));
                return 1;
        }
/*
        move(msgLine, 0);
        prints(splitStr(" ** 사용자 정보입니다.  "));
	refresh();
*/
	sprintf(buf, "\n****** %s님의 사용자 정보 ****** \n", uident); TALK_STR(buf);
	sprintf(buf, "이  름 : %s (%s)\n", lookupuser.userid, lookupuser.username); TALK_STR(buf);
	sprintf(buf, "이메일 : %s\n", lookupuser.email); TALK_STR(buf);
	sprintf(buf, "방  문 : %-30d 올린글 : %d\n", lookupuser.numlogins, lookupuser.numposts); TALK_STR(buf);

	sprintf(buf, "마지막 : %-30s 어디서 : %s\n",
			time_string_no_newline(lookupuser.lastlogin),
			(lookupuser.lasthost[0] == '\0' ? "" : lookupuser.lasthost)); TALK_STR(buf);

	sprintf(buf, "본  명 : %s\n", lookupuser.realname); TALK_STR(buf);
	sprintf(buf, "소  속 : %s\n", lookupuser.address); TALK_STR(buf);

        sprintf(buf, "- %s님에 대한 조사가 끝났습니다. ", uident);
        move(msgLine, 0);
        prints(splitStr(buf));
	return 1;
}

int do_talk(int fd)
{
	int             i;
/*- by bugman : move to globale variable for talk_fly
	int             myln, mycol, myfirstln, mylastln;
	int             itsln, itscol, itsfirstln, itslastln;
	char            itswordbuf[80], mywordbuf[80];
	int             itswordbuflen, mywordbuflen;
*/
	int             page_pending = NA;
	struct userec   p_userec;
	char            tempbuf[100], *p;
	extern int      msgLine;

	itswordbuflen = 0;
	mywordbuflen = 0;
	talkobuflen = 0;
	talkflushfd = fd;
	uinfo.mode = TALK;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	myfirstln = 0;
	mylastln = (tgetnum("li") - 1) / 2 - 1;
	/*
	 * fixed by wings!!! mylastln = (t_lines - 1) / 2 - 1;
	 */
	move(msgLine = mylastln + 1, 0);

	/* +6 lines are added by skji */
	get_record(PASSFILE, &p_userec, sizeof(p_userec), uinfo.destuid);
	sprintf(tempbuf, "* %s(%s) *---* %s *", p_userec.userid,
		p_userec.username, p_userec.email);
	prints("-----%s", tempbuf);
	for (i = 80 - strlen(tempbuf) - 5; i > 0; i--)
		addch('-');

	itsfirstln = mylastln + 2;
	itslastln = (tgetnum("li") - 1);
	/*
	 * fixed by wings!!! itslastln = (t_lines - 1);
	 */
	myln = myfirstln;
	mycol = 0;
	itsln = itsfirstln;
	itscol = 0;
	move(myln, mycol);
	add_io(fd, 0);
	add_flush(talkflush);
	while (YEA) {
		int             ch;
		if (talkrequest)
			page_pending = YEA;
		if (page_pending)
			page_pending = servicepage(mylastln + 1);
		ch = igetch();
		if (ch == I_OTHERDATA) {
			char            data[80];
			int             datac;
			int    i;

			datac = read(fd, data, 80);
			if (datac <= 0)
				break;
			for (i = 0; i < datac; i++)
				do_talk_char(itsfirstln, itslastln, &itsln, &itscol, itswordbuf,
					     &itswordbuflen, data[i]);
		} else {
			if (ch == CTRL('D') || ch == CTRL('C'))
				break;
#ifdef BIT8
			if (isprint2(ch) || ch == CTRL('H') || ch == '\177' || ch == CTRL('G')
#else
			if (isprint(ch) || ch == CTRL('H') || ch == '\177' || ch == CTRL('G')
#endif
			    || ch == CTRL('M') ) {
				talkobuf[talkobuflen++] = ch;
				if (talkobuflen == 80)
					talkflush();
				do_talk_char(myfirstln, mylastln, &myln, &mycol, mywordbuf,
					     &mywordbuflen, ch);
			} else if (ch == CTRL('U') || ch == CTRL('W'))
				dotalkuserlist(myfirstln, mylastln, &myln,
					  &mycol, mywordbuf, &mywordbuflen);
#ifdef PERMS
			else if (ch == CTRL('P') && HAS_PERM(PERM_PAGE)) {
#else
			else if (ch == CTRL('P')) {
#endif
				if (uinfo.pager == YEA) {
					do_talk_string(myfirstln, mylastln, &myln, &mycol, mywordbuf, &mywordbuflen, "*** 삐삐를 껐습니다(OFF) ***\n");
					uinfo.pager = NA;
				} else {
					do_talk_string(myfirstln, mylastln, &myln, &mycol, mywordbuf, &mywordbuflen, "*** 삐삐를 켰습니다(ON) ***\n");
					uinfo.pager = YEA;
				}
				substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
			} else if ((ch == CTRL('K') || ch == CTRL('O')) && HAS_PERM(PERM_POST)){
				talk_fly_fd = fd;
				if (do_talk_fly() <0) break;
			} else if (ch == CTRL('T')){
				time_t t;
				char buf[100];
				time(&t);
				sprintf(buf, "** 현재 시간 : %s\n", time_string_no_newline(t));
				do_talk_string(myfirstln, mylastln, &myln, &mycol, mywordbuf, &mywordbuflen, buf);
			} else if ((ch == CTRL('Q') || ch == CTRL('A')) && HAS_PERM(PERM_POST)){
				talk_fly_fd = fd;
				if (do_talk_query() <0) break;
			} else if (ch == CTRL('B')){
				        FILE           *fp;
        				char            buf[100];
        				struct msgRec   msgBuf;
        				int             i;

        				sprintf(buf, "msgtmp/%s", currentuser.userid);
        				fp = fopen(buf, "rb");
        				if (fp == NULL) {
						TALK_STR("\n** 아무 쪽지도 없습니다. **\n");
						continue;
        				}
        				fread(&msgBuf, sizeof(msgBuf), 1, fp);
        				fclose(fp);
					TALK_STR("\n** 지난쪽지 보기 **\n");
        				for (i=msgBuf.index; i>=0; i--){
                				sprintf(buf, "%s\n", msgBuf.msg[i]);
                				TALK_STR(buf);
        				}
        				for (i=MSGNUM-1; i>msgBuf.index; i--){
                				sprintf(buf, "%s\n", msgBuf.msg[i]);
                				TALK_STR(buf);
        				}
			} else
				bell();
		}
	}
	add_io(0, 0);
	talkflush();
	add_flush(NULL);
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int shortulist(void *record_data)
{
    struct user_info * uentp = record_data;

	static int      fullcount = 0;
	static int      lineno;
	static int      cnt;
	static char    *nullstring = "                        ";
	int             finaltally;
	char            uentry[30];
	struct userec   utmp;
	if (!lineno) {
		lineno = 5;
		move(lineno, 0);
	}
	if (uentp == NULL) {
		clrtoeol();
		move(++lineno, 0);
		clrtobot();
		lineno = 0;
		cnt = 0;
		finaltally = fullcount;
		fullcount = 0;
		return finaltally;
	}
	uentry[0] = '\0';
	if (!uentp->active || !uentp->pid)
	return 0;
		/*strcpy(uentry, nullstring);*/
#ifdef PERMS
	if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
#else
	if (currentuser.userlevel < SEE_CLOAK && uentp->invisible)
#endif
		return 0;
/*
		strcpy(uentry, nullstring);
*/
	if (kill(uentp->pid, 0) == -1)
		return 0;
/*
		strcpy(uentry, nullstring);
*/
	get_record(PASSFILE, &utmp, sizeof(utmp), uentp->uid);
	if (!uentry[0]) {
		sprintf(uentry, "%-12s %c%-10s",
			utmp.userid,
			uentp->invisible ? '#' : ' ',
		    modestring(uentp->mode, uentp->destuid, 0, NULL, NULL));
		/*
		 * modestring(uentp->mode, uentp->destuid, 0, NULL,
		 * (uentp->mode == BBSNET ? uentp->chatid : NULL)));
		 */
		fullcount++;
	}
	if (++cnt < 3)
		strcat(uentry, " | ");
	prints(uentry);
	if (cnt == 3) {
		cnt = 0;
		clrtoeol();
		move(++lineno, 0);
		/*- assuming no more than 60 logins at once */
	}
	return 0;
}

int do_list(void)
{
	int             count;
	move(3, 0);
	prints("%-12s  %-10s | %-12s  %-10s | %-12s  %-10s",
	       "아이디 ", "상태", "아이디 ", "상태", "아이디 ", "상태");
	move(4, 0);
	prints("%-12s  %-10s | %-12s  %-10s | %-12s  %-10s",
	       "-------", "----", "-------", "----", "-------", "----");
	if (apply_record(ULIST, shortulist, sizeof(struct user_info)) == -1) {
		prints("사용자가 없습니다.\n");
		return 0;
	}
	count = shortulist(NULL);
	if (uinfo.mode == MONITOR) {
		time_t          thetime = time(0);

		print_newmail_message();

		/*- skji
			move(0, 30);
			if (chkmail())
			    prints("(You have mail.)");
		*/

		move(t_lines - 1, 0);
		prints("%d명의 사용자가 있습니다. 지금  시간은 %s",
		       count, time_string(thetime));
	}
	if (dumb_term)
		oflush();
	else
		refresh();
	return 0;
}

int t_list(void)
{
	move(1, 0);
	clrtobot();
	uinfo.mode = LUSERS;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	/*-	report("talk user list");*/
	do_list();
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pressanykey();
	return 0;
}

int             idle_monitor_time;

void sig_catcher(int dummy)
{
	if (uinfo.mode != MONITOR) {
#ifdef DOTIMEOUT
		init_alarm();
#else
		signal(SIGALRM, SIG_IGN);
#endif
		return;
	}
	if (signal(SIGALRM, sig_catcher) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
#ifdef DOTIMEOUT
	idle_monitor_time += M_INT;
	if (idle_monitor_time > MONITOR_TIMEOUT) {
		clear();
		fprintf(stderr, "%d분 이상은 사용자구경을 할 수 없습니다. 안녕히!!\n", MONITOR_TIMEOUT / 60);
		kill(getpid(), SIGHUP);
	}
#endif
	do_list();
	alarm(M_INT);
}

int t_monitor(void)
{
	char            c;
	int             i;
	alarm(0);
	signal(SIGALRM, sig_catcher);
	uinfo.mode = MONITOR;
	idle_monitor_time = 0;
	/*- report("monitor"); */
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	move(1, 0);
	prints("** 구경중 ** ");
	/*
	 * move(2, 0);
	 */
	prints("매 %d초 마다 변경됩니다. -- CTRL-D를 누르면 마칩니다.\n", M_INT);
	clrtobot();
	do_list();
	alarm(M_INT);
	while (YEA) {
		i = read(0, &c, 1);
		if (!i || c == CTRL('D') || c == CTRL('C'))
			break;
		else if (i == -1) {
			if (errno != EINTR) {
				perror("read");
				exit(1);
			}
		} else
			idle_monitor_time = 0;
	}
	move(2, 0);
	clrtobot();
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

#ifdef IRC
int t_irc(void)
{
	char            buf[512];
	int             save_pager = uinfo.pager;
	uinfo.pager = NA;
	uinfo.mode = IRCCHAT;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(buf, "irc %s %s", currentuser.userid, IRC_SERVER);
	report("IRC Chat");
	reset_tty();
	do_exec(buf, NULL);
	restore_tty();
	uinfo.pager = save_pager;
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
}
#endif				/*- IRC */

#ifdef MMMM

int t_4m(void)
{				/*- Bill Schwartz */
	char            buf[512];
	int             save_pager = uinfo.pager;
	uinfo.pager = NA;
	uinfo.mode = FOURM;
	report("4m started");
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(buf, "fourm_rc/%s", currentuser.userid);
	if (access(buf, R_OK) != 0)
		sprintf(buf, "4m -rn %s -l %s", currentuser.userid, currentuser.userid);
	else
		sprintf(buf, "4m -r -l %s -f fourm_rc/%s", currentuser.userid, currentuser.userid);
	reset_tty();
	do_exec(buf, NULL);
	restore_tty();
	uinfo.pager = save_pager;
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
}

#endif				/*- MMMM */

/*- rrr -- stuff for providing pager override lists follows */

int can_override(char * userid, int uid, char * whoasks)
{
	struct userec   utmp;
	FILE           *fp;
	char            buf[STRLEN];
	int             blen;
	if (userid == NULL) {
		if (get_record(PASSFILE, &utmp, sizeof(utmp), uid) == -1)
			return 0;
		userid = utmp.userid;
	}
	sprintf(buf, "overrides/%s", userid);
	if ((fp = fopen(buf, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		blen = strlen(buf);
		if (buf[blen - 1] == '\n')
			buf[--blen] = '\0';
		if (blen == strlen(whoasks)) {
			if (!strncmp(buf, whoasks, blen)) {
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

int listfriends(void)
{
	FILE           *fp;
	int             x = 0, y = 3, cnt = 0;
	char           *nl;
	move(y, x);
	CreateNameList();
	sprintf(genbuf, "overrides/%s", currentuser.userid);
	if ((fp = fopen(genbuf, "r")) == NULL) {
		prints("# 아무도 없어요.\n");
		return 0;
	}
	while (fgets(genbuf, STRLEN, fp) != NULL) {
		prints("%s", genbuf);
		if (nl = strrchr(genbuf, '\n'))
			*nl = '\0';
		AddNameList(genbuf);
		cnt++;
		if ((++y) >= t_lines - 1) {
			y = 3;
			x += 16;
		}
		move(y, x);
	}
	fclose(fp);
	if (cnt == 0)
		prints("(친구 없음)\n");
	return cnt;
}

int addtooverride(char * uident)
{
	FILE           *fp;
	int             rc;
	if (can_override(currentuser.userid, usernum, uident))
		return -1;

	sprintf(genbuf, "overrides/%s", currentuser.userid);
	if ((fp = fopen(genbuf, "a")) == NULL)
		return -1;
	flock(fileno(fp), LOCK_EX);
	rc = fputs(uident, fp);
	fputc('\n', fp);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return (rc == EOF ? -1 : 0);
}

int deleteoverride(char * uident)
{
	FILE           *fp, *nfp;
	int             deleted = NA;
	int 		blen;
	char            fn[STRLEN], fnnew[STRLEN];
	sprintf(fn, "overrides/%s", currentuser.userid);
	if ((fp = fopen(fn, "r")) == NULL)
		return -1;
	sprintf(fnnew, "overrides/%s.%d", currentuser.userid, getuid());
	if ((nfp = fopen(fnnew, "w")) == NULL)
		return -1;
	while (fgets(genbuf, STRLEN, fp) != NULL) {
		blen = strlen(genbuf);
		if (genbuf[blen - 1] == '\n')
			genbuf[blen-1] = '\0';
		if (strcmp(genbuf, uident)) {
			fputs(genbuf, nfp);
			fputc('\n', nfp);
		}else{
			deleted = YEA;
		}
	}
	fclose(fp);
	fclose(nfp);
	if (!deleted)
		return -1;
	return (rename(fnnew, fn));
}

int t_override(void)
{
	char            uident[STRLEN];
	char            ans[8];
	int             count;
	/*-
		uinfo.mode =
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	*/
	while (1) {
		clear();
		prints("친구 명단 정리\n");
		count = listfriends();
		if (count)
			getdata(1, 0, "(A)더함 (D)뺌 (E)나감? [E]: ", ans, 7, DOECHO, NULL);
		else
			getdata(1, 0, "(A)더함 (E)나감? [E]: ", ans, 7, DOECHO, NULL);
		if (*ans == 'A' || *ans == 'a') {
			u_namelist();
			move(2, 0);
			namecomplete("친구의 아이디: ", uident);
			move(2, 0);
			clrtoeol();
			if (uident[0] != '\0' && getuser(uident))
				addtooverride(uident);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(2, 0);
			namecomplete("아이디: ", uident);
			move(2, 0);
			clrtoeol();
			if (uident[0] != '\0')
				deleteoverride(uident);
		} else
			break;
	}
	clear();
	return 0;
}
