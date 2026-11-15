/* Modified 2026-11-16. */
#include <stdlib.h>
#include <string.h>
/* This file is created by skji */
/* 채팅방에서 제목을 사용합니다. */
/* 제목은 ".CHATROOM_TITLE" 에 저장됩니다. */

#include <stdio.h>
#include "chat.h"
#include "edit.h"

/* This function is added by skji */
int change_chatroom_title(int nth_chatroom, char * newtitle)
{
	FILE           *inf, *outf;
	int             retval;
	char            readbuf[500], tmpfname[500];
	if ((inf = fopen(CHATROOM_TITLE, "r"))) {
		sprintf(tmpfname, "%s.TMP", CHATROOM_TITLE);
		if ((outf = fopen(tmpfname, "w"))) {
			int             i;
			for (i = 0; i < CHATROOM_CNT; i++) {
				if (fgets(readbuf, 100, inf) == NULL) {
					fprintf(outf, "\n");
				} else if (i == (nth_chatroom - 1)) {
					fprintf(outf, "%s\n", newtitle);
				} else {
					fprintf(outf, readbuf);
				}
			}
			fclose(outf);
			fflush(inf);
			fflush(outf);
			rename(tmpfname, CHATROOM_TITLE);
		}
		fclose(inf);
	} else if ((outf = fopen(CHATROOM_TITLE, "w"))) {
		int             i;
		for (i = 0; i < CHATROOM_CNT; i++)
			fprintf(outf, "\n");
		fclose(outf);
	}
	return 0;
}
