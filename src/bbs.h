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

#ifndef BBS_H
#define BBS_H

#ifndef BBSIRC

/* Global includes, needed in most every source file... */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "bbs_callbacks.h"
#include <stdint.h>


#if NO_TERMIO
#include <sgtty.h>
#else
#include <termio.h>
#endif
#include <setjmp.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/dir.h>
#include <errno.h>
#include "config.h"		/* User-configurable stuff */
#ifdef PERMS
#include "permissions.h"
#endif

#define VERSION_ID "DRY 1.0"

#ifndef LOCK_EX
#define LOCK_EX         2	/* exclusive lock */
#define LOCK_UN         8	/* unlock */
#endif

/*#ifdef SYSV*/ /* - foo1 */
#undef CTRL			/* SVR4 CTRL macro is hokey */
#define CTRL(c) (c&037)	/* This gives ESIX a warning...ignore it! */
/*#endif*/

#ifdef XINU
extern int      errno;
#endif

#define YEA (1)			/* Booleans  (Yep, for true and false) */
#define NA  (0)

#define DOECHO (1)		/* Flags to getdata input function */
#define NOECHO (0)

//char           *
//strdup();			/* External function declarations */

extern FILE    *ufp;		/* External variable declarations */
extern long     ti;

#endif				/* BBSIRC */

#define STRLEN   80		/* Length of most string data */
#define IDLEN	 12		/* Length of userids */
#define PASSLEN  14		/* Length of encrypted passwd field */

#define PASSFILE ".PASSWDS"	/* Name of file User records stored in */
#define ULIST    ".UTMP"	/* Names of users currently on line */

#ifndef BBSIRC

#define FLUSH    ".PASSFLUSHFLAG"	/* Stores date for user cache flushing */
#define BOARDS   ".BOARDS"	/* File containing list of boards */
#define DIR      "/.DIR"	/* Name of Directory file info */
#define NAMEFILE "BoardName"	/* File containing site name of bbs */

#define QUIT 0x666		/* Return value to abort recursive functions */

#define FILE_READ  0x1		/* Ownership flags used in fileheader
				 * structure */
#define FILE_OWND  0x2		/* accessed array */
#define FILE_VISIT 0x4
#define FILE_MARKED 0x8

/*- #define FILE_ZAPPED 0x16	added by skji */

#define ZAPPED	0x1		/* For boards...tells if board is Zapped */

/* these are flags in userec.flags[0] */
#define PAGER_FLAG 0x1		/* true if pager was OFF last session */
#define CLOAK_FLAG 0x2		/* true if cloak was ON last session */
#define VOTE_FLAG 0x4		/* for controlling Votes */
#define SIG_FLAG 0x8		/* true if sig was turned OFF last session */
#define NOCLEAN_FLAG 0x10	/* true is user is immune from User Clean */

/* these are for the Zap users */
#define FILE_SPARE 0x8

#define SHIFTMODE(usernum,mode) ((usernum<MAXUSERS)?mode:mode<<4)

#define SETFILEMODE(array,usernum,mode) \
     (array[usernum%MAXUSERS] |= ((usernum<MAXUSERS)?mode:mode<<4))

#define CLRFILEMODE(array,usernum,mode) \
	  (array[usernum%MAXUSERS] &= ((usernum<MAXUSERS)?~mode:~(mode<<4)))

#define CHECKFILEMODE(array,usernum,mode) \
       (array[usernum%MAXUSERS] & ((usernum<MAXUSERS)?mode:mode<<4))
#define USERIDSIZE (16)
#define USERNAMESZ (24)
#define TERMTYPESZ (10)
/* END */
/*- chopin from this */
#define MAX_DIRNAME		512	/* 디렉토리명 최대 길이 */

#endif				/* BBSIRC */

/*
 * Note the protocol field is not inside an #ifdef FILES... this is a waste
 * but allows you to add/remove UL/DL support without rebuilding the PASSWDS
 * file (and it's only a lil ole int anyway).
 */

