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

int             numboards = -1;
struct shortfile bcache[MAXBOARD];

/*
 * .BOARDS 화일로 부터 보드에 대한 정보를 가져와서 bcache[]에 채워
 * (),Select()등의 명령에서 이 bcache[]의 정보를 이용함 bcache[].accessed�
 * ptr->accessed[usernum]로 부터 현 사용자의 정보를 가져와 한바이트로 넣음
 *
 */
int fillcache(void *record_data)
{
    struct fileheader * fptr = record_data;

#ifdef PERMS
	if (!(fptr->level & PERM_POSTMASK) && !HAS_PERM(fptr->level))
#else
	if (fptr->level > currentuser.userlevel)
#endif
		bcache[numboards].filename[0] = ' ';
	else
		strcpy(bcache[numboards].filename, fptr->filename);
	strcpy(bcache[numboards].owner, fptr->owner);
	strcpy(bcache[numboards].title, fptr->title);
	bcache[numboards].level = fptr->level;
	bcache[numboards].tm_year = fptr->tm_year;
	bcache[numboards].tm_mon = fptr->tm_mon;
	bcache[numboards].tm_mday = fptr->tm_mday;
	/* chopin */
	bcache[numboards].isdirectory = fptr->isdirectory;
	bcache[numboards].isNoname = fptr->isNoname;
	if (fptr->isdirectory)
		strcat(bcache[numboards].filename, "/");
	bcache[numboards].readcnt = fptr->readcnt;
	bcache[numboards].accessed = fptr->accessed[usernum];
	if (numboards < MAXBOARD)
		numboards++;
	return 0;
}

/*- chopin
 apply_childboards():현 디렉토리 아래의 모든 보드를 func에 적용 시키는 함수
 주의: 디렉토리는 따로 적용되지 않음
*/

#define BOARD_ONLY 1
#define DIR_AND_BOARD 2
#define UNZAPPED_BOARD 3
#define ALL_BOARD 4
int apply_childboards(bbs_board_visitor func)
{
	int             i;

	numboards = 0;
	apply_childboards2(func, board_dir, board_dir, BOARD_ONLY, UNZAPPED_BOARD);
	return 0;
}
/*- chopin
apply_child_dir(func): 디렉토리까지 func()를 적용시킴
	나머지는 apply_childboards() 와 사용이 같음
	Select()에서 사용
*/
/*
 * apply_child_dir(func) int(*func)(); {
 *
 * if(numboards <= 0 || strcmp(bcache[numboards-1].filename,PARENT_DIR_NAME)) {
 * numboards = 0 ;
 * apply_childboards2(func,board_dir,board_dir,DIR_AND_BOARD,ALL_BOARD);
 * if(strcmp(board_dir,"boards")) /*디렉토리가 Top이 아니면 면 .. 를 넣음* {
 * bcache[numboards].owner[0]=0;
 * strcpy(bcache[numboards].filename,PARENT_DIR_NAME);
 * bcache[numboards].title[0]=0; bcache[numboards].level=0;
 * bcache[numboards].isdirectory=1; numboards++; }
 *
 * } }
 *
 */
