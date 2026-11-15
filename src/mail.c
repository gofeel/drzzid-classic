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
#include "more.h"

// #define MaxCc	10	/*	skji: 동시에 받을 수 있는 최대 인원 */

#include <time.h>

char            currmaildir[4096];

int m_init(void)
{
	sprintf(currmaildir, "mail/%s/%s", currentuser.userid, DIR);
	return 0;
}

int send_each_mail(char * userid, char * filename, char * mail_subject)
{
	struct fileheader newmessage;
	char            target_fullpath[4096];
	char           *ip;
	struct stat     st;
	int             fp;
	FILE           *inf, *outf;
	char            rbuf[500];

	if (!getuser(userid))
		return -1;
#ifdef PERMS
	if ((lookupuser.userlevel & PERM_READMAIL) == 0)
		return -3;
#endif
	if (! HAS_PERM(PERM_POST)){
		if (strcmp(lookupuser.userid, "SYSOP")) return -4;
	}

	/* 메일 디렉토리가 없으면 새로 만든다 */
	sprintf(genbuf, "mail/%s", userid);
	if (stat(genbuf, &st) == -1) {
		if (mkdir(genbuf, 0770) == -1) {
			if (HAS_PERM(PERM_SYSOP)) {
				prints("메일 디렉토리를 만드는 데 실패했습니다.\n");
				pressanykey();
			}
			return -1;
		}
		if (0 /* skji */ && HAS_PERM(PERM_SYSOP)) {
			prints("메일 디렉토리를 새로 만들었습니다.\n");
			prints("디렉토리: %s\n", genbuf);
			pressanykey();
		}
	} else {
		if (!(st.st_mode & S_IFDIR)) {
			return -1;
		}
	}

	/*- 타겟 파일 이름 고르기 */
	memset(&newmessage, 0, sizeof(newmessage));
	sprintf(target_fullpath, "M.%d.%s.A", time(0), currentuser.userid);
	sprintf(genbuf, "mail/%s/%s", userid, target_fullpath);
	ip = strrchr(target_fullpath, 'A');
	/* 이름이 중복되면 다시 고른다. */
	while ((fp = open(genbuf, O_CREAT | O_EXCL | O_WRONLY, 0664)) == -1) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(genbuf, "mail/%s/%s", userid, target_fullpath);
	}
	close(fp);

	strcpy(newmessage.filename, target_fullpath);	/* 파일네임만 */
	strcpy(target_fullpath, genbuf);	/* 풀 패스 */

	in_mail = YEA;
#if defined(REALINFO) && defined(MAIL_REALNAMES)
	sprintf(genbuf, "%s (%s)", currentuser.userid, currentuser.realname);
#else
	sprintf(genbuf, "%s (%s)", currentuser.userid, currentuser.username);
#endif
	strncpy(newmessage.owner, genbuf, STRLEN);
	strcpy(newmessage.title, mail_subject);
	{			/*- 타임 세팅 */
		time_t          clk;
		struct tm      *tm;
		clk = time(NULL);
		tm = localtime(&clk);
		newmessage.tm_year = tm->tm_year;
		newmessage.tm_mon = tm->tm_mon;
		newmessage.tm_mday = tm->tm_mday;
	}

	{
		/* 메일 파일 복사, 에러메시지는 시삽만 봅니다 */
		if ((inf = fopen(filename, "r"))) {
			if ((outf = fopen(target_fullpath, "w"))) {
				while (fgets(rbuf, 500, inf))
					fprintf(outf, rbuf);
				fclose(outf);
			} else if (HAS_PERM(PERM_SYSOP)) {
				prints("ERROR: 쓰기에러\n");
				pressanykey();
			}
			fclose(inf);
		} else if (HAS_PERM(PERM_SYSOP)) {
			prints("ERROR: 읽기에러\n");
			pressanykey();
		}
		/* security 를 위해서 모드를 바꿈 + by bugman */
		chmod(target_fullpath, 0600);
	}
	/* 인덱스에 등록 */
	sprintf(genbuf, "mail/%s%s", userid, DIR);
	if (append_record(genbuf, &newmessage, sizeof(newmessage)) == -1)
		return -1;

	/*
	 * if (HAS_PERM(PERM_SYSOP)) { clear(); prints("[DEBUG]\n");
	 * prints("(시삽 등급만 이 메시지를 � )\n"); prints(" %s\n",
	 * currentuser.username); prints("받 는 이 : %s\n", userid); prints("�
	 *  %s\n", newmessage.title); prints(" %s\n", filename); prints("
	 *  %s\n", target_fullpath); prints("인 � %s\n", genbuf); prints("시
	 *  %02d.%02d.%02d\n", newmessage.tm_year, newmessage.tm_mon,
	 * newmessage.tm_mday); pressanykey(); }
	 */
	sprintf(genbuf, "mailed %s", userid);
	report(genbuf);
	return 0;
}

