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
#include "edit.h"
#include <time.h>

time_t          cut_time;

int do_user_check(void *record_data)
{
    struct userec * uptr = record_data;

	if (*(uptr->userid) == '\0')
		return 0;
	if ((cut_time > uptr->lastlogin) && !(uptr->flags[0] & NOCLEAN_FLAG))
		AddNameList(uptr->userid);
	return 0;
}

int d_u_list(char * s)
{
	int             id;
	extern int      delusernum;

	if (*s == '*')
		return 0;
	if (!(id = getuser(s)))
		return 0;
	delusernum = id;
	/*
	 * if (!isalpha(lookupuser.userid[0])) return -1;
	 */
	if (lookupuser.userid[0] == '\0') return 0;
	sprintf(genbuf, "/bin/rm -rf mail/%s signature/%s plans/%s mybox/%s overrides/%s msgtmp/%s memo/%s",
		lookupuser.userid, lookupuser.userid, lookupuser.userid,
		lookupuser.userid, lookupuser.userid, lookupuser.userid,
		lookupuser.userid);
	system(genbuf);
	lookupuser.userid[0] = '\0';
	substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	apply_boards(duentbds);
	prints(".");
	refresh();
	return 0;
}

int m_uclean(void)
{
	int             daysold;
	char buf[STRLEN];
	char		*kbuf;

	getdata(1, 0, "지정한 날 수동안 사용하지 않은 사용자를 지웁니다: ",
		buf, 5, DOECHO, NULL);
	kbuf = killsp(buf);
	if ( isdigit(kbuf[0]) )
		daysold = atoi(kbuf);
	else
		daysold = 30;
	cut_time = time(NULL);
	cut_time -= (daysold * 24 * 60 * 60);
	CreateNameList();
	AddNameList("*");
	if (apply_record(PASSFILE, do_user_check, sizeof(struct userec)) == -1)
		return 0;
	move(2, 0);
	namecomplete("사람들을 어떻게 지울까요(* 전부): ", genbuf);
	if (*genbuf == '*') {
		move(2, 0);
		prints("해당되는 사람들을 다 지울까요?  ");
		getdata(3, 0, "(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
		if (*genbuf != 'y' && *genbuf != 'Y') {
			move(2, 0);
			prints("사람들을 지우지 않았습니다\n");
			pressreturn();
			clear();
			return 0;
		}
		move(3, 0);
		/*-		ApplyToNameList(d_u_list) ;*/
		report("deleted ALL inactive users");
		fastdelete();
	} else if (*genbuf != '\0') {
		char            tbuf[10], buf[64];
		move(2, 0);
		prints("활동 안하는 '%s'를 지울까요?  ", genbuf);
		Getyn(tbuf);
		if (tbuf[0] != 'y' && tbuf[0] != 'Y') {
			move(2, 0);
			prints("그 사용자를 지우지 않았습니다\n");
			pressreturn();
			clear();
			return 0;
		}
		d_u_list(genbuf);
		sprintf(buf, "사용자 %s를 지웠습니다", genbuf);
		report(buf);
	}
	clear();
	return 0;
}

int m_xempt(void)
{
	int             id;
	u_namelist();
	clear();
	standout();
	prints("'사용자 지우기'에서 특정 사용자를 제외/포함");
	standend();
	move(1, 0);
	namecomplete("제외/포함 될 사용자 ID를 넣으세요: ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return 0;
	}
	move(3, 0);
	if (!(id = getuser(genbuf))) {
		prints("사용자 ID가 틀립니다");
		clrtoeol();
		pressreturn();
		return 0;
	}
	if (lookupuser.flags[0] & NOCLEAN_FLAG) {
		lookupuser.flags[0] &= ~NOCLEAN_FLAG;
		prints("사용자 '%s'는 더 이상 '사용자 지우기'에서 제외되지 않습니다",
		       lookupuser.userid);
		sprintf(genbuf, "exempt %s", lookupuser.userid);
	} else {
		lookupuser.flags[0] |= NOCLEAN_FLAG;
		prints("사용자 '%s'는 이제부터 '사용자 지우기'에서 제외됩니다", lookupuser.userid);
		sprintf(genbuf, "unexempt %s", lookupuser.userid);
	}
	report(genbuf);
	substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	clrtoeol();
	pressreturn();
	clear();
	return 0;
}

int m_info(void)
{
	int             id;
	struct userec   changes;
	struct userec   initial;

	u_namelist();
	clear();
	standout();
	prints("사용자 정보 얻고 바꾸기");
	standend();
	move(1, 0);
	namecomplete("접근할 사용자 ID를 넣으세요: ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("사용자 ID가 틀립니다");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	memmove(&changes, &lookupuser, sizeof(changes));
	memmove(&initial, &lookupuser, sizeof(initial));
	user_display(&initial);
	getdata(11, 0, "사용자 정보를 바꿀까요(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (*genbuf == 'y' || *genbuf == 'Y') {
		move(12, 0);
		prints("필드를 무시하기 위해서는 리턴을 치세요\n");
		/*-        user_display(&initial) ;*/
enteruid:
		getdata(13, 0, "아이디 : ", genbuf, IDLEN, DOECHO, NULL);
		if (*genbuf != '\0') {
			if (getuser(genbuf)) {
				move(3, 0);
				prints("Error! 동일한 사용자 ID가 이미 존재합니다\n");
				bell();
				move(13, 0);
				clrtobot();
				goto enteruid;
			}
			strncpy(changes.userid, genbuf, IDLEN + 2);
		}
dopasswd:
		getdata(14, 0, "암  호 : ", genbuf, 20, NOECHO, NULL);
		if (genbuf[0] != '\0') {
			char            temp[20];
			getdata(15, 0, "암호를 다시 넣으세요 : ", temp, 20, NOECHO, NULL);
			if (strcmp(temp, genbuf)) {
				move(3, 0);
				prints("Error!  암호가 맞지 않습니다\n");
				move(14, 0);
				clrtobot();
				goto dopasswd;
			}
			strncpy(changes.passwd, genpasswd(genbuf), PASSLEN);
		}
		getdata(15, 0, "별  명 : ", genbuf, STRLEN, DOECHO, NULL);
		if (*genbuf != '\0')
			strncpy(changes.username, genbuf, STRLEN);
		getdata(16, 0, "터미날 : ", genbuf, STRLEN, DOECHO, NULL);
		if (*genbuf != '\0')
			strncpy(changes.termtype, genbuf, STRLEN);
#ifndef PERMS
		if (currentuser.userlevel == 255) {
			getdata(17, 0, "권  한 : ", genbuf, 5, DOECHO, NULL);
			if (*genbuf != '\0')
				changes.userlevel = atoi(genbuf);
		}
#endif
#ifdef REALINFO
		getdata(17, 0, "본  명 : ", genbuf, STRLEN - 40, DOECHO, NULL);
		if (*genbuf != '\0')
			strncpy(changes.realname, genbuf, STRLEN - 40);
		getdata(18, 0, "소  속 : ", genbuf, STRLEN, DOECHO, NULL);
		if (*genbuf != '\0')
			strncpy(changes.address, genbuf, STRLEN);
		getdata(19, 0, "이메일 : ", genbuf, STRLEN, DOECHO, NULL);
		if (*genbuf != '\0')
			strncpy(changes.email, genbuf, STRLEN);
		getdata(20, 0, "바꾼 것을 적용할까요(Y/N)[N] ? ",
#else
		getdata(17, 0, "이메일 : ", genbuf, STRLEN, DOECHO, NULL);
		if (*genbuf != '\0')
			strncpy(changes.email, genbuf, STRLEN);
		getdata(20, 0, "바꾼 것을 적용할까요(Y/N)[N] ? ",
#endif
			genbuf, 4, DOECHO, NULL);
		if (*genbuf == 'y' || *genbuf == 'Y') {
			if (strcmp(changes.userid, initial.userid)) {
				char            from[512], to[512];
				struct stat st;

				sprintf(from, "mail/%s", initial.userid);
				sprintf(to, "mail/%s", changes.userid);
				if ( stat(from, &st) == 0){
					if (rename(from, to) < 0){
						prints("\n편지함을 옮길수 없습니다.  아이디 변경을 취소합니다.");
						pressanykey();
						clear();
						return 0;
					}
				}
				sprintf(from, "plans/%s", initial.userid);
				sprintf(to, "plans/%s", changes.userid);
				rename(from, to);
				sprintf(from, "signatures/%s", initial.userid);
				sprintf(to, "signatures/%s", changes.userid);
				rename(from, to);
				sprintf(from, "overrides/%s", initial.userid);
				sprintf(to, "overrides/%s", changes.userid);
				rename(from, to);
				sprintf(from, "mybox/%s", initial.userid);
				sprintf(to, "mybox/%s", changes.userid);
				rename(from, to);
				sprintf(from, "msgtmp/%s", initial.userid);
				sprintf(to, "msgtmp/%s", changes.userid);
				rename(from, to);
				sprintf(from, "memo/%s", initial.userid);
				sprintf(to, "memo/%s", changes.userid);
				rename(from, to);
				sprintf(from, "memo/%s.old", initial.userid);
				sprintf(to, "memo/%s.old", changes.userid);
				rename(from, to);
				sprintf(from, "wait_me/%s", initial.userid);
				sprintf(to, "wait_me/%s", changes.userid);
				rename(from, to);
			}
			substitute_record(PASSFILE, &changes, sizeof(changes), id);
			sprintf(genbuf, "modified %s user info --> %s",
				initial.userid, changes.userid);
			report(genbuf);
		}
	}
	clear();
	return 0;
}

extern int      numboards;

int m_editbrd(void)
{
	char            bname[STRLEN];
	char            bpath[STRLEN * 2];
	struct stat     statb;
	int             pos;
	struct fileheader fh, newfh;
	make_blist();
	clear();
	standout();
	prints("글상자 정보 바꾸기");
	standend();
	move(1, 0);
	namecomplete("글상자 이름을 넣으세요 : ", bname);
	if (*bname == '\0') {
		move(2, 0);
		prints("글상자 이름이 잘못되었습니다");
		pressreturn();
		clear();
		return 0;
	}
	/* 디렉토리일때는 ㅤ 끝의 '/'를빼줌 */
	if (bname[strlen(bname) - 1] == '/')
		bname[strlen(bname) - 1] = '\0';

	pos = search_record(bfile(BOARDS), &fh, sizeof(fh), cmpbnames, (intptr_t) bname);
	if (!pos) {
		move(2, 0);
		prints("글상자 이름이 잘못되었습니다");
		pressreturn();
		clear();
		return 0;
	}
	move(3, 0);
	memmove(&newfh, &fh, sizeof(newfh));
	prints("글상자 이름 :        %s\n", fh.filename);
	prints("글상자 설명 : %s\n", fh.title);
#ifdef PERMS
	if (fh.level & PERM_POSTMASK)
		prints("글쓰기 권한 등급 : %s\n", (fh.level & ~PERM_POSTMASK) == 0 ? "Unrestricted" : "Restricted");
	else
		prints("읽기 권한 등급 : %s\n", fh.level == 0 ? "Unrestricted" : "Restricted");
#else
	prints("사용 권한 등급 :   %d\n", fh.level);
#endif
	/*
	 * prints("비실명 글상자%s\n", isNonameBoard(bname) ?  "임": "아님");
	 */
	prints("비실명 글상자%s\n", fh.isNoname ? "임" : "아님");

	/* <---- by wings */
	prints("글상자의 주인 : %s\n", fh.owner);
	/* by wings ----> */

	getdata(7, 0, "글상자 정보를 바꿀까요(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (*genbuf == 'y' || *genbuf == 'Y') {
		move(9, 0);
		prints("Field를 무시할려면 리턴을 치세요\n");
enterbname:
		getdata(10, 0, "새로운 글상자 이름 :         ", genbuf, 18, DOECHO, NULL);
		if (*genbuf != 0) {
			struct fileheader dh;
			/*
			 * chopin sprintf(bpath, "boards/%s", genbuf);
			 * if(search_record(BOARDS, &dh, sizeof(dh),
			 * cmpbnames, (intptr_t) genbuf))
			 */
			/* chopin:bpath??? */
			sprintf(bpath, "%s", bfile(genbuf));
			if (search_record(bfile(BOARDS), &dh, sizeof(dh), cmpbnames, (intptr_t) genbuf)) {
				move(3, 0);
				prints("Error! 동일한 이름의 글상자가 있습니다\n");
				bell();
				move(10, 0);
				clrtobot();
				goto enterbname;
			}
			strncpy(newfh.filename, genbuf, sizeof(newfh.filename));
		}
		getdata(11, 0, "새 글상자 설명 :  ", genbuf, 60, DOECHO, NULL);
		if (*genbuf != 0)
			strncpy(newfh.title, genbuf, sizeof(newfh.title));

		/* <---- by wings */
enterowner:
		getdata(12, 0, "글상자 주인의 아이디 :  ", genbuf, 60, DOECHO, NULL);
		if (*genbuf != 0) {
			if (!getuser(genbuf)) {
				prints("사용자명이 이상합니다.\n");
				goto enterowner;
			} else
				strncpy(newfh.owner, genbuf, sizeof(newfh.owner));
		}
		/* by wings ----> */

#ifdef PERMS
		getdata(13, 0, "비실명 글상자로 할까요(Y/N)[N] ? ", genbuf, 2, DOECHO, NULL);
		if (genbuf[0] == 'y' || genbuf[0] == 'Y') {
			newfh.isNoname = YEA;
		} else {
			newfh.isNoname = NA;
		}
		getdata(14, 0, "권한 등급을 바꿀까요(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
		if (*genbuf == 'Y' || *genbuf == 'y') {
			char            ans[4];
			sprintf(genbuf, "읽기(R) 혹은 쓰기(P) 제한 [%c]: ",
				(newfh.level & PERM_POSTMASK ? 'P' : 'R'));
			getdata(13, 0, genbuf, ans, 4, DOECHO, NULL);
			if ((newfh.level & PERM_POSTMASK) && (*ans == 'R' || *ans == 'r'))
				newfh.level &= ~PERM_POSTMASK;
			else if (!(newfh.level & PERM_POSTMASK) && (*ans == 'P' || *ans == 'p'))
				newfh.level |= PERM_POSTMASK;
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("글상자 '%s'에 %s 권한을 설정했습니다\n", newfh.filename, newfh.level & PERM_POSTMASK ? "쓰기" : "읽기");
			newfh.level = setperms(newfh.level);
			clear();
			getdata(0, 0, "바꾼 것을 적용할까요(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
		} else
			getdata(14, 0, "바꾼 것을 적용할까요(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
#else
		getdata(12, 0, "새로운 권한 등급 :       ", genbuf, 4, DOECHO, NULL);
		if (*genbuf != 0)
			newfh.level = atoi(genbuf);
		getdata(14, 0, "바꾼 것을 적용할까요(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
#endif
		if (*genbuf == 'Y' || *genbuf == 'y') {
			if (strcmp(fh.filename, newfh.filename)) {
				char            from[256], to[256];
				/*
				 * chopin sprintf(from, "boards/%s",
				 * fh.filename); sprintf(to, "boards/%s",
				 * newfh.filename);
				 */
				sprintf(from, "%s", bfile(fh.filename));
				sprintf(to, "%s", bfile(newfh.filename));
				rename(from, to);
			}
			/*
			 * chopin substitute_record(BOARDS, &newfh,
			 * sizeof(newfh), pos);
			 */
			substitute_record(bfile(BOARDS), &newfh, sizeof(newfh), pos);
			sprintf(genbuf, "changed %s board info --> %s",
				fh.filename, newfh.filename);
			report(genbuf);
			numboards = -1;	/*- force re-caching */
		}
	}
	clear();
	return 0;
}

int user_display(struct userec * u)
{
	move(4, 0);
	prints("아이디 : %s\n", u->userid);
	prints("별  명 : %s\n", u->username);
	prints("터미널 : %s\n", u->termtype);
#ifndef PERMS
	if (currentuser.userlevel > 0)
		prints("권  한 : %d\n", u->userlevel);
#endif
#ifdef REALINFO
	prints("본  명 : %s\n", u->realname);
	prints("이메일 : %s\n", u->email);
	prints("소  속 : %s\n", u->address);
#endif
	return 0;
}

FILE           *cleanlog;
char            curruser[IDLEN + 2];
extern int      delmsgs[];
extern int      delcnt;

/*******************************************/
int monthday[13] = {
	0,
	31,
	31+28,
	31+28+31,
	31+28+31+30,
	31+28+31+30+31,
	31+28+31+30+31+30,
	31+28+31+30+31+30+31,
	31+28+31+30+31+30+31+31,
	31+28+31+30+31+30+31+31+30,
	31+28+31+30+31+30+31+31+30+31,
	31+28+31+30+31+30+31+31+30+31+30,
	31+28+31+30+31+30+31+31+30+31+30+31
};
int totalDay(int y, int m, int d)
{
	return y*365 + monthday[m-1] + d;
}

int isOlder(struct fileheader * fhdrp, int day)
{
	time_t clk;
	struct tm *ttm;
	clk = time(NULL);
	ttm = localtime(&clk);
	return  (totalDay(ttm->tm_year, ttm->tm_mon, ttm->tm_mday) -
		totalDay(fhdrp->tm_year, fhdrp->tm_mon, fhdrp->tm_mday)) > day;
}
/*******************************************/

int domailclean(void *record_data)
{
    struct fileheader * fhdrp = record_data;

	static int      newcnt, savecnt, deleted, idc;
	char            buf[STRLEN];
	if (fhdrp == NULL) {
		fprintf(cleanlog, "new = %d, saved = %d, deleted = %d\n",
			newcnt, savecnt, deleted);
		newcnt = savecnt = deleted = idc = 0;
		if (delcnt) {
			sprintf(buf, "mail/%s%s", curruser, DIR);
			delete_records(buf, sizeof(struct fileheader),
				delmsgs, delcnt);
/*
			while (delcnt--){
				move(5, 0);
				prints("%03d", delcnt);
				refresh();
				delete_record(buf, sizeof(struct fileheader), delmsgs[delcnt]);
			}
*/
		}
		delcnt = 0;
		return 0;
	}
	idc++;
	if (!(fhdrp->accessed[0] & FILE_READ))
		newcnt++;
	else if (fhdrp->accessed[0] & FILE_MARKED)
		savecnt++;
	else if ( isOlder(fhdrp, 92) ) {
		deleted++;
		sprintf(buf, "mail/%s/%s", curruser, fhdrp->filename);
		unlink(buf);
		delmsgs[delcnt++] = idc;
		move(5, 0);
		prints("%d", delcnt); refresh();
	}
	return 0;
}

int cleanmail(void *record_data)
{
    struct userec * urec = record_data;

	struct stat     statb;
	if (urec->userid[0] == '\0' || !strcmp(urec->userid, "new"))
		return 0;
	sprintf(genbuf, "mail/%s%s", urec->userid, DIR);
	fprintf(cleanlog, "%s: ", urec->userid);
	if (stat(genbuf, &statb) == -1)
		fprintf(cleanlog, "no mail\n");
	else if (statb.st_size == 0)
		fprintf(cleanlog, "no mail\n");
	else {
		strcpy(curruser, urec->userid);
		delcnt = 0;
		move(4,0);
		prints(">> [%s] 님의 편지함을 정리하고 있습니다. >>", urec->userid);
		clrtoeol();
		move(5, 0);
		clrtoeol();
		refresh();
		apply_record(genbuf, domailclean, sizeof(struct fileheader));
		domailclean(NULL);
	}
	return 0;
}

int m_mclean(void)
{
	char            ans[4];
	clear();
	move(0, 0);
	standout();
	prints("편지함 비우기");
	standend();
	move(1, 0);
	prints("92일 이상 되고 마크가 안된 모든 편지를 지웁니다. 그래서 느리지요..\n");
	getdata(2, 0, "정말(Y/N)[N] ? ", ans, 3, DOECHO, NULL);
	if (ans[0] != 'Y' && ans[0] != 'y') {
		clear();
		return 0;
	}
	cleanlog = fopen("mailclean.log", "w");
	move(3, 0);
	prints("작동중 -- 끈기있게 기다리세요.\n");
	refresh();
	if (apply_record(PASSFILE, cleanmail, sizeof(struct userec)) == -1) {
		move(4, 0);
		prints("PASSFILE을 못 열겠습니다, 관둠...\n");
		pressreturn();
		clear();
		return 0;
	}
	move(4, 0);
	fclose(cleanlog);
	prints("우와! 다 됐다! Log는 mailclean.log에 있습니다.\n");
	report("Mail Clean");
	pressreturn();
	clear();
	return 0;
}

int m_trace(void)
{
	struct stat     ostatb, cstatb;
	int             otraceoff, ctraceoff, done = 0;
	char            ans[2];
	char           *msg;
	clear();
	move(0, 0);
	standout();
	prints("추적 옵션 설정");
	standend();
	clrtobot();
	while (!done) {
		move(2, 0);
		otraceoff = stat("trace", &ostatb);
		ctraceoff = stat("trace.chatd", &cstatb);
		prints("Current Trace Settings:\n");
		if (otraceoff)
			prints("일반 추적은 OFF\n");
		else
			prints("비비 추적은 ON (파일 크기 = %d)\n", ostatb.st_size);
		if (ctraceoff)
			prints("대화 수호신 추적은 OFF\n");
		else
			prints("대화 수호신 추적은 ON (파일 크기 = %d)\n", cstatb.st_size);
		move(6, 0);
		prints("넣으세요:\n");
		prints("비비 추적을 %s하기는 <1>\n", otraceoff ? "가능" : "불가능");
		prints("대화 수호신 추적을 %s하기는 <2> \n", ctraceoff ? "가능" :
		       "불가능");
		getdata(9, 0, "그밖에 따른 글쇠는 나갑니다: ", ans, 2, DOECHO, NULL);
		switch (ans[0]) {
		case '1':
			if (otraceoff) {
				system("/bin/touch trace");
				msg = "비비 추적 시작.";
				report("opened report log");
			} else {
				report("closed report log");
				system("/bin/mv trace trace.old");
				msg = "비비 추적 불가능 ; log는 trace.old에 있습니다.";
			}
			break;
		case '2':
			if (ctraceoff) {
				system("/bin/touch trace.chatd");
				msg = "대화 수호신 추적 시작.";
				report("chatd trace log opened");
			} else {
				system("/bin/mv trace.chatd trace.chatd.old");
				msg = "대화 수호신 추적 불가능 ; log는 trace.chatd.old에 있습니다.";
				report("chatd trace log closed");
			}
			break;
		default:
			msg = NULL;
			done = 1;
		}
		move(t_lines - 1, 0);
		if (msg)
			prints("%s\n", msg);
	}
	clear();
	return 0;
}

#ifdef VOTE

#include <sys/file.h>

int clearflags(void)
{
	int             fd;
	struct userec   uent;
	if ((fd = open(PASSFILE, O_RDWR)) == -1) {
		prints("윽! 패스 파일은 못 열겠습니다!\n");
		return -1;
	}
	flock(fd, LOCK_EX);
	while (read(fd, &uent, sizeof(uent)) == sizeof(uent)) {
		uent.flags[0] &= ~VOTE_FLAG;	/*- clear that sucker */
		lseek(fd, -sizeof(uent), L_INCR);	/*- back up */
		if (write(fd, &uent, sizeof(uent)) != sizeof(uent))
			break;
	}
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

int suckinfile(FILE * fp, char * fname)
{
	char            inbuf[256];
	FILE           *sfp;
	if ((sfp = fopen(fname, "r")) == NULL)
		return -1;
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
		fputs(inbuf, fp);
	fclose(sfp);
	return 0;
}

int closepolls(void)
{
	FILE           *cfp, *tfp;
	char            inchar, inbuf[80];
	int             bfd, counts[10];
	time_t          endtime;
	if ((cfp = fopen("vote/control", "r")) == NULL) {
		if (currentuser.flags[0] & VOTE_FLAG) {
			currentuser.flags[0] &= ~VOTE_FLAG;
			substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
		}
		return 0;
	}
	if (fgets(inbuf, sizeof(inbuf), cfp) == NULL) {
		fclose(cfp);
		return 0;
	}
	fclose(cfp);
	endtime = (time_t) atol(inbuf);
	if (endtime > time(0) || endtime <= 0)
		return 0;
	if (rename("vote/control", "vote/temp") == -1)
		return 0;
	report("closed the polls");
	for (bfd = 0; bfd < 10; bfd++)
		counts[bfd] = 0;
	if ((bfd = open("vote/ballots", O_RDONLY)) != -1) {
		while (read(bfd, &inchar, 1) == 1)
			counts[(int) (inchar - '0')]++;
		close(bfd);
	}
	if (tfp = fopen("vote/newresults", "w")) {
		int             totvotes = 0;
		fprintf(tfp, "-------------------------------------------------------------------------\n\n");
		fprintf(tfp, "** 투표가 끝난 시간 : %s\n", time_string_no_newline(endtime));
		fprintf(tfp, "** 투표 주제 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
		suckinfile(tfp, "vote/desc");
		fprintf(tfp, "\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		fprintf(tfp, "** 투표 결과 :\n\n");
		if (cfp = fopen("vote/temp", "r")) {
			int             cindex;
			fgets(inbuf, sizeof(inbuf), cfp);
			while (fgets(inbuf, sizeof(inbuf), cfp)) {
				inbuf[(strlen(inbuf) - 1)] = '\0';
				inbuf[23] = '\0';	/*- truncate */
				cindex = inbuf[0] - '0';
				fprintf(tfp, "    %-20s  %3d\n", inbuf + 3, counts[cindex]);
				totvotes += counts[cindex];
			}
			fclose(cfp);
		}
		fprintf(tfp, "\n투표에 모두 %d분이 참여하셨습니다.\n\n", totvotes);
		suckinfile(tfp, "vote/results");
		fclose(tfp);
	} else
		return -1;	/*- couldn't write out the results! */
	unlink("vote/results");
	rename("vote/newresults", "vote/results");
	unlink("vote/desc");
	unlink("vote/temp");
	unlink("vote/ballots");
	clearflags();
	currentuser.flags[0] &= ~VOTE_FLAG;
	return 0;
}

int m_vote(void)
{
	struct stat     statb;
	FILE           *fp;
	char            inbuf[80];
	char           *newline;
	int             numselections = 0, aborted;
	time_t          closetime, numseconds;
	clear();
	move(0, 0);
	standout();
	prints("투표함 열기");
	standend();
	move(2, 0);
	if ((fp = fopen("vote/control", "r")) != NULL) {
		fgets(inbuf, sizeof(inbuf), fp);
		fclose(fp);
		if (newline = strchr(inbuf, '\n'))
			*newline = '\0';
		closetime = atof(inbuf);
		prints("미안, 투표함이 벌써 열렸는데.\n");
		prints("투표 마감 시간은 %s로 예정 되었습니다.", ctime(&closetime));
		prints("투표를 일찍 마칠려면 'vote/control'를 고치세요.");
		pressreturn();
		clear();
		return 0;
	}
	prints("투표 용지 설명을 고치기 위해서는 아무글쇠나 누르세요: \n");
	igetch();
	aborted = vedit("vote/desc", NA, TEXT_IS_EDIT);
	if (aborted) {
		clear();
		prints("투표함 열기 실패.\n");
		pressreturn();
		clear();
		return 0;
	}
	fp = fopen("vote/control", "w");
	clear();
	getdata(0, 0, "투표함을 열어 놓고 있을 날수 : ", inbuf, 4, DOECHO, NULL);
	numseconds = atof(inbuf) * 86400.0;
	time(&closetime);
	closetime += numseconds;
	fprintf(fp, "%lu\n", closetime);
	fflush(fp);
	while (!aborted) {
		getdata(numselections + 1, 0, "내용을 하나씩 입력하세요.(다 되었으면 리턴): ",
			inbuf, 40, DOECHO, NULL);
		if (*inbuf) {
			if (numselections == 9)
				numselections = 0;
			else
				numselections++;
			fprintf(fp, "%1d) %s\n", numselections, inbuf);
		}
		if (*inbuf == '\0' || numselections == 0)
			aborted = 1;
	}
	fclose(fp);
	clearflags();
	report("opened the polls");
	prints("투표함을 열었습니다!\n");
	pressreturn();
	clear();
	return 0;
}

#endif				/*- VOTE */
