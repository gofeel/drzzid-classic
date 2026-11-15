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
#include "bbs.h"
#include "more.h"
#include "edit.h"
#include "chat.h"		/*- <-- added by skji (decl. for * chatroom_title[][]) */
#include "permissions.h"

#include <sys/file.h>
#include <time.h>		/*- 포스팅 날짜 필드 추가; by skji */
#include <string.h>		/*- by chopin */
#include <unistd.h>

int             msgLine = 23;	/* 쪽지 받을 위치 */
int             mot;
static int      quiting;
int             scrint = 0;

struct userec   currentuser;
int             usernum;
char            currboard[STRLEN];
int             selboard = 0;

extern int      numboards;

char            genbuf[1024];

/*- +2 lines are added by skji */
char            tempbuf[100];

extern int      chatroom;

int exit_warning(void)
{
	bell();
	return 0;
}

int BBSMenu(void)
{

	uinfo.mode = BOARD_MENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(tempbuf, "%s/[글상자마당]", BoardName);
	docmd(tempbuf, "글상자마당 명령을 치세요 : ",
	      "n\n", BBSlist, boardmargin, BBS_help);
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}
int CabinetMenu(void)
{

	uinfo.mode = CABINET_MENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(tempbuf, "%s/[사물함마당]", BoardName);
	docmd(tempbuf, "사물함마당 명령을 치세요 : ",
	      "V\n", cabinetlist, boardmargin, Cabinet_help);
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

int Maintenance(void)
{

	char           *p;
	uinfo.mode = ADMIN;

	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(tempbuf, "%s/[관리자마당]", BoardName);

	if (HAS_PERM(PERM_SYSOP))	/*- +4 lines are changed by skji */
		p = "(U/X/L/I/A/D/M)계정관리 (N/B/C)글상자관리 (T)추적 (V)선거 : ";
	else			/*- if (HAS_PERM(PERM_BOARDS)) */
		p = "(N/B/C)글상자관리 : ";
	/* by bugman */
	p = "관리자마당 명령을 치세요 : ";
	docmd(tempbuf, p, "i\n", maintlist, boardmargin, a_help);
	clear();
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int Xyz(void)
{

	uinfo.mode = XMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(tempbuf, "%s/[개인정보보기]", BoardName);
	/*
	 * docmd(tempbuf, "(U)모든사용자보기 (X)지우기 (Q)소개말변경 (S) Z)
	 *  ", "h\n", xyzlist, boardmargin, x_help);
	 */
	docmd(tempbuf,
	      "개인정보 명령을 치세요 : ",
	      "o\n", xyzlist, boardmargin, x_help);
	clear();
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

int Talk(void)
{

	/*--	report("Enter Talk menu") ;*/
	uinfo.mode = TMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	sprintf(tempbuf, "%s/[이야기마당]", BoardName);
	/*
	 * docmd(tempbuf, "(U)현재사용자 (P)삐삐 (C)대화방 (T) 사용자관찰 :
	 * ", "u\n", talklist, boardmargin, t_help);
	 */
	docmd(tempbuf,
	      "이야기마당 명령을 치세요 : ",
	      "u\n", talklist, boardmargin, t_help);
	/*--	report("Exit Talk menu") ;*/
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

#ifdef FILES
int Files(void)
{

	/*--	report("Files menu") ; */
	uinfo.mode = ULDL;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	sprintf(tempbuf, "%s/[파일주고받기]", BoardName);
	docmd(tempbuf, "파일주고받기 명령을 치세요 : ", "s\n", filelist, filemargin, f_help);
	clear();
	uinfo.mode = MMENU;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	return 0;
}
#endif

int Mail(void)
{

	int             cmd;
	char           *cmds;

	uinfo.mode = MAIL;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	if (chkmail()) {
		cmd = 'r';
		cmds = "r";
	} else {
		cmd = 's';
		cmds = "s";
	}
	sprintf(tempbuf, "%s/[편지함]", BoardName);
	docmd(tempbuf, "편지함 명령을 치세요 : ",
	      cmds, maillist, boardmargin, m_help);

	uinfo.mode = MMENU;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	clear();
	return 0;
}

#ifdef PERMS
char uleveltochar(unsigned int lvl)
{
	lvl &= ~PERM_POSTMASK;
	if (lvl == PERM_DEFAULT)
		return 0;
	else if (lvl < PERM_DEFAULT)
		return 0;
	else
		return 0;
}

char bleveltochar(unsigned int lvl)
{
	unsigned        reallvl = lvl & ~PERM_POSTMASK;
	if (!reallvl)
		return 0;
	else if (lvl & PERM_POSTMASK)
		return 0;
	else
		return 0;
}
#endif

char * Ctime(time_t * clock)
{

	char           *foo;
	char           *ptr = time_string_short(*clock);
	return (ptr);
}

int             usercounter;

int printuent(void *record_data)
{
    struct userec * uentp = record_data;

	static int      i;
	char            tmpstring[10];
	char           *field2;
#if defined(REALINFO) && defined(ACTS_REALNAMES)
	field2 = "본 명";
#else
	field2 = "별 명";
#endif
	if (uentp == NULL) {
		move(3, 0);
		clrtobot();
		move(3, 0);
		standout();
		prints("%-14s %-25s %-6s %-6s %-6s %-11s    \n", "아이디", field2,
#ifdef PERMS
		       "#접속", "#글", HAS_PERM(PERM_SEEULEVELS) ? "Lvl" : " ", "마지막");
#else
		       "#접속", "#글", (currentuser.userlevel >= SEEUSRLEVELS) ? "Level" : " ", "마지막");
#endif
		standend();
		i = 3;
		return 0;
	}
	if (uentp->userid[0] == '\0')
		return 0;
	if (i == 22) {
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
		move(3, 0);
		prints("%-14s %-25s %-6s %-6s %-6s %-11s    \n", "아이디", "별 명",
#ifdef PERMS
		       "#접속", "#글", HAS_PERM(PERM_SEEULEVELS) ? "Lvl" : " ", "마지막");
#else
		       "#접속", "#글", (currentuser.userlevel >= SEEUSRLEVELS) ? "Level" : " ", "마지막");
#endif
		i = 3;
		clrtobot();
	}
#ifdef PERMS
	sprintf(tmpstring, " %c ", uleveltochar(uentp->userlevel));
#else
	sprintf(tmpstring, "%d", uentp->userlevel);
#endif
	prints("%-14s %-25s  %5d %5d  %-3s %-16s %c\n", uentp->userid,
#if defined(REALINFO) && defined(ACTS_REALNAMES)
	       uentp->realname,
#else
	       uentp->username,
#endif
#ifdef PERMS
	       uentp->numlogins, uentp->numposts, HAS_PERM(PERM_SEEULEVELS) ? tmpstring : " ", Ctime(&uentp->lastlogin), (HAS_PERM(PERM_UCLEAN) && (uentp->flags[0] & NOCLEAN_FLAG) ? 'X' : ' '));
#else
	       uentp->numlogins, uentp->numposts, (currentuser.userlevel >= SEEUSRLEVELS) ? tmpstring : " ", Ctime(&uentp->lastlogin), (currentuser.userlevel >= SEEUSRLEVELS && (uentp->flags[0] & NOCLEAN_FLAG) ? 'X' : ' '));
#endif
	i++;
	usercounter++;
	return 0;
}

int countusers(void *record_data)
{
    struct userec * uentp = record_data;

	if (uentp->userid[0] != '\0')
		usercounter++;
	return 0;
}

int Users(void)
{
	int             totalusers;
	usercounter = 0;
	apply_record(PASSFILE, countusers, sizeof(struct userec));
	totalusers = usercounter;
	usercounter = 0;
	/*--	report("User List") ;*/
	uinfo.mode = LAUSERS;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	printuent((struct userec *) NULL);
	if (apply_record(PASSFILE, printuent, sizeof(struct userec)) == -1) {
		prints("사용자가 없어요.");
		pressreturn();
		uinfo.mode = MMENU;
		substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
		pressanykey();
		return 0;
	}
	clrtobot();
	move(t_lines - 2, 0);
	prints("전체 %d명 중 %d명의 사용자를 보셨습니다. (최대 %d명)",
	       totalusers, usercounter, MAXUSERS);
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pressanykey();
	return 0;
}

#ifndef PERMS
int SetBoardAccess(int lineno)
{
	char            instr[7];
	int             lvl;
	getdata(lineno, 0, "이 글상자를 사용하기 위한 사용자 등급 : ", instr,
		7, DOECHO, NULL);
	lvl = abs(atoi(instr));
	prints("이 글상자의 사용자 등급은 %u입니다.\n", lvl);
	pressanykey();
	return lvl;
}
#endif

int Create(void)
{
	/* <---- by wings */
	char            tbuf[100];
	/* by wings ----> */

	struct fileheader newboard;
	char           *s;
	char            ans[4];
	extern int      numboards;
	clear();
	uinfo.mode = ADMIN;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	memset(&newboard, 0, sizeof(newboard));
	prints("Create a Board:Enter Board Name (use single word):");
cagain:
	getdata(3, 0, "글상자 이름 (파일 이름으로 가능한 이름): ",
		newboard.filename, 18, DOECHO, NULL);
	for (s = newboard.filename; *s != '\0'; s++)
		if (!isalnum2(*s) ) {
			prints("잘못된 글상자 이름입니다.\n");
			goto cagain;
		}
	getdata(4, 0, "글상자 설명 : ", newboard.title, 60, DOECHO, NULL);

	/*- chopin 글상자들의 디렉토리 추가(계층구조 글상자) */
	getdata(5, 0, "성격을 글상자 디렉토리로 하시겠습니까?(y/n)[n]", ans, 4, DOECHO, NULL);
	if (*ans == 'y' || *ans == 'Y')
		newboard.isdirectory = 1;
	else
		newboard.isdirectory = 0;
	strcpy(genbuf, bfile(newboard.filename));
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	prints("mkdir(%s,0770)실행\n", genbuf);
	if (mkdir(genbuf, 0770) == -1) {
		prints("mkdir(%s,0770) 실패\n", genbuf);
		pressreturn();
		clear();
		return -1;
	}
	/* <---- by wings */
	/* board owner setting */
	/* make id list for auto-id setting. */
getowner:
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
	/* get id */
	move(6, 0);
	clrtoeol();
	prints("글상자 주인의 아이디: ");
	tbuf[0] = 0;
	namecomplete(NULL, tbuf);
	if (!getuser(tbuf)) {
		prints("아이디가 이상합니다.\n");
		goto getowner;
	}
	strcpy(newboard.owner, tbuf);

	/* by wings ----> */

	/*-
         * if (getbnum(newboard.filename) > 0 ){ prints("\n실수: 잘못 \n");
         * pressreturn(); clear(); return -1; }
         */
#ifdef PERMS
	getdata(7, 0, "이 글상자를 이용하기 위한 권한이 필요합니까? (y/n)[n] ",
		ans, 4, DOECHO, NULL);
	if (*ans == 'y' || *ans == 'Y') {
		getdata(8, 0, "제한 : (R)읽기 제한, (P)쓰기 제한? [R] ", ans, 4, DOECHO, NULL);
		if (*ans == 'P' || *ans == 'p')
			newboard.level = PERM_POSTMASK;
		else
			newboard.level = 0;
		move(1, 0);
		clrtobot();
		move(2, 0);
		prints("'%s'글상자 사용을 위한 %s 권한을 제한합니다.\n", newboard.filename,
		       (newboard.level & PERM_POSTMASK ? "POST" : "READ"));
		newboard.level = setperms(newboard.level);
		clear();
	} else
		newboard.level = 0;
#else
	newboard.level = SetBoardAccess(5);
#endif
	newboard.isNoname = NA;
	if (append_record(bfile(BOARDS), &newboard, sizeof(newboard)) == -1) {
		prints("%s파일에 %s글상자 정보 쓰기에서  에러입니다\n", bfile(BOARDS), newboard.filename);
		pressreturn();
		clear();
		return -1;
	}
	numboards = -1;
	/*- chopin */
	if (newboard.isdirectory == 1)
		prints("\n새 글상자 디렉토리가 만들어졌습니다\n");
	else
		prints("\n새 글상자가 만들어졌습니다.\n");
	sprintf(genbuf, "added board %s", newboard.filename);
	report(genbuf);
	pressanykey();
	return 0;
}

/*
 * filepr(fhdrp): board의 이름과  설명을 프린트하는 함수 Boards() ()에 의해 각
 * ileheader를 fhdrp에 넣어서 각각
 */
int             at;

int filepr(struct shortfile * fhdrp)
{

	char            level[10];
	int             i;
	if (at == -1)
		return 0;
	if (at == 23) {
		int             ch;

		standout();
		prints("-- 계속 --");
		standend();
		while ((ch = igetch()) != EOF) {
			if (ch == 'q' || ch == CTRL('C')) {
				move(23, 0);
				clrtoeol();
				return QUIT;
			}
			if (ch == '\n' || ch == '\r' || ch == ' ')
				break;
			bell();
		}
		move(5, 0);
		clrtobot();
		at = 5;
	}
#ifdef PERMS
	if (HAS_PERM(PERM_SEEBLEVELS))
		sprintf(level, "    %c ", bleveltochar(fhdrp->level));
	if ((fhdrp->level & PERM_POSTMASK) || HAS_PERM(fhdrp->level)) {
#else
	if (currentuser.userlevel >= SEEBRDLEVELS)
		sprintf(level, "%6d", fhdrp->level);
	if (currentuser.userlevel >= fhdrp->level) {
#endif
		/*- chopin 디렉토리 인경우 보드이름마지막에 '/'를 붙임  */
		/*
		 * if (is_dir(getbnum(fhdrp->filename)))
		 * strcat(fhdrp->filename, "/");
		 */
		/*- chopin
			shortfile에서는 accessed가 char type 임.
			prints("%c%-20s %s %s\n", (fhdrp->accessed & ZAPPED) ? '*' : ' ', fhdrp->filename,
		*/
		prints("%c%-20s %s %s\n", (fhdrp->accessed & ZAPPED) ? '*' : ' ', fhdrp->filename,
#ifdef PERMS
		       HAS_PERM(PERM_SEEBLEVELS) ? level : "", fhdrp->title);
#else
		       currentuser.userlevel >= SEEBRDLEVELS ? level : "", fhdrp->title);
#endif

		at++;
	}
	return 0;
}

int getstats(char * fname, int * total, int * owned, int * visited, int * unread, char * lastpost)
{
	int             i, dir, offset;
	char            c, bname[STRLEN];
	struct fileheader fh;
	struct stat     stbuf;
	lastpost[0] = '\0';
	*total = *owned = *visited = *unread = 0;
	offset = (int) (&((struct fileheader *) 0)->accessed[usernum]);
	strcpy(bname, (char*) curbdir(fname));
	pushdir(bname, "/.DIR");
	if ((dir = open(bname, O_RDONLY)) == -1)
		return 0;
	fstat(dir, &stbuf);
	*total = *unread = stbuf.st_size / sizeof(struct fileheader);
	for (i = 1; i <= *total; i++) {
		if (i == 1)
			lseek(dir, offset, L_SET);
		else
			lseek(dir, sizeof(struct fileheader) - 1, L_INCR);
		read(dir, &c, 1);
		c = c & (FILE_OWND | FILE_VISIT | FILE_READ);
		if (c) {
			(*unread)--;
			if (c & FILE_OWND)
				(*owned)++;
			else if (c & FILE_VISIT)
				(*visited)++;
		}
	}
	close(dir);
	if (*total > 0) {
		char            datestr[15], *pptr;
		time_t          posttime;
		get_record(bname, &fh, sizeof(fh), *total);
		strcpy(datestr, fh.filename + 2);
		if (pptr = strchr(datestr, '.'))
			*pptr = '\0';
		posttime = atol(datestr);
		strcpy(lastpost, Ctime(&posttime));
		lastpost[16] = '\0';	/*-- chop off year */
	} else
		lastpost[0] = '\0';
	return 0;
}

/*-- skji */
int update_postcount2(struct shortfile * fhdrp)
{

	int             total, owned, visited, unread;
	static int      ttotal, ototal, vtotal, utotal;
	struct userec   uec;
	if (fhdrp == NULL) {
		char            tbuf[500];
		sprintf(tbuf,
			"\n총=%d 내글=%d(%d%%) 넘긴글%d(%d%%) 안본글=%d(%d%%) 나머지 잽\n",
			ttotal,
			ototal, ototal * 100 / ttotal,
			vtotal, vtotal * 100 / ttotal,
			utotal, utotal * 100 / ttotal);
		prints(tbuf);
		prints("패스워드 파일을 갱신합니다.\n");

		/*-- 패스워드 파일 갱신 */
		get_record(PASSFILE, &uec, sizeof(struct userec),
			   usernum);
		currentuser.numposts = ototal;
		substitute_record(PASSFILE, &currentuser, sizeof(currentuser),
				  usernum);

		ttotal = ototal = vtotal = utotal = 0;
		return 0;
	} {
		char            dummy[500];
		prints(".");
		oflush();
		getstats(fhdrp->filename, &total, &owned, &visited, &unread, dummy);
	}
	ttotal += total;
	ototal += owned;
	vtotal += visited;
	utotal += unread;
	return 0;
}

/*-- skji */
int update_postcount(void)
{
	char            buf[MAX_DIRNAME];
	numboards = 0;
	strcpy(buf, board_dir);
	apply_childboards(update_postcount2);
	update_postcount2(NULL);
	strcpy(board_dir, buf);
	return 0;
}

#define ZAPPED_BRDS	1
#define UNZAPPED_BRDS	2
int             whichboards;

int postcount(struct shortfile * fhdrp)
{

	int             total, owned, visited, unread;
	static int      ttotal, ototal, vtotal, utotal;
	int             zapped;
	char            lastpost[30];
	if (at == -1)
		return 0;
	if (at == 23) {
		int             ch;

		/*-- standout(); */
		prints("(Q/E)종료 [그외의 글쇠를 치면 계속됩니다]");
		/*-- standend(); */
		while ((ch = igetch()) != EOF) {
			if (ch == 'q' || ch == 'Q' || ch == 'e' || ch == 'E') {
				move(23, 0);
				clrtoeol();
				ttotal = ototal = vtotal = utotal = 0;
				return QUIT;
			}
			/*-- skji
			            if (ch == '\n' || ch == '\r' || ch == ' ')
				        break;
			            bell();
		        */
			move(24, 0);
			clrtoeol();
			break;
		}
		move(5, 0);
		clrtobot();
		at = 5;
	}
	if (fhdrp == NULL) {
		prints(" %-20s  %4d    %4d     %4d    %4d\n", "*** 합 계 ***",
		       ttotal, ototal, vtotal, utotal);
		ttotal = ototal = vtotal = utotal = 0;
		return 0;
	}
	zapped = ((int) fhdrp->accessed & ZAPPED) ? ZAPPED_BRDS : UNZAPPED_BRDS;
#ifdef PERMS
	if (((fhdrp->level & PERM_POSTMASK) || HAS_PERM(fhdrp->level)) &&
#else
	if (currentuser.userlevel >= fhdrp->level &&
#endif
	    (whichboards & zapped)) {
		getstats(fhdrp->filename, &total, &owned, &visited, &unread, lastpost);
		prints("%c%-20s  %4d    %4d     %4d    %4d   %16s\r",
		       (fhdrp->accessed & ZAPPED) ? '*' : ' ',
		       xboard_dir(fhdrp->filename),
		       total, owned, visited, unread, lastpost);
		at++;
		refresh();
		ttotal += total;
		ototal += owned;
		vtotal += visited;
		utotal += unread;
	}
	return 0;
}

int Boards(void)
{
	uinfo.mode = SHOW;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	move(3, 0);
	clrtobot();
	prints("  글상자들 (* : 안읽는 글상자)\n");
	standout();
#ifdef PERMS
	prints(" 글상자                  %s  설명                                             \n", HAS_PERM(PERM_SEEBLEVELS) ? "Lvl" : "   ");
#else
	prints(" 글상자                  %s  설명                                             \n", currentuser.userlevel >= SEEBRDLEVELS ? "Lvl" : "   ");
#endif
	standend();
	at = 5;
	apply_boards(filepr);
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pressanykey();
	return 0;
}

int x_count(void)
{
	char            ans[2];

	if (tooManyUsers()) {
		move(3, 0);
		prints("현재 사용자가 너무 많아서 이 기능을 사용할 수 없습니다.");
		pressanykey();
		return -1;	/* Here */
	}
	uinfo.mode = CNTBRDS;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	move(3, 0);
	clrtobot();
	/*- chopin 모든 보드의 글 수를 세는 부분을 추가하는게 필요
        getone(3, 0, "(A)ll, (Z)apped only, or (U)nzapped only? [U] ",
	       ans, 2, DOECHO, NULL);
        */
	getdata(3, 0, "(UnZapped only)글상자의 글 수를 셉니다(Y/N)[Y] ",
		ans, 2, DOECHO, NULL);
	*ans = toupper(*ans);
	if (*ans == 'A')
		whichboards = ZAPPED_BRDS | UNZAPPED_BRDS;
	else if (*ans == 'Z')
		whichboards = ZAPPED_BRDS;
	/*--
        else if (*ans == 'S' && HAS_PERM) {
	    move(4, 0); prints("글쓴수를 셉니다.\n");
	    update_postcount();
	    return;
        }
        */
	else
		whichboards = UNZAPPED_BRDS;
	move(3, 0);
	clrtoeol();
	if (whichboards & ZAPPED_BRDS)
		prints("글상자에 올린 글 수 (* = 안보는 글상자)\n");
	else
		prints("글상자에 올린 글 수\n");
	prints(" 글상자 이름          #글수     내글    넘긴글  새글\n");
	at = 5;
	if (apply_childboards(postcount) != QUIT)
		postcount(NULL);
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pressanykey();
	return 0;
}

/*- chopin 보드 디렉토리의 화일 이름 fname을 보드 디렉토리의 path를 붙여서 리�
.BOARDS,보드 디렉토리,보드이름,모두 사용 가능
*/
char * bfile(const char *fname)
{
	/*-
            static char     buf[STRLEN];
            strcpy(buf, "boards/");
            strcat(buf, boardname);
            return buf;
        */
	int             i;
	int             bnum;
	if (fname[0] == '.')	/*- 보드가 아니라 .BOARDS등의 인덱스 화일 이름일때 */
		return ((char *) curbdir(fname));
	if (!strcmp(fname, &board_dir[strlen(board_dir) - strlen(fname)]))
		return ((char *) curbdir(""));	/*- 디렉토리인경우 rdname(디렉토리)이 들어 있음 */
	bnum = getbnum(fname);
	return ((char *) curbdir(fname));
}

/*-chopin
 namecomplete()에서 사용하는 보드 리스트를 만들어냄
 */
int g_board_names(struct shortfile * fhdrp)
{

#ifdef PERMS
	if ((fhdrp->level & PERM_POSTMASK) || HAS_PERM(fhdrp->level))
#else
	if (currentuser.userlevel >= fhdrp->level)
#endif
		AddNameList(fhdrp->filename);
	return 0;
}

/*
 * make_blist(): 현 디렉토리에 위치한 보드들의 리스트를 만들어 줌.
 * _complete()등의 함수에서 사용
 */
int make_blist(void)
{
	CreateNameList();
	numboards -= 1;
	apply_boards(g_board_names);
	return 0;
}

/*-chopin
make_child_blist():
현 디렉토리 아래의 모�
보드 리스트를 만들어 줌
make_blist()의 디렉토리화에 의한 확장 모듈임
*/
int make_child_blist(void)
{
	CreateNameList();
	numboards = -1;
	apply_child_dir(g_board_names);
	return 0;
}

char * room_title(int nth_minus_one)
{
	FILE           *inf;
	char            readbuf[500], *p;

	if ((inf = fopen(CHATROOM_TITLE, "r"))) {
		int             i;
		for (i = 0; i < nth_minus_one; i++)
			fgets(readbuf, 100, inf);
		if (fgets(readbuf, 100, inf)) {
			if (readbuf[strlen(readbuf) - 1] == '\n')
				readbuf[strlen(readbuf) - 1] = 0;
			p = (char *) strdup(readbuf);
		} else
			p = "-= 빈 방 =-";
		fclose(inf);
		return p;
	}
	return NULL;
}

/*- This function is added by skji */
int Zchat(void)
{
	int             chatters[CHATROOM_CNT], *chatroom_title[CHATROOM_CNT];
	char           *chatters_idlist[CHATROOM_CNT];
	char            buf[100];
	char           *prompt;
	int             i;

	clear();
	move(0, 0);
	prints("[NONAME]/[CHATROOM]");
	move(1, 0);
	prints("맘대로 떠들어 보서요~");

	move(4, 0);
	prints("별명을 쓰세요(off) : ");
	move(5, 0);
	prints("채팅방의 제목을 적어주세요 : ");
	move(6, 0);
	prints("시간제한 가능 (분단위,0은 무제한) ");
	count_chatters(chatters, chatters_idlist);

	move(9, 0);
	standout();
	prints(" == 채팅방 제목 ==                                                        \n\n");
	standend();
	for (i = 0; i < CHATROOM_CNT; i++) {
		prints("방[%d]: ", i + 1);
		if (chatters[i] == 0) {
			change_chatroom_title(i + 1, "");
			prints("아무도 안계십니다");
		} else {
			prints("(%d명) \"%s\" [%s]", chatters[i],
			       room_title(i), chatters_idlist[i]);
		}
		prints("\n\n");
	}
	standout();
	prints("                                                                          \n\n");
	standend();

	/*
	 * do {
	 */
	buf[0] = '\0';
	move(3, 0);
	if (HAS_PERM(PERM_CHAT)) {
		prints("몇 번 방으로 들어가시겠습니까 (1/2/3/4/Q) ? ");
		getone(3, 20, NULL,
		       buf, 6, DOECHO, NULL);
		chatroom = buf[0] - '0';
		switch (buf[0]) {
		case 'q':
		case 'Q':
			clear();
			return 0;
		}
		if (1 > chatroom || chatroom > 4) {
			chatroom = 1;
		}
	} else {
		prints("손님은 1번방만 가능합니다. 들어갈래요 (y/N) ? ");
		getone(3, 20, NULL,
		       buf, 6, DOECHO, NULL);
		if (buf[0] != 'y' && buf[0] != 'Y') {
			clear();
			return 0;
		}
		chatroom = 1;
	}
	/*
	 * } while (YEA);
	 */

	/*- prints("\n\n(%d)번 방으로 들어갑니다.\n", chatroom); */

	switch (chatroom) {
	case 1:
		ent_chat1();
		break;
	case 2:
		ent_chat2();
		break;
	case 3:
		ent_chat3();
		break;
	case 4:
		ent_chat4();
		break;
	}
	return 0;
}
int Select(void)
{
	char            bname[STRLEN];
	struct stat     st;
	int             bnum;
	uinfo.mode = SELECT;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	make_child_blist();
	move(3, 0);
	clrtoeol();		/*- 잘못된 글상자 ... 메시지 지움 */
	move(0, 0);
	prints("글상자 이름을 고르세요.(사이띄개: 이름보기)\n");
	prints("글상자 이름 : ");
	clrtoeol();
	bnum = namecomplete_ignorecase((char *) NULL, bname);
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	if (((*bname == '\0') || (stat(bfile(bname), &st) == -1)) && !is_dir(bnum)) {
		move(3, 0);
		/*- +chopin 보드 선택이 되어 있는 상태에서는 메시지필요 없음->멀티 레벨로 선택할때 중요 */
		if (!selboard)
			prints("잘못된 글상자 이름입니다.");
		/*-- pressreturn(); <--- deleted by skji */
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		/*-- move(2, 0); clrtoeol(); <--- delete by skji */
		return 1;
	}
	if (!(st.st_mode & S_IFDIR)) {
		move(1, 0);
		prints("글상자 이름이 잘못되었습니다.\n");
		pressreturn();
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return 1;
	}
	/*- chopin 선택된 보드가 디렉토리이면 현보드디렉토리정보를 바꿈 */
	if (bnum <= 0)
		if (!strcmp(bname, PARENT_DIR_NAME))
			popdir(board_dir, bname);
		else {
			if (!selboard)
				prints("입력이 잘못되었습니다.\n");
			return 0;
		}
	else if (bnum > 0 && is_dir(bnum)) {
		if (!strcmp(bname, PARENT_DIR_NAME)) {	/*- .. 보드,  윗 디렉토리로 올라가는 경우 */
			popdir(board_dir, bname);
		} else {	/*- + chopin  아래 디렉토리로 내려가는 경우 */
			pushdir(board_dir, bname);
		}
		numboards -= 1;
		apply_record(bfile(BOARDS), fillcache, sizeof(struct fileheader));
		selboard = 0;
		strcpy(currboard, bname);
		Boards();
		return 0;
	}
	/*-- report(genbuf) ;*/
	if (bnum > 0)
		selboard = 1;
	strcpy(currboard, bname);
	move(0, 0);
	clrtoeol();
	move(1, 0);
	clrtoeol();
	return 0;
}

int receiveFile(char * fn)
{
	/*
	 * prints("서비스 준비중 입니다."); pressreturn();
	 */
	reset_tty();
	do_exec("bin/rz -bpe", fn);
	restore_tty();
	return 0;
}
int Post(void)
{
	struct fileheader postfile;
	char            fname[STRLEN];
	char           *ip;
	int             fp, aborted;
	struct tm      *tm;
	time_t          clk;

	char            sbuf[5];

	if (!selboard) {
		prints("\n (S)elect명령으로 글상자를 먼저 고르세요.\n");
		/*-- chopin
			pressreturn();
		*/
		pressanykey();
		return 0;
	}
#ifdef PERMS
	if (!haspostperm(currboard)) {
		prints("\n\n이 글상자는 쓰기 제한이 있습니다.\n");
		pressreturn();
		clear();
		return 0;
	}
#endif
	memset((void *) &postfile, 0, sizeof(postfile));
	if (!isNonameBoard(currboard)) {
		postfile.accessed[usernum] = FILE_READ | FILE_OWND;
	}
	/*-- skji 날짜 추가 */
	clk = time(NULL);	/*- 현재 날짜 */
	tm = localtime(&clk);
	postfile.tm_year = tm->tm_year;	/*- 출력은 안해도 내부적으로 저장함 */
	postfile.tm_mon = tm->tm_mon;
	postfile.tm_mday = tm->tm_mday;

	clear();
	prints("'%s'에 글을 올립니다.\n\n", xboard_dir(currboard));
	if (!isNonameBoard(currboard))
		sprintf(fname, "M.%d.%s.A", time(0), currentuser.userid);
	else
		sprintf(fname, "M.%d.A", time(0));
	sprintf(genbuf, "%s/%s", bfile(currboard), fname);
	ip = strrchr(fname, 'A');
	while ((fp = open(genbuf, O_CREAT | O_EXCL | O_WRONLY, 0664)) == -1) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(genbuf, "%s/%s", bfile(currboard), fname);
	}
	close(fp);
	strcpy(postfile.filename, fname);
	getdata(1, 0, "제목 : ", postfile.title, STRLEN, DOECHO, NULL);
	strncpy(save_title, postfile.title, STRLEN);
	strncpy(save_filename, fname, 4096);
	in_mail = NA;
	if (isNonameBoard(currboard)) {
		strncpy(postfile.owner, "내글아님!!", STRLEN);
	} else {
		strncpy(postfile.owner, currentuser.userid, STRLEN);
		uinfo.mode = POSTING;
		substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	}
	sprintf(genbuf, "%s/%s", bfile(currboard), postfile.filename);

	/*- by bugman Zmodem이 잘 안돼서 막아놨음..
	    	getdata(2, 0, "방식 : (1) 직접 쓰기  (2) zmodem 으로 보낼까요 [1] ?", sbuf, 2, DOECHO, NULL);
	*/
	getone(2, 0, "방식 : (1) 직접 쓰기  (2) 사물함에서 복사 [1] ?", sbuf, 2, DOECHO, NULL);
	if (sbuf[0] == '2') {
		char            mybox[512];
		sprintf(mybox, "mybox/%s", currentuser.userid);
		aborted = vedit_from_file(mybox, genbuf, YEA, isNonameBoard(currboard) ? TEXT_IS_NONAME : TEXT_IS_POST);
	} else {
		aborted = vedit(genbuf, YEA, isNonameBoard(currboard) ? TEXT_IS_NONAME : TEXT_IS_POST);
	}
	uinfo.mode = MMENU;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	clear();
	if (!aborted) {
		if (append_record(strcat(bfile(currboard), DIR), &postfile,
				  sizeof(postfile)) == -1) {
			sprintf(genbuf, "'%s'을 '%s'에 올리는데 실패했습니다!",
				postfile.title, bfile(currboard));
			report(genbuf);
			pressreturn();
			clear();
			return -1;
		}
		if (!isNonameBoard(currboard)) {
			sprintf(genbuf, "'%s'을 '%s'에 올렸습니다.",
				postfile.title, xboard_dir(currboard));
			report(genbuf);
		}
		/* fooz */
#ifdef FORWARD_ARTICLE
		if (!isNonameBoard(currboard)) {
			char            tbuf[500];
			FILE           *fp;
			sprintf(tbuf, "%s/.forward", bfile(currboard));

			if (fp = fopen(tbuf, "rt")) {
				char            addr[100];
				char           *p;

				while(fgets(addr, 99, fp)){
					if ((p = strrchr(addr, '\n')) != NULL) {
						*p = '\0';
					}
					if (!invalidaddr(addr)) {
						sprintf(tbuf,
							"bin/Mail -s \"NARI Article(%s:%03d:%s)\" %s < %s/%s > /dev/null",
							bfile(currboard)+7, (int)get_num_records(strcat(bfile(currboard), DIR), sizeof(postfile)), currentuser.userid, addr, bfile(currboard), postfile.filename);
						system(tbuf);
					}
				}
				fclose(fp);
			}
		}
#endif
		prints("글을 올려놨어요\n");
		if (!isNonameBoard(currboard))
			currentuser.numposts++;
		substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	}
	pressanykey();
	return 0;
}

/*-- Deleted by skji
int ztest(void)
{
	pressanykey();
    return 0;
}
*/

/*--design edited by chopin*/
int readtitle(void)
{
	char            buf[160];
	move(0, 0);
	clrtoeol();
	prints("[읽기] %-20s %s",owner_string(), boardmargin());

	print_newmail_message();

	move(1, 0);
	prints("(R)답장 (S)새글차례로보기 (E)글고침 (F)돌리기 ");
	prints("(v)isit ($)마지막글 (h)도움말\n\n");

	standout();
	prints(" 번호  %-20s  %-20s   [명령:rdsvwESVF^T^P^R] \n", " 쓴사람", "   제목");
	standend();
	clrtobot();
	return 0;
}

char * readdoent(int num, struct fileheader * ent)
{
	static char     buf[512];
	char            datestring[100];
	int             type;

	if (ent->accessed[usernum] & FILE_VISIT)
		type = 'U';
	else if (!(ent->accessed[usernum] & FILE_READ))
		type = 'N';
	else
		type = ' ';
#ifdef PERMS
	if ((ent->accessed[0] & FILE_MARKED) && HAS_PERM(PERM_MARKPOST))
#else
	if ((ent->accessed[0] & FILE_MARKED) && currentuser.userlevel >= DELPOSTLEVEL)
#endif
	{
		if (type == ' ')
			type = 'm';
		else
			type = 'M';
	}
	/*- 날짜, Date, date, day */
	if (ent->tm_mon == 0 && ent->tm_mday == 0) {
		sprintf(datestring, "     ");
	} else {
		sprintf(datestring, "%2d/%2d", ent->tm_mon + 1, ent->tm_mday);
	}
	sprintf(buf, " %4d %c %-12s %s %4d %s", num, type,
		ent->owner, datestring, ent->readcnt, ent->title);
	return buf;
}

char            currfile[STRLEN];

int cmpfilename(const void *record_data)
{
    const struct fileheader * fhdr = record_data;

	if (!strncmp(fhdr->filename, currfile, STRLEN))
		return 1;
	return 0;
}

void mark_flag_as_read(void *record_data)
{
    struct fileheader * fhdr = record_data;

	/*
	 * if (strcmp(currentuser.userid, fhdr->owner))
	 */
	if (
	    ((fhdr->accessed[usernum] & FILE_READ) == 0) &&
	    ((fhdr->accessed[usernum] & FILE_OWND) == 0)
		) {
		fhdr->readcnt++;/*- skji */
	}			/*-by bugman: 자신의 글이나 한번 읽은 글을 다시 읽었을 때는
									count가 증가되지 않도록 함 */
	/*
	 * if(HAS_PERM(PERM_SYSOP)){ prints("\n\nFILE_READ : %d",
	 * fhdr->accessed[usernum] & FILE_READ); prints("\   FILE_OWND :
	 * %d\n", fhdr->accessed[usernum] & FILE_OWND); pressanykey(); }
	 */
	fhdr->accessed[usernum] |= FILE_READ;
	fhdr->accessed[usernum] &= ~FILE_VISIT;
}

/*- added by skji */
void mark_flag_as_visit(void *record_data)
{
    struct fileheader * fhdr = record_data;

	fhdr->accessed[usernum] |= FILE_VISIT;
}

/*- This functions is added by skji */
int visit_on_readboard(int ent, struct fileheader * fileinfo, char * dirent)
{
	char           *t, ans[6], buf[512];
	strcpy(buf, direct);
	if ((t = strrchr(buf, '/'))==NULL)
		*t = '\0';
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
	strncpy(currfile, fileinfo->filename, STRLEN);
	update_file(direct, sizeof(struct fileheader), ent,
		    cmpfilename, mark_flag_as_visit);
	return PARTUPDATE;
}

/*- This functions is added by skji */
int mark_as_read_on_readboard(int ent, struct fileheader * fileinfo, char * dirent)
{
	char           *t;
	char            ans[6];
	char            buf[512];
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
	strncpy(currfile, fileinfo->filename, STRLEN);
	update_file(direct, sizeof(struct fileheader), ent,
		    cmpfilename, mark_flag_as_read);
	return PARTUPDATE;
}

int read_post(int ent, struct fileheader * fileinfo, char * dirent)
{
	char           *t;
	char            ans[6];
	char            buf[512];

	clear();
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
	strncpy(currfile, fileinfo->filename, STRLEN);
	update_file(direct, sizeof(struct fileheader), ent,
		    cmpfilename, mark_flag_as_read);
	more(genbuf, NA, MORE_IS_NORMAL);
	{
		int              key;
		{
			char            msgbuf[200];
			sprintf(msgbuf, "<%s> \"%-.40s\" (R)답장...",
				currboard, fileinfo->title);
			key = presskey(msgbuf);
		}
		if (key == 'R' || key == 'r') {
			do_reply(fileinfo->title, fileinfo->filename);
		}
	}
	return FULLUPDATE;
}
int read_post_galmuri(int ent, struct fileheader * fileinfo, char * dirent)
{
	char           *t;
	char            ans[6];
	char            buf[512];

	clear();
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);

	strncpy(currfile, fileinfo->filename, STRLEN);
	update_file(direct, sizeof(struct fileheader), ent,
		    cmpfilename, mark_flag_as_read);
	getdata(2, 0, "갈무리 준비가 되었습니까? [Y/n]", buf, 2, DOECHO, NULL);
	if (buf[0] == 'n' || buf[0] == 'N') {
		return FULLUPDATE;
	}
	disableTalkFly();
	cat(genbuf);
	enableTalkFly();
	pressanykey();
	return FULLUPDATE;
}

