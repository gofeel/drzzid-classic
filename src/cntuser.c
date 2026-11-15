/* Modified 2026-11-16. */
/* .UTMP 화일을 읽어서 내용을 보여주는 프로그램 */
#include <stdio.h>
#include "bbs.h"

struct user_info uinfo;
int main(int argc, char ** argv)
{
	FILE *fp;
	int num;
	int usercn;

	if (argc <2) {
		fprintf(stderr, "USAGE:\n\t%s UTMP filename\n", argv[0]);
		return 1;
	}
	fp = fopen(argv[1], "rb");
	if (fp == NULL){
		fprintf(stderr, "%s: cannot open file %s\n", argv[0], argv[1]);
		return 1;
	}
	for(num=1, usercn=0; fread(&uinfo, sizeof(uinfo), 1, fp) == 1; num++){
		if (uinfo.active && uinfo.pid && kill(uinfo.pid, 0) != -1) usercn++;
	}
	fclose(fp);
	printf("%d\n", usercn);
	return 0;
}
