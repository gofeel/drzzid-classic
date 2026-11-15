/* Modified 2026-11-16. */
/* .UTMP 화일을 읽어서 내용을 보여주는 프로그램 */
#include <stdio.h>
#include "bbs.h"

struct user_info uinfo;
int main(int argc, char ** argv)
{
	int fd;
	struct userec urec;

	if (argc <10) {
		fprintf(stderr, "USAGE:\n\t%s PASSWDFIEL ID Password Terminal username realname address email PERMISSION\n", argv[0]);
		return -1;
	}
	memset(&urec, 0, sizeof(urec));

	strncpy(urec.userid, argv[2], IDLEN);
	strncpy(urec.passwd, genpasswd(argv[3]), PASSLEN);
	strncpy(urec.termtype, argv[4], STRLEN);
	strncpy(urec.username, argv[5], STRLEN);
	strncpy(urec.realname, argv[6], sizeof(urec.realname) - 1);
	strncpy(urec.address, argv[7], STRLEN);
	strncpy(urec.email, argv[8], STRLEN);
	if (!strcmp(argv[9], "SYSOP")){
		urec.userlevel  = PERM_POWEROVERWHELMING;
	} else if (!strcmp(argv[9], "NORMAL")){
		urec.userlevel  = PERM_NORMALUSER;
	} else if (!strcmp(argv[9], "GUEST")){
		urec.userlevel  = PERM_BASIC;
	}
	umask(0);
	fd = open(argv[1], O_CREAT | O_RDWR, 0660);
	if (fd < 0) {
		fprintf(stderr, "Cannot open password file '%s'\n", argv[1]);
		return -1;
	}
	lseek(fd, 0, SEEK_END);
	if (write(fd, &urec, sizeof(urec)) != sizeof(urec)){
		fprintf(stderr, "Write() Fail\n");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}
