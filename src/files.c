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

#ifdef FILES

struct protocols {
	char           *pname;
	char           *sendbin;
	char           *recvbin;
};

struct protocols protos[] =
{
	{"Kermit", "bin/kermit -si", "bin/kermit -ri"},
	{"Xmodem", "bin/sx -be", "bin/rx -bpe"},
	{"Ymodem", "bin/sb -be", "bin/rb -bpe"},
	{"Zmodem", "bin/sz -be", "bin/rz -bpe"}
};

#define MAXDIRSIZE 80

#define NUMPROTOS (sizeof(protos)/sizeof(struct protocols))

char * NameProtocol(int id)
{
	if (id < 0 || id >= NUMPROTOS)
		return NULL;
	return protos[id].pname;
}

char * CurrentProtocol(void)
{
	return NameProtocol(currentuser.protocol);
}

char            dirs[MAXDIRSIZE][STRLEN];
int             numdirs = 0;
static char     currfile[STRLEN];
int             fileselect = 0;

char * filemargin(void)
{
	static char     buf[STRLEN];

	if (fileselect)
		sprintf(buf, "파일 상자 : '%s'", currfile);
	else
		sprintf(buf, "파일 상자를 고르세요.");
	return buf;
}

#undef DIR

int get_dir(void)
{
	DIR            *dp;
	struct direct  *ent;
	struct stat     status;

	numdirs = 0;
	if ((dp = opendir("ftp/files")) == NULL)
		return -1;
	for (ent = readdir(dp); ent != NULL; ent = readdir(dp)) {
		if (!strcmp(ent->d_name, "."))
			continue;
		if (!strcmp(ent->d_name, ".."))
			continue;
		sprintf(genbuf, "ftp/files/%s", ent->d_name);
		if (stat(genbuf, &status))
			continue;
		if (!(status.st_mode & S_IFDIR))
			continue;
		if (numdirs >= MAXDIRSIZE)
			continue;
		strcpy(dirs[numdirs], ent->d_name);
		numdirs++;
	}
	closedir(dp);
	return 0;
}

int f_list(void)
{
	DIR            *dp;
	struct direct  *ent;
	struct stat     status;
	int             i;
	int             col;
	char            buf[512];

	if (!fileselect) {
		move(3, 0);
		clrtobot();
		prints("먼저 (S)elect로 파일 상자를 고르세요!\n");
		pressreturn();
		return -1;
	}
	move(3, 0);
	clrtobot();
	prints("다운로드를 위한 파일 목록\n");
	prints("%-19s  크기", "파일이름");
	sprintf(buf, "ftp/files/%s", currfile);
	if ((dp = opendir(buf)) == NULL)
		return -1;
	i = 5;
	col = 0;
	for (ent = readdir(dp); ent != NULL; ent = readdir(dp)) {
		if (!strcmp(ent->d_name, "."))
			continue;
		if (!strcmp(ent->d_name, ".."))
			continue;
		sprintf(genbuf, "ftp/files/%s/%s", currfile, ent->d_name);
		if (stat(genbuf, &status))
			continue;
		if ((status.st_mode & S_IFDIR))
			continue;
		if (i == 23) {
			col++;
			if (col == 3) {
				int             ch;
				move(23, 0);
				standout();
				prints("-- 계속 --");
				standend();
				while ((ch = igetch()) != EOF) {
					if (ch == 'q') {
						closedir(dp);
						move(23, 0);
						clrtoeol();
						return 0;
					}
					if (ch == '\n' || ch == '\r' || ch == ' ')
						break;
					bell();
				}
				col = 0;
				move(4, 0);
				clrtobot();
			}
			move(4, col * 26);
			prints("%-19s  크기", "파일이름");
			i = 5;
		}
		move(i, col * 26);
		prints("%-19s %4dk", ent->d_name, (status.st_size + 512) >> 10);
		i++;
	}
	closedir(dp);
	pressanykey();
	return 0;
}

int f_create_namelist(void)
{
	DIR            *dp;
	struct direct  *ent;
	struct stat     status;

	if (!fileselect) {
		return -1;
	}
	CreateNameList();
	sprintf(genbuf, "ftp/files/%s", currfile);
	if ((dp = opendir(genbuf)) == NULL)
		return -1;
	for (ent = readdir(dp); ent != NULL; ent = readdir(dp)) {
		if (!strcmp(ent->d_name, "."))
			continue;
		if (!strcmp(ent->d_name, ".."))
			continue;
		sprintf(genbuf, "ftp/files/%s/%s", currfile, ent->d_name);
		if (stat(genbuf, &status))
			continue;
		if ((status.st_mode & S_IFDIR))
			continue;
		AddNameList(ent->d_name);
	}
	closedir(dp);
	return 0;
}