int do_select(void)
{
	char            bname[STRLEN];
	struct stat     st;
	int             bnum;
	extern char     dirbuf[4096];

	uinfo.mode = SELECT;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	make_child_blist();
	move(0, 0);
	prints("글상자 이름을 고르세요. (사이띄개:이름보기)\n");
	prints("글상자 이름 : ");
	clrtoeol();
	bnum = namecomplete_ignorecase((char *) NULL, bname);
	uinfo.mode = READING;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);

	if (((*bname == '\0') || (stat(bfile(bname), &st) == -1)) && !is_dir(bnum)
		) {
		move(2, 0);
		if (!selboard)
			prints("글상자 이름이 잘못되었습니다.");
		clrtoeol();
		/* pressreturn(); <---deleted by chopin */
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return FULLUPDATE;
	}
	if (!(st.st_mode & S_IFDIR)) {
		move(1, 0);
		prints("글상자 이름이 잘못되었습니다.");
		clrtoeol();
		pressreturn();
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return FULLUPDATE;
	}
	/*- chopin 선택된 보드가 디렉토리이면 현보드디렉토리정보를 바꿈 */
	if (bnum <= 0)
		if (!strcmp(bname, PARENT_DIR_NAME))
			popdir(board_dir, bname);
		else {
			if (!selboard)
				prints("입력이 잘못되었습니다.\n");
			return 0;
		}
	else if (bnum > 0 && is_dir(bnum)) {
		if (!strcmp(bname, PARENT_DIR_NAME)) {	/*- .. 보드,  윗 디렉토리로 올라가는 경우 */
			popdir(board_dir, bname);
		} else		/*- + chopin  아래 디렉토리로 내려가는 경우 */
			pushdir(board_dir, bname);
		numboards -= 1;
		apply_record(bfile(BOARDS), fillcache, sizeof(struct fileheader));
		selboard = 0;
		strcpy(currboard, bname);
		/* Boards() */
		return FULLUPDATE;
	}
	/*--	sprintf(genbuf,"read menu Select board '%s'",bname) ;
	        report(genbuf) ;*/
	if (bnum > 0)
		selboard = 1;
	strcpy(currboard, bname);
	/* 다음 한 줄이 Select()와 다름 */
	strcpy(dirbuf, strcat(bfile(currboard), DIR));
	move(0, 0);
	clrtoeol();
	move(1, 0);
	clrtoeol();
	return NEWDIRECT;
}

