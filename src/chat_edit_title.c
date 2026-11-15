/* Modified 2026-11-16. */
#include "bbs.h"
/* This file is created by skji */
/* 채팅방에서 제목을 사용합니다. */
/* 제목은 ".CHATROOM_TITLE" 에 저장됩니다. */

#include <stdio.h>
#include "edit.h"

/* This function is added by skji */
int edit_chatroom_title(int nth_chatroom)
{
	char            editbuf[100], prompt[200];

	sprintf(prompt, "왁자지껄방의 제목을 적어주세요 [%d] : ", nth_chatroom);
	getdata(5, 0, prompt, editbuf, 60, 1 /* == DOECHO */ , NULL);
	if (editbuf[0] != 0) {
		change_chatroom_title(nth_chatroom, editbuf);
	}
	return 0;
}