int do_send(char ** userids, char * title, char * srcFile)
{
	char            firstmail[500];
	char            firstmail_subject[500];
	int             i;

	if (*userids == 0)
		return 0;

	sprintf(firstmail, "mail/M.%d.A.%s.TMP", time(0), currentuser.userid);
	strcpy(firstmail_subject, "");

	/* '참조'란 출력 */
	if (Cc_list[0])
		move(3, 0), prints("참  조: %s", Cc_list);

	if (title == 0 || title[0] == 0)
		getdata(2, 0, "제  목: ", firstmail_subject, STRLEN, DOECHO, NULL);
	else
		strcpy(firstmail_subject, title);

	/*- 헤더를 저장하는 펑션인, write_header함수는
           글의 제목이 전역변수은 save_title에 들어있을 것으�
           가정하므로... save_title에 제목을 미리 세팅해 주어야 한다. */

	strcpy(save_title, firstmail_subject);

	if (srcFile == NULL) {
		if (vedit(firstmail, YEA, TEXT_IS_MAIL) == -1) {
			clear();
			return -2;
		}
	} else {
		if (vedit_from_file(srcFile, firstmail, YEA, TEXT_IS_MAIL) == -1) {
			clear();
			return -2;
		}
	}
	chmod(firstmail, 0600);
	clear();
	prints("\n\n\n");
	/*
	 * if (HAS_PERM(PERM_SYSOP)) { prints("[DEBUG]\n"); prints("시삽 등급만 �
	 * "); prints("고친 편지 파일 이름: %s\n", firstmail); prints("
	 * firstmail_subject); pressanykey(); }
	 */

	while (*userids) {
		int             errcode;
		char            tbuf[500];
		FILE           *fp;
		errcode = send_each_mail(*userids, firstmail, firstmail_subject);

		if (errcode == 0){
#ifdef MAIL_FORWARD
			sprintf(tbuf, "mail/%s/.forward", *userids);
			if (fp = fopen(tbuf, "rt")) {
				char            addr[100];
				char           *p;
				while (fgets(addr, 99, fp)) {
					if ((p = strrchr(addr, '\n')) != NULL) {
						*p = '\0';
					}
					if (!invalidaddr(addr)) {
						sprintf(tbuf,
							"bin/Mail -s \"Mail from NARI(%s to %s)\" %s < %s > /dev/null",
							currentuser.userid, *userids, addr, firstmail);
						system(tbuf);
					}
					break;
				}
				fclose(fp);
			}
#endif
		}
		strcpy(tbuf, *userids);
		/*
		 * prints("%-20s", tbuf);
		 */
		switch (errcode) {
		case -1:
			prints("아이디가 틀립니다.[%12s]\n", tbuf);
			break;
		case -2:
			prints("[%12s] 님에게 쓰던 편지를 찢었습니다.\n", tbuf);
			break;
#ifdef PERMS
		case -3:
			prints("[%12s] 님은 편지를 받을 수 없습니다.\n", tbuf);
			break;
#endif
		case -4:
			prints("[%12s] 님에게 편지를 보낼 수 없습니다.\n", tbuf);
			break;
		default:
			prints("[%12s] 님에게 편지를 보냈습니다.\n", tbuf);
			break;
		}
		userids++;
		refresh();
	}
	unlink(firstmail);

	pressanykey();
	return 0;
}

int m_send(void)
{
	char           *uident[MaxCc + 1], cc_buf[500], tbuf[100];
	int             cc_cnt, i;

#ifdef SEABASS
	if (!HAS_PERM(PERM_BASIC)) {
		CreateNameList();
		if (!strcmp(currentuser.userid, "advice"))
			AddNameList("Gabby");
		else {
			AddNameList("ACCOUNT");
			AddNameList("SYSOP");
		}
	} else
#endif				/*- SEABASS */
		u_namelist();
	move(1, 0);
	clrtoeol();
	prints("편지보내기");
	/* skji */
	prints("(%d명까지 동시에 보낼 수 있습니다. 차례로 적으세요.)", MaxCc);
	cc_cnt = 0;
	do {
		move(2, 0);
		clrtoeol();
		strcpy(cc_buf, "받을 사람: ");
		for (i = 0; i < cc_cnt && i < MaxCc; i++) {
			strcat(cc_buf, uident[i]);
			strcat(cc_buf, " ");
		}
		tbuf[0] = 0;
		namecomplete(cc_buf, tbuf);	/* 이름 선택 */
		if (tbuf[0] == 0) {
			if (cc_cnt == 0) {
				clear();
				return 0;
			} else
				break;
		}
		if (getuser(tbuf)) {	/* by bugman 없는 사용자 체크 */
			uident[cc_cnt] = (char *) strdup(tbuf);
			cc_cnt++;
		}
	} while (cc_cnt < MaxCc);
	uident[cc_cnt] = 0;

	strcpy(Cc_list, "");
	if (cc_cnt > 1) {
		for (i = 0; uident[i]; i++)
			strcat(Cc_list, uident[i]), strcat(Cc_list, " ");
	}
	uinfo.mode = SMAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	in_mail = YEA;		/* sangil:메일의 헤더 형식이 게시판과 */
	do_send(uident, NULL, NULL);	/* skji: 여러사람에게 메일 발사! */
	for (i = 0; uident[i]; i++) {
		free(uident[i]);
	}

	/*- pressanykey(); */
	uinfo.mode = MAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	return 0;
}
int m_send_mybox(void)
{
	char           *uident[MaxCc + 1], cc_buf[500], tbuf[100];
	int             cc_cnt, i;
	char            buf[120];

	sprintf(buf, "mybox/%s", currentuser.userid);
	{
		FILE           *fp;
		fp = fopen(buf, "r");
		if (fp == NULL) {
			prints("사물함에 내용이없네요.. ");
			pressanykey();
			return 0;
		}
	}
	u_namelist();
	move(1, 0);
	clrtoeol();
	prints("사물함 내용 보내기");
	/* skji */
	prints("(%d명까지 동시에 보낼 수 있습니다. 차례로 적으세요.)", MaxCc);
	cc_cnt = 0;
	do {
		move(2, 0);
		clrtoeol();
		strcpy(cc_buf, "받을 사람: ");
		for (i = 0; i < cc_cnt && i < MaxCc; i++) {
			strcat(cc_buf, uident[i]);
			strcat(cc_buf, " ");
		}
		tbuf[0] = 0;
		namecomplete(cc_buf, tbuf);	/* 이름 선택 */
		if (tbuf[0] == '\0') {
			if (cc_cnt == 0) {
				clear();
				return 0;
			} else
				break;
		}
		if (getuser(tbuf)) {	/* by bugman 없는 사용자 체크 */
			uident[cc_cnt] = (char *) strdup(tbuf);
			cc_cnt++;
		}
	} while (cc_cnt < MaxCc);
	uident[cc_cnt] = 0;

	strcpy(Cc_list, "");
	if (cc_cnt > 1) {
		for (i = 0; uident[i]; i++)
			strcat(Cc_list, uident[i]), strcat(Cc_list, " ");
	}
	uinfo.mode = SMAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	in_mail = YEA;		/* sangil:메일의 헤더 형식이 게시판과 */
	do_send(uident, NULL, buf);	/* skji: 여러사람에게 메일 발사! */
	for (i = 0; uident[i]; i++) {
		free(uident[i]);
	}

	/*- pressanykey(); */
	uinfo.mode = MAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}
