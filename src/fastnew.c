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
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#ifndef XINU
#include <sys/stat.h>
#endif

extern char     saveboard[STRLEN];
extern int      mot;
extern int      numboards;
struct fileheader fh;

int print_fileheader(struct fileheader * fptr)
{
	prints("제  목 : \"%s\"\n", fptr->title);
	prints("쓴사람 : %s\n", fptr->owner);
	prints("읽은수 : %d\n", (int) fptr->readcnt);
	return 0;
}

#define NEW_COUNT ".NEW"
/*- chopin
init_newcount(포스팅글 수)
: new할 포인터(첫포스팅 부터의 글수) 를 0으�
  초기화
*/
int init_newcount(char * board)
{
	int             i, u[MAXUSERS];
	int             newfp;
	char            buf[200];

	sprintf(buf, "%s/%s", bfile(board), NEW_COUNT);
	if ((newfp = open(buf, O_CREAT | O_RDWR, 0664)) < 0) {
		prints("\nerror: %s file open error\n", buf);
		return -1;
	}
	for (i = 0; i < MAXUSERS; i++)
		u[i] = 0;
	flock(newfp, LOCK_EX);
	write(newfp, (char *) u, sizeof(int) * MAXUSERS);
	flock(newfp, LOCK_UN);

	return newfp;
}
/*- chopin
newstart():
New할 첫 글의 위치(처음으로부터의 첫새글의 위치)를 .NEW에서 읽어서
리�
*/
int newstart(int numfiles, char * board)
{
	int             newfp;
	int             first_new_position;
	char            buf[200];

	sprintf(buf, "%s/%s", bfile(board), NEW_COUNT);
	if ((newfp = open(buf, O_RDONLY)) < 0) {
		if ((newfp = init_newcount(board)) < 0)
			/* 에러이면 처음부터 읽어라 */
			return 0;
	}
	lseek(newfp, sizeof(int) * usernum, SEEK_SET);	/* 화일 처음부터 */
	read(newfp, (char *) &first_new_position, sizeof(int) * 1);
	close(newfp);
	/*-bugman : New가 글을 한두개 놓치는 경우 때문에 안전을 위해서
	  5개 만큼을 더 감소 시킴 */
	if ((first_new_position-5) < 0)
		return 0;
	else
		return (first_new_position-5);
}
/*- chopin
update_new_pointer(새로운 new 위치)
: 새로 바뀐 New의 위치리를 받아서
  .NEW화일에 저장
*/
int update_new_pointer(int new_position, char * board)
{
	int             newfp;
	char            buf[200];

	sprintf(buf, "%s/%s", bfile(board), NEW_COUNT);

	if ((newfp = open(buf, O_WRONLY)) < 0) {
		prints("\nerror: %s file open error\n", buf);
		return 0;
	}
	flock(newfp, LOCK_EX);
	lseek(newfp, sizeof(int) * usernum, 0);	/* 화일 처음 부터 */
	write(newfp, (char *) &new_position, sizeof(int) * 1);
	close(newfp);
	flock(newfp, LOCK_UN);
	return 0;
}
/*- chopin
        decrease_new_pointer(board,deleted):
        새로 new할 위치를 .NEW 가지고 있음...
        이 것을 글하나를 지울때마다 모든 사람의 new 위치를 -deleted 함.
        see also: newstart(),init_newcount(),update_new_pointer()
                  read_boards()
*/
int decrease_new_pointer(char * board, int deleted)
{
	int             i, u[MAXUSERS];
	int             newfp;
	char            buf[200];

	sprintf(buf, "%s/%s", bfile(board), NEW_COUNT);
	if ((newfp = open(buf, O_RDWR)) < 0) {
		char            ans[2];
		prints("\nerror: %s file open error\n", buf);
		return 0;
	}
	/*-bugman : 원래 소스의 버그... read가 빠져있었음 */
	read(newfp, (char *)u, sizeof(int) * MAXUSERS);
	lseek(newfp, 0, SEEK_SET);
	/*-bugman : New가 글을 한두개 놓치는 경우 때문에 안전을 위해서
	  10개 만큼을 더 감소 시킴 */
	for (i = 0; i < MAXUSERS; i++)
		if (u[i] - (deleted+10) > 0)
			u[i] -= (deleted+10);
		else
			u[i] = 0;
	flock(newfp, LOCK_EX);
	write(newfp, (char *) u, sizeof(int) * MAXUSERS);
	close(newfp);
	flock(newfp, LOCK_UN);
	return 0;
}