int nop(struct shortfile *board)
{

	return 0;
}
int apply_child_dir(bbs_board_visitor func)
{
	int             i;
	char            curdir[MAX_DIRNAME];
	char            startdir[MAX_DIRNAME];

	if (numboards <= 0 || strcmp(bcache[numboards - 1].filename, PARENT_DIR_NAME)) {
		numboards = 0;

		strcpy(curdir, board_dir);
		strcpy(startdir, board_dir);
		if (startdir[strlen(startdir) - 1] == '/')
			startdir[strlen(startdir) - 1] = '\0';

		apply_childboards2(nop, startdir, curdir, DIR_AND_BOARD, ALL_BOARD);
		if (strcmp(board_dir, "boards")) {	/* 디렉토리가 Top이 아니면 . 를 넣음 */
			bcache[numboards].owner[0] = 0;
			strcpy(bcache[numboards].filename, PARENT_DIR_NAME);
			bcache[numboards].title[0] = 0;
			bcache[numboards].level = 0;
			bcache[numboards].isdirectory = 1;
			numboards++;
		}
		for (i = 0; i < numboards; i++)
			if ((*func) (&bcache[i]) == QUIT)
				return QUIT;

	}
	return 0;
}
int apply_childboards2(bbs_board_visitor func, char * startdir, char * curdir, int dir_flag, int zap_flag)
{
	int    i;
	char            nextdir[MAX_DIRNAME];
	int             old_numboards, num;

	old_numboards = numboards;
	/* chopin 보드 캐쉬시작 */
	strcpy(nextdir, curdir);
	pushdir(nextdir, BOARDS);

	/* fillcache()에서는 path를 뺀 보드 이름만 추가됨. */
	apply_record(nextdir, fillcache, sizeof(struct fileheader));

	/* path 추가 */
	for (i = old_numboards; i < numboards; i++) {
		char            temp[255];
		strcpy(temp, curdir);
		pushdir(temp, bcache[i].filename);
		if (strlen(startdir) > 0 &&	/* startdir끝에 '/'가 없으면 */
		    startdir[strlen(startdir) - 1] != '/')
			strcpy(bcache[i].filename, &temp[strlen(startdir) + 1]);
		/* startdir 끝에 '/'가 있으면 */
		else
			strcpy(bcache[i].filename, &temp[strlen(startdir)]);
	}
	num = numboards;
	/* 주의 numboards 값은 루프 안에서 계속 변함 */
	for (i = old_numboards; i < num; i++)
		if ((zap_flag == UNZAPPED_BOARD && !(bcache[i].accessed & ZAPPED))
		    || zap_flag == ALL_BOARD) {
			if (bcache[i].isdirectory) {
				int             k;
				char            ndir[255];

				if (dir_flag == DIR_AND_BOARD)
					if ((*func) (&bcache[i]) == QUIT)
						return QUIT;

				strcpy(ndir, curdir);
				pushdir(ndir, bcache[i].filename);
				if (apply_childboards2(func, startdir, ndir, dir_flag, zap_flag) == QUIT)
					return QUIT;
			} else
			 /* 보드이면 */ if ((*func) (&bcache[i]) == QUIT)
				return QUIT;
		}
	return 0;
}
/*
 * apply_childboards2(func,flag) bbs_board_visitor func ; int flag; { register int i ;
 * char nextdir[MAX_DIRNAME]; int old_numboards,num; old_numboards=numboards;
 * /*chopin 보드 캐쉬시작 * strcpy(nextdir,board_dir);
 * pushdir(nextdir,BOARDS); apply_record(nextdir,fillcache,sizeof(struct
 * fileheader)) ; num=numboards; /* 주의 numboards 값은 루프 안에서 계속 변함 *
 * for(i=old_numboards;i<num;i++) if(!(bcache[i].accessed & ZAPPED))
 * if(bcache[i].isdirectory) {	int k; pushdir(board_dir,bcache[i].filename);
 *
 * if(flag==DIR_AND_BOARD) { if((*func)(&bcache[i]) == QUIT) return QUIT; }
 *
 * k=apply_childboards2(func,flag) ; popdir(board_dir,nextdir); if (k==QUIT)
 * return QUIT; } else 	/* 게시판인 경우 * if((*func)(&bcache[i]) == QUIT)
 * return QUIT; return 0; }
 */