int print_mailerror_message(char * userid, int errcode)
{
	char            tbuf[100];
	sprintf(tbuf, "[%s]", userid);
	prints("%-12s: ", tbuf);
	switch (errcode) {
	case -1:
		prints("아이디가 틀립니다.\n");
		break;
	case -2:
		prints("쓰던 편지를 찢었습니다.\n");
		break;
#ifdef PERMS
	case -3:
		prints("편지를 받을 수 없습니다.\n");
		break;
#endif
	default:
		prints("편지를 우체통에 넣었습니다.\n");
	}
	return 0;
}

int read_mail(struct fileheader * fptr)
{
	sprintf(genbuf, "mail/%s/%s", currentuser.userid, fptr->filename);
	more(genbuf, 0, MORE_IS_NORMAL);
	fptr->accessed[0] |= FILE_READ;
	return 0;
}

int             mrd;

int             delmsgs[1024];
int             delcnt;

#define DEBUG1(S1) if (HAS_PERM(PERM_SYSOP)) \
	prints("\n"), clrtoeol(), prints("DEBUG: %s", S1)

int read_new_mail(void *record_data)
{
    struct fileheader * fptr = record_data;

	static int      idc;
	char            ans[7], done = NA, delete_it;
	char            fname[256];

	if (HAS_PERM(PERM_SYSOP)) {	/* skji */
		clear();
		if (fptr == NULL) {
			prints("변수 delcnt, idc를 0으로 초기화합니다.\n");
		} else {
			prints("fptr->filename : %s\n", fptr->filename);
			prints("fptr->owner    : %s\n", fptr->owner);
			prints("fptr->title    : %s\n", fptr->title);
		}
	}
	if (fptr == NULL) {
		delcnt = 0;
		idc = 0;
		return 0;
	}
	idc++;
	if (fptr->accessed[0])
		return 0;
	prints("%s님이 보낸 '%s'라는 편지를 볼까요?\n", fptr->owner, fptr->title);
	getdata(1, 0, "(Y/N)[Y] ? ", genbuf, 4, DOECHO, NULL);
	if (genbuf[0] != 'y' && genbuf[0] != 'Y' && genbuf[0] != '\0') {
		clear();
		return 0;
	}
	read_mail(fptr);
	strcpy(fname, genbuf);
	mrd = 1;

	if (HAS_PERM(PERM_SYSOP)) {	/* skji */
		prints("read_new_mail()\n");
		prints("currmaildir    : %s\n", currmaildir);
		prints("idc            : %d\n", idc);
		pressanykey();
	}
	if (substitute_record(currmaildir, fptr, sizeof(*fptr), idc)) {
		return -1;
	}
	delete_it = NA;
	while (!done) {
		getdata(t_lines - 1, 0, "(R)답장, (D)지움, (G)계속? [G]: ",
			ans, 7, DOECHO, NULL);
		switch (ans[0]) {
		case 'R':
		case 'r':
			mail_reply(idc, fptr, currmaildir);
			break;
		case 'D':
		case 'd':
			delete_it = YEA;
		default:
			done = YEA;
		}
		if (!done)
			more(fname, 0, MORE_IS_NORMAL);	/*- re-read */
	}
	if (delete_it) {
		clear();
		prints("'%s'를 지웁니다. ", fptr->title);
		Getyn(genbuf);
		if (genbuf[0] == 'Y' || genbuf[0] == 'y') {	/*- if not yes quit */
			sprintf(genbuf, "mail/%s/%s", currentuser.userid, fptr->filename);
			unlink(genbuf);
			delmsgs[delcnt++] = idc;
		}
	}
	clear();
	return 0;
}

