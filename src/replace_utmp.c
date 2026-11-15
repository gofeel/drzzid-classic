/* Modified 2026-11-16. */
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include "bbs.h"
/*
실행:replace_utmp filename rptr size id
script로 .UTMP화일을 바꿀때 사용함.
filename을 ".UTMP" 로 하고
	rptr:현재행동
	size:한레코드의 크기
	id  :바꿀 유저의 번호
를 넣은 다음 바꾸게 됨.
size(한레코드 크기)는 보통 struct user_info의 크기인데 sizeof_user_info실행화일을 실행하면 그 크기를 얻을 수 있다.
bbs프로그램에서 [Talk/User]명령에서 보이는 현재행동을
스크립트 실행으로 바꾸기 위해 작성함

			Thu Aug 11 01:20:49 KDT 1994
				chopin
*/
/*
	잘못된 유저번호를 argument 로 받아들일 경우 .UTMP 화일을 엉망으로
	만드는 버그가 있어 수정함.
		1995년 11월 20일  송민철
*/

int main(int argc, char ** argv)
{
	char filename[200];
	char status[200];
	struct user_info buf;
	int size;
	int id;
	if(argc<4)
	{
		printf("exec: replace_utmp <filename> <id> <status>\n");
		printf("      replace_utmp <화일이름> <id> <상태>\n");
		exit(0);
	}
	strcpy(filename,argv[1]);
	strcpy(status,argv[3]);
	size=sizeof(struct user_info);
	id=atoi(argv[2]);
	get_record(filename,&buf,size,id);
	strncpy(buf.chatid,status, sizeof(buf.chatid)/sizeof(buf.chatid[0]));
	buf.chatid[sizeof(buf.chatid) - 1] = '\0';
	substitute_record(filename, &buf, size, id);
	return 0;
}

/*substitute_record()와 safewrite(),get_record()는 record.c에서 복사해서
	적당히 고친 것임
*/
int substitute_record(char * filename, void * rptr, int size, int id)
{
    int             fd;

    if (id <=0 ) {
	fprintf(stderr, "Invalid utmp index %d", id);
	return -1;
    }
    if ((fd = open(filename, O_WRONLY | O_CREAT, 0664)) == -1)
        return -1;
    flock(fd, LOCK_EX);
    if (lseek(fd, size * (id - 1), L_SET) == -1){
        perror("substitute_record failed!!! (lseek)");
	flock(fd, LOCK_UN);
	close(fd);
        return -1;
    }
    if (safewrite(fd, rptr, size) != size){
        perror("substitute_record failed!!! (safewrite)");
	flock(fd, LOCK_UN);
	close(fd);
	return -1;
    }
    flock(fd, LOCK_UN);
    close(fd);
    return 0;
}
int safewrite(int fd, const void * buf, int size)
{
    int             cc, sz = size, origsz = size;
    const char     *bp = buf;
                                                        /*- #ifdef POSTBUG */
    do {
        cc = write(fd, bp, sz);
        if ((cc < 0) && (errno != EINTR)) {
            printf("safewrite failed!");
            return -1;
        }
        if (cc > 0) {
            bp += cc;
            sz -= cc;
        }
    }
    while (sz > 0);
    return origsz;
}

int get_record(char * filename, void * rptr, int size, int id)
{
    int             fd;

    if ((fd = open(filename, O_RDONLY, 0)) == -1)
        return -1;
    if (lseek(fd, size * (id - 1), L_SET) == -1) {
        close(fd);
        return -1;
    }
    if (read(fd, rptr, size) != size) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

