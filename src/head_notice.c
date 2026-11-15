/* Modified 2026-11-16. */
#include "bbs.h"
#include "edit.h"
#include "more.h"
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NOTICEDIR "etc/Notice"
#define NOTICEIDX "etc/Notice/.NOTICEDIR"
#define NOTICEMAX  14

 /* record.c 에 있는 함수들은 구조체 크기가 fileheader와
    같으면 특이한 동작을 하므로 fileheader와 거의 비슷한
    구조체를 만들어 사용.  -bugman */

struct noticeheader {
        char            filename[STRLEN];
        char            title[STRLEN];
        unsigned char   tm_year;/* struct tm 구조멤버와 동일하게 이름붙였음 */
        unsigned char   tm_mon;
        unsigned char   tm_mday;
        unsigned short int readcnt;     /* 조회수 */
        unsigned char   isNoname;
        unsigned char   accessed[MAXUSERS];

};

void setNoticeToNew(void)
{
	struct noticeheader newhd;
	int fd, i;
	int recNum;
	long offset;
	struct stat st;

	fd = open(NOTICEIDX, O_RDWR);
        fstat(fd, &st);
        recNum = st.st_size / sizeof(newhd);
        offset = (long)&(newhd.accessed[usernum]) - (long)&(newhd);
        flock(fd, LOCK_EX);
        for(i=1; i<= recNum; i++){
                newhd.accessed[0] = 0;
                lseek(fd, offset+ (i-1)*sizeof(newhd), SEEK_SET);
                write(fd, &(newhd.accessed[0]), sizeof(newhd.accessed[0]));
        }
        flock(fd, LOCK_UN);
        close(fd);
}

void dispNotice(int isLoginTime)
{
        int fd;
        struct noticeheader ih;
        int num, cn;
        int xx, yy;
        char buf[10];
	struct stat stbuf;
	int sel;
	int newArticleExist;
	int warningMessage = 0;

		/* New user can login without read notice !! */
	if (currentuser.numlogins <10 ) isLoginTime = NA;
	if (stat(NOTICEIDX, &stbuf) <0 || stbuf.st_size == 0 ) return;
/*
	newArticleExist  = NA;
        fd = open(NOTICEIDX, O_RDONLY);
        for (num=1; read(fd, &ih, sizeof(ih)) == sizeof(ih); num++){
		if (!(ih.accessed[usernum] & FILE_READ)){
			newArticleExist = YEA;
		}
	}
	close(fd);
	if (isLoginTime && !newArticleExist )
		return;
*/
        while(1){
		newArticleExist  = NA;
                clear();
                move (2, 30);
                prints("SYSOP이 여러분에게");
                move (3, 30);
                prints("~~~~~~~~~~~~~~~~~~");
                xx = 10, yy = 4;
		move(yy++, xx);
		standout();
		prints("번호  날    짜    제            목                        ");
		standend();

        	fd = open(NOTICEIDX, O_RDONLY);
                for (num=1; read(fd, &ih, sizeof(ih)) == sizeof(ih); num++){
                        move(yy, xx-6);
			standout();
                        if (!(ih.accessed[usernum] & FILE_READ)){
				prints("New!!");
			}
			standend();
			move(yy, xx);
                        prints("%2d.  %2d월 %2d일    %s",
				num, ih.tm_mon+1, ih.tm_mday, ih.title);
                        yy ++;
			if (!(ih.accessed[usernum] & FILE_READ)){
				newArticleExist = YEA;
			}
			/*
			prints("\nih.accessed[usernum] = %d\n", ih.accessed[usernum]);
			prints("ih.accessed[usernum] & FILE_READ = %d\n", ih.accessed[usernum] &FILE_READ);
			prints("newArticleExist = %d\n", newArticleExist);
			refresh();
			igetch();
			*/
                }
        	close(fd);
		move(19, 10);
                prints("==========================================================");
		if ( newArticleExist  ){
			if (isLoginTime) bell();
			move(22, 15);
			standout();
			if (isLoginTime && warningMessage)
 				prints("** 새 알림말을 읽지 않으면 들어갈 수 없습니다. **");
			else
				prints("           ** 새 알림말이 있습니다. **           ");
			standend();
		}
		if (isLoginTime && newArticleExist ){
                	getone( 20, 10, "번호를 고르세요. ", buf, 5, DOECHO, NULL);
/*
                	getone( 20, 10, "번호를 고르세요. 끝내려면 [Return] ", buf, 5, DOECHO, NULL);
*/
			switch(buf[0]){
			case ' ': 	case '\0':
			case '\r':	case '\n':
				warningMessage = YEA;
			}
		} else {
                	getone( 20, 10, "번호를 고르세요. 끝내려면 [Space] ", buf, 5, DOECHO, NULL);
			switch( buf[0] ){
			case ' ': 	case '\0':
			case '\r':	case '\n':
				return;
			}
		}
		sel =  atoi(buf);
                if (1 <= sel &&  sel < num){
			get_record(NOTICEIDX, &ih, sizeof ih, sel);
                        more(ih.filename, YEA, MORE_IS_NORMAL);
			if ( !(ih.accessed[usernum] & FILE_READ)){
				int fd;
				int offset;
				char ch;

				fd = open(NOTICEIDX, O_RDWR);
				offset = (int) &(ih.accessed[usernum]) - (int) (&ih);
				ch = FILE_READ;
				flock(fd, LOCK_EX);
				lseek(fd, offset + ((sel-1)*sizeof(ih)), SEEK_SET);
				write(fd, &ch, sizeof(ch));
				flock(fd, LOCK_UN);
				close(fd);
			}
                }
        }
}