int f_download(void)
{
	char            fname[STRLEN];
	char            path[512];
	struct stat     st;

	if (!fileselect) {
		move(3, 0);
		clrtobot();
		prints("먼저 (S)elect로 파일 상자를 고르세요!\n");
		pressreturn();
		return -1;
	}
	if (currentuser.protocol < 0 || currentuser.protocol >= NUMPROTOS)
		return 0;
	f_create_namelist();
	move(0, 0);
	prints("%s 다운로드\n", CurrentProtocol());
	prints("파일이름: ");
	clrtoeol();
	namecomplete(NULL, fname);
	/*
	 * chopin sprintf(path,"/ftp/files/%s/%s",currfile,fname) ;
	 */
	sprintf(path, "ftp/files/%s/%s", currfile, fname);
	if ((*fname == '\0') || (stat(path, &st) == -1)) {
		move(2, 0);
		prints("파일이름이 이상해요.\n");
		pressreturn();
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return 1;
	}
	if ((st.st_mode & S_IFDIR)) {
		move(1, 0);
		prints("파일이름이 이상해요.\n");
		pressreturn();
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return 1;
	}
	sprintf(genbuf, "downloading file '%s'", path);
	report(genbuf);
	move(3, 0);
	clrtobot();
	disableTalkFly();
	reset_tty();
	printf("%s protocol 사용\n", CurrentProtocol());
	sprintf(genbuf, "%s %s", protos[currentuser.protocol].sendbin, path);
	/*
	 * prints("ready to run"); prints(genbuf);
	 */
	do_exec(genbuf, NULL);
	restore_tty();
	enableTalkFly();
	clear();
	pressreturn();
	return 0;
}

int f_upload(void)
{
	char            buf[512];
	char            buf2[512];

	extern char home_dir[];

	if (!fileselect) {
		move(3, 0);
		clrtobot();
		prints("먼저 (S)elect로 파일 상자를 고르세요!\n");
		pressreturn();
		return -1;
	}
	if (currentuser.protocol < 0 || currentuser.protocol >= NUMPROTOS)
		return 0;
	move(3, 0);
	clrtobot();
	disableTalkFly();
	reset_tty();
	printf("%s protocol 사용\n", CurrentProtocol());
	sprintf(buf, "ftp/files/%s", currfile);
	strcpy(buf2, home_dir);
	if (buf2[strlen(buf2)-1] != '/') {
		strcat(buf2, "/");
	}
	strcat(buf2, protos[currentuser.protocol].recvbin);
	do_exec(buf2, buf);
	restore_tty();
	enableTalkFly();
	clear();
	pressreturn();
	return 0;
}

int f_protocol(void)
{
	int             i;
	char            proto[STRLEN];

	move(3, 0);
	clrtobot();
	prints("PROTOCOL MENU\n");
	for (i = 0; i < NUMPROTOS; i++)
		prints("%d)  %s\n", i, protos[i].pname);
	move(2, 0);
	prints("프로토콜 번호 (0-%d): ", NUMPROTOS - 1);
	clrtoeol();
	getdata(0, 0, NULL, proto, 3, DOECHO, NULL);
	if (proto[0] == '\0')
		return 0;
	i = atoi(proto);
	if (i < 0 || i >= NUMPROTOS)
		return 0;
	clear();
	currentuser.protocol = i;
	substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	return 0;
}

int f_select(void)
{
	int             i;
	extern int      numdirs;
	extern char     dirs[MAXDIRSIZE][STRLEN];
	char            buf[STRLEN];

	get_dir();
	move(3, 0);
	clrtobot();
	prints("파일 상자 마당\n");
	clrtoeol();
	for (i = 0; i < numdirs; i++) {
		move(i % 20 + 4, (i / 20) * 20);
		prints("%2d) %s", i, dirs[i]);
	}
	move(2, 0);
	prints("파일 상자 고름 (0-%d): ", numdirs - 1);
	clrtoeol();
	getdata(0, 0, NULL, buf, 3, DOECHO, NULL);
	if (buf[0] == '\0')
		return 0;
	i = atoi(buf);
	if (i < 0 || i >= numdirs)
		return 0;
	clear();
	strcpy(currfile, dirs[i]);
	fileselect = 1;
	return 0;
}

int f_text(void)
{
	char            fname[STRLEN];
	char            path[512];
	struct stat     st;

	if (!fileselect) {
		move(3, 0);
		clrtobot();
		prints("먼저 (S)elect로 파일 상자를 고르세요!\n");
		pressreturn();
		return -1;
	}
	f_create_namelist();
	move(0, 0);
	prints("문서 보기\n");
	prints("파일이름: ");
	clrtoeol();
	namecomplete(NULL, fname);
	/*
	 * chopin sprintf(path,"/ftp/files/%s/%s",currfile,fname) ;
	 */
	sprintf(path, "ftp/files/%s/%s", currfile, fname);
	if ((*fname == '\0') || (stat(path, &st) == -1)) {
		move(2, 0);
		prints("파일이름이 이상해요\n");
		pressreturn();
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return 1;
	}
	if ((st.st_mode & S_IFDIR)) {
		move(1, 0);
		prints("파일이름이 이상해요\n");
		pressreturn();
		move(0, 0);
		clrtoeol();
		move(1, 0);
		clrtoeol();
		move(2, 0);
		clrtoeol();
		return 1;
	}
	sprintf(genbuf, "Examining file '%s'", path);
	report(genbuf);
	more(path, YEA, MORE_IS_NORMAL);
	clear();
	return 0;
}

#endif				/* FILES */