#ifndef NOREPLY
char            replytitle[STRLEN];

int do_reply(char * oldtitle, char * oldfilename)
{
	strcpy(replytitle, oldtitle);
	do_post(oldfilename);
	replytitle[0] = '\0';
	return 0;
}
#endif

/*-- skji */
int copy_article_to_reply(char * src, char * dest)
{
	FILE           *inf, *outf;
	char            buf[512], i;
	if (src && (inf = fopen(src, "r"))) {
		if (dest && (outf = fopen(dest, "w"))) {
			for (i = 0; i < 4; i++)
				fgets(buf, 512, inf);	/*- 헤더 건너뛰고 */
			while (fgets(buf, 512, inf) != NULL)
				fprintf(outf, "> %s", buf);
			fprintf(outf, ">");
			fflush(outf);
			fclose(outf);
		}
		fclose(inf);
	}
	return 0;
}

/*-- skji */
char * make_reply_title(char * title)
{
	static char     buf[500];
	char            askbuf[500], key[10], oldtitle[500];

	strcpy(oldtitle, title);
	if (strncmp(oldtitle, "답:", 3) == 0) {
		char           *p = &oldtitle[3];
		if (*p == ' ')
			p++;
		sprintf(buf, "답^2: %s", p);
	} else if (strncmp(oldtitle, "답^", 3) == 0) {
		int             replycnt = 0;
		char           *p;
		for (p = &oldtitle[3]; isdigit(*p); p++)
			replycnt = replycnt * 10 + (*p - '0');
		replycnt++;
		if (*p == ':')
			p++;
		if (*p == ' ')
			p++;
		sprintf(buf, "답^%d: %s", replycnt, p);
	} else {
		sprintf(buf, "답: %s", oldtitle);
	}
	sprintf(askbuf, "제목을 \"%s\"(으)로 할까요?\n(Y/N/E)[Y] ", buf);
	move(1, 0);
	clrtoeol();
	getone(1, 0, askbuf, key, 4, DOECHO, NULL);
	if (key[0] == 'N' || key[0] == 'n') {
		buf[0] = 0;
		getdata(2, 0, "새제목:", buf, STRLEN, DOECHO, NULL);
	} else if (key[0] == 'E' || key[0] == 'e') {
		move(2, 0);
		prints("편  집:");
		editline(2, 8, buf, 70);
	}
	return buf;
}

