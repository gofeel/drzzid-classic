/* Modified 2026-11-16. */
/*--
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

#define EXTERN
#include "bbs.h"
#include "edit.h"
#include "more.h"
#include <unistd.h>
/* chopin */
extern char    home_dir[];

int x_passwd(void)
{
	char *pass, passbuf[PASSLEN];

	move(3, 0);
	clrtobot();
	getdata(3, 0, "옛날 암호: ", passbuf, PASSLEN, NOECHO, NULL);
	if (passbuf[0] == '\0' || !checkpasswd(currentuser.passwd, passbuf)) {
		prints("\n암호가 틀려요.\n");
		pressreturn();
		return 0;
	}
	getdata(3, 0, "새 암호: ", passbuf, PASSLEN, NOECHO, NULL);
	if (passbuf[0] == '\0') {
		prints("\n암호바꾸기 그만둠\n");
		pressreturn();
		return 0;
	}
	/*--
	    strncpy(genbuf,pass,PASSLEN) ;
	*/
	getdata(4, 0, "암호 한번 더: ", genbuf, PASSLEN, NOECHO, NULL);
	if (strncmp(passbuf, genbuf, PASSLEN)) {
		prints("암호가 틀려요.\n");
		return -1;
	}
	genbuf[8] = '\0';
	pass = genpasswd(genbuf);
	strncpy(currentuser.passwd, pass, PASSLEN);
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	report("changed password");
	clear();
	return 0;
}

int x_name(void)
{
	move(2, 0);
	prints("현재의 별명은 '%s' 입니다", currentuser.username);
	getdata(1, 0, "새별명은 (Return은 관둠) ? ", genbuf, 21, DOECHO, NULL);
	if (genbuf[0] != 0) {
		strncpy(currentuser.username, genbuf, STRLEN);
		substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	}
	clear();
	return 0;
}

/*- added by skji */
#ifdef REALINFO
int x_realname(void)
{
	move(2, 0);
	prints("Current realname = [%s]", currentuser.username);
	getdata(1, 0, "본명 (넘어갈려면 리턴) ? ", genbuf, 21, DOECHO, NULL);
	if (genbuf[0] != '\0') {
		strncpy(currentuser.realname, genbuf, STRLEN);
		substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	}
	clear();
	return 0;
}
#endif

#ifdef INTERNET_EMAIL
int x_setemail(void)
{
	move(2, 0);
	prints("원래는 %s이었어요.", currentuser.email);
	getdata(1, 0, "인터넷 e-mail 주소: ", genbuf,
		sizeof(currentuser.email), DOECHO, NULL);
	strncpy(currentuser.email, genbuf, sizeof(currentuser.email));
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	clear();
	return 0;
}
#endif