int manageNotice(void)
{
        int fd, i;
	int noticeNum;
        struct noticeheader ih;
        struct stat stBuf;
	char buf[8];
	char mbuf[100];
#define ACT_ADD 0
#define ACT_DELE 1
#define ACT_EDIT 2
#define ACT_NONE 3
	int action;
	int sel;
        struct tm      *tm;
        time_t          clk;

	uinfo.mode = OPEN;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	if (!HAS_PERM(PERM_SYSOP)){
		dispNotice(NA);
		goto QQUIT;
	}
        fd = open(NOTICEIDX, O_RDWR);

	move(4,0);
	standout();
	prints("  공지 사항 목차                                                        \n");
	standend();

	clrtobot();
	for (noticeNum=1; read(fd, &ih, sizeof(ih)) == sizeof(ih); i++, noticeNum++){
		prints("%2d. %s\n", noticeNum, ih.title);
	}
	close(fd);
	sprintf(mbuf, "%s 지우기(d) 고치기(e) 보기(v) : ", (noticeNum < NOTICEMAX-1) ? "추가(a)" : "-");
	getdata(3, 0, mbuf, buf, 4, DOECHO, NULL);
	if (buf[0] == 'v' || buf[0] == 'V'){
		dispNotice(NA);
		goto QQUIT;
	}
	switch (buf[0]){
	case 'A': case 'a':
		if (noticeNum < NOTICEMAX-1){
			action = ACT_ADD;
		} else {
			action = ACT_NONE;
		}
		break;
	case 'D': case 'd':
		action = ACT_DELE; break;
	case 'E': case 'e':
		action = ACT_EDIT; break;
	default	:
		action = ACT_NONE;
	}

	if ( action == ACT_NONE) {
		goto QQUIT;
	}
	clk = time(NULL);
        tm = localtime(&clk);
	if (action == ACT_ADD) {
		char fn[80];
		int aborted;

		memset(&ih, 0, sizeof(ih));
		sprintf(fn, "%s/NOTICE.XXXXXX", NOTICEDIR);
		mktemp(fn);
        	getdata(1, 0, "제목 : ", ih.title, 60, DOECHO, NULL);
        	strncpy(save_title, ih.title, STRLEN);
        	strncpy(save_filename, fn, 4096);
		strcpy(ih.filename, fn);
        	in_mail = NA;
		aborted = vedit(fn, YEA, TEXT_IS_NOTICE);
		if (!aborted){
        		ih.tm_year = tm->tm_year;
        		ih.tm_mon = tm->tm_mon;
        		ih.tm_mday = tm->tm_mday;
			/* append_record 를 함부로 쓰지 말것
			   record의 크기가 fileheader와 같으면 게시판으로 간주해버림
			append_record(NOTICEIDX, &ih, sizeof ih);
			*/
			fd = open(NOTICEIDX, O_RDWR | O_CREAT, 0664);
			flock(fd, LOCK_EX);
			lseek(fd, 0, SEEK_END);
			write(fd, &ih, sizeof (ih));
			flock(fd, LOCK_UN);
			close(fd);
		}
		goto QQUIT;
	}
	sprintf(mbuf, "%s 공지사항의 번호를 고르세요. : ", action == ACT_DELE ? "지울" : "고칠");
	getdata( 3, 0, mbuf, buf, 5, DOECHO, NULL);
	sel = atoi(buf);
	if (sel >= noticeNum || sel < 1){
		close(fd);
		move(3, 0);
		prints(" 잘못 고르셨습니다. ");
		clrtoeol();
		pressanykey();
		goto QQUIT;
	}
	if (action == ACT_DELE){
		int fd, fdout, i;
		char	tmpFn[100];
		char	backupFn[100];

		sprintf(tmpFn, "%s/.tmpfile", NOTICEDIR);
		sprintf(backupFn, "%s/.deleted", NOTICEDIR);
		fd = open(NOTICEIDX, O_RDONLY);
		fdout = open(tmpFn, O_WRONLY | O_CREAT | O_EXCL, 0660);
		if  (fd < 0 || fdout <0){
			prints("파일을 지우는데 문제가 있습니다");
			close(fd);
			close(fdout);
			pressanykey();
			goto QQUIT;
		}
		flock(fd, LOCK_EX);
		flock(fdout, LOCK_EX);
		for (i=1; read(fd, &ih, sizeof(ih)) == sizeof(ih); i++){
			if (i != sel){
				write(fdout, &ih, sizeof(ih));
			}else {
				unlink(ih.filename);
			}
		}
		close(fd);
		close(fdout);
		flock(fd, LOCK_UN);
		flock(fdout, LOCK_UN);
		rename(NOTICEIDX, backupFn);
		rename(tmpFn, NOTICEIDX);
		goto QQUIT;
	}
	if (action == ACT_EDIT){
		get_record(NOTICEIDX, &ih, sizeof(ih), sel);
		vedit(ih.filename, NA, TEXT_IS_EDIT);
        	ih.tm_year = tm->tm_year;
        	ih.tm_mon = tm->tm_mon;
        	ih.tm_mday = tm->tm_mday;
/* - 공지사항을 고쳤을 경우 "New"로 표시되게하는 부분 막아놓음
		bzero(ih.accessed, sizeof(ih.accessed));
*/
		substitute_record(NOTICEIDX, &ih, sizeof(ih), sel);
		goto QQUIT;
	}
QQUIT:
	uinfo.mode = XMENU;
	substitute_record(ULIST, &uinfo, sizeof(uinfo), utmpent);
	return 0;
}