int m_new(void)
{
	clear();

	/* skji: (버그) New할 때, 인덱스파일에 등록이 제대로 되지 않음 */
	prints("메일의 New명령이 동작하지 않습니다.\n");
	prints("Read명령을 사용해 주세요\n");
	pressanykey();
	return 0;
	mrd = 0;
	uinfo.mode = RMAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	read_new_mail(NULL);
	if (apply_record(currmaildir, read_new_mail, sizeof(struct fileheader)) == -1) {
		clear();
		move(0, 0);
		prints("새편지는 없습니다.\n\n\n");
		uinfo.mode = MAIL;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		return -1;
	}
	if (delcnt) {
		while (delcnt--)
			delete_record(currmaildir, sizeof(struct fileheader), delmsgs[delcnt]);
	}
	clear();
	move(0, 0);
	if (mrd)
		prints("더이상은 없습니다.\n\n\n");
	else
		prints("새편지는 없습니다.\n\n\n");
	uinfo.mode = MAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return -1;
}

int m_forward(void)
{
	FILE *fp;
	char abuf[100];

	sprintf(genbuf, "mail/%s/.forward", currentuser.userid);
	fp = fopen(genbuf, "r");
	if (fp){
		if (fgets(abuf, 99, fp)) {
			move(2, 0);
			prints("원래 주소 : %s", abuf);
		}
		fclose(fp);
	}
	getdata(1, 0, "편지를 돌릴 e-mail 주소 : ", abuf,
		60, DOECHO, NULL);
	if (abuf[0] != '\0'){
		if (!invalidaddr(abuf)){
			fp = fopen(genbuf, "w");
			if (fp){
				fprintf(fp, "%s\n", abuf);
				fclose(fp);
				prints("이제부터 %s님에게 오는 편지를 %s에도 보냅니다.", currentuser.userid, abuf);
				clrtoeol();
			}
		} else {
			prints("'%s'는 잘못된 주소 입니다.", abuf);
			clrtoeol();
		}
	} else {
		prints("편지 돌리기를 중지합니다.");
		clrtoeol();
		unlink(genbuf);
	}
	pressanykey();
	clear();
	return 0;
}
/*-design edited by chopin */
int mailtitle(void)
{
	prints("[편지읽기]\n");
	prints("(R)답장 (SP)읽기 (m)마크 ($)마지막편지로 (h)도움말\n\n");
	standout();
	prints("번호   %-20s %-46s\n", "쓴사람", "제목");
	standend();
	clrtobot();
	return 0;
}

char * maildoent(int num, struct fileheader * ent)
{
	static char     buf[512];
	char            b2[512];
	char            status;
	char           *t;

	strncpy(b2, ent->owner, STRLEN);
	if (t = strchr(b2, ' '))
		*t = '\0';
	if (ent->accessed[0] & FILE_READ) {
		if (ent->accessed[0] & FILE_MARKED)
			status = 'm';
		else
			status = ' ';
	} else {
		if (ent->accessed[0] & FILE_MARKED)
			status = 'M';
		else
			status = 'N';
	}
	if (ent->tm_mday == 0) {/* 날짜가 세트되어 있을 때만, 날짜출력 */
		sprintf(buf, " %3d %c %-14s       %s", num, status, b2, ent->title);
	} else {
		sprintf(buf, " %3d %c %-14s %2d/%2d %s",
		num, status, b2, ent->tm_mon + 1, ent->tm_mday, ent->title);
	}
	return buf;
}

int set_new_title_NOT_USED(int y, int x, struct fileheader * fh)
{
	char            temptitle[200], tempbuf[200];
	char            ans[100];

	sprintf(temptitle, "Re: %s", fh->title);
	sprintf(tempbuf, "제목을:\"%s\"으로 할까요 (Y/N/E) [Y] ? ", temptitle);
	getone(y, x, tempbuf, ans, 2, DOECHO, NULL);
	move(y, x);
	clrtoeol();
	if (ans[0] == 'e' || ans[0] == 'E') {
		prints("편  집: ");
		editline(y, x + 8, temptitle, 72);
		strcpy(fh->title, temptitle);
	} else if (ans[0] == 'n' || ans[0] == 'N') {
		prints("새제목: ");
		temptitle[0] = 0;
		editline(y, x + 8, temptitle, 72);
		strcpy(fh->title, temptitle);
	}			/* else ... no change */
	return 0;
}

#ifdef POSTBUG
extern int      bug_possible;
#endif

