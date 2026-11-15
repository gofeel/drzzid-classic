/* Modified 2026-11-16. */
#include "bbs.h"
#include "edit.h"
#include "more.h"

int c_edit(void)
{
	int             aborted;

	sprintf(genbuf, "mybox/%s", currentuser.userid);
	aborted = vedit(genbuf, NA, TEXT_IS_EDIT);
	clear();
	if (!aborted) {
		if (chmod(genbuf, 0600) < 0)
			perror("Chmod fail");
		prints("사물함 내용을 변경하였습니다.");
		report("edited mybox");
	}
	pressreturn();
	return 0;
}

int c_view(void)
{
	FILE *fp;
	char buf[512];

	sprintf(buf, "mybox/%s", currentuser.userid);
	fp = fopen(buf, "r");
	if (fp == NULL){
		move(3,0);
		prints("사물함에 내용이 없네요.. ");
		pressanykey();
		return 0;
	}
	fclose(fp);
	sprintf(genbuf, "mybox/%s", currentuser.userid);
	more(genbuf, YEA, MORE_IS_NORMAL);
	return 0;
}

int c_remove(void)
{
	sprintf(genbuf, "mybox/%s", currentuser.userid);
	unlink(genbuf);
	move(3, 0);
	prints("엎질러진 물이 되었습니다.\n");
	report("delete mybox");
	pressreturn();
	clear();
	return 0;
}

int c_upload(void)
{

	char buf[512], buf2[512];

	move(3,0);
	clrtobot();
	getdata(4, 0, "Zmodem으로 파일을 올릴 준비가 되었습니까? [Y/n]", buf, 3, DOECHO, NULL);
	if (buf[0] == 'n' || buf[0] == 'N')
		return 0;
	sprintf(buf, "tmp/%s", currentuser.userid);
	unlink(buf);
	sprintf(buf, "bin/rzSaveToCabinet -a ");
	disableTalkFly();
	reset_tty();
	do_exec(buf, NULL);
	restore_tty();
	enableTalkFly();
	sprintf(buf, "tmp/%s--", currentuser.userid);
	sprintf(buf2, "mybox/%s", currentuser.userid);
	unlink(buf2);
	rename(buf, buf2);
	pressanykey();
	return 0;
}

int c_download(void)
{
	char            buf[512], notgenbuf[512], filename[512];
	char           *t;

	{
		FILE *fp;
		sprintf(buf, "mybox/%s", currentuser.userid);
		fp = fopen(buf, "r");
		if (fp == NULL){
			move(3,0);
			prints("사물함에 내용이 없네요.. ");
			pressanykey();
			return 0;
		}
		fclose(fp);
	}
	move(3,0);
	clrtobot();
	sprintf(filename, "nari%04d.txt", time(0) % 10000);
	sprintf(buf, "파일 이름 지정 [%s] ", filename);
	getdata(4, 0, buf, buf, 14, DOECHO, NULL);
	if (buf[0] != '\0'){
		strcpy(filename, buf);
	}
	getdata(6, 0, "Zmodem으로 다운받을 준비가 되었습니까? [Y/n]", buf, 3, DOECHO, NULL);
	if (buf[0] == 'n' || buf[0] == 'N')
		return 0;
	clrtobot();
	sprintf(notgenbuf, "mybox/%s", currentuser.userid);
	sprintf(buf, "tmp/%s", filename);
	unlink(buf);
	link(notgenbuf, buf);
	sprintf(notgenbuf, "bin/sz -ae %s", buf);
	disableTalkFly();
	reset_tty();
	do_exec(notgenbuf, NULL);
	restore_tty();
	enableTalkFly();
	unlink(buf);
	pressanykey();
	return 0;
}
