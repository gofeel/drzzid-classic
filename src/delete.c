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
#include <unistd.h>

int d_exit(void)
{
	clear();
	return QUIT;
}

/* ARGSUSED */
int boardcheck(const void *record_data)
{
    const struct fileheader * fptr = record_data;

	/*
	 * if(!strcmp(fptr->filename,genbuf)) return 1 ; return 0 ;
	 */
	return 1;
}

int d_board(void)
{
	struct fileheader binfo;
	int             bid;
	extern int      numboards;

	move(3, 0);
	clrtobot();
	move(0, 0);
	standout();
	prints("글상자 지우기 글상자 지우기 글상자 지우기");
	standend();
	clrtoeol();
	move(1, 0);
	make_blist();
	namecomplete("지우려는 글상자 이름 : ", genbuf);
	bid = getbnum(genbuf);
	/*
	 * chopin if(get_record(BOARDS,&binfo,sizeof(binfo),bid) == -1) {
	 */
	if (get_record(bfile(BOARDS), &binfo, sizeof(binfo), bid) == -1) {
		move(2, 0);
		prints("글상자 이름이 이상하군요.\n");
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	prints("'%s'을 지웁니다.", binfo.filename);
	clrtoeol();
	getdata(2, 0, "(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		prints("글상자 지우기 중지\n");
		pressreturn();
		clear();
		return 0;
	}
	if (!isalnum2(binfo.filename[0]))
		return -1;	/* rrr - precaution */
	sprintf(genbuf, "/bin/rm -fr %s", bfile(binfo.filename));
	system(genbuf);
	strcpy(genbuf, binfo.filename);
	/*
	 * chopin delete_file(BOARDS,sizeof(binfo),bid,boardcheck) ;
	 */
	delete_file(bfile(BOARDS), sizeof(binfo), bid, boardcheck);
	move(2, 0);
	numboards = -1;
	sprintf(genbuf, "deleted board %s", binfo.filename);
	report(genbuf);
	prints("글상자를 지웠습니다.\n");
	pressreturn();
	clear();
	return 0;
}

int             delusernum;

/*- bugman : 원래 코드를 사용하면 사용자 지우기가 너무 느려지고
.NEW에 대한 업데이트 부분이 빠져 있으므로 새로 만듦 -

int duentposts(struct fileheader *fptr)
{
	char           *bfile();
	struct fileheader update;
	static int      id;

	if (fptr == NULL) {
		id = 0;
		return 0;
	}
	id++;
	/- bugman : 필요 없는 코드.. fptr에 update에 해당하는게 담겨 옮
	if (get_record(strcat(bfile(currboard), DIR), &update, sizeof(update), id))
		return -1;
	-/
	update.accessed[delusernum] = 0;
	if (substitute_record(strcat(bfile(currboard), DIR), &update, sizeof(update), id))
		return -1;
	return 0;
}

int duentbds(struct shortfile * fptr)
{
	char           *bfile();
	duentposts(NULL);
	strcpy(currboard, fptr->filename);
	apply_record(strcat(bfile(currboard), DIR), duentposts, sizeof(struct fileheader));
	/- BUG zapped boards need to be cleared -/
}
*/

int duentbds(struct shortfile * fptr)
{

	char		buf[100];
	struct fileheader newhd;
	int 		fd;
	off_t		offset;
	int		zeroVal=0;
	int		i, recNum;
	struct stat	st;

	strcpy(currboard, fptr->filename);
	sprintf(buf, "%s/%s", bfile(currboard), ".NEW");
	move(5,0);
	prints("%s를 업데이트 합니다.",buf);
	clrtoeol();
	refresh();

	substitute_record(buf, &zeroVal, sizeof(int), delusernum);

	sprintf(buf, "%s%s", bfile(currboard), DIR);
	move(5,0);
	prints("%s를 업데이트 합니다. ", buf);
	clrtoeol();
	refresh();

	if ((fd = open(buf, O_RDWR))<0){
		prints("Cannot open %s\n", buf);
		return 0;
	}
	flock(fd, LOCK_EX);
	/*- bugman: 너무 속도가 느려서 해당 바이트만 바꾸는 식으로 개선
	while (read (fd, &newhd, sizeof(newhd)) == sizeof(newhd)){
		newhd.accessed[delusernum] = 0;
		lseek(fd, - sizeof(newhd), SEEK_CUR);
		write(fd, &newhd, sizeof(newhd));
	}
	*/
	fstat(fd, &st);
	recNum = st.st_size / sizeof(newhd);
	offset = (char *)&newhd.accessed[delusernum] - (char *)&newhd;
	for(i=1; i<= recNum; i++){
		newhd.accessed[0] = 0;
		lseek(fd, offset+ (i-1)*sizeof(newhd), SEEK_SET);
		write(fd, &(newhd.accessed[0]), sizeof(newhd.accessed[0]));
	}
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

int del_me(void)
{
	int             id;

	/*
	 * if (!strcmp(currentuser.userid, "guest") ||
	 * !strcmp(currentuser.userid, "손님")){
	 */
	if (isGuest(currentuser.userid)) {
		/* by bugman : guest 는 지울 수 없다 */
		move(2, 0);
		prints(" 에구에구 안돼요.. 한번 손님은 영원한 손님 !!");
		pressanykey();
		return 0;
	}
	/* u_namelist() ; */
	move(0, 0);
	standout();
	prints(" [자살바위] .. 다시한번 생각해 보세요!!");
	standend();
	clrtoeol();
	move(1, 0);
	prints("%s님을 삭제합니다.", currentuser.userid);
	clrtoeol();
	getdata(2, 0, "(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		prints("자살 관둠!!!\n");
		pressreturn();
		clear();
		return 0;
	}
	if (!(id = getuser(currentuser.userid))) {
		move(3, 0);
		prints("아이디가 이상하군요. SYSOP에게 문의하세요.");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	{ /* Here */
		move(2,0);
		prints("현재 자살바위가 폐쇄된 상태입니다. ");
		clrtoeol();
		prints("\n아이디를 삭제하고 싶으신 분은 'SYSOP' 에게 편지를 보내 십시요.");
		pressanykey();
		return 0;
	}
	delusernum = id;
	if (currentuser.userid[0] == '\0') return 0;
	sprintf(genbuf,
	 "/bin/rm -fr mail/%s signatures/%s plans/%s overrides/%s mybox/%s msgtmp/%s memo/%s memo/%s.old wait_me/%s",
		currentuser.userid, currentuser.userid, currentuser.userid,
		currentuser.userid, currentuser.userid, currentuser.userid,
		currentuser.userid, currentuser.userid, currentuser.userid);
	system(genbuf);
	sprintf(genbuf, "Deleted user %s by himself", currentuser.userid);
	currentuser.userid[0] = '\0';
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), id);
	report(genbuf);
	clrtobot();
	apply_child_dir(duentbds);
	clear();
	prints("안녕히 가세요...\n");
	touchnew();
	exit(0);
	return 0;
}
int d_user_by_id(int id)
{
	int userIsNew=NA;

	delusernum = id;
	move(1, 0);
	prints("'%s'을 날립니다.", lookupuser.userid);
	clrtoeol();
	move(5,0);
	standout();
	prints("  다음은 사용자의 정보입니다. 참고 하세요       \n");
	standend();
	prints("  ID     : %s\n\n", lookupuser.userid);
	prints("* 본명   : %s\n\n", lookupuser.realname);
	prints("* 소속   : %s\n\n", lookupuser.address);
	prints("  별명   : %s\n\n", lookupuser.username);
	prints("  e-mail : %s", lookupuser.email);
	getdata(2, 0, "(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		prints("아이디 날리기 관둠\n");
		pressreturn();
		clear();
		return 0;
	}
	if (lookupuser.userid[0] == '\0') return 0;
	sprintf(genbuf,
	 "/bin/rm -fr mail/%s signatures/%s plans/%s overrides/%s mybox/%s msgtmp/%s memo/%s memo/%s.old wait_me/%s",
		lookupuser.userid, lookupuser.userid, lookupuser.userid,
		lookupuser.userid, lookupuser.userid, lookupuser.userid,
		lookupuser.userid, lookupuser.userid, lookupuser.userid);
	system(genbuf);
	if(kick_user_by_id(lookupuser.userid)){
		move(2,0);
		prints("사용자를 추방했습니다. ");
		refresh();
	}
	sprintf(genbuf, "Deleted user %s", lookupuser.userid);
	if (!strcmp(lookupuser.userid, "new")){
		userIsNew = YEA;
	}
	lookupuser.userid[0] = '\0';
	substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	report(genbuf);
	clrtobot();
	if (!userIsNew)
		apply_child_dir(duentbds);
	move(2, 0);
	prints("아이디를 날렸습니다.\n");
	touchnew();
	pressreturn();
	clear();
	return 0;
}
int d_user(void)
{
	int             id;

	u_namelist();
	move(0, 0);
	standout();
	prints("[아이디 날리기]");
	standend();
	clrtoeol();
	move(1, 0);
	namecomplete("날리려는 아이디 : ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("아이디가 이상하군요.");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	/*
	 * if (!isalpha(lookupuser.userid[0])) return 0;
	 */
	/* rrr - don't know how... */
	return (d_user_by_id(id));

}

int kick_user_by_id(char * kickuser)
{
        int             id, ind;
        struct user_info uin;
        struct userec   kuinfo;
	char buffer[40];

        if (!(id = getuser(kickuser))) {
		return NA;
	}
        search_record(PASSFILE, &kuinfo, sizeof(kuinfo), cmpuids, (intptr_t)kickuser);
        ind = search_record(ULIST, &uin, sizeof(uin), t_cmpuids, id);
        if (!ind || !uin.active || (kill(uin.pid, 0) == -1)) {
                return NA;
        }

	/* -by bugman : dumpEdit를 위해서 signal을 SIGHUP으로 바꿈
        kill(uin.pid, 9);
	*/
        kill(uin.pid, SIGHUP);
        sprintf(buffer, "kicked %s", kickuser);
        report(buffer);
		usieslog("KICK", kuinfo.userid, kuinfo.username, "");
        uin.active = NA;
        uin.pid = 0;
        uin.invisible = YEA;
        uin.sockactive = 0;
        uin.sockaddr = 0;
        uin.destuid = 0;
        substitute_record(ULIST, &uin, sizeof(uin), ind);
        return YEA;

}

int kick_user(void)
{
	int             id;
	char            kickuser[40];

	u_namelist();
	clear();
	move(0, 0);
	standout();
	prints("사용자 추방");
	standend();
	clrtoeol();
	move(1, 0);
	creat_list();
	namecomplete("추방할 아이디 : ", kickuser);
	if (*kickuser == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(kickuser))) {
		move(3, 0);
		prints("이상한 아이디!");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	prints("'%s'를 추방합니다.", kickuser);
	clrtoeol();
	getdata(2, 0, "(Y/N)[N] ? ", genbuf, 4, DOECHO, NULL);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		prints("추방하기 관둠\n");
		pressreturn();
		clear();
		return 0;
	}
	if ( kick_user_by_id(kickuser)){
                move(3, 0);
                prints("사용자가 도망갔어요.");
                clrtoeol();
                return 0;
	} else {
        	move(2, 0);
        	prints("사용자를 추방했어요.\n");
        	return 0;
	}
	return 0;
}