int x_csh(void)
{
	int             save_pager;
	clear();
	refresh();
	reset_tty();
	save_pager = uinfo.pager;
	uinfo.pager = NA;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	report("shell out");
#ifdef SYSV
	do_exec("/bin/sh", NULL);
#endif
#ifdef LINUX
	do_exec("/bin/bash", NULL);
#else
	do_exec("/bin/csh", NULL);
#endif
	restore_tty();
	uinfo.pager = save_pager;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

int x_terminal(void)
{
	move(2, 0);
	prints("원래는 %s이었어요.", currentuser.termtype);
	getdata(1, 0, "터미널 종류: ", genbuf, STRLEN, DOECHO, NULL);
	if (genbuf[0] == '\0')
		return 0;
	if (!term_init(genbuf)) {
		move(2, 0);
		prints("이상한 터미널이군요.\n");
		return -1;
	}
	strncpy(currentuser.termtype, genbuf, STRLEN);
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	initscr();
	clear();
	pressreturn();
	return 0;
}

unsigned int setperms(unsigned int pbits)
{
	int             lastperm = NUMPERMS - 1;
	int             i, done = NA;
	char            buf[STRLEN], choice[2];
	move(4, 0);
	prints("A에서 O사이의 글자를 치시면 기능이 토글됩니다.\n");
	move(6, 0);
	for (i = 0; i <= lastperm; i++) {
		sprintf(buf, "%c. %-40s %3s\n", 'A' + i, permstrings[i],
			((pbits >> i) & 1 ? "ON" : "OFF"));
		prints(buf);
	}
	clrtobot();
	while (!done) {
		getone(t_lines - 1, 0, "[나갈려면 SPACE]: ", choice, 2, DOECHO, NULL);
		*choice = toupper(*choice);
		if (*choice == '\n' || *choice == '\0' || *choice == ' ')
			done = YEA;
		else if (*choice < 'A' || *choice > 'A' + lastperm)
			bell();
		else {
			i = *choice - 'A';
			if ((pbits >> i) & 1)
				pbits &= (~(1 << i));
			else
				pbits |= (1 << i);
			sprintf(buf, "%c. %-40s %3s\n", 'A' + i, permstrings[i],
				((pbits >> i) & 1 ? "ON" : "OFF"));
			move(i + 6, 0);
			prints(buf);
		}
	}
	return (pbits);
}

int x_level(void)
{
	int             id;
#ifdef PERMS
	int             newlevel;
#endif

	u_namelist();
	move(0, 0);
	prints("%s/[시스템관리]/[사용자 등급 설정]\n", BOARDNAME);
	clrtoeol();
	move(1, 0);
	namecomplete("아이디: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("아이디가 틀립니다.");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
#ifdef PERMS
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("'%s'에게 부여할 권한 설정\n", genbuf);
	newlevel = setperms(lookupuser.userlevel);
#else
	move(2, 0);
	prints("원래는 %d입니다.", lookupuser.userlevel);
	getdata(1, 0, "우선권(0 - 255): ", genbuf, 7, DOECHO, NULL);
	lookupuser.userlevel = atoi(genbuf);
#endif
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		prints("'%s'의 등급이 바뀌지 않았습니다.\n", lookupuser.userid);
	else {
		lookupuser.userlevel = newlevel;
		substitute_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		prints("'%s'의 등급이 바뀌었습니다.\n", lookupuser.userid);
		sprintf(genbuf, "changed permissions for %s", lookupuser.userid);
		report(genbuf);
	}
	pressreturn();
	clear();
	return 0;
}

int x_cloak(void)
{
	uinfo.invisible = (uinfo.invisible) ? NA : YEA;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	move(2, 0);
	if (!uinfo.in_chat)
		prints("마법의 외투를 %s었습니다.\n", (uinfo.invisible) ? "입" : "벗");
	report("toggle cloak");
	pressanykey();
	return 0;
}

int x_date(void)
{
	time_t          t;
	move(3, 0);
	time(&t);
	/*
	 * prints("이 지역의 현재 날짜와 시간: %s", ctime(&t));
	 */
	prints("한국 기준 지금 날짜와 시간: %s", time_string(t));
	clrtobot();
	pressanykey();
	return 0;
}

#ifdef REALINFO
int x_info(void)
{
	user_display(&currentuser);
	prints("\n");
	prints("문제가 있으면 SYSOP에게 문의하세요.\n");
	clrtobot();
	pressreturn();
	clear();
	return 0;
}
#endif

int x_editsig(void)
{
	int             aborted;
	char            ans[7];
	sprintf(genbuf, "signatures/%s", currentuser.userid);
	move(3, 0);
	clrtoeol();
	clrtobot();
	if (currentuser.flags[0] & SIG_FLAG)
		getdata(3, 0, "(E)고침 (D)지움 (T)서명을 켬(ON) [E]: ", ans, 7, DOECHO, NULL);
	else
		getdata(3, 0, "(E)고침 (D)지움 (T)서명을 끔(OFF) [E]: ", ans, 7, DOECHO, NULL);
	if (ans[0] == 'D' || ans[0] == 'd') {
		unlink(genbuf);
		move(5, 0);
		prints("아! 님은 갔습니다.\n");
		pressreturn();
		clear();
		return 0;
	}
	if (ans[0] == 'T' || ans[0] == 't') {
		move(5, 0);
		if (currentuser.flags[0] & SIG_FLAG) {
			currentuser.flags[0] &= ~SIG_FLAG;
			prints("서명을 사용합니다(ON)\n");
		} else {
			currentuser.flags[0] |= SIG_FLAG;
			prints("서명을 사용하지 않습니다(OFF)\n");
		}
		substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
		pressreturn();
		clear();
		return 0;
	}
	uinfo.mode = EDITSIG;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	aborted = vedit(genbuf, NA, TEXT_IS_EDIT);
	clear();
	if (!aborted)
		prints("서명이 바뀌었습니다");
	pressreturn();
	uinfo.mode = XMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

#ifdef MMMM

int x_edit4mrc(void)
{				/*-- Bill Schwartz */
	int             aborted;
	char            ans[8];
	char            buf[128];
	sprintf(genbuf, "fourm_rc/%s", currentuser.userid);
	move(3, 0);
	clrtoeol();
	clrtobot();
	getdata(3, 0, "(E)dit or (D)elete 4mrc? [E]: ", ans, 7, DOECHO, NULL);
	if (ans[0] == 'D' || ans[0] == 'd') {
		unlink(genbuf);
		move(5, 0);
		prints("아! 님은 갔습니다.\n");
		pressreturn();
		clear();
		return;
	}
	uinfo.mode = EDITPLAN;
	report("edit .4mrc");
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(buf, "fourm_rc/%s", currentuser.userid);
	if (access(buf, R_OK) != 0) {	/*-- see if a copy of default is needed*/
		sprintf(buf, "cp .4mrc fourm_rc/%s", currentuser.userid);
		system(buf);
	}
	aborted = vedit(genbuf, NA, TEXT_IS_EDIT);
	clear();
	if (!aborted)
		prints("4mrc updated.");
	pressreturn();
	uinfo.mode = XMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
}
#endif

int x_editplan(void)
{
	int             aborted;
	char            ans[7];
	sprintf(genbuf, "plans/%s", currentuser.userid);
	move(3, 0);
	clrtoeol();
	clrtobot();
	getdata(3, 0, "(E)고침 (D)지움 [E]: ", ans, 7, DOECHO, NULL);
	if (ans[0] == 'D' || ans[0] == 'd') {
		unlink(genbuf);
		move(5, 0);
		prints("Plan을 삭제 하였습니다.\n");
		report("delete plan");
		pressreturn();
		clear();
		return 0;
	}
	uinfo.mode = EDITPLAN;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	aborted = vedit(genbuf, NA, TEXT_IS_EDIT);
	clear();
	if (!aborted) {
		prints("소개말을 바꾸었습니다.");
		report("edited plan");
	}
	pressreturn();
	uinfo.mode = XMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

#ifdef VOTE

#ifndef SYSV
#include <sys/file.h>
#endif

int x_vote(void)
{
	FILE           *cfp;
	char            inbuf[80], choices[10], vote[2];
	int             fd, count = 0;
	time_t          closetime;
	move(3, 0);
	if ((cfp = fopen("vote/control", "r")) == NULL) {
		prints("죄송합니다. 지금은 투표기간이 아닙니다.\n");
		clrtobot();
		pressreturn();
		clear();
		return 0;
	}
	if (currentuser.flags[0] & VOTE_FLAG) {
		prints("이미 투표를 하셨습니다.!\n");
		prints("한사람이 한표!!!\n");
		clrtobot();
		pressreturn();
		clear();
		return 0;
	}
	uinfo.mode = VOTING;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	more("vote/desc", YEA, MORE_IS_NORMAL);
	clear();
	move(0, 0);
	standout();
	prints("<<< 투표 용지 >>>");
	standend();
	move(2, 0);
	prints("투표를 하려면 다음에 나오는 항목에 대한 숫자를 고르세요.\n");
	prints("다른 글쇠를 누르면 투표를 하지 않습니다.\n\n");
	fgets(inbuf, sizeof(inbuf), cfp);
	closetime = (time_t) atol(inbuf);
	/*
	 * prints("이 투표는 %s에 마감됩니다.\n", ctime(&closetime));
	 */
	prints("마감 : %s", time_string(closetime));
	move(7, 0);
	memset(choices, 0, sizeof(choices));
	while (fgets(inbuf, sizeof(inbuf), cfp) != NULL) {
		choices[count++] = inbuf[0];
		prints("%s", inbuf);
	}
	fclose(cfp);
	vote[0] = vote[1] = '\0';
	getdata(count + 8, 0, "위의 보기 중에서 하나를 고르세요 : ", vote, 2, DOECHO, NULL);
	move(count + 10, 0);
	if (vote[0] == '\0' || strchr(choices, vote[0]) == NULL)
		prints("잘못 골랐습니다. 투표를 하지 않습니다.\n");
	else {
		if ((fd = open("vote/ballots", O_WRONLY | O_CREAT | O_APPEND, 0660)) == 0)
			prints("투표함을 열수 없습니다. 투표 관둠....\n");
		else {
			struct stat     statb;
			flock(fd, LOCK_EX);
			write(fd, vote, 1);
			flock(fd, LOCK_UN);
			fstat(fd, &statb);
			close(fd);
			currentuser.flags[0] |= VOTE_FLAG;
			substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
			prints("투표를 하셨습니다.! (지금까지 투표한 사람 : %d)\n", statb.st_size);
		}
	}
	pressreturn();
	uinfo.mode = XMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

int x_results(void)
{
	if (more("vote/results", YEA, MORE_IS_NORMAL) == -1) {
		move(3, 0);
		prints("지금은 투표 결과가 나와 있지 않습니다.\n");
		clrtobot();
		pressreturn();
	} else
		clear();
	return 0;
}

#endif

#ifdef BBSDOORS

int ent_bnet(void)
{				/*-- Bill Schwartz */
	int             save_pager = uinfo.pager;
	/*- chopin
	    네트여행에서 pager를 끄는 것을 없�
        uinfo.pager = NA;
        */
	uinfo.mode = BBSNET;
	/*-
        report("신나는 네트웍 여행~") ;
        */
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	/*-- bbsnet.sh is a shell script that can be customized without */
	/*-- having to recompile anything.  If you edit it while someone */
	/*-- is in bbsnet they will be sent back to the xyz menu when they */
	/*-- leave the system they are currently in. */
	reset_tty();
	do_exec("bin/bbsnet.sh", NULL);
	restore_tty();
	uinfo.pager = save_pager;
	uinfo.mode = MMENU;
	/*-
        report("네트웍 여행 끄~읕") ;
        */
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

#endif

/*-- chopin */
int nlp(void)
{
	int             save_pager = uinfo.pager;
	uinfo.pager = NA;
	uinfo.mode = NLP;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	reset_tty();
	do_exec("bin/dict.sh", NULL);
	restore_tty();
	uinfo.pager = save_pager;
	uinfo.mode = NLP;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

int x_viewmemo(void)
{
	struct stat st;
	char buf[100];

	sprintf(buf, "memo.old/%s", currentuser.userid);
	if (stat(buf, &st) == -1 || st.st_size == 0){
		move(3,0);
		prints("저장된 메모가 없습니다. ");
		pressanykey();
		return 0;
	}
        uinfo.mode = VIEWMEMO;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	more(buf, YEA, MORE_IS_NORMAL);
        uinfo.mode = XMENU;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

static char *finduserSearchKey;
static int queryResults;

int printQueryResult(void *record_data)
{
    struct userec * uentp = record_data;

        static int      i;

        if (uentp == NULL) {
		char 		buf[100];
                move(2, 0);
                clrtobot();
                move(2, 0);
                standout();
		sprintf(buf, ">>%s<<를 찾은 결과", finduserSearchKey);
		prints("%-79s\n",  buf);
                standend();
		move(4,0);
                i = 4;
                return 0;
        }
        if (uentp->userid[0] == '\0')
                return 0;
	if (
		(strstr(uentp->userid, finduserSearchKey) == NULL) &&
		(strstr(uentp->realname, finduserSearchKey) == NULL) &&
		(strstr(uentp->username, finduserSearchKey) == NULL) &&
		(strstr(uentp->address, finduserSearchKey) == NULL) &&
		(strstr(uentp->email, finduserSearchKey) == NULL) )
		return 0;
        if (i >= 21) {
                int             ch;
                standout();
                prints("--계 속--");
                standend();
                clrtoeol();
                while ((ch = igetch()) != EOF) {
                        if (ch == '\n' || ch == '\r' || ch == ' ')
                                break;
                        if (ch == 'q' || ch == CTRL('C')) {
                                move(23, 0);
                                clrtoeol();
                                return QUIT;
                        }
                        bell();
                }
                move(4, 0);
                i = 4;
                clrtobot();
        }
	standout();
	prints("%-12s", uentp->userid);
	standend();
	prints(": %-10s : %-52s\n", uentp->realname, uentp->username);
	prints(" > 소  속 : %s\n", uentp->address);
	prints(" > E-mail : %s\n", uentp->email);
	refresh();
	queryResults ++;
        i+=3;
        return 0;
}

int x_findusers(void)
{
	char buf[100];
	getdata(2, 0, "찾을 단어를 넣으세요 : ", buf, 50, DOECHO, NULL);
	if (buf[0] == '\0')
		return 0;

	finduserSearchKey = buf;
        uinfo.mode = QUERYUSERS;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	queryResults = 0;
        printQueryResult((struct userec *) NULL);
        if (apply_record(PASSFILE, printQueryResult, sizeof(struct userec)) == -1) {
                prints("사용자가 없어요.");
                pressreturn();
                uinfo.mode = MMENU;
                substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
                pressanykey();
                return 0;
        }
        clrtobot();
        move(t_lines - 2, 0);
	prints("전부 %d 명을 보셨습니다.", queryResults);
	clrtoeol();
        pressanykey();
        uinfo.mode = MMENU;
        substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
        return 0;
}

