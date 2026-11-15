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
#include "more.h"
#include <time.h>
#include <unistd.h>
#define BADLOGINSTRSZ	72
#define BADLOGINFILE	"logins.bad"
/*- chopin */
char            home_dir[MAX_DIRNAME];	/* 비비홈디렉토리 */
char            board_dir[MAX_DIRNAME];	/* 현재디렉토리 */
char            board_home[MAX_DIRNAME];	/* 보드 홈디렉토리 */

jmp_buf         byebye;

FILE           *ufp;
long            ti;
int             talkrequest = NA;
int             bfinger = 0;

struct user_info uinfo;

char            fromhost[STRLEN];
char            BoardName[STRLEN];

int             utmpent = -1;
void u_enter(void)
{
	uinfo.active = YEA;
	uinfo.pid = getpid();
#ifdef PERMS
	if (HAS_PERM(PERM_LOGINCLOAK))
#else
	if (currentuser.userlevel >= CLOAKONLOGIN)
#endif
		uinfo.invisible = currentuser.flags[0] & CLOAK_FLAG ? YEA : NA;
	else
		uinfo.invisible = NA;
	uinfo.sockactive = NA;
	uinfo.sockaddr = 0;
	uinfo.destuid = 0;
	uinfo.mode = LOGIN;
	uinfo.pager = !(currentuser.flags[0] & PAGER_FLAG);
	uinfo.in_chat = NA;
	uinfo.refuse_call = NA;
	memset(uinfo.chatid, 0, sizeof(uinfo.chatid));
	uinfo.uid = usernum;
	strncpy(uinfo.from, fromhost, STRLEN);
	utmpent = getnewutmpent(&uinfo);
}

int setflags(int mask, int value)
{
	if (((currentuser.flags[0] & mask) && 1) != value) {
		if (value)
			currentuser.flags[0] |= mask;
		else
			currentuser.flags[0] &= ~mask;
		return 1;
	} else
		return 0;
}