int mail_read(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[512], notgenbuf[128];
	char           *t;
	char            done = NA, ans[7], delete_it, replied;

	clear();
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(notgenbuf, "%s/%s", buf, fileinfo->filename);
	delete_it = replied = NA;
	while (!done) {
		more(notgenbuf, 0, MORE_IS_NORMAL);
		if (HAS_PERM(PERM_SYSOP)) {
			getone(t_lines - 1, 0, "(R)답장,(D)지움,(A)계정발급,(X)계정삭제,(Z)계정닫기,(U)계정닫기취소,(G)계속?:",
			       ans, 7, DOECHO, NULL);
		} else {
			getone(t_lines - 1, 0, "(R)답장, (D)지움, (G)계속? [G]: ",
			       ans, 7, DOECHO, NULL);
		}
		switch (ans[0]) {
		case 'R':
		case 'r':
			/*- skji replied = YEA; */
			/*- skji  set_new_title(3, 0, fileinfo); */
			mail_reply(ent, fileinfo, direct);
			break;
		case 'A':
			if (HAS_PERM(PERM_SYSOP))
				mail_adduser(ent, fileinfo, direct);
			done = YEA;
			break;
		case 'X':
			if (HAS_PERM(PERM_SYSOP))
				mail_deluser(ent, fileinfo, direct);
			done = YEA;
			break;
		case 'Z':
			if (HAS_PERM(PERM_SYSOP))
				mail_disableuser(ent, fileinfo, direct);
			done = YEA;
			break;
		case 'U':
			if (HAS_PERM(PERM_SYSOP))
				mail_enableuser(ent, fileinfo, direct);
			done = YEA;
			break;
		case 'D':
		case 'd':
			delete_it = YEA;
		default:
			done = YEA;
		}
	}
	if (delete_it)
		mail_del(ent, fileinfo, direct);
	else {
		fileinfo->accessed[0] |= FILE_READ;
#ifdef POSTBUG
		if (replied)
			bug_possible = YEA;
#endif
		substitute_record(currmaildir, fileinfo, sizeof(*fileinfo), ent);
#ifdef POSTBUG
		bug_possible = NA;
#endif
	}
	return FULLUPDATE;
}

int mail_read_galmuri(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[512], notgenbuf[512];
	char           *t;

	clear();
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(notgenbuf, "%s/%s", buf, fileinfo->filename);
	getdata(2, 0, "갈무리 준비가 되었습니까? [Y/n]", buf, 3, DOECHO, NULL);
	if (buf[0] == 'n' || buf[0] == 'N')
		return FULLUPDATE;
	disableTalkFly();
	cat(notgenbuf);
	enableTalkFly();
	pressanykey();
	return FULLUPDATE;
}

/*-ARGSUSED*/
int mail_reply(int ent, struct fileheader * fileinfo, char * dirent)
{
	char           *uids[MaxCc];
	char            title[STRLEN];
	char           *t;
	int             i;

	uinfo.mode = SMAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	uids[0] = (char *) strdup(fileinfo->owner);
	if (t = strchr(uids[0], ' '))
		*t = '\0';
	uids[1] = 0;

	/*- skji */
	strcpy(title, (char *) make_reply_title(fileinfo->title));
	do_send(uids, title, NULL);
	for (i = 0; uids[i]; i++) {
		free(uids[i]);
	}

	uinfo.mode = RMAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return FULLUPDATE;
}

int mail_del(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[512];
	char           *t;

	extern char     currfile[];

	clear();
	prints("'%s'를 지웁니다. ", fileinfo->title);
	Getyn(genbuf);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/*- if not yes quit */
		move(2, 0);
		prints("편지를 찢다가 다시 붙입니다\n");
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	strncpy(currfile, fileinfo->filename, STRLEN);
	if (!delete_file(direct, sizeof(*fileinfo), ent, cmpfilename)) {
		sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
		unlink(genbuf);
		return FULLUPDATE;
	}
	move(2, 0);
	prints("지우지 못했습니다\n");
	pressreturn();
	clear();
	return FULLUPDATE;
}

#ifdef INTERNET_EMAIL

int mail_forward(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[STRLEN];
	char           *p;
#ifdef PERMS
	if (!HAS_PERM(PERM_FORWARD)) {
		bell();
		return DONOTHING;
	}
#endif
	strncpy(buf, direct, sizeof(buf));
	if ((p = strrchr(buf, '/')) != NULL)
		*p = '\0';
	clear();
	/* bbs.c 에 있는 forward_post 의 코드를 그대로 가져다 씀 */
	/*
	 * <-- 원래 코드 switch (doforward(buf, fileinfo)) { case 0: prints("
	 * s \n", currentuser.email); sprintf(genbuf, "forwarded file to %s",
	 * currentuser.email); report(genbuf); break; case -1: prints("system
	 * error 편지 돌리기 실패.\n"); break; case -2: prints("주소가 잘못 되어서
	 * "); }
	 */
	/* <- 이 아래부터 forward_post의 코드 */
	if (invalidaddr(currentuser.email)) {
		prints("주소가 잘못 되어서 편지를 못 돌렸습니다\n");
	} else {
		char            buf2[1000];
		/*- skji: 한글메일 포워딩을 해결할 수 없어서 그냥 elm을 사용
	                  mail은 한글의 MSB를 stripping을 하기 때문에 ... */
		/*
		 * bugman: 한글 sendmail 깔려있으면 그냥 Mail(/usr/ucb/Mail)
		 * ..
		 */
		sprintf(buf2, "bin/Mail -s \"Nari BBS [Mail] %d (fwd)\" %s < %s/%s > /dev/null", ent, currentuser.email, buf, fileinfo->filename);
		/*- prints("Executing ... %s\r", buf2); */
		/*-- chopin redirection does not work when bbs use execv() in do_exec()
	                redirection works only in system() func.
	                do_exec(buf2, "");
	        */
		/*- bugman : remove useless debug code
		        if ( HAS_PERM(PERM_SYSOP)) {
		            clear();
		            prints("Mail Forwarding Menu Debugging\n");
		            prints("Command to be executed:\n");
		            prints("fileinfo->filename=%s\n", fileinfo->filename);
		            prints("bfile(fileinfo->filename)=%s\n",
		                   bfile(fileinfo->filename));
		            prints("%s\n", buf2);
		            pressanykey();
		        }
		*/
		system(buf2);
		prints("편지를 %s에게 돌립니다.\n", currentuser.email);
		sprintf(genbuf, "forwarded file to %s", currentuser.email);
		report(genbuf);
	}
	pressreturn();
	clear();
	return FULLUPDATE;
}