int read_boards(struct shortfile * fptr)
{

	int    i, offset;
	int             numfiles = 0;
	int             fd;
	int             readone = NA;
	unsigned char   ch;
	char            ans[6];
	char            msgbuf[200];
	int             key;
#if 0
	struct stat     st;
#endif
	int             own = 0, vis = 0, unread = 0;

	short int       kk;
	if (fptr->accessed & ZAPPED)
		return 0;
	offset = ((int) &(fh.accessed[usernum]) - (int) &(fh));
	/*- chopin  getstats()함수의 vis,unread카운트 하는 부분�
		    New의 속도 저하의 주범임!!
	    getstats(fptr->filename, &numfiles, &own, &vis, &unread, genbuf);
	    if (numfiles <= 0 || unread <= 0)
		return 0;
	*/
	/*- chopin
	    sprintf(genbuf,"boards/%s/%s", fptr->filename, DIR);
         	sprintf(genbuf,"%s/%s", bfile(fptr->filename), DIR);
         */
	strcpy(genbuf, bfile(fptr->filename));
	pushdir(genbuf, DIR);
	if ((fd = open(genbuf, O_RDWR)) < 0) {	/* chopin */
/*-bugman : 읽기 제한된 게시판인 경우
  genbuf가 "boards/ /.DIR"이 되어서 에러가남.
		prints("\n\n %s file open error \n\n", genbuf);
		igetch();
*/
		return 0;
	}
	/* - chopin */
	{
		struct stat     st;

		fstat(fd, &st);
		numfiles = st.st_size;
		numfiles = numfiles / sizeof(fh);
	}
#if 0
	fstat(fd, &st);
	numfiles = st.st_size;
	numfiles = numfiles / sizeof(fh);
	if (numfiles <= 0) {
		close(fd);
		/* chopin */
		prints("\n\n numfiles:%d (in read_boards)\n\n", numfiles);

		return 0;
	}
#endif
	lseek(fd, offset, L_SET);
	/*- man lseek
	off_t lseek(fd, offset, whence)
	     int fd;
	     off_t offset;
	     int whence;
	whence:defined in <unistd.h>:

	               SEEK_SET
	               SEEK_CUR 1
	               SEEK_END

	file.h:#define  L_SET           0       // absolute offset //
	*/
	/*- belami
	    {
		int temp = newstart(numfiles,fptr->filename);
		if (HAS_PERM(PERM_SYSOP)) {

		    move(0,0);
		    clrtobot();
		    prints("%s/%s\n", bfile(fptr->filename), NEW_COUNT);
		    prints("\n\rnewstart(numfiles:%d,board:%s) = %d\n\r",numfiles,fptr->filename, temp);
		    pressanykey();
		}
		i = temp;
	    }
	*/
	move(0, 0);
	prints("-=[새글 찾기:%s]=-\n", xboard_dir(fptr->filename));
	refresh();
	for (i = newstart(numfiles, fptr->filename)
	     ,lseek(fd, offset + i * sizeof(fh), L_SET);
	     i < numfiles
	     ; i++, lseek(fd, offset + i * sizeof(fh), L_SET)
		) {
		read(fd, &ch, 1);
		if (!(ch & FILE_READ || ch & FILE_VISIT)) {
			int             xx;
			mot = 1;
			lseek(fd, i * sizeof(fh), L_SET);
			read(fd, &fh, ((int) &(fh.accessed[0]) - (int) &(fh)));
			/* 조회수 add by chopin */
			lseek(fd, ((int) &(fh.readcnt) - (int) &(fh))
			      + (int) (i * sizeof(fh)), L_SET);
			read(fd, &fh.readcnt, sizeof(fh.readcnt));

			/*-	조회수 테스트
				    lseek(fd, ((int)&(fh.isdirectory)-(int)&(fh))+
						(int)(i * sizeof(fh)), L_SET);
				    read(fd,&fh.isdirectory,sizeof(fh.isdirectory));
				    read(fd,&fh.readcnt,sizeof(fh.readcnt));
				    read(fd,&xx,sizeof(xx));
				    prints("isdirectory:%d\n",fh.isdirectory);
				    prints("readcnt:%d\n",fh.readcnt);
				    prints("dummy first:%d\n",xx);
					igetch();
			*/

			/*
			 * clear();
			 */
			move(0, 0);
			prints("-=[새글 찾기:%s]=-\n", xboard_dir(fptr->filename));
			prints("남은글 : %d개\n", numfiles - i);

			print_fileheader(&fh);

			prints("-------------------\n");
			getone(6, 0, "(Y/SP)읽기 (V)통과 (Q)마치기 (S)글상자통과 [Y] ? ",
			       genbuf, 12, DOECHO, NULL);
			genbuf[0] = toupper(genbuf[0]);
			if (genbuf[0] == 'V' || genbuf[0] == 'v' ||
			    genbuf[0] == 'S' || genbuf[0] == 's' ||
			    genbuf[0] == 'Q' || genbuf[0] == 'q') {
				if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
					move(6, 0);
					clrtoeol();
					refresh();
					close(fd);
					mot = -1;
					update_new_pointer(i, fptr->filename);
					return QUIT;
				}
				if (genbuf[0] == 's' || genbuf[0] == 'S') {
					move(6, 0);
					clrtoeol();
					refresh();
					close(fd);
					update_new_pointer(i, fptr->filename);
					return 0;
				}
				flock(fd, LOCK_EX);
				lseek(fd, offset + i * sizeof(fh), L_SET);
				ch |= FILE_VISIT;
				write(fd, &ch, 1);
				flock(fd, LOCK_UN);
				move(20, 0);
				/*
				 * prints("** 새 글을 찾고 있습니다 **\n");
				 * refresh();
				 */
				continue;
			}
			sprintf(genbuf, "%s/%s", bfile(fptr->filename), fh.filename);
			readone = YEA;
#ifdef NOREPLY
			if (unread == 0)
				more(genbuf, 0, MORE_IS_NORMAL);
			else
				more(genbuf, 0, MORE_IS_NORMAL);
#else
			more(genbuf, 0, MORE_IS_NORMAL);

			sprintf(msgbuf, "<%s> \"%-.40s\" (R)답장 (Q)끝내기",
				xboard_dir(fptr->filename), fh.title);
			key = presskey(msgbuf);
			if (key == 'R' || key == 'r') {
				int os = selboard;
				strncpy(saveboard, currboard, STRLEN);
				strncpy(currboard, fptr->filename, STRLEN);
				selboard = 1;
				do_reply(fh.title, fh.filename);
				strncpy(currboard, saveboard, STRLEN);
				selboard = os;
			} else if (key == 'q' || key == 'Q') {
				move(4, 0);
				clrtoeol();
				refresh();
				close(fd);
				mot = -1;
				update_new_pointer(i, fptr->filename);
				return QUIT;
			}
			/*-
			#ifdef PERMS
				    if (haspostperm(fptr->filename)) {
			#endif
					getone(t_lines - 1, 0, "Reply (Y/N,SP)[N] ? ", ans, 6, DOECHO, NULL);
					if (ans[0] == 'Y' || ans[0] == 'y') {
					    strncpy(saveboard, currboard, STRLEN);
					    strncpy(currboard, fptr->filename, STRLEN);
					    do_reply(fh.title, fh.filename);	//- added by skji //
					    strncpy(currboard, saveboard, STRLEN);
					}
			#ifdef PERMS
				    } else
					pressreturn();
			#endif
				    clear();
			*/
#endif
			flock(fd, LOCK_EX);
			lseek(fd, offset + i * sizeof(fh), L_SET);
			ch |= FILE_READ;
			ch &= ~FILE_VISIT;
			write(fd, &ch, 1);

			/* new에서 본글 카운트chopin */
			lseek(fd, ((int) &(fh.readcnt) - (int) &(fh)) + (i * sizeof(fh)), L_SET);
			/* bugman : 한사용자가 read를 하고 있을 때 다른 사용자가
			같은 글을 읽을 경우 조회수가 다르게 나타남 */

			read(fd, &(fh.readcnt), sizeof(fh.readcnt));
			fh.readcnt++;
			lseek(fd, ((int) &(fh.readcnt) - (int) &(fh)) + (i * sizeof(fh)), L_SET);
			write(fd, &fh.readcnt, sizeof(fh.readcnt));

			flock(fd, LOCK_UN);
			move(20, 0);
			/*
			 * prints("** 새 글을 찾고 있습니다 **\n");
			 * refresh();
			 */
			clear();
		}
	}
	close(fd);
	update_new_pointer(i, fptr->filename);