int u_exit(void)
{
	int             f1 = 0, f2 = 0;
	f1 = setflags(PAGER_FLAG, !uinfo.pager);
#ifdef PERMS
	if (HAS_PERM(PERM_LOGINCLOAK))
#else
	if (currentuser.userlevel >= CLOAKONLOGIN)
#endif
		f2 = setflags(CLOAK_FLAG, uinfo.invisible);
	if (f1 || f2)
		substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	uinfo.active = NA;
	uinfo.pid = 0;
	uinfo.invisible = YEA;
	uinfo.sockactive = NA;
	uinfo.sockaddr = 0;
	uinfo.destuid = 0;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int cmpuids(intptr_t argument, const void *record_data)
{
    char * uid = (char *)argument;
    const struct userec * up = record_data;

	return !ci_strncmp(uid, up->userid, sizeof(up->userid));
}

int dosearchuser(char * userid)
{
	if (!strcmp(userid, "new"))
		return YEA;	/* + by bugman "new" 라는 아이디를1 만들 수
				 *  */
	return usernum = search_record(PASSFILE, &currentuser, sizeof(currentuser),
				       cmpuids, (intptr_t)userid);
}

int             started = 0;

void print_talk_request(void)
{
	int 	orgx, orgy;
	getyx(&orgy, &orgx);
	move(0, 64);
	prints("[");
	standout();
	prints(" 삐삐진동 ");
	standend();
	prints("]");
	move(orgy, orgx);
	return;
}
char * getLastMessageSender(void)
{
        FILE           *fp;
        char            fn[100];
	static char retVal[14];
        int             i;
        struct msgRec   msgBuf;
	char *s;
	char *loc;

	retVal[0] = '\0';
        sprintf(fn, "msgtmp/%s", currentuser.userid);
        fp = fopen(fn, "rb");
        if (fp == NULL) {
                return retVal;
        }
        if (fread(&msgBuf, sizeof(msgBuf), 1, fp) != 1) {
		goto AQUIT;
        }
	s = &(msgBuf.msg[msgBuf.index][3]);
	loc = strstr(s, "님의 ");
	if (loc == NULL) loc = strstr(s, "님이 ");
	if (loc == NULL) {
		goto AQUIT;
	}
	for (i=0; i<13 && s != loc && s != NULL; i++){
		retVal[i] = *s;
		s++;
	}
	retVal[i] = '\0';
AQUIT:
	fclose(fp);
	return retVal;
}
void print_msg_request(void)
{
	FILE           *fp;
	char            fn[100];
	int		orgx, orgy;
	extern int      msgLine;
	struct msgRec	msgBuf;

	sprintf(fn, "msgtmp/%s", currentuser.userid);
	fp = fopen(fn, "rb");
	if (fp == NULL) {
		return;
	}
	if (fread(&msgBuf, sizeof(msgBuf), 1, fp) != 1) {
		fclose(fp);
		return;
	}
	if (msgBuf.index <0 || msgBuf.index >= MSGNUM){
		fclose(fp);
		return;
	}
	fclose(fp);

	getyx(&orgy, &orgx);
	move(0, 64);
	prints("[");
	standout();
	prints( (msgBuf.msg[msgBuf.index][0] != 'I') ? " 쪽지왔네 ": " 채팅초대 ");
	standend();
	prints("]");
	move(msgLine, 0);
	standout();
	prints(msgBuf.msg[msgBuf.index]);
	standend();
	move(orgy, orgx);
	refresh();
	return;
}
void talk_request(int dummy)
{
	signal(SIGUSR1, talk_request);
	if (uinfo.refuse_call) return;
	talkrequest = YEA;
	bell();			/* 토크신청 메시지 찍어주기 */
	print_talk_request();
	refresh();
	return;
}
void msg_request(int dummy)
{
	signal(SIGUSR2, msg_request);
	if (uinfo.refuse_call) return;
	bell();			/* 이야기초대/쪽지 메시지 찍어주기 */
	print_msg_request();
	return;
}

_Noreturn void abort_bbs(int dummy)
{

	if (started){
		dumpEditing();
		usieslog("AXXED", currentuser.userid, currentuser.username, "");
		u_exit();
	}
	exit(0);
}

int cmpuids2(intptr_t unum, const void *record_data)
{
    const struct user_info * urec = record_data;

	return (unum == urec->uid);
}

int multi_user_check(void)
{
	struct user_info uin;
	char            buffer[40];
#ifdef PERMS
	if (HAS_PERM(PERM_MULTILOG))
		return 0;		/*- don't check sysops */
#else
	if (currentuser.userlevel >= MULTILOGOK)
		return 0;		/*- don't check sysops */
#endif

	if (isGuest(currentuser.userid))
		return 0;		/* by bugman : 손님, guest는  멀티로긴을
				 *   */
	if (!search_record(ULIST, &uin, sizeof(uin), cmpuids2, usernum))
		return 0;		/*- user isn't logged in */
	if (!uin.active || (kill(uin.pid, 0) == -1))
		return 0;		/*- stale entry in utmp file */
	prints("이 아이디는 이미 사용중입니다.\n멀티로긴은 안되요...\n\n");
	prints("이미 들어온 아이디를 쫓아낼까요?\n");
	getdata(0, 0, "(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
		printf("\n쩝~.\n안녕히 가세요..!\n\n");
		oflush();
		Goodbye();
	} else {
		/*- by bugman : dumpEdit 를 위해서 signal을 SIGHUP으로 바꿈
		kill(uin.pid, 9);
		*/
		kill(uin.pid, SIGHUP);
		sprintf(buffer, "kicked (multi-login)");
		report(buffer);
		usieslog("KICK", currentuser.userid, currentuser.username, "");
	}
	return 0;
}

int account_disabled(char * userid)
{
	FILE           *fp;
	char            buf[STRLEN];
	int             blen;
	if ((fp = fopen(".disabled", "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		blen = strlen(buf);
		if (buf[blen - 1] == '\n')
			buf[--blen] = '\0';
		if (!strncmp(buf, userid, blen)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

#include <sys/file.h>
#include <signal.h>
#include <stdlib.h>		/* - chopin for getenv() */

#define sameStr(a, b)   !strcmp(a, b)
int isGuest(char * userid)
{
	return (
		sameStr(userid, "guest") ||
		sameStr(userid, "손님") ||
		sameStr(userid, "게스트") ||
		sameStr(userid, "구경꾼") ||
		sameStr(userid, "sonnim")
		);
}

int dispMiscInfo(void)
{				/*- bugman :  장난삼아 만들어 본것.. */
#define LOGINCOUNTFILE "./.totallogin"

//	char           *bannerChar = "@#0OZAYDKP--";
	int             total;
	char            sbuf[100];
	char            fn[100];
	int             fd = open(LOGINCOUNTFILE, O_RDWR, 0660);

	if (fd < 0)
		return 0;

	flock(fd, LOCK_EX);
	read(fd, &total, sizeof(total));
	total++;
	lseek(fd, 0, SEEK_SET);
	write(fd, &total, sizeof(total));
	flock(fd, LOCK_UN);
	close(fd);

	sprintf(fn, "tmp/bbsFortuneCountTemporary.XXXXXX");
	sprintf(sbuf, "bin/mkFortune %d %s", total, mktemp(fn));
//		bannerChar[getpid() % 10]);
	system(sbuf);
	more(fn, YEA, MORE_IS_LOGIN);
	unlink(fn);
	return 0;
}

void enableTalkFly(void)
{
	uinfo.refuse_call = NA;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
}
void disableTalkFly(void)
{
	uinfo.refuse_call = YEA;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
}

int main(int argc, char ** argv)
{

	char            uid[STRLEN];
	char *pass;
	char            passbuf[PASSLEN];
	char            ans[4];
	char           *cmds;	/* instead of 'cmd' */
	int             cmd;
	int             nu = 0, numbad;
	int             fd;
	int             getin;
	int             baduserid, attempts = 0;
#ifdef SINGLE
	{
		char            c[256];
		gethostname(c, 256);
		if (strcmp(c, SINGLE)) {
			printf("Not on a valid machine!\n");
			exit(-1);
		}
	}
#endif
	getin = NA;
	if (argc == 2) {
		switch (*argv[1]) {
		case 'U':
		case 'u':
			/*- User status check */
			UserStatus();
			exit(0);
		case 's':
		case 'S':
			getin = YEA;
			break;

		case 'l':
		case 'L':
			bfinger = 1;
			dumb_user_list();
			exit(0);
		case 'a':
			list_all_users();
			exit(0);
		default:
			fprintf(stderr, "UNKNOWN OPTION\n");
			exit(0);
		}
	}
	if (argc == 3){
		strncpy(fromhost, argv[2], STRLEN);
		fromhost[30]='\0';

		if (argv[1][0] == 'R') getin = YEA;
	}
	else {
		fromhost[0] = '\0';
		}

#ifndef lint
	signal(SIGHUP, abort_bbs);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
#ifdef DOTIMEOUT
	init_alarm();
#else
	signal(SIGALRM, SIG_IGN);
#endif
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
#endif
	signal(SIGUSR1, talk_request);
	signal(SIGUSR2, msg_request);

	umask(0007);

	/*- added from pbbs 1.8 --gtv */
	/* num_active_users() 는 talk.c에 있음 - ohno */
	/*
	fprintf(stderr, "current users = %d  getin = %d\n", num_active_users(), getin);
	*/
	if (!getin && num_active_users() >= MAXACTIVE) {
		if ((ufp = fopen("etc/boot.off", "r")) == NULL)
			fprintf(stderr, "\n정원이 다 찼습니다.\n다음 기회에 또...\n");
		else
			while (fgets(genbuf, 256, ufp) != NULL)
				fprintf(stderr, "%s", genbuf);
		exit(-1);
	}
	/*- added from pbbs 1.8 --gtv */

	if (setjmp(byebye)) {
		if (started){
			usieslog("ABOR", currentuser.userid, currentuser.username, "");
			u_exit();
		}
		printf("안녕히 가세요.\n");
		oflush();
		Goodbye();
	}
	get_tty();
	init_tty();

		strcpy(BoardName, BOARDNAME);

	lmore("etc/Welcome", NA, MORE_IS_NORMAL);

grok:
	lmore("etc/issue", NA, MORE_IS_NORMAL);
	uid[0] = 0;
#ifdef LOGINASNEW
	getdata(0, 0, "\n아이디를 넣어 주세요.\n\n * 새로 가입하실분은 'new', 둘러보시려면 'guest/구경꾼', 볼일 없으시면 'off'\n\n아이디 [대/소문자 구별] : ", uid, STRLEN, DOECHO, NULL);
#else
	getdata(0, 0, "\n아이디를 넣어 주세요. * 둘러 보실려면 'guest/구경꾼' , 볼일 없으면 'off'\n\n아이디 [대/소문자 구별] : ", uid, STRLEN, DOECHO, NULL);
#endif
	if (uid[0] == '\0')
		goto grok;
	if (!strcmp(uid, "off")) {
		prints("안녕히 가세요!\n");
		oflush();
		Goodbye();
	}
	if (!strcmp(uid, "new")) {	/*- new 이면 */
#ifdef LOGINASNEW
		struct userec   newuser;
		char           *s;
		int             allocid;

	/* Here
		prints("죄송합니다. 시스템 문제로 당분간 새사용자 신청을 받지 않\n습니다.");
		refresh();
		sleep(5);
		exit(1);
	*/

		nu = 1;
		memset(&newuser, 0, sizeof(newuser));
		allocid = getnewuserid();
		if (allocid >= (MAXUSERS-1) || allocid <= 0) {
			prints(
"\n\n+--------------------------------------+\n| 더 이상 사용자를 받을 자리가 없군요! |\n| 아쉽지만 다음 기회에...              |\n+--------------------------------------+\n\n\n");
			oflush();
			Goodbye();
		}
		/* redo: */
		cat("etc/New.Info/All");
		pressanykey();
		printf("\n");
redo:
		cat("etc/New.Info/NewID");
/*		prints("* 아이디를 새로 만듭니다 * -'한글'아이디가 가능합니다.\n\n"); */
		getdata(0, 0, "아이디('off'는 관둠) : ", newuser.userid, IDLEN + 1, DOECHO, NULL);
		if (!strcmp(newuser.userid, "off")) {
			newuser.userid[0] = 0;
			substitute_record(PASSFILE, &newuser, sizeof(newuser), allocid);
			prints("안녕히 가세요!\n");
			sleep(1);
			exit(1);
		}
		if (*newuser.userid == '\0' || !strcmp(newuser.userid, "new") ||
			!strcmp(newuser.userid, "NEW") ) {
			prints("아이디가 잘못되었습니다.\n");
			goto redo;
		}
		for (s = newuser.userid; *s != '\0'; s++){
			if (isalnum(*s)) continue;
			if ((*s & 0x80) != 0){
				s++;
				if ( *s != '\0' ){
					if ( (*s & 0x80) != 0) continue;
				}
			}
			prints(" !! 이 아이디는 적합한 아이디가 아닙니다. 다시 입력해 주십시오.\n   한글과 영문, 숫자만 가능합니다.\n");
			goto redo;
		}
		if (dosearchuser(newuser.userid)) {
			prints("이 아이디는 이미 다른 사람이 사용하고 있습니다.\n\n");
			goto redo;
		}
dopassagain:
		cat("etc/New.Info/Password");
		getdata(0, 0, "암  호 : ", passbuf, PASSLEN, NOECHO, NULL);
		if (passbuf[0] == '\0') {
			prints("암호를 넣으세요.\n");
			goto dopassagain;
		}
		strncpy(newuser.passwd, passbuf, PASSLEN);
		getdata(0, 0, "다시 한번 : ", passbuf, PASSLEN, NOECHO, NULL);
		if (strncmp(passbuf, newuser.passwd, PASSLEN)) {
			prints("암호가 다릅니다.\n");
			goto dopassagain;
		}
		newuser.passwd[8] = '\0';
		strncpy(newuser.passwd, (char *)genpasswd(newuser.passwd), PASSLEN);
		cat("etc/New.Info/Nick");
		getdata(0, 0, "별  명 : ", newuser.username, STRLEN, DOECHO, NULL);
#ifdef REALINFO
		cat("etc/New.Info/Realname");
		{
			int spaced, koreaned, i;
regetRealName:
			getdata(0, 0, "본  명 : ", newuser.realname, STRLEN - 40, DOECHO, NULL);
                        spaced = 0;
                        koreaned = 1;
			if (!strcmp(newuser.realname, "off")){
				exit(0);
			}
                        for(i=0; newuser.realname[i]; i++){
                                if (newuser.realname[i] == ' '){
                                        spaced = 1;
                                }
                        }
                        for(i=0; newuser.realname[i]; i++){
				if (newuser.realname[i] & 0x80){
					i++;
					if (newuser.realname[i] == '\0') break;
				} else {
                                	koreaned = 0;
				}
                        }
                        if (spaced || !koreaned || newuser.realname[0] == '\0'){
				prints(" !! 이 이름은 적합한 이름이 아닙니다. 다시 입력해 주십시오.\n    빈칸이나 영문은 쓰지말아 주십시오.\n");
				goto regetRealName;
			}

		}
		cat("etc/New.Info/Address");
		getdata(0, 0, "소  속 : ", newuser.address, STRLEN, DOECHO, NULL);
#endif
		cat("etc/New.Info/Email");
		getdata(0, 0, "이메일 : ", newuser.email, STRLEN, DOECHO, NULL);
getterm:
		cat("etc/New.Info/Terminal");
		getdata(0, 0, "터미널 (ansi,dansi,vt100,xterm,dumb,...)[기본=vt100]:",
			newuser.termtype, STRLEN, DOECHO, NULL);
		if (newuser.termtype[0] == '\0') {
			strcpy(newuser.termtype, "vt100");
		}
		if (!term_init(newuser.termtype)) {
			prints("터미널 종류가 잘못되었습니다.\n");
			goto getterm;
		}
		/*
		 * { char buf[10]; getdata(0, 0, "아이디를 만들까요? (Y/N)[N}
		 * ", buf, 2, DOECHO, NULL); if ( buf[0] != 'Y' && buf[0] !=
		 * 'y'){ fprintf(stderr, "\n아이디�
		 * ....\n"); exit(1); } }
		 */

#ifdef PERMS
		newuser.userlevel = PERM_DEFAULT;
#else
		newuser.userlevel = 0;
#endif
		newuser.lastlogin = time(NULL);
		newuser.protocol = 0;
/**-
		reset_tty();
		printf("\n\n** 지금까지 입력한 정보입니다.  **\n");
		printf("아이디 : %s\n", newuser.userid);
		printf("별  명 : %s\n", newuser.username);
		printf("본  명 : %s\n", newuser.realname);
		printf("소  속 : %s\n", newuser.address);
		printf("터미널 : %s\n", newuser.termtype);
		printf("이메일 : %s\n\n", newuser.email);
		restore_tty();
		getdata(10, 0, "위와 같은 내용으로 아이디를 신청하시겠습니까? (Y/N)[N] ", genbuf, 4, DOECHO, NULL);
		if (genbuf[0] != 'y' && genbuf[0] != 'Y'){
			fprintf(stderr, "다음기회에.........\n");
			exit(1);
		}
-**/
		if (substitute_record(PASSFILE, &newuser, sizeof(newuser), allocid) == -1) {
			fprintf(stderr, "꽉 찼습니다. 미안해요!\n");
			Goodbye();
		}
		touchnew();
		if (!dosearchuser(newuser.userid)) {
			fprintf(stderr, "새 사용자 만들기 실패\n");
			Goodbye();
		} {
			char            target_fullpath[100];
			char            sbuf[100];
			struct fileheader msg;
			int             fp;
			char           *ip;

			memset(&msg, 0, sizeof(msg));
			sprintf(target_fullpath, "M.%d.A", time(0));
			sprintf(sbuf, "mail/SYSOP/%s", target_fullpath);
			ip = strrchr(target_fullpath, 'A');
			/* 이름이 중복되면 다시 고른다 */
			while ((fp = open(sbuf, O_CREAT | O_EXCL | O_WRONLY, 0664)) == -1) {
				if (*ip == 'Z')
					ip++, *ip = 'A', *(ip + 1) = '\0';
				else
					(*ip)++;
				sprintf(sbuf, "mail/SYSOP/%s", target_fullpath);
			}
			close(fp);

			strcpy(msg.filename, target_fullpath);	/* 파일네임만 */
			strcpy(target_fullpath, sbuf);	/* 풀패스 */

			sprintf(sbuf, "%s (%s)", newuser.userid, newuser.username);
			strncpy(msg.owner, sbuf, STRLEN);
			sprintf(msg.title, "** 새사용자 신청(본명:%s)", newuser.realname);
			{	/* 타임세팅 */
				time_t          clk;
				struct tm      *tm;
				clk = time(NULL);
				tm = localtime(&clk);
				msg.tm_year = tm->tm_year;
				msg.tm_mon = tm->tm_mon;
				msg.tm_mday = tm->tm_mday;
			}

			{
				FILE           *outf;
				/* 메일 파일 복사, 에러메시지는 시삽만 봅니다 */
				if ((outf = fopen(target_fullpath, "wt"))) {
					fprintf(outf, "------------------------------------------------\n\n");
					fprintf(outf, "   * 아  이   디 : %s\n\n", newuser.userid);
					fprintf(outf, "   * 터미널 타입 : %s\n\n", newuser.termtype);
					fprintf(outf, "   * 본       명 : %s\n\n", newuser.realname);
					fprintf(outf, "   * 소       속 : %s\n\n", newuser.address);
					fprintf(outf, "   * 이  메   일 : %s\n\n", newuser.email);
					fprintf(outf, "------------------------------------------------\n");
					fprintf(outf, "위의 신청자가 아이디를 신청하였습니다.\n\n");
					fprintf(outf, "본명과 소속을 잘 확인하세요.\n\n");
					fprintf(outf, "옳게 되었으면 대문자 A, 잘못되었으면 대문자 X를 눌러주세요.\n");

					fclose(outf);
				} else {
					printf("SYSOP에게 편지를 보낼수 없습니다");
					sleep(4);
					exit(0);
				}
			}
			/* 인덱스에 등록 */
			sprintf(sbuf, "mail/SYSOP%s", DIR);
			if (append_record(sbuf, &msg, sizeof(msg)) == -1) {
				prints("SYSOP에게 편지를 보낼수 없습니다.");
				sleep(4);
				exit(0);
			} else {
				int             i;

				cat("etc/New.Info/Enter");

				setNoticeToNew();
				for (i = 0; i < 18; i++) {
					sleep(2);
					puts("");
				}
				for (; i < 24; i++) {
					puts("");
				}
			}
		}

		report("new account");
#else
		prints("아이디가 잘못되었습니다..\n");
		oflush();
		goto grok;
#endif
	} else {
		baduserid = NA;
		if (!dosearchuser(uid))
			baduserid = YEA;
		if (isGuest(uid)) {
			strcpy(passbuf, "nari");
			if (tooManyUsers()){
				prints(" 사용자가 너무 많아서 손님은 들어오실 수 없습니다.\n");
				oflush();
				Goodbye();
			}
		} else {
			char buf[80];
			sprintf(buf, "암  호 [ %12s ] : ", uid);
			getdata(0, 0, buf, passbuf, PASSLEN, NOECHO, NULL);
		}
		passbuf[8] = '\0';
		if (baduserid || !checkpasswd(currentuser.passwd, passbuf)) {
			//logattempt(uid, fromhost, baduserid);
			 prints("%s 잘못되었습니다.\n", baduserid ? "아이디가":"암호가");
			oflush();
			if (passbuf!= NULL) attempts++;
			if (attempts == LOGINATTEMPTS) {
				prints("아이디와 암호를 잘 생각해 보세요!\n");
				oflush();
				Goodbye();
			} else {
				goto grok;
			}
		}
		if (account_disabled(currentuser.userid) || !HAS_PERM(PERM_BASIC)) {
			prints("죄송합니다. 이 아이디는 사용이 중지되었습니다.\n");
			prints("안녕히!\n\n");
			oflush();
			Goodbye();
		}
#ifndef MULTI_LOGINS
		multi_user_check();
#endif
	}
	if (!term_init(currentuser.termtype)) {
		prints("터미날 종류 오류. 그냥 'dumb'으로 해드립니다.\n");
		term_init("dumb");
	}
	if (!strcmp(currentuser.userid, "SYSOP"))
#ifdef PERMS
		currentuser.userlevel = ~0;	/*- SYSOP gets all permission bits */
#else
		currentuser.userlevel = 255;
#endif
		fromhost[16] = '\0';
		usieslog("LOGIN", currentuser.userid, currentuser.realname, fromhost);
		u_enter();
		if (nu) {
			uinfo.mode = NEW;
			substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		}
	report("Enter");
	started = 1;
	initscr();
	scrint = 1;
	move(0, 0);
	clear();
	dispMiscInfo();
	dispNotice(YEA);
	chkMemo();
	chkWaiters();
	if ((numbad = countattempts()) < 0)
		showattempts(NA);
	else {			/* + semi : to here... */
		if (numbad > 8) {	/* + chopin */
			refresh();
			sprintf(genbuf, "알림 : %d번의 암호입력 시도가 있었습니다. 볼까요 (Y/N,sp)? [N]: ", numbad);
			getone(t_lines - 1, 0, genbuf, ans, 4, DOECHO, NULL);
			if (*ans == 'y' || *ans == 'Y')
				showattempts(YEA);
			else
				showattempts(NA);
		}
	}
	strncpy(currentuser.lasthost, fromhost, 16);
	currentuser.lasthost[15] = '\0';	/*- dumb mistake on my part */
	currentuser.lastlogin = time(NULL);
	currentuser.numlogins++;
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	m_init();
#ifdef SYSV
	if (get_semaphore() == -1) {
		printf("Can't get semaphore! Exiting...\n");
		exit(1);
	}
#endif
#ifdef VOTE
	closepolls();
#endif
	if (nu || !HAS_PERM(PERM_BASIC)) {
		cmd = 'x';
		cmds = "x\n";
	} else {
		cmd = 'n';
		cmds = "n";
	}
	if (chkmail()) {
		cmd = 'm';
		cmds = "m";
	}
	/*- chopin 시작 디렉토리는 ~bbs 로 하고
	    현 보드디렉토리 정보를 board_dir에 가진다*/
	strcpy(home_dir, getenv("BBSHOME"));
	strcpy(board_home, "boards");	/* 시작 보드 디렉토리는 ./boards */
	strcpy(board_dir, "boards");	/* current 보드디렉토리 */

	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	docmd(BoardName,"명령을 치세요 : ",cmds, cmdlist, boardmargin, Help);
	exit(0);
	return 0;
}

char * boardmargin(void)
{
	static char     buf[STRLEN];
	if (selboard && currboard != 0)
		sprintf(buf, "<%s>", xboard_dir(currboard));
	else {
		sprintf(buf, "<*Unselected*>");
	}
	return buf;
}

int             refscreen = NA;

int egetch(void)
{
	char            c;
	int             rval;

	if (talkrequest) {
		talkreply();
		refscreen = YEA;
		return -1;
	}
egetagain:
	c = igetch();
	if (talkrequest) {
		talkreply();
		refscreen = YEA;
		return -1;
	}
	if (c == CTRL('O')){
		do_fly_on_line(3, NA);
	}
#ifdef BIT8
	if (!isprint2(c)) {
#else
	if (!isprint(c)) {
#endif
		switch (c) {
		case '\r':
		case '\n':
		case '\010':
		case '\177':
			break;
		case CTRL('L'):
			redoscr();
		default:
			goto egetagain;
		}
	}
	rval = c;
	refscreen = NA;
	return rval;
}

/******************************************************** inserted by bugman ---> */
void viewScreen(char * cmdtitle, char * cmdprompt, struct commands * cmdtable, bbs_margin marginnote, bbs_action helpFn)
{
	if (dumb_term) {
		prints("%-36s %s\n%s", cmdtitle, (*marginnote) (), cmdprompt);
		return;
	};
	clear();
	if (helpFn != NULL)
		(*helpFn) ();
	move(0, 0);
	prints("%-36s %s", cmdtitle, (*marginnote) ());
	clrtoeol();
	print_newmail_message();
	move(1, 0);
	prints("%s", cmdprompt);
}
/* <----- inserted by bugman *************************************************** */

int docmd(char * cmdtitle, char * cmdprompt, char * firstcmds, struct commands * cmdtable, bbs_margin marginnote, bbs_action helpFn)
{
	int             cmd, lastcmdptr;
	int             cmdplen;
	int             reprint;

	cmdplen = strlen(cmdprompt);
grok:
	/*
	 * - by bugman clear(); reprint = YEA; prints("%s ", cmdtitle);
	 *
	 * print_newmail_message(); / print_current_boardname(); /
	 *
	 * / skji prints("%-56s\n", (*marginnote) ()); / prints("%-56s\n",
	 * (*marginnote) ()); move(1, 0); prints("%s", cmdprompt);
	 */
	reprint = YEA;
	cmd = firstcmds[0];
	lastcmdptr = 0;
	viewScreen(cmdtitle, cmdprompt, cmdtable, marginnote, helpFn);	/* + by bugman */
	do {
		extern int      dumb_term;
		int             i = 0;
		int             err;

		if (cmd == '\n' || cmd == '\r') {
			if (dumb_term)
				prints("\n");
			else {
				move(1, cmdplen);
				clrtoeol();
				refresh();
			}
			clrstandout();
			if ((err = (*cmdtable[lastcmdptr].cmdfunc) ()) == QUIT)
				return 0;
			/*
			 * - by bugman move(0, 0); clrtoeol(); reprint = YEA;
			 * prints("%s ", cmdtitle); / move(0, 30); if
			 * (chkmail()) prints("(편지가 왔습니다.)"); else
			 * prints("                  "); move(0, 50);
			 * prints("%s\n", (*marginnote) ()); / /
			 * print_current_boardname(); /
			 * print_newmail_message();
			 *
			 * move(1, 0); prints("%s", cmdprompt); clrtoeol();
			 */
			reprint = YEA;
			viewScreen(cmdtitle, cmdprompt, cmdtable, marginnote, helpFn);	/* + by bugman */
			if (err)
				cmd = *(cmdtable[lastcmdptr].errorcmd);
			else
				cmd = *(cmdtable[lastcmdptr].nextcmd);
		}
		while (i != 100) {
			if (cmdtable[i].cmdname == NULL)
				break;
			if ((*(cmdtable[i].cmdname) & ~0x20) == (cmd & ~0x20))
				break;
			i++;
		}
		if (cmdtable[i].cmdname == NULL) {
			bell();
			continue;
		}
#ifdef PERMS
		if (!HAS_PERM(cmdtable[i].level)) {
#else
		if (cmdtable[i].level > currentuser.userlevel) {
#endif
			bell();
			continue;
		}
		if (dumb_term) {
			int             lastcmdlen;
			int             newcmdlen;
			int    j;

			if (reprint) {
				reprint = NA;
				prints("%s", cmdtable[i].cmdname);
				lastcmdptr = i;
				continue;
			}
			if (lastcmdptr == i)
				continue;
			if (lastcmdptr == 0) {
				prints("%s", cmdtable[i].cmdname);
				lastcmdptr = i;
				continue;
			}
			lastcmdlen = strlen(cmdtable[lastcmdptr].cmdname);
			newcmdlen = strlen(cmdtable[i].cmdname);
			for (j = 0; j < lastcmdlen; j++)
				ochar(CTRL('H'));
			prints("%s", cmdtable[i].cmdname);
			if (lastcmdlen > newcmdlen) {
				int    k = lastcmdlen - newcmdlen;
				for (j = 0; j < k; j++)
					ochar(' ');
				for (j = 0; j < k; j++)
					ochar(CTRL('H'));
			}
			lastcmdptr = i;
		} else {
			lastcmdptr = i;
			move(1, cmdplen);
			standout();
			prints("%s", cmdtable[i].cmdname);
			standend();
			clrtoeol();
		}
	} while ((cmd = egetch()) != EOF);
	/*- chopin
	    if (refscreen)
		goto grok;
	*/
	if (refscreen)
		goto grok;	/* + semi */

	abort_bbs(0);
}

int m_adduser(void)
{
	struct userec   newuser;
	struct userec   saveuser;
	char           *s;
	int             allocid, saveunum;
	char            passbuf[PASSLEN], ans[4];
	clear();
	memmove(&saveuser, &currentuser, sizeof(saveuser));
	memset(&newuser, 0, sizeof(newuser));
	saveunum = usernum;
	allocid = getnewuserid();
	if (allocid >= MAXUSERS || allocid <= 0) {
		prints("새 사용자를 만들 공간이 없습니다!\n");
		sleep(1);
		return 0;
	}
	standout();
	prints("새로운 사용자 추가");
	standend();
	clrtoeol();
redo:
	getdata(1, 0, "아이디 ('off'는 관둠): ",
		newuser.userid, IDLEN + 1, DOECHO, NULL);
	if (!strcmp(newuser.userid, "off")) {
		prints("사용자 만들기를 관둡니다.\n");
		pressreturn();
		clear();
		return (0);
	}
	if (*newuser.userid == '\0' || !strcmp(newuser.userid, "new")) {
		prints("아이디가 잘못되었습니다.\n");
		goto redo;
	}
	for (s = newuser.userid; *s != '\0'; s++)
		/*
		 * if (!isalpha(*s) || (*s & 0x80)) {
		 */
		if (!(isalnum(*s) || ((*s & 0x80) != 0))) {
			prints("아이디가 잘못됐습니다.\n");
			goto redo;
		}
	if (dosearchuser(newuser.userid)) {
		prints("이미 있는 아이디입니다.\n");
		goto redo;
	}
dopassagain:
	getdata(2, 0, "암호 : ", passbuf, PASSLEN, NOECHO, NULL);
	if (passbuf[0] == '\0') {
		prints("암호를 넣으세요.\n");
		goto dopassagain;
	}
	strncpy(newuser.passwd, passbuf, PASSLEN);
	getdata(2, 0, "암호 한 번 더 : ", passbuf, PASSLEN, NOECHO, NULL);
	if (strncmp(passbuf, newuser.passwd, PASSLEN)) {
		prints("암호 입력이 잘못되었습니다.\n");
		goto dopassagain;
	}
	newuser.passwd[8] = '\0';
	strncpy(newuser.passwd, (char *)genpasswd(newuser.passwd), PASSLEN);
	getdata(3, 0, "별  명 : ", newuser.username, STRLEN, DOECHO, NULL);
getterm:
	getdata(4, 0, "터미널(ansi,dansi,vt100,xterm,dumb,...)[기본=vt100]:",
		newuser.termtype, STRLEN, DOECHO, NULL);
	if (newuser.termtype[0] == '\0') {
		strcpy(newuser.termtype, "vt100");
	}
#ifdef PERMS
	newuser.userlevel = PERM_DEFAULT;
#else
	newuser.userlevel = 0;
#endif
	newuser.lastlogin = time(NULL);
	newuser.protocol = 0;
#ifdef REALINFO
	getdata(5, 0, "본  명 : ", newuser.realname, STRLEN - 40, DOECHO, NULL);
	getdata(6, 0, "소  속 : ", newuser.address, STRLEN, DOECHO, NULL);
	getdata(7, 0, "이메일 : ", newuser.email, STRLEN, DOECHO, NULL);
#else
	getdata(5, 0, "이메일 : ", newuser.email, STRLEN, DOECHO, NULL);
#endif
	memmove(&currentuser, &saveuser, sizeof(currentuser));
	usernum = saveunum;
	getdata(9, 0, "이 사용자를 추가할까요 (Y/N)? [Y]: ", ans, 4, DOECHO, NULL);
	if (ans[0] == 'n' || ans[0] == 'N') {
		newuser.userid[0] = '\0';
		substitute_record(PASSFILE, &newuser, sizeof(newuser), allocid);
		prints("사용자 만들기를 관둡니다.\n");
		pressreturn();
		clear();
		return (0);
	}
	if (substitute_record(PASSFILE, &newuser, sizeof(newuser), allocid) == -1) {
		prints("스페이스가 부족합니다!\n");
		pressreturn();
		clear();
		return 0;
	}
	sprintf(genbuf, "%s가 추가되었습니다.", newuser.userid);
	report(genbuf);
	prints("\n새 사용자가 만들어졌습니다.\n");
	touchnew();
	pressreturn();
	clear();
	return 0;
}


int logattempt(char * uid, char * frm, int badid)
{
	int a;
	char            buf[BADLOGINSTRSZ],badfile[STRLEN];
	char            timestr[40];
	time_t t;
	t=time(NULL);
	sprintf(timestr,"%s",time_string_no_newline(t));
	//sprintf(timestr,"%ld",t);
	uid[12] = '\0';
	strcpy(badfile,"logins.bad");
	sprintf(buf, strlen(frm) >= 23 ? "%-12s %-32s %.23s %c\n":"%-12s %-32s %-23s %c\n", uid, timestr, frm, badid ? 'Y' : 'N');
//	a=append_record(BADLOGINFILE, buf, BADLOGINSTRSZ);
	return 0;
}

int countattempts(void)
{
	int             fd, cnt = 0;
	char            buf[BADLOGINSTRSZ];
#ifdef PERMS
	if (!HAS_PERM(PERM_BASIC))
		return -1;	/*- don't show the bad login attempts */
#endif
	if ((fd = open(BADLOGINFILE, O_RDONLY)) == -1)
		return 0;
	while (read(fd, buf, BADLOGINSTRSZ) == BADLOGINSTRSZ)
		if (!strncmp(buf, currentuser.userid, strlen(currentuser.userid))
		    && buf[63] == 'N')
			cnt++;
	close(fd);
	return cnt;
}

int showattempts(int really)
{
	int             fd, ln = 1;
	char            buf[BADLOGINSTRSZ];
	if ((fd = open(BADLOGINFILE, O_RDWR)) == -1)
		return 0;
	if (really)
		clear();
	while (read(fd, buf, BADLOGINSTRSZ) == BADLOGINSTRSZ) {
		if (!strncmp(buf, currentuser.userid, strlen(currentuser.userid))
		    && buf[63] == 'N') {
			buf[63] = 'Y';
			flock(fd, LOCK_EX);
			if (lseek(fd, -BADLOGINSTRSZ, L_INCR) != -1)
				write(fd, buf, BADLOGINSTRSZ);
			flock(fd, LOCK_UN);
			if (really) {
				buf[45] = buf[62] = '\0';
				if (ln >= t_lines) {
					prints("계속  -- 아무 글쇠나 누르세요.\n");
					igetch();
					clear();
					ln = 1;
				}
				prints("UNSUCCESSFUL LOGIN from %s on %s\n", &buf[46], &buf[13]);
				ln++;
			}
		}
	}
	close(fd);
	if (really) {
		pressreturn();
		clear();
	}
	return 0;
}

void chkWaiters(void)
{
        int     tuid;
	char fn[50];
	char buf[100];
	char uident[30];
	FILE *fp;
        struct user_info uin;

	sprintf(fn, "wait_me/%s", currentuser.userid);
	fp = fopen(fn, "rt");
	if (fp == NULL) return;
	sprintf(buf, ">> %s님이 떳다는 소문이 들리네요.", currentuser.userid);
	clear();
	move(1,0);
	while (fscanf(fp, "%s", uident) == 1){;
        	if ( uident[0] == '\0'  || !(tuid = getuser(uident)) || tuid == usernum)
			continue;
		prints("** %s 님이 기다리고 있었습니다. ", uident);
        	if (search_record(ULIST, &uin, sizeof(uin), t_cmpuids, tuid)){
			if (uin.refuse_call){
				prints("--> 쪽지를 보낼수가 없습니다.");
			}if (uin.active){
        			writeFlyingMessage(uident, buf);
				kill(uin.pid, SIGUSR2);
				prints("--> 쪽지를 보냈습니다.\n");
			} else
				prints("--> 지금은 안계시는군요.\n");
		} else {
			prints("--> 지금은 안계시는군요.\n");
		}
	}
	pressanykey();
	unlink(fn);
}

void chkMemo(void)
{
	char buf[100];
	char buf2[100];
	struct stat sb;

	sprintf(buf, "memo/%s", currentuser.userid);

	if (stat(buf, &sb) < 0 || sb.st_size == 0) {
	} else {
		more(buf, YEA, MORE_IS_NORMAL);
	}
	sprintf(buf2, "memo.old/%s", currentuser.userid);
	rename(buf, buf2);
}