/*-- skji: 파라미터 추가
    포스팅에 답장을 하기를 원하는 경우에
    oldfilename을 복사해준다. */
int do_post(char * oldfilename)
{
	struct fileheader postfile;
	char            fname[STRLEN], ans[4], buf[256], origfname[STRLEN];
	char           *ip;
	int             fp, aborted;
	int             savemode;
	int             FLAG_copy_original_file_to_reply = 0;
	int             FLAG_title_edited = 0;
	struct tm      *tm;
	time_t          clk;

	/* chopin */
	if (!selboard) {
		prints("\n (S)elect명령으로 글상자를 먼저 고르세요.\n\n");
		pressreturn();
		return 0;
	}
#ifdef PERMS
	if (!haspostperm(currboard)) {
		bell();
		return DONOTHING;
	}
#endif

	memset(&postfile, 0, sizeof(postfile));
	if (!isNonameBoard(currboard)) {
		postfile.accessed[usernum] = FILE_READ | FILE_OWND;
	}
	/*- 날짜 추가 */
	clk = time(NULL);	/*- 현재 날짜 */
	tm = localtime(&clk);
	postfile.tm_year = tm->tm_year;	/*- 출력은 안해도 내부적으로 저장함 */
	postfile.tm_mon = tm->tm_mon;
	postfile.tm_mday = tm->tm_mday;

	clear();
	prints("'%s'에 글을 올립니다.\n\n", xboard_dir(currboard));
	if (!isNonameBoard(currboard))
		sprintf(fname, "M.%d.%s.A", time(0), currentuser.userid);
	else
		sprintf(fname, "M.%d.A", time(0));
	sprintf(genbuf, "%s/%s", curbdir(currboard), fname);
	ip = strrchr(fname, 'A');
	while ((fp = open(genbuf, O_CREAT | O_EXCL | O_WRONLY, 0664)) == -1) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		/*-
		 * sprintf(genbuf, "boards/%s/%s", currboard, fname);
		 */
		sprintf(genbuf, "%s/%s", bfile(currboard), fname);
	}
	close(fp);
	strcpy(postfile.filename, fname);