#ifdef NOREPLY
#ifdef PERMS
	/* !!!!!! chopin */
	if (readone == YEA && haspostperm(fptr->filename)) {
#endif
		char            localbuf[STRLEN];
		int             lineno = t_lines - 1;
		strncpy(saveboard, currboard, STRLEN);
		strncpy(currboard, fptr->filename, STRLEN);
		sprintf(localbuf,
			"'%s' 글상자의 끝입니다. 글을 올리겠습니까 (Y/N)[N] ? ", bfile(currboard));
		while (1) {
			getdata(lineno, 0, localbuf, ans, 6, DOECHO, NULL);
			if (ans[0] == 'Y' || ans[0] == 'y')
				do_post();
			else
				break;
			if (lineno == t_lines - 1) {
				lineno = 0;
				sprintf(localbuf,
					"'%s' 글상자에 다른 글을 올릴까요(Y/N)[N] ? ", bfile(currboard));
			}
		}
		strncpy(currboard, saveboard, STRLEN);
#ifdef PERMS
	} else if (readone == YEA)
		pressreturn();
#endif
#endif

	return 0;
}

int New(void)
{
	/*- report("New"); */
	clear();
	mot = 0;

	uinfo.mode = READNEW;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	/*- chopin 계층화에 의해 바뀜
	    apply_boards(read_boards);
        */
	apply_childboards(read_boards);
	/*- clear(); <--- deleted; skji */
	uinfo.mode = MMENU;
	substitute_record(ULIST, (void *) &uinfo, sizeof(uinfo), utmpent);
	clear();
	switch (mot) {
	case 1:
		prints("글이 더 이상 없습니다.\n");
		/* pressanykey(); */
		break;
	case 0:
		prints("다 읽었네요. 글을 써주세요.\n");
		break;
	case -1:
		prints("나중에 읽으세요.\n");
	}
	pressanykey();
	return -1;
}