int apply_boards(bbs_board_visitor func)
{
	int    i;

	if (numboards <= 0 || strcmp(bcache[numboards - 1].filename, PARENT_DIR_NAME)) {
		numboards = 0;

		/* chopin 보드 캐쉬시작 */
		apply_record(bfile(BOARDS), fillcache, sizeof(struct fileheader));
		if (strcmp(board_dir, "boards")) {	/* 디렉토리가 Top이 아니�
							 * . 를 넘 */
			bcache[numboards].owner[0] = 0;
			strcpy(bcache[numboards].filename, PARENT_DIR_NAME);
			bcache[numboards].title[0] = 0;
			bcache[numboards].level = 0;
			bcache[numboards].isdirectory = 1;
			numboards++;
		}
	}
	for (i = 0; i < numboards; i++)
		if (bcache[i].filename[0] != ' ')
			if ((*func) (&bcache[i]) == QUIT)
				return QUIT;
	return 0;
}
/*
 * is_dir(): bnum게시판 번호에 해당하는 게시판이 디렉토리인가 검사 주의!
 * bfile()함수를 사용해서는 안됨! bfile()함수가 is_dir을 부름
 */
int is_dir(int bnum)
{
	if (numboards <= 0)
		return 0;
	if (bcache[bnum - 1].isdirectory == 1)	/* 보드는 0번부터
						 * d는 1번부터임! */
		return 1;
	return 0;
}

/*
 * chopin 주의!!! getbnum()함수 안에서는 bfile()함수를 절대로 부르면 안됨!
 * bfile() 함수 안에 getbnum()함수를 부르는 부분이 있음
 */
/* getbnum();게시판 이름을 받아서 bcache[]상의 게시판 번호+1을 리턴함 */
int getbnum(const char *bname)
{
	int    i;

	if (numboards < 0) {
		numboards = 0;
		apply_record(curbdir(BOARDS), fillcache, sizeof(struct fileheader));
	}
	for (i = 0; i < numboards; i++)
		if (!ci_strncmp(bname, bcache[i].filename, sizeof(bcache[i].filename)))
			return i + 1;	/* chopin : 주의!! board 0은 안씀!! */
	return 0;
}

#ifdef PERMS
int haspostperm(char * bname)
{
	int    i;
	if ((i = getbnum(bname)) < 0) {
		prints("\n선택한 글상자가  존재하지 않습니다\n");
		return 0;
	}
	if (!HAS_PERM(PERM_POST))
		return 0;
	return (HAS_PERM(bcache[i - 1].level & ~PERM_POSTMASK));
}
int isNonameBoard(char * bname)
{
	int    i;
	if ((i = getbnum(bname)) < 0) {
		prints("\n선택한 글상자가  존재하지 않습니다\n");
		return -1;
	}
	return (bcache[i - 1].isNoname);
}

#endif

long            ucached = 0;
char            ucache[MAXUSERS][IDLEN + 1];
int             numuents = 0;

int fillucache(void *record_data)
{
    struct userec * uentp = record_data;

	if (numuents < MAXUSERS) {
		strncpy(ucache[numuents], uentp->userid, IDLEN + 1);
		ucache[numuents++][IDLEN] = '\0';
	}
	return 0;
}

int resolve_ucache(void)
{
	struct stat     st;
	if (stat(FLUSH, &st) < 0) {
		st.st_mtime++;
	}
	if (ucached < st.st_mtime) {
		numuents = 0;
		apply_record(PASSFILE, fillucache, sizeof(struct userec));
		ucached = st.st_mtime;
	}
	return 0;
}

int u_namelist(void)
{
	int    i;

	resolve_ucache();
	CreateNameList();
	for (i = 0; i < numuents; i++)
		if (ucache[i][0] != '\0')
			AddNameList(ucache[i]);
	return 0;
}

int searchuser(char * userid)
{
	int    i;

	resolve_ucache();
	for (i = 0; i < numuents; i++)
		if (!ci_strncmp(userid, ucache[i], IDLEN + 1))
			return i + 1;
	return 0;
}

int apply_users(bbs_user_visitor func)
{
	int    i;
	resolve_ucache();
	for (i = 0; i < numuents; i++)
		(*func) (ucache[i], i + 1);
	return 0;
}

struct userec   lookupuser;

int getuser(char * userid)
{
	int             uid = searchuser(userid);
	if (uid == 0)
		return 0;
	get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	return uid;
}