struct userec {			/* Structure used to hold information in */
	char            userid[IDLEN + 2];	/* PASSFILE */
	char            filler[34];
	char            lasthost[16];
	unsigned int    numlogins;
	unsigned int    numposts;
	char            flags[2];
	char            passwd[PASSLEN];
	char            username[STRLEN];
	char            termtype[STRLEN];
	unsigned        userlevel;
	time_t          lastlogin;
	int             protocol;
#ifdef REALINFO
	char            realname[STRLEN - 40];
	char            address[STRLEN];
#endif
	char            email[STRLEN];
};

struct user_info {		/* Structure used in UTMP file */
	int             active;	/* When allocated this field is true */
	int             uid;	/* Used to find user name entry in passwd
				 * file */
	int             pid;	/* kill() is used to notify user of talk
				 * request */
	int             invisible;	/* Used by cloaking function in Xyz
					 * menu */
	int             sockactive;	/* Used to coordinate talk requests */
	int             sockaddr;	/* ... */
	int             destuid;/* talk uses this to identify who called */
	int             mode;	/* UL/DL, Talk Mode, Chat Mode, ... */
	int             pager;	/* pager toggle, YEA, or NA          */
	int             in_chat;/* for in_chat commands   */
	int 		refuse_call; /* for refuse talk & fly */
	char            from[STRLEN];	/* machine name the user called in
					 * from */
	char            chatid[15];	/* chat id, if in chat mode */
};

#ifndef BBSIRC

struct commands {		/* Structure used to hold descriptions of */
	char           *cmdname;/* menu commands for docmd function.      */
	bbs_action      cmdfunc;	/* Usually this info is stored in an
					 * array */
	int             level;	/* that is initialized at compile time.   */
	char           *nextcmd;/* this field tells where to go after this
				 * one */
	char           *errorcmd;	/* this field tells where if error
					 * occurs      */
};

/*
 * 주의: 1. fileheader와 shortfile의 offset들은 모두 같은 순서로 같은 크기로 정의 되어 있어야 하고 새로운 항목을 추가할때도 둘�
 * shortfile은 fileheader의 accessed[MAXUSERS]를 한바이트로 줄여서 사용하기 위해 정의 된 것이므로 그 이외의 항목은 모�
 * !
 *
 * 2. fileheader의 구조는 실제 .BOARDS .DIR 화일에 저장되는 구조이므로 이를 바꿀때에는 화일에 저장된 내용도 모�
 * (될 수 있으면 안바꾸도록 해야함) dummy 필드를 이용해서 새 항목을 추가하는데 사용하고 그
 * ccessed[]도 offset이 밀리지 않도록 주의를 요함.
 */
struct fileheader {		/* This structure is used to hold data in */
	char            filename[STRLEN];	/* the BOARDS and DIR files               */
	char            owner[STRLEN];
	char            title[STRLEN];
	unsigned        level;
	unsigned char   tm_year;/* struct tm 구조멤버와 동일하게 이름붙였음 */
	unsigned char   tm_mon;
	unsigned char   tm_mday;
	/* chopin */
	unsigned char   isdirectory;	/* .BOARDS 파일에만 사용되는 플랙이고 */
	/* 보드이름이면 0 		 */
	/* 디렉토리 이름이면 1 	 */
	unsigned short int readcnt;	/* 조회수 */
	/* dummy */
	unsigned char   isNoname;
	unsigned char   dummy[273];
	unsigned char   accessed[MAXUSERS];
};

/*
 * 주의 : shortfile은 fileheader중 accessed만 다르고 나머지는 같아�
 * cture는 디스크의 정보를 bcache로 읽어 들일때 사용하므로 새 필드를 추가 하는
 * illcache()에서 채워 넣는 것을 추가 해야 함.
 */