#endif

int mail_del_range(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            num1[10], num2[10];
	int             inum1, inum2;

	/* <--- inserted by wings */
	struct fileheader fh;
	int             pos;

	clear();
	prints("Delete RANGE\n");
	getdata(1, 0, "편지 많이 지우기 시작 번호: ", num1, 10, DOECHO, NULL);
	inum1 = atoi(num1);
	if (inum1 <= 0) {
		prints("번호가 잘못되었습니다..\n");
		pressreturn();
		return FULLUPDATE;
	}
	getdata(2, 0, "편지 많이 지우기 마지막 번호: ", num2, 10, DOECHO, NULL);
	inum2 = atoi(num2);
	if (inum2 <= inum1) {
		prints("번호가 잘못되었습니다.\n");
		pressreturn();
		return FULLUPDATE;
	}
	getdata(3, 0, "정말로 지울까요? (Y/N)[N] ", num1, 10, DOECHO, NULL);
	if (*num1 == 'Y' || *num1 == 'y') {
		delete_range(direct, inum1, inum2);
		fixkeep(direct, inum1, inum2);
		sprintf(genbuf, "delete range %d-%d on mail", inum1, inum2);
		/*- chopin 지운 갯수만큼 new pointer에서 뺌*/
		/*
		 * decrease_new_pointer(temp_board_name, inum2 - inum1);
		 */

		report(genbuf);
		prints("정말로 지워버렸습니다.\n");
		pressreturn();
		return FULLUPDATE;
	}
	prints("안 지웠습니다.\n");
	pressreturn();
	return FULLUPDATE;
}

int mail_mark(int ent, struct fileheader * fileinfo, char * dirent)
{
	if (fileinfo->accessed[0] & FILE_MARKED)
		fileinfo->accessed[0] &= ~FILE_MARKED;
	else
		fileinfo->accessed[0] |= FILE_MARKED;
	substitute_record(currmaildir, fileinfo, sizeof(*fileinfo), ent);
	return (PARTUPDATE);
}

int mail_adduser(int ent, struct fileheader * fileinfo, char * dirent)
{
	char           *uids[2];
	char            buf[10];
	char            buf2[100];
	char           *t;
	int             id;
	int             i;

	if (!HAS_PERM(PERM_SYSOP))
		return 0;
	uids[0] = (char *) strdup(fileinfo->owner);
	uids[1] = NULL;
	if (t = strchr(uids[0], ' '))
		*t = '\0';
	if (!(id = getuser(uids[0]))) {
		clear();
		move(3, 0);
		prints("아이디 [%s]는 죽었는데요....\n\n", uids[0]);
		prints("         고인의 명복을 빕니다.");
		pressreturn();
		goto QQUIT;
	}
	clear();
	move(8, 0);
	standout();
	prints("  다음은 사용자의 정보입니다. 참고 하세요       \n");
	standend();
	prints("  ID     : %s\n\n", lookupuser.userid);
	prints("* 본명   : %s\n\n", lookupuser.realname);
	prints("* 소속   : %s\n\n", lookupuser.address);
	prints("  별명   : %s\n\n", lookupuser.username);
	prints("  e-mail : %s", lookupuser.email);
	{
		int ind;
		struct user_info uin;

		ind = search_record(ULIST, &uin, sizeof(uin), t_cmpuids, id);
		if (ind && uin.active && kill(uin.pid, 0) != -1) {
			sprintf(buf2, "'%s'님은 현재 사용중입니다. 추방할까요? (Y/N)[N]", uids[0]);
			getone(3, 0, buf2, buf, 2, DOECHO, NULL);
			if (buf[0] == 'Y' || buf[0] == 'y') {
				kick_user_by_id(uids[0]);
			} else {
				prints("\n관두었습니다.\n");
				pressanykey();
				goto QQUIT;
			}
		}
	}
	sprintf(buf2, "'%s'의 등급을 바꿀까요? (Y/N)[Y]", uids[0]);
	move(3, 0);
	clrtoeol();
	getone(3, 0, buf2, buf, 2, DOECHO, NULL);
	if (buf[0] == 'N' || buf[0] == 'n') {
		prints("\n취소되었습니다.\n");
		pressanykey();
		goto QQUIT;
	}
	lookupuser.userlevel = PERM_NORMALUSER;
	substitute_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	move(4, 0);
	prints("'%s'의 등급을 바꾸었습니다.", lookupuser.userid);
	clrtoeol();
	getone(6, 0, "사물함의 내용을 전송할까요? (Y/N)[Y] ", buf, 2, DOECHO, NULL);
	if (buf[0] == 'N' || buf[0] == 'n') {
		prints("\n사용자의 권한만 올렸어요..\n");
		pressanykey();
		goto QQUIT;
	}
	sprintf(buf2, "mybox/%s", currentuser.userid);
	strcpy(Cc_list, "");
	in_mail = YEA;
	do_send(uids, "☆☆☆☆가입하신 것을 환영합니다.★★★★", buf2);
QQUIT:
	free(uids[0]);
	return 0;
}