#ifndef NOREPLY

	if (replytitle[0] != '\0') {

		strcpy(buf, make_reply_title(replytitle));
		strcpy(postfile.title, buf);

		/*- 본문 복사 (+6 lines are added by skji) */
		getone(2 + FLAG_title_edited, 0,
		       "본문을 복사할까요? (Y/N)[N] ", ans, 4, DOECHO, NULL);
		fflush(stdout);
		if (ans[0] == 'Y' || ans[0] == 'y') {
			FLAG_copy_original_file_to_reply = 1;
		} else
			FLAG_copy_original_file_to_reply = 0;
	} else
#endif
		getdata(1, 0, "제목 : ", postfile.title, STRLEN, DOECHO, NULL);
	strncpy(save_title, postfile.title, STRLEN);
	strncpy(save_filename, fname, 4096);
	in_mail = NA;
	savemode = uinfo.mode;
	if (isNonameBoard(currboard)) {
		strncpy(postfile.owner, "내글아님!!", STRLEN);
	} else {
		strncpy(postfile.owner, currentuser.userid, STRLEN);
		uinfo.mode = POSTING;
		substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	}

	/*- sprintf(genbuf, "boards/%s/%s", currboard, postfile.filename); */
	sprintf(genbuf, "%s/%s", bfile(currboard), postfile.filename);

	/*-- +5 by skji: 원래글을 답장으로 복사 */
	if (oldfilename && FLAG_copy_original_file_to_reply == 1) {
		sprintf(origfname, "%s/%s", bfile(currboard), oldfilename);
		copy_article_to_reply(origfname, genbuf);
	}
	aborted = vedit(genbuf, YEA, isNonameBoard(currboard) ? TEXT_IS_NONAME : TEXT_IS_POST);

	uinfo.mode = savemode;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	clear();
	if (aborted) {
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	if (append_record(strcat(bfile(currboard), DIR), &postfile,
			  sizeof(postfile)) == -1) {
		sprintf(buf, "'%s'을 '%s'에 올리는데 실패했습니다!",
			postfile.title, bfile(currboard));
		report(buf);
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	if (!isNonameBoard(currboard)) {
		sprintf(buf, "'%s'에 '%s'을 올렸습니다.",
			postfile.title, bfile(currboard));
		report(buf);
	}
	/* fooz */
#ifdef FORWARD_ARTICLE
	if (!isNonameBoard(currboard)) {
		char            tbuf[500];
		FILE           *fp;

		sprintf(tbuf, "%s/.forward", bfile(currboard));
		if (fp = fopen(tbuf, "rt")) {
			char            addr[100];
			char           *p;
			while(fgets(addr, 99, fp)){
				if ((p = strrchr(addr, '\n')) != NULL) {
					*p = '\0';
				}
				if (!invalidaddr(addr)) {
					sprintf(tbuf,
						"bin/Mail -s \"NARI Article(%s:%03d:%s)\" %s < %s/%s > /dev/null",
						bfile(currboard)+7, (int)get_num_records(strcat(bfile(currboard), DIR), sizeof(postfile)), currentuser.userid, addr, bfile(currboard), postfile.filename);
					system(tbuf);
				}
			}
			fclose(fp);
		}
	}
#endif
	prints("글을 올려놨어요\n");
	if (!isNonameBoard(currboard))
		currentuser.numposts++;
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	pressreturn();
	return FULLUPDATE;
}

/*-- Reply():made by chopin */
int Reply(int ent, struct fileheader * fileinfo, char * dirent)
{
	do_reply(fileinfo->title, fileinfo->filename);
	/*- fileheader->filename의 내용을 답장에 복사 */
	pressanykey();
	return FULLUPDATE;
}

/*- This function is added by skji: 아티클의 헤더와 본문을 나누어 저장 */
int split_header_and_body(char * fname)
{
	char            buf[512], header[512], body[512];
	int             i;
	FILE           *inf, *outf;
	if ((inf = fopen(fname, "r"))) {
		sprintf(header, "%s.header", fname);
		if ((outf = fopen(header, "w"))) {
			for (i = 0; i < 4; i++) {
				fgets(buf, 100, inf);
				fputs(buf, outf);
			}
			fclose(outf);
		}
		sprintf(body, "%s.body", fname);
		if ((outf = fopen(body, "w"))) {
			while (fgets(buf, 100, inf) != NULL)
				fputs(buf, outf);
			fclose(outf);
		}
		fclose(inf);
	}
	return 0;
}

/*- This function is added by skji: 나누어 저장되어 있는 헤더와 본문 합침 */
int merge_header_and_new_body(char * fname)
{
	char            buf[512], header[512], body[512];
	FILE           *inf, *outf;
	if ((outf = fopen(fname, "w"))) {
		sprintf(header, "%s.header", fname);
		sprintf(body, "%s.body", fname);
		if ((inf = fopen(header, "r"))) {
			while (fgets(buf, 100, inf) != NULL)
				fputs(buf, outf);
			fclose(inf);
		}
		if ((inf = fopen(body, "r"))) {
			while (fgets(buf, 100, inf) != NULL)
				fputs(buf, outf);
			fclose(inf);
		}
		fclose(outf);
		unlink(body);
		unlink(header);
	}
	return 0;
}

int edit_only_body_of_article(char * infname)
{
	char            bodyfilename[200];
	split_header_and_body(infname);
	sprintf(bodyfilename, "%s.body", infname);
	vedit(bodyfilename, NA, TEXT_IS_EDIT);
	merge_header_and_new_body(infname);
	return 0;
}

/*- by bugman : 게시판의 글 내용을 사물함으로 전송 */
int transferToMybox(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[512];
	char           *t;
	int             readlen;
	FILE           *fpSrc;
	FILE           *fpDst;

	if (!HAS_PERM(PERM_POST))
		return PARTUPDATE;
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
	fpSrc = fopen(genbuf, "rt");
	if (fpSrc == NULL) {
		fprintf(stderr, "ERROR: 원본 파일을 열수 없습니다.\n");
		return FULLUPDATE;
	}
	sprintf(genbuf, "mybox/%s", currentuser.userid);
	fpDst = fopen(genbuf, "at");
	if (fpDst == NULL) {
		fprintf(stderr, "ERROR: 사물함을 열수 없습니다.\n");
		return FULLUPDATE;
	}
	fprintf(fpDst, "[%s 에서 옮겨온것] *********** 요기부터 \n", buf);
	while ((readlen = fread(genbuf, 1, 80, fpSrc)) > 0) {
		fwrite(genbuf, 1, readlen, fpDst);
	}
	fprintf(fpDst, "* 요기까지 ********************************\n", buf);
	fclose(fpSrc);
	fclose(fpDst);
	clear();
	move(3, 0);
	prints(" 다음 글을 사물함에 덧붙였습니다. \n\n");
	prints(" 글쓴이 : %s\n 제  목 : %s\n 글상자 : %s\n",
	       fileinfo->owner, fileinfo->title, buf);
/*
	update_file(direct, sizeof(struct fileheader), ent,
		cmpfilename, mark_flag_as_read);
*/
	pressanykey();
	return FULLUPDATE;
}

int transferToMyboxOnBoard(int ent, struct fileheader * fileinfo, char * dirent)
{
/*
	if ((fileinfo->accessed[usernum] & (FILE_OWND | FILE_READ)) == 0) {
		clear();
		move(3, 0);
		prints(" 읽은 글만을 옮길 수 있습니다.  ");
		pressanykey();
		return FULLUPDATE;
	}
	*/
	return transferToMybox(ent, fileinfo, direct);
}
/*--ARGSUSED*/
int edit_post(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[512];
	char           *t;
	char            bodyfilename[512];

	/*--chopin
        #ifdef PERMS
	        if(!HAS_PERM(PERM_SYSOP)) {
        #else
        */
#ifdef PERMS
	int             owned;
	owned = fileinfo->accessed[usernum] & FILE_OWND;
	if (!HAS_PERM(PERM_SYSOP) && !(owned)) {
#else

	if (currentuser.userlevel < 255) {
#endif
		bell();
		return DONOTHING;
	}
	clear();
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);

	if (HAS_PERM(PERM_MARKPOST)) {	/*- 루트는 타이틀까지 편집할 수 있음 */
		vedit(genbuf, NA, TEXT_IS_EDIT);
	} else {
		/*- 사용자는 본문만 편집할 수 있음 (+4 lines are added by skji) */
		edit_only_body_of_article(genbuf);
		/*--
			split_header_and_body(genbuf);
			sprintf(bodyfilename, "%s.body", genbuf);
			vedit(bodyfilename, NA);
			merge_header_and_new_body(genbuf);
		*/
	}

	sprintf(genbuf, "edited post '%s' on %s", fileinfo->title, bfile(currboard));
	report(genbuf);
	strncpy(currfile, fileinfo->filename, STRLEN);
	return FULLUPDATE;
}

int mark_post(int ent, struct fileheader * fileinfo, char * dirent)
{
#ifdef PERMS
	if (!HAS_PERM(PERM_MARKPOST)) {
#else
	if (currentuser.userlevel < DELPOSTLEVEL) {
#endif
		bell();
		return DONOTHING;
	}
	if (fileinfo->accessed[0] & FILE_MARKED)
		fileinfo->accessed[0] &= ~FILE_MARKED;
	else
		fileinfo->accessed[0] |= FILE_MARKED;
	substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
	return PARTUPDATE;
}

/* <--- by wings */

/* by wings ---> */

int del_range(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            num1[10], num2[10];
	int             inum1, inum2;
	char            temp_dir_var[100];
	char            temp_board_name[100];

	/* <--- inserted by wings */
	struct fileheader fh;
	int             pos;

	/* set file path */
	strcpy(temp_dir_var, bfile(currboard));
	popdir(temp_dir_var, temp_board_name);
	pushdir(temp_dir_var, BOARDS);
	/* open board file */
	pos = search_record(temp_dir_var, &fh, sizeof(fh), cmpbnames, (intptr_t) temp_board_name);

	if (!strcmp(fh.owner, currentuser.userid))
		goto range_del;
	/* inserted by wings. for board owner delete. ---> */

#ifdef PERMS
	if (uinfo.mode == READING && !HAS_PERM(PERM_BOARDS)) {
#else
	if (uinfo.mode == READING && currentuser.userlevel < DELPOSTLEVEL) {
#endif
		bell();
		return DONOTHING;
	}
	/* <--- inserted by wings. for board owner delete. */
range_del:
	/* inserted by wings. for board owner delete. ---> */

	clear();
	prints("Delete RANGE\n");
	getdata(1, 0, "글 많이 지우기 시작 번호: ", num1, 10, DOECHO, NULL);
	inum1 = atoi(num1);
	if (inum1 <= 0) {
		prints("번호가 잘못되었습니다..\n");
		pressreturn();
		return FULLUPDATE;
	}
	getdata(2, 0, "글 많이 지우기 마지막 번호: ", num2, 10, DOECHO, NULL);
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
		sprintf(genbuf, "delete range %d-%d on %s", inum1, inum2, bfile(currboard));
		/*- chopin 지운 갯수만큼 new pointer에서 뺌*/
		/*- by bugman : 음.. 안되네.. 바꿔야지..
				decrease_new_pointer(temp_board_name, inum2 - inum1);
		*/
		decrease_new_pointer(currboard, inum2 - inum1);
		report(genbuf);
		prints("정말로 지워버렸습니다.\n");
		pressreturn();
		return FULLUPDATE;
	}
	prints("안 지웠습니다.\n");
	pressreturn();
	return FULLUPDATE;
}

/* <--- by wings */

/* by wings ---> */

/* Here */
/* by bugman : List readers of article */
int do_reader_list(int ent, struct fileheader * fileinfo, char * dirent)
{
	struct userec   ur;
	int             i, fd;
	int             x, y;

	fd = open(PASSFILE, O_RDONLY);
	if (fd < 0)
		return -1;
	sprintf(genbuf, "\"%s\"를 읽은 사람들", fileinfo->title);
	move(3, 0);
	standout();
	prints("%-78s\n", genbuf);
	standend();
	x = 0, y = 4;
	if (isNonameBoard(currboard)) {
		move(y, x);
		prints(" ** 아무도 몰라요~ 뭘 알라구~ **\n");
		clrtobot();
		close(fd);
		pressanykey();
		return 0;
	}
	/*
	 * move(5, 0); prints("usernum %d\n", usernum-1); lseek(fd,
	 * sizeof(ur)*(usernum-1), SEEK_SET); read(fd, &ur, sizeof(ur));
	 * prints("userid %s\n", ur.userid); prints("username %s\n",
	 * ur.username);
	 *
	 * prints("usernum %d\n", usernum); lseek(fd, sizeof(ur)*usernum,
	 * SEEK_SET); read(fd, &ur, sizeof(ur)); prints("userid %s\n",
	 * ur.userid); prints("username %s\n", ur.username);
	 *
	 * prints("usernum %d\n", usernum+1); lseek(fd, sizeof(ur)*(usernum+1),
	 * SEEK_SET); read(fd, &ur, sizeof(ur)); prints("userid %s\n",
	 * ur.userid); prints("username %s\n", ur.username);
	 *
	 * pressanykey();
	 */
	for (i = 1; i < MAXUSERS; i++) {
		if ((fileinfo->accessed[i] & FILE_OWND) == 0)
			if (fileinfo->accessed[i] & FILE_READ) {
				if (lseek(fd, sizeof(ur) * (i - 1), SEEK_SET) == -1) {
					break;
				}
				read(fd, &ur, sizeof(ur));
				if (ur.userid[0] != '\0') {
					if (x == 5) {
						clrtoeol();
						y++;
						if (y >= t_lines - 1) {
							int             ch;
							move(y, 0);
							standout();
							prints("--계 속--");
							standend();
							clrtoeol();
							while ((ch = igetch()) != EOF) {
								if (ch == '\n' || ch == '\r' || ch == ' ')
									break;
								if (ch == 'q' || ch == CTRL('C')) {
									close(fd);
									return 0;
								}
							}
							y = 4;
						}
						x = 0;
						move(y, x);
					}
					x++;
					prints("%14s", ur.userid);
				}
			}
	}
	clrtoeol();
	prints("\n");
	clrtobot();
	close(fd);
	pressanykey();
	return 0;
}

int del_post(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[512];
	char           *t;
	int             owned;
	char            temp_dir_var[100];
	char            temp_board_name[100];

	/* <--- inserted by wings */
	struct fileheader fh;
	int             pos;
	/* inserted by wings ---> */

	owned = fileinfo->accessed[usernum] & FILE_OWND;

	/* <--- inserted by wings. for board owner delete. */
	/* set file path */
	/*
	 * temp_dir_var = (char *)malloc(100); temp_board_name = (char
	 * *)malloc(100);
	 */
	strcpy(temp_dir_var, bfile(currboard));
	popdir(temp_dir_var, temp_board_name);
	pushdir(temp_dir_var, BOARDS);

	/* open board file */
	pos = search_record(temp_dir_var, &fh, sizeof(fh), cmpbnames, (intptr_t) temp_board_name);

	if (!strcmp(fh.owner, currentuser.userid))
		goto article_del;
	/* inserted by wings. for board owner delete. ---> */

#ifdef PERMS
	if ((!HAS_PERM(PERM_BOARDS)) && !(owned)) {
#else
	if ((currentuser.userlevel < DELPOSTLEVEL) && !(owned)) {
#endif
		bell();
		return DONOTHING;
	}
	/* <--- inserted by wings. for board owner delete. */
article_del:
	/* inserted by wings. for board owner delete. ---> */

	clear();
	prints("'%s'을 지웁니다.", fileinfo->title);
	getone(1, 0, "(Y/N)[N] ? ", genbuf, STRLEN, DOECHO, NULL);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/*-- if not yes quit */
		move(2, 0);
		prints("글 지우기를 관둡니다.\n");
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	strcpy(buf, direct);
	if (t = strrchr(buf, '/'))
		*t = '\0';
	sprintf(genbuf, "Deleted '%s' on '%s'", fileinfo->title, bfile(currboard));
	report(genbuf);
	strncpy(currfile, fileinfo->filename, STRLEN);
	if (!delete_file(direct, sizeof(struct fileheader), ent, cmpfilename)) {
		sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
		unlink(genbuf);
		if (owned) {
			if (currentuser.numposts > 0)
				currentuser.numposts--;
			substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
		}
		/*- chopin
			decrease_new_pointer(board,1):
			새로 new할 위치를 .NEW 가지고 있음...
			이 것을 글하나를 지울때마다 모든 사람의 new 위치를 -1 함.
			see also: newstart(),init_newcount(),update_new_pointer()
				  read_boards()
		*/
		decrease_new_pointer(temp_board_name, 1);
		return FULLUPDATE;
	}
	move(2, 0);
	prints("글지우기 실패\n");
	pressreturn();
	clear();
	return FULLUPDATE;
}

static int      sequent_ent;

int sequent_messages(void *record_data)
{
    struct fileheader * fptr = record_data;

	static int      idc;
	char            ans[6];
	if (fptr == NULL) {
		idc = 0;
		clear();
		prints("[새 글 연속으로 읽기]\n");
		refresh();
		return 0;
	}
	idc++;
	if (idc < sequent_ent)
		return 0;
	if (fptr->accessed[usernum] & FILE_READ &&
	    !(fptr->accessed[usernum] & FILE_VISIT))
		return 0;
	mot = 1;
	/*-- skji
            prints("'%s'글상자에 있는 \n%s님이 쓴  \"%s\"라는 글을 읽겠습니까?\n",
	           xboard_dir(currboard), fptr->owner, fptr->title);
        */
	clear();
	prints("[새 글 연속으로 읽기]\n");
	prints("글상자 : %s\n", xboard_dir(currboard));
	print_fileheader(fptr);
	getone(4, 0, "(Y/Space)읽기 (N)통과 (Q)마치기 [Y] ? ",
	       genbuf, 5, DOECHO, NULL);

	switch (genbuf[0]) {
	case 'y':
	case 'Y':
	case '\0':
	case ' ':
	case '\n':
	case '\r':
		break;
	default:
		if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
			clear();
			return QUIT;
		}
		clear();
		return 0;
	}

	/*-
        #ifdef NOREPLY
            more(genbuf, YEA);
        #else
            more(genbuf, YEA);
        */

	/*-- chopin
        #ifdef PERMS
            if (haspostperm(currboard)) {
        #endif
	        getone(t_lines - 1, 0, "Reply(Y/N,sp)[N] ? ", ans, 6, DOECHO, NULL);

	        if (ans[0] == 'Y' || ans[0] == 'y')
	            do_reply(fptr->title, fptr->filename);	// + by skji //
        #ifdef PERMS
            } else
	        pressreturn();
        #endif
        #endif
        */
	/*- clear(); */
	sprintf(genbuf, "%s/%s", bfile(currboard), DIR);
	strncpy(currfile, fptr->filename, STRLEN);
	update_file(genbuf, sizeof(struct fileheader), idc,
		    cmpfilename, mark_flag_as_read);
	sprintf(genbuf, "%s/%s", bfile(currboard), fptr->filename);
	/*
	 * if (HAS_PERM(PERM_SYSOP)) { clear(); prints("[DEBUG]\n새글
	 * n"); prints("(이 메시지는 시삽등급만 볼 수 있습니다.)\n\n");
	 * prints("디렉토리: %s\n", bfile(currboard)); prints("파일네임:
	 * %s\n", fptr->filename); prints("ready to open file : %s\n",
	 * genbuf); pressanykey(); }
	 */
	more(genbuf, YEA, MORE_IS_NORMAL);
	return 0;
}

/*--ARGSUSED*/
int sequential_read(int ent, struct fileheader * fileinfo, char * dirent)
{
	clear();
	sequent_messages((struct fileheader *) NULL);
	sequent_ent = ent;
	quiting = NA;
	apply_record(strcat(bfile(currboard), DIR), sequent_messages,
		     sizeof(struct fileheader));
	return FULLUPDATE;
}

#ifdef INTERNET_EMAIL
int forward_post(int ent, struct fileheader * fileinfo, char * dirent)
{
	char            buf[STRLEN];
	char            buf2[STRLEN + 100];
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

	if (invalidaddr(currentuser.email)) {
		prints("주소가 잘못 되어서 편지를 못 돌렸습니다\n");
	} else {
		/*- skji: 한글메일 포워딩을 해결할 수 없어서 그냥 elm을 사용
			  mail은 한글의 MSB를 stripping을 하기 때문에 ... */
		sprintf(buf2, "bin/Mail -s \"Nari BBS [%s] %d (fwd)\" %s < %s/%s > /dev/null",
			buf + 7,/*- 보드이름 */
			ent,
			currentuser.email,
			buf,
			fileinfo->filename);
		/*- prints("Executing ... %s\r", buf2); */
		/*-- chopin redirection does not work when bbs use execv() in do_exec()
			redirection works only in system() func.
			do_exec(buf2, "");
		*/
		/*
		 * Remove useless debug code : by bugman if
		 * (HAS_PERM(PERM_SYSOP)) { clear(); prints("Mail Forwarding
		 * Menu Debugging\n\r"); prints("Command to be executed:\r");
		 * prints("fileinfo->filename=%s\r", fileinfo->filename);
		 * prints("bfile(fileinfo->filename)=%s\r",
		 * bfile(fileinfo->filename)); prints("%s\r", buf2);
		 * pressanykey(); }
		 */
		system(buf2);
		prints("편지를 %s에게 돌립니다.\n", currentuser.email);
		sprintf(genbuf, "forwarded file to %s", currentuser.email);
		report(genbuf);
	}

	/*--
            prints("이 기능은 지금 제공되지 않습니다.\n");
        */
	pressreturn();
	clear();
	return FULLUPDATE;
}

#endif

static int read_select_entry(int index, struct fileheader *entry, char *directory)
{
    (void)index;
    (void)entry;
    (void)directory;
    return do_select();
}

static int read_post_entry(int index, struct fileheader *entry, char *directory)
{
    (void)index;
    (void)entry;
    (void)directory;
    return do_post(NULL);
}

static int read_help_entry(int index, struct fileheader *entry, char *directory)
{
    (void)index;
    (void)entry;
    (void)directory;
    return mainreadhelp();
}

struct one_key  read_comms[] = {
	'v', visit_on_readboard,/*- by skji */
	'V', mark_as_read_on_readboard,	/*-- by skji */
	'r', read_post,
	' ', read_post,
	'g', read_post_galmuri,
	/*-- removed by chopin
          Since,it makes unwilling reading by double return typing when exit reading
            '\n',       read_post,
            '\r',       read_post,
        */
	'd', del_post,
	'D', del_range,
	'm', mark_post,
	'E', edit_post,
	's', read_select_entry,
	CTRL('P'), read_post_entry,
	'w', read_post_entry,
	CTRL('R'), do_reader_list,
	'S', sequential_read,
#ifdef INTERNET_EMAIL
	'F', forward_post,
#endif
	CTRL('T'), transferToMyboxOnBoard,
	'h', read_help_entry,
	CTRL('J'), read_help_entry,
	'R', Reply,		/*-- by chopin*/
	'\0', NULL
};

int Read(void)
{
	if (!selboard) {
		move(2, 0);
		standout();
		prints("(S)elect 명령으로 글상자를 먼저 고르세요.\n");
		standend();
		pressanykey();
		return -1;
	}
	uinfo.mode = READING;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	in_mail = NA;
	i_read(strcat(bfile(currboard), DIR), readtitle, readdoent, &read_comms[0]);
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}
int Login(void)
{
	char buf[256];
	char *env[2];
	extern char fromhost[];
	extern char home_dir[];
	clear();
	refresh();
	reset_tty();
	u_exit();

	sprintf(buf, "BBSHOME=%s", home_dir);
	env[0] = buf;
	env[1] = NULL;
	execle("bin/bbs", "bbs", "R", fromhost, NULL, env);
	return 0;
}

int Goodbye(void)
{
	char            buf[20];
	extern int      started;

	clear();
	refresh();
	reset_tty();
	sprintf(buf, "etc/bye.%d", getpid() & 0x7);
	cat(buf);
	printf("-------------------------------------------------------------------------------\n");
	printf("                         reverse - The DrZZID BBS \n");
	printf("-------------------------------------------------------------------------------\n");

	sleep(2);
	report("exit");
	if (started){
		usieslog("EXIT", currentuser.userid, currentuser.username, "");
		u_exit();
	}
	exit(0);
	return 0;
}

void usieslog(char * action, char * id, char * name, char * from)
{
	if ((ufp = fopen("usies", "a")) != NULL) {
		flock(fileno(ufp), LOCK_EX);
		ti = time(0);
		fprintf(ufp, "%-5s %-10s %-10s %-20s %s", action, id, name, from, time_string(ti));
		fflush(ufp);
		flock(fileno(ufp), LOCK_UN);
		fclose(ufp);
	}
}

int report(char * s)
{
	static int      disable = NA;
	int             fd;

	if (disable)
		return 0;
	if ((fd = open("trace", O_WRONLY, 0664)) != -1) {
		char            buf[512];
		char            timestr[10], *thetime;
		time_t          dtime;
		time(&dtime);
		thetime = time_string(dtime);
		strncpy(timestr, &(thetime[11]), 8);
		timestr[8] = '\0';
		flock(fd, LOCK_EX);
		lseek(fd, 0, L_XTND);
		sprintf(buf, "%s %s %s\n", currentuser.userid, timestr, s);
		write(fd, buf, strlen(buf));
		flock(fd, LOCK_UN);
		close(fd);
		return 0;
	}
	/*
	 * disable = YEA;
	 */
	return 0;
}

char            saveboard[STRLEN];

int             tuid;
int             tfile, rfile;

int get_messages(void *record_data)
{
    struct fileheader * fptr = record_data;

	if (fptr->accessed[tuid] & FILE_READ)
		rfile++;
	tfile++;
	return 0;
}

int get_boards(struct fileheader * fptr)
{
	char            buf[512];
	sprintf(buf, "%s/%s", bfile(fptr->filename), DIR);
	apply_record(buf, get_messages, sizeof(struct fileheader));
	return 0;
}

int get_users(void *record_data)
{
    struct userec * uptr = record_data;

	struct userec   u;
	char            tbuf[STRLEN];

	memmove(&u, uptr, sizeof(u));
	tfile = 0;
	rfile = 0;
	if (*(uptr->userid) == '\0') {
		tuid++;
		return 0;
	}
	strcpy(tbuf, time_string(u.lastlogin));
	strncpy(genbuf, tbuf + 4, 7);
	genbuf[7] = '\0';
	strcat(genbuf, tbuf + 22);
#ifndef FILES
	printf("%-10s %-19s %-9s %-4d  %s", u.userid, u.username,
	       u.termtype, u.userlevel, genbuf);
#else
	printf("%-10s %-19s %-9s %-4d %6s  %s", u.userid, u.username,
	       u.termtype, u.userlevel, NameProtocol(u.protocol), genbuf);
#endif
	tuid++;
	return 0;
}

int UserStatus(void)
{
	tuid = 1;
#ifndef FILES
	printf("%-10s %-19s %-9s %-4s  %s\n", "아이디", "별 명",
	       "터미널", "등급", "마지막 방문");
#else
	printf("%-10s %-19s %-9s %-4s %6s  %s\n", "아이디", "별 명",
	       "터미널", "등급", "프로토", "마지막 방문");
#endif
	if (apply_record(PASSFILE, get_users, sizeof(struct userec)) == -1) {
		fprintf(stderr, "Error in apply_record\n");
		exit(-1);
	}
	pressanykey();
	return 0;
}

int             m_count;
int             m_total;

char            atot[MAXUSERS];

int g_messages(void *record_data)
{
    struct fileheader * fptr = record_data;

	int             i;
	int             count = 0;

	printf("File: %s\n\n", fptr->title);
	for (i = 1; i < MAXUSERS; i++)
		if (fptr->accessed[i] & FILE_READ) {
			struct userec   us;
			atot[i] = 1;
			get_record(PASSFILE, &us, sizeof us, i);
			count++;
			printf("%s ", us.userid);
		}
	m_count++;
	m_total += count;
	printf("\nCount = %d\n", count);
	return 0;
}

int g_boards(struct shortfile * fptr)
{

	char            buf[512];
	int             i, total;
	sprintf(buf, "boards/%s/%s", fptr->filename, DIR);
	printf("-----------------------------------------글상자: %s\n\n", fptr->filename);
	memset(atot, 0, sizeof(atot));
	m_count = 0;
	m_total = 0;
	apply_record(buf, g_messages, sizeof(struct fileheader));
	printf("AVG Read = %f\n", (double) m_total / (double) m_count);
	total = 0;
	for (i = 1; i < MAXUSERS; i++) {
		if (atot[i])
			total++;
	}
	printf("DVRS Read = %d\n", total);
	return 0;
}

int BoardStatus(void)
{
	apply_boards(g_boards);
	return 0;
}

int Info(void)
{
	more("Version.Info", YEA, MORE_IS_NORMAL);
	/*-- chopin
            clear();
        */
	return 0;
}

int Conditions(void)
{
	more("COPYING", YEA, MORE_IS_NORMAL);
	/*-- chopin
            clear();
        */
	return 0;
}

int Welcome(void)
{
	if (more("etc/Welcome", YEA, MORE_IS_LOGIN) == -1) {
		clear();
		prints("환영인사가 잘 안되는군요....\n");
		pressreturn();
		clear();
		return 0;
	}
	/*- chopin
            clear();
        */
	return 0;
}

int EditWelcome(void)
{
	int             aborted;
	char            ans[7];
	move(3, 0);
	clrtoeol();
	clrtobot();
	getone(3, 0, "(E) 고침 (D) 지움 (Q)끝 [E] ? ", ans, 7, DOECHO, NULL);
	if (ans[0] == 'Q' || ans[0] == 'q')
		return 0;
	if (ans[0] == 'D' || ans[0] == 'd') {
		unlink("etc/motd");
		move(5, 0);
		prints("음....!\n");
		pressreturn();
		clear();
		sprintf(genbuf, "%s deleted Welcome\n", currentuser.userid);
		report(genbuf);
		return 0;
	}
	uinfo.mode = EDITWELC;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	aborted = vedit("etc/motd", NA, TEXT_IS_EDIT);
	clear();
	if (aborted)
		prints("관두었습니다.\n");
	else {
		report("edited motd file");
		prints("환영인사가 고쳐졌습니다.\n");
	}
	pressreturn();
	uinfo.mode = XMENU;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	return 0;
}
/*
 * - by funboy EditWelcome2() { int             aborted; char
 * ans[7]; move(3, 0); clrtoeol(); clrtobot(); getone(3, 0, "(E)고침 (D)
 * Q)끝 [E] ? ", ans, 7, DOECHO, NULL); if (ans[0] == 'Q' || ans[0] == 'q')
 * return; if (ans[0] == 'D' || ans[0] == 'd') { unlink("etc/motd2"); move(5,
 * 0); prints("음....!\n"); pressreturn(); clear(); sprintf(genbuf, "%s
 * deleted Welcome\n", currentuser.userid); report(genbuf); return; }
 * uinfo.mode = EDITWELC; substitute_record(ULIST, &uinfo, sizeof(uinfo),
 * utmpent); aborted = vedit("etc/motd2", NA, TEXT_IS_EDIT); clear(); if
 * (aborted) prints("관두었습니다.\n"); else { report("edited motd file");
 * prints("환영인사II가 고쳐졌습니다.\n"); } pressreturn(); uinfo.mode =
 * XMENU; substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
 * return 0; }
 */
/* (char *)n 과 brec를  string compare를 함 */
int cmpbnames(intptr_t n, const void *record_data)
{
    const struct fileheader * brec = record_data;

	if (!ci_strncmp((char *) n, brec->filename, sizeof(brec->filename)))
		return 1;
	else
		return 0;
}

int Zap(void)
{
	char            bname[STRLEN];
	struct fileheader fh;
	int             pos, zapped;
	uinfo.mode = ZAP;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	make_blist();
	clear();
	move(0, 0);
	prints("Zap/Unzap을 위한 글상자 고르기\n");
	prints("글상자 이름: ");
	clrtoeol();
	namecomplete_ignorecase((char *) NULL, bname);
	uinfo.mode = MMENU;

	/* 디렉토리일때는 ㅤ 끝의 '/'를빼줌 */
	if (bname[strlen(bname) - 1] == '/')
		bname[strlen(bname) - 1] = '\0';

	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	pos = search_record(bfile(BOARDS), &fh, sizeof(fh), cmpbnames, (intptr_t) bname);
	if (!pos) {
		move(3, 0);
		prints("글상자 이름이 이상합니다.");
		move(0, 0);
		clrtoeol();
		return 0;
	}
	zapped = (fh.accessed[usernum] & ZAPPED);
	/*-- semi
            zapped ? (fh.accessed[usernum] &= ~ZAPPED) : (fh.accessed[usernum] |= ZAPPED);
		    *//*-+ semi : for toggling, XOR works good */
	fh.accessed[usernum] ^= ZAPPED;
	move(3, 0);
	if (substitute_record(bfile(BOARDS), &fh, sizeof(fh), pos) == -1)
		prints("BOARDS file 수정중 에러 발생\n");
	else {
		numboards = -1;
		prints("%s 글상자는 이제 %s.\n", bname, zapped ? "다시 봅니다" : "안봅니다");
		/*--            sprintf(genbuf,"Zap '%s'",bname) ;
		            report(genbuf) ;*/
	}
	move(0, 0);
	clrtoeol();
	move(1, 0);
	clrtoeol();
	pressanykey();
	return 0;
}

char * time_string_no_newline(time_t t)
{
	struct tm      *tm;
	static char     buf[30];
	static char    *wday[] = {"일", "월", "화", "수", "목", "금", "토"};
	int             hour;

	tm = localtime((time_t *) & t);
	hour = tm->tm_hour % 12;
	sprintf(buf, "%d년 %d월 %d일(%s) %2d:%02d %s",
		tm->tm_year + 1900,
		tm->tm_mon + 1, tm->tm_mday, wday[tm->tm_wday],
		(hour == 0) ? 12 : hour,
		tm->tm_min /* ,tm->tm_sec */ ,
		(tm->tm_hour >= 12) ? "PM" : "AM");
	return buf;
}
char * date_string_no_newline(time_t t)
{
	struct tm      *tm;
	static char     buf[30];
	static char    *wday[] = {"일", "월", "화", "수", "목", "금", "토"};

	tm = localtime((time_t *) & t);
	sprintf(buf, "%d년 %d월 %d일(%s) %s",
		tm->tm_year + 1900,
		tm->tm_mon + 1, tm->tm_mday, wday[tm->tm_wday],
		(tm->tm_hour >= 12) ? "오후" : "오전");
	return buf;
}

/* by bugman */
char * time_string_short_no_newline(time_t t)
{
	struct tm      *tm;
	static char     buf[30];
	static char    *wday[] = {"일", "월", "화", "수", "목", "금", "토"};
	int             hour;

	tm = localtime((time_t *) & t);
	hour = tm->tm_hour % 12;
	sprintf(buf, "%2d/%2d(%s) %2d:%02d%s",
		tm->tm_mon + 1, tm->tm_mday, wday[tm->tm_wday],
		(hour == 0) ? 12 : hour,
		tm->tm_min,	/* tm->tm_sec, */
		(tm->tm_hour >= 12) ? "pm" : "am");
	return buf;
}
char * time_string_short(time_t t)
{
	static char     buf[30];
	sprintf(buf, "%s\n", time_string_short_no_newline(t));
	return buf;
}
char * time_string(time_t t)
{
	static char     buf[30];
	sprintf(buf, "%s\n", time_string_no_newline(t));
	return buf;
}

char * owner_string(void)
{
        char            temp_dir_var[100];
        char            temp_board_name[100];
        static char     buf[80];
        struct fileheader fh;
        int             pos;
        /* set file path */
        strcpy(temp_dir_var, bfile(currboard));
        popdir(temp_dir_var, temp_board_name);
        pushdir(temp_dir_var, BOARDS);
        /* open board file */
        pos = search_record(temp_dir_var, &fh, sizeof(fh), cmpbnames, (intptr_t) temp_board_name);
        sprintf(buf, "[쥔장 : %s]",fh.owner);
        return buf;
}