struct shortfile {		/* used for caching files and boards */
	char            filename[STRLEN];	/* also will do for mail
						 * directories */
	char            owner[STRLEN];
	char            title[STRLEN];
	unsigned        level;
	unsigned char   tm_year;/* struct tm 구조멤버와 동일하게 이름붙였음 */
	unsigned char   tm_mon;
	unsigned char   tm_mday;
	/* chopin */
	unsigned char   isdirectory;	/* .BOARDS 파일에만 사용되는 플랙이고 */
	/* 보드이름이면 0            */
	/* 디렉토리 이름이면 1       */
	unsigned short int readcnt;	/* 조회수 */
	/* dummy */
	unsigned char   isNoname;
	unsigned char   dummy[217];
	unsigned char   accessed;	/* 이부분이 fileheader와 다름 */
};

struct one_key {		/* Used to pass commands to the readmenu */
	int             key;
	bbs_entry_action fptr;
};

#define MSGNUM 6	/* 화일 크기를 512byte로 제한하기 위해 6개로 설정 */
struct msgRec {		/* 쪽지 보내기 (Fly) 기능에 사용하는 레코드 */
	int index;
	char msg[MSGNUM][80];
	time_t time[MSGNUM];
}; /* sizeof msgRec = 508 */

#include "modes.h"		/* The list of valid user modes */

#define DONOTHING  0		/* read menu command return states */
#define FULLUPDATE 1		/* Entire screen was destroyed in this oper */
#define PARTUPDATE 2		/* Only the top three lines were destroyed */
#define DOQUIT     4		/* Exit read menu was executed */
#define NEWDIRECT  8		/* Directory has changed, re-read files */

/*
 * #define I_TIMEOUT   (-2)         #define I_OTHERDATA (-3)      wjlee
 */

#define I_TIMEOUT   (0x101)	/* Used for the getchar routine select call */
#define I_OTHERDATA (0x102)	/* interface */

#define SCREEN_SIZE (23)	/* Used by read menu  */

extern int      scrint;		/* Set when screen has been initialized */
/* Used by namecomplete *only* */

extern struct userec currentuser;	/* user structure is loaded from
					 * passwd */
/* file at logon, and remains for the   */
/* entire session */

extern struct user_info uinfo;	/* Ditto above...utmp entry is stored here
				 * and written back to the utmp file when
				 * necessary (pretty darn often). */
extern int      usernum;	/* Index into passwds file user record */
extern int      utmpent;	/* Index into this users utmp file entry */

extern int      t_lines, t_columns;	/* Screen size / width */
extern struct userec lookupuser;/* Used when searching for other user info */

extern char     currboard[STRLEN];	/* name of currently selected board */
extern int      selboard;	/* THis flag is true if above is active */

extern char     genbuf[1024];	/* generally used global buffer */

extern struct commands cmdlist[];	/* main menu command list */

extern jmp_buf  byebye;		/* Used for exception condition like I/O
				 * error */

extern struct commands xyzlist[];	/* These are command lists for all
					 * the */
extern struct commands talklist[];	/* sub-menus */
#ifdef FILES
extern struct commands filelist[];
#endif
extern struct commands maillist[];
extern struct commands dellist[];
extern struct commands maintlist[];
extern struct commands BBSlist[];
extern struct commands cabinetlist[];

extern char     save_title[];	/* These are used by the editor when
				 * inserting */
extern char     save_filename[];/* header information */
extern char     Cc_list[];	/* skji */
extern int      in_mail;
extern int      dumb_term;
extern char     BoardName[];

extern struct shortfile bcache[];
extern char     board_dir[];
extern char     board_home[];
#define PARENT_DIR_NAME ".."
#ifndef SYSV
#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)
#endif

#include "bbs_api.h"

#endif				/* !BBSIRC */

#define MaxCc	25		/* 동시에 받을 수 있는 최대 인원 */

#endif