int visit_boards(struct shortfile * fptr)
{

	int    i, offset;
	long   numfiles;
	int             fd;
	unsigned char   ch;
	struct stat     st;

	offset = ((int) &(fh.accessed[usernum]) - (int) &(fh));
	/*
	 * chopin sprintf(genbuf, "boards/%s/%s", fptr->filename, DIR);
	 */
	sprintf(genbuf, "%s/%s", bfile(fptr->filename), DIR);
	if ((fd = open(genbuf, O_RDWR)) < 0)
		return 0;
	fstat(fd, &st);
	numfiles = st.st_size;
	numfiles = numfiles / sizeof(fh);
	if (numfiles <= 0) {
		close(fd);
		return 0;
	}
	flock(fd, LOCK_EX);
	lseek(fd, offset, L_SET);
	for (i = 0; i < numfiles; i++, lseek(fd, offset + i * sizeof(fh), L_SET)) {
		read(fd, &ch, 1);
		if (!(ch & FILE_READ)) {
			lseek(fd, offset + i * sizeof(fh), L_SET);
			/*- bugman : Visit 했을 경우 읽은 것으로 count
			하는 것은 잘못, 글읽은 사용자 보기 에서 부정확한 정보 출력
			ch |= FILE_READ;
			*/
			ch |= FILE_VISIT;
			write(fd, &ch, 1);
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
	move(2,0);
	prints(genbuf);
	clrtoeol();
	refresh();
	return 0;
}

int Visit(void)
{
	/*-    report("Visit") ;*/
	char            ans[5];
	char            bname[STRLEN];
	struct fileheader fh;

	if (tooManyUsers()){
		move( 3, 0);
		prints("현재 사용자가 너무 많아 이 기능을 사용할수 없습니다.");
		pressanykey();
		return -1; /* Here */
	}
	clear();
	refresh();
	move(0, 0);
	uinfo.mode = VISIT;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	prints("글상자 읽은 것으로 하기\n");
	move(1, 0);
	getdata(1,0,"(A)지금방 아래의 모든글상자 (1)글상자 고름 (Q)마침 [Q] ? ", ans,5,DOECHO,NULL);

/* - by funboy
	getone(1, 0, "(A)지금방 아래의 모든 글상자 (1)글상자 고름 (Q)마침 [Q] ? ", ans, 5, DOECHO, NULL);
*/
	move(1, 0);
	clrtoeol();
	switch (ans[0]) {
	case 'A':
	case 'a':
		/*- chopin
			    apply_boards(visit_boards);
		*/
		apply_childboards(visit_boards);
		break;
	case '1':
		make_blist();
		prints("글상자를 고르세요: ");
		namecomplete((char *) NULL, bname);
		move(5, 0);
		/*
		 * chopin if (*bname == '\0' || !search_record(BOARDS, &fh,
		 * sizeof(fh), cmpbnames, (intptr_t) bname))
		 */
		if (*bname == '\0' ||
		    !search_record(bfile(BOARDS), &fh, sizeof(fh), cmpbnames, (intptr_t) bname)) {
			prints("글상자 이름이 잘못되었습니다.\n");
			break;
		}
		if (fh.isdirectory) {
			pushdir(board_dir, fh.filename);
			apply_childboards(visit_boards);
		} else {
            struct shortfile board = {0};
            strcpy(board.filename, fh.filename);
            visit_boards(&board);
        }
		prints("..완료!\n");
		break;
	default:
		move(5, 0);
		prints("읽은 걸로 하기 관둠\n");
	}
	pressreturn();
	clear();
	uinfo.mode = MMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return -1;
}

int             ismail = 0;
long            lasttime = 0;
int chkmail(void)
{
	struct stat     st;
	int             fd;
	int    i, offset;
	long   numfiles;
	unsigned char   ch;
	extern char     currmaildir[4096];

	offset = ((int) &(fh.accessed[0]) - (int) &(fh));
	if ((fd = open(currmaildir, O_RDONLY)) < 0)
		return (ismail = 0);
	fstat(fd, &st);
	if (lasttime >= st.st_mtime) {
		close(fd);
		return ismail;
	}
	lasttime = st.st_mtime;
	numfiles = st.st_size;
	numfiles = numfiles / sizeof(fh);
	if (numfiles <= 0) {
		close(fd);
		return (ismail = 0);
	}
	lseek(fd, offset, L_SET);
	for (i = 0; i < numfiles; i++, lseek(fd, offset + i * sizeof(fh), L_SET)) {
		read(fd, &ch, 1);
		if (!(ch & FILE_READ)) {
			close(fd);
			return (ismail = 1);
		}
	}
	close(fd);
	return (ismail = 0);
}

int getnewuserid(void)
{
	int             fd;
	int             i;
	struct userec   utmp;

	if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0660)) == -1) {
		perror("open");
		return -1;
	}
	flock(fd, LOCK_EX);
	i = 0;
	while (read(fd, &utmp, sizeof(utmp)) == sizeof(utmp)) {
		if (utmp.userid[0] == '\0')
			break;
		i++;
	}
	if (i >= MAXUSERS) {
		flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	strcpy(utmp.userid, "new");
	if (lseek(fd, sizeof(utmp) * i, L_SET) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	write(fd, &utmp, sizeof(utmp));
	flock(fd, LOCK_UN);
	close(fd);
	return i + 1;
}

int getnewutmpent(struct user_info * up)
{
	int             fd;
	int             i;
	struct user_info utmp;

	if ((fd = open(ULIST, O_RDWR | O_CREAT, 0660)) == -1) {
		perror("open");
		return -1;
	}
	flock(fd, LOCK_EX);
	i = 0;
	while (read(fd, &utmp, sizeof(utmp)) == sizeof(utmp)) {
		if (!utmp.active || !utmp.pid)
			break;
		if (kill(utmp.pid, 0) == -1)
			break;
		i++;
	}
	if (lseek(fd, sizeof(utmp) * i, L_SET) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		perror("lseek");
		return -1;
	}
	write(fd, up, sizeof(utmp));
	flock(fd, LOCK_UN);
	close(fd);
	return i + 1;
}

unsigned char   uarray[MAXUSERS];

int genuarray(char * s)
{
	int             id;

	if (*s == '*')
		return 0;
	if (!(id = searchuser(s)))
		return 0;
	uarray[id] = 1;
	return 0;
}

int mass_delete(struct shortfile * fptr)
{

	int    i, j, offset;
	long   numfiles;
	int             fd;
	int zeroVal=0;
	unsigned char   ch;
	struct stat     st;

	offset = ((int) &(fh.accessed[usernum]) - (int) &(fh));
	/*
	 * chopin sprintf(genbuf, "boards/%s/%s", fptr->filename, DIR);
	 */
	sprintf(genbuf, "%s%s", bfile(fptr->filename), DIR);
	if ((fd = open(genbuf, O_RDWR)) < 0)
		return 0;
	fstat(fd, &st);
	numfiles = st.st_size;
	numfiles = numfiles / sizeof(fh);
	if (numfiles <= 0) {
		close(fd);
		return 0;
	}
	move (5,0);
	prints("%s를 업데이트 합니다.",  genbuf);
	clrtoeol();
	refresh();
	ch = 0;
	flock(fd, LOCK_EX);
	for (i = 0; i < numfiles; i++) {
		move(5, 65); prints("%d/%d",  i+1, numfiles);
		refresh();
		for (j = 0; j < MAXUSERS; j++) {
			if (uarray[j]) {
				offset = ((int) &(fh.accessed[j]) - (int) &(fh));
				lseek(fd, offset + i * sizeof(fh), L_SET);
				write(fd, &ch, 1);
			}
		}
	}
	flock(fd, LOCK_UN);
	close(fd);

        sprintf(genbuf, "%s/%s", bfile(fptr->filename), NEW_COUNT);
	move (5,0);
	prints("%s를 업데이트 합니다.",  genbuf);
	clrtoeol();
	refresh();

	if ((fd = open(genbuf, O_RDWR)) < 0)
		return 0;
        flock(fd, LOCK_EX);
        for (j = 0; j < MAXUSERS; j++) {
        	if (uarray[j]) {
        		lseek(fd, j, L_SET);
        		write(fd, &zeroVal, sizeof(int));
        	}
        }
        flock(fd, LOCK_UN);
        close(fd);

	return 0;
}

int mass_delmail(char * s)
{
	struct stat     st;

	if (*s == '*')
		return 0;
	if (*s == '\0')
		return 0;
	if (!isalnum(s[0]) && (s[0] & 0x80) == 0)
 	                  /*  ^^^^^^^^^^^^^^^^^^ bugman : for hangul id */
		return -1;	/*- rrr - extra precaution */
	if (s[0] == '\0') return 0;
	sprintf(genbuf, "mail/%s", s);
	if (stat(genbuf, &st) < 0)
		return 0;
	sprintf(genbuf, "/bin/rm -fr mail/%s signatures/%s plans/%s overrides/%s mybox/%s msgtmp/%s memo/%s",
		s, s, s, s, s, s, s);
	system(genbuf);

	return 0;
}

int mass_delusers(char * s)
{
	int             id;

	if (*s == '*')
		return 0;
	if (!(id = searchuser(s)))
		return 0;
	substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	return 0;
}

int newclean(char * s, int id)
{
	if (strcmp(s, "new"))
		return 0;
	substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	return 0;
}

int fastdelete(void)
{
	memset(uarray, 0, sizeof(uarray));
	ApplyToNameList(genuarray);

	move(4,0);
	clrtobot();
	prints(">>>> 사용자를 날리고 있습니다. >>>>");
	refresh();
	ApplyToNameList(mass_delmail);

	memset(&lookupuser, 0, sizeof(lookupuser));
	ApplyToNameList(mass_delusers);

	apply_users(newclean);

	move(4,0);
	prints(">>>> 글상자 정보 업데이트 >>>>>>>>>");
	apply_boards(mass_delete);

	truncate(PASSFILE, MAXUSERS * sizeof(lookupuser));

	touchnew();

	move(4,0);
	prints("왕창 다 지웠습니다.");
	clrtoeol();
	pressanykey();
	return 0;
}