int mail_manage_user(int ent, struct fileheader * fileinfo, char * dirent, int isEnable)
{
	char           *uid;
	char            buf[10];
	char            buf2[100];
	char           *t;
	int             id;
	int             i;

	if (!HAS_PERM(PERM_SYSOP))
		return 0;
	uid = (char *) strdup(fileinfo->owner);
	if (t = strchr(uid, ' '))
		*t = '\0';
	if (!(id = getuser(uid))) {
		clear();
		move(3, 0);
		prints("아이디 [%s]는 죽었는데요....\n\n", uid);
		prints("         고인의 명복을 빕니다.");
		pressreturn();
		goto KQQUIT;
	}
	clear();
	move(8, 0);
	standout();
	prints("  다음은 사용자의 정보입니다. 참고 하세요       \n");
	standend();
	prints("  ID     : %s\n\n", lookupuser.userid);
	prints("* 본명   : %s\n\n", lookupuser.realname);
	prints("* 소속   : %s\n\n", lookupuser.address);
	prints("  별명   : %s\n\n", lookupuser.username);
	prints("  e-mail : %s", lookupuser.email);
	if (!isEnable) {
		sprintf(buf2, "'%s'를 방문 할 수 없게 만들까요? (Y/N)[Y]", uid);
	} else {
		sprintf(buf2, "'%s'를 방문 할 수 있게 만들까요? (Y/N)[Y]", uid);
	}
	move(3, 0);
	clrtoeol();
	getone(3, 0, buf2, buf, 2, DOECHO, NULL);
	if (buf[0] == 'N' || buf[0] == 'n') {
		prints("\n취소되었습니다.\n");
		pressanykey();
		goto KQQUIT;
	} {
		int ind;
		struct user_info uin;

		ind = search_record(ULIST, &uin, sizeof(uin), t_cmpuids, id);
		if (ind && uin.active && kill(uin.pid, 0) != -1) {
			kick_user_by_id(uid);
		}
	}
	if (isEnable) {
		lookupuser.userlevel = PERM_BASIC;
		lookupuser.lastlogin = time(0);
	} else {
		lookupuser.userlevel = 0;
		lookupuser.lastlogin = (time_t) 0;
	}
	substitute_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	move(4, 0);
	if (!isEnable) {
		prints("'%s'를 방문 못하게 했습니다.", lookupuser.userid);
	} else {
		prints("'%s'를 방문 할 수 있게 했습니다.", lookupuser.userid);
	}
	clrtoeol();
	pressanykey();
KQQUIT:
	free(uid);
	return 0;
}
int mail_disableuser(int ent, struct fileheader *fileinfo, char *dirent)
{
	return mail_manage_user(ent, fileinfo, direct, 0);
}
int mail_enableuser(int ent, struct fileheader *fileinfo, char *dirent)
{
	return mail_manage_user(ent, fileinfo, direct, 1);
}
int mail_deluser(int ent, struct fileheader * fileinfo, char * dirent)
{

	char           *uid;
	char           *t;
	int             id;

	if (!HAS_PERM(PERM_SYSOP))
		return 0;
	uid = (char *) strdup(fileinfo->owner);

	if (t = strchr(uid, ' '))
		*t = '\0';
	if (!(id = getuser(uid))) {
		clear();
		move(3, 0);
		prints("아이디 [%s]는 죽었는데요....\n\n ", uid);
		prints("         고인의 명복을 빕니다.");
		pressreturn();
		free(uid);
		return 0;
	}
	clear();
	d_user_by_id(id);
	free(uid);
	return 0;
}

static int mail_help_entry(int index, struct fileheader *entry, char *directory)
{
    (void)index;
    (void)entry;
    (void)directory;
    return mailreadhelp();
}

struct one_key  mail_comms[] = {
	'd', mail_del,
	'D', mail_del_range,
	'r', mail_read,
	' ', mail_read,
	'\n', mail_read,
	'\r', mail_read,
	'g', mail_read_galmuri,
	'R', mail_reply,
	'm', mail_mark,
#ifdef INTERNET_EMAIL
	'F', mail_forward,
#endif
	'h', mail_help_entry,
	CTRL('J'), mail_help_entry,
	CTRL('T'), transferToMybox,
	/*
	 * 'A', mail_adduser, 'X', mail_deluser,
	 */
	'\0', NULL
};

int m_read(void)
{
	uinfo.mode = RMAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	in_mail = YEA;
	i_read(currmaildir, mailtitle, maildoent, &mail_comms[0]);
	in_mail = NA;
	uinfo.mode = MAIL;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

#ifdef INTERNET_EMAIL

#include <netdb.h>
#include <pwd.h>
#define BBSMAILDIR "/usr/spool/mqueue"
extern char     BoardName[];

static	unsigned char myaddr[2][4] = {
	{ 165,132,35,57 },
	{ 127,0,0,1}
};

int invalidaddr(char * src)
{
	struct hostent *h;
	char *host, *addr=src;

	if (*addr == '\0')
		return 1;	/*- blank */
	while (*addr) {
		if (!isalnum(*addr) && strchr(".%!@:;-_", *addr) == NULL)
			return 1;
		addr++;
	}
	if ((host = strchr(src, '@')) == NULL) return 1;
	h = gethostbyname(host+1);
	if (!h){
		return 1;
	} else {
		int match, i, j;
		for (j=0; j<2; j++){
			for (i =0, match = YEA ;i <4; i++){
				if (myaddr[j][i] != (unsigned char)(h->h_addr[i])){
/*
prints("DEBUG: %d %d\n", myaddr[j][i], (unsigned char)(h->h_addr[i]));
*/
						match = NA;
						break;
				}
			}
			if (match) return 1;
		}
	}

	return 0;
}

int spacestozeros(char * s)
{
	while (*s) {
		if (*s == ' ')
			*s = '0';
		s++;
	}
	return 0;
}

int getqsuffix(char * s)
{
	struct stat     stbuf;
	char            qbuf[STRLEN], dbuf[STRLEN];
	char            c1 = 'A', c2 = 'A';
	int             pos = strlen(BBSMAILDIR) + 3;
	sprintf(dbuf, "%s/dfAA%5d", BBSMAILDIR, getpid());
	sprintf(qbuf, "%s/qfAA%5d", BBSMAILDIR, getpid());
	spacestozeros(dbuf);
	spacestozeros(qbuf);
	while (1) {
		if (stat(dbuf, &stbuf) && stat(qbuf, &stbuf))
			break;
		if (c2 == 'Z') {
			c2 = 'A';
			if (c1 == 'Z')
				return -1;
			else
				c1++;
			dbuf[pos] = c1;
			qbuf[pos] = c1;
		} else
			c2++;
		dbuf[pos + 1] = c2;
		qbuf[pos + 1] = c2;
	}
	strcpy(s, &(qbuf[pos]));
	return 0;
}

int convert_tz(int local, int gmt, char * buf)
{
	local -= gmt;
	if (local < -11)
		local += 24;
	else if (local > 12)
		local -= 24;
	sprintf(buf, " %4d", abs(local * 100));
	spacestozeros(buf);
	if (local < 0)
		buf[0] = '-';
	else if (local > 0)
		buf[0] = '+';
	else
		buf[0] = '\0';
	return 0;
}

int createqf(char * title, char * qsuffix)
{
	static int      configured = 0;
	static char     myhostname[STRLEN];
	static char     myusername[20];
	char            mytime[STRLEN];
	char            idtime[STRLEN];
	char            qfname[STRLEN];
	char            t_offset[6];
	FILE           *qfp;
	time_t          timenow;
	int             savehour;
	struct tm      *gtime, *ltime;
	struct hostent *hbuf;
	struct passwd  *pbuf;

	if (!configured) {
		/*- get host name */
		gethostname(myhostname, STRLEN);
		hbuf = gethostbyname(myhostname);
		if (hbuf)
			strncpy(myhostname, hbuf->h_name, STRLEN);

		/*- get bbs uident */
		pbuf = getpwuid(getuid());
		if (pbuf)
			strncpy(myusername, pbuf->pw_name, 20);
		if (hbuf && pbuf)
			configured = 1;
		else
			return -1;
	}
	/*- get file name */
	sprintf(qfname, "%s/qf%s", BBSMAILDIR, qsuffix);
	if ((qfp = fopen(qfname, "w")) == NULL)
		return -1;

	/*- get time */
	time(&timenow);
	ltime = localtime(&timenow);
#ifdef SYSV
	ascftime(mytime, "%a, %d %b %Y %T ", ltime);
#else
	strftime(mytime, sizeof(mytime), "%a, %d %b %Y %T ", ltime);
#endif
	savehour = ltime->tm_hour;
	gtime = gmtime(&timenow);
	strftime(idtime, sizeof(idtime), "%Y%m%d%y%H%M", gtime);
	convert_tz(savehour, gtime->tm_hour, t_offset);
	strcat(mytime, t_offset);
	fprintf(qfp, "P1000\nT%lu\nDdf%s\nS%s\nR%s\n", timenow, qsuffix,
		myusername, currentuser.email);
#ifdef ERRORS_TO
	fprintf(qfp, "E%s\n", ERRORS_TO);
#endif
	/*- do those headers! */
	fprintf(qfp, "HReceived: by %s (%s)\n\tid %s; %s\n",
		myhostname, VERSION_ID, qsuffix, mytime);
	fprintf(qfp, "HReturn-Path: <%s@%s>\n", myusername, myhostname);
	fprintf(qfp, "HDate: %s\n", mytime);
	fprintf(qfp, "HMessage-Id: <%s.%s@%s>\n", idtime, qsuffix,
		myhostname);
	fprintf(qfp, "HFrom: %s@%s (%s)\n", myusername, myhostname, BoardName);
	fprintf(qfp, "HSubject: %s (fwd)\n", title);
	fprintf(qfp, "HTo: %s\n", currentuser.email);
	fprintf(qfp, "HX-Forwarded-By: %s (%s)\n", currentuser.userid,
#ifdef REALNAME
		currentuser.realname);
#else
		currentuser.username);
#endif
	fprintf(qfp, "HX-Disclaimer: %s is not responsible for the contents of this message.\n", BoardName);
	fclose(qfp);
	return 0;
}

int doforward(char * dirent, struct shortfile * fh)
{
	char            dfname[STRLEN];
	char            fname[STRLEN];
	char            qsuffix[STRLEN];
	if (invalidaddr(currentuser.email))
		return -2;
	/*- create data file */
	if (getqsuffix(qsuffix) == -1)
		return -1;
	sprintf(fname, "%s/%s", direct, fh->filename);
	sprintf(dfname, "%s/df%s", BBSMAILDIR, qsuffix);
	if (link(fname, dfname) != 0)
		return -1;
	return (createqf(fh->title, qsuffix));
}

#endif
