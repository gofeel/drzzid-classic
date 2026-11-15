/* Modified 2026-11-16. */

/*
 * These are the 16 basic permission bits. All but the last one are used in
 * comm_lists.c and help.c to control user access to priviliged functions.
 * The symbolic names are given to roughly correspond to their actual usage;
 * feel free to use them for different purposes though. The PERM_SPECIAL1 and
 * PERM_SPECIAL2 are not used by default and you can use them to set up
 * restricted boards or chat rooms.
 */
 #ifndef _PERMISSIONS_H
 #define _PERMISSIONS_H

#define	PERM_BASIC	000001
#define PERM_CHAT	000002
#define	PERM_PAGE	000004
#define PERM_POST	000010
#define	PERM_SPECIAL1	000020
#define PERM_SPECIAL2	000040
#define PERM_CLOAK	000100
#define PERM_SEECLOAK	000200
#define PERM_UPLOAD	000400
#define PERM_WELCOME	001000
#define PERM_BOARDS	002000
#define PERM_ACCOUNTS	004000
#define PERM_CHATCLOAK	010000
#define	PERM_OVOTE	020000
#define PERM_SYSOP	040000
#define PERM_POSTMASK	0100000	/* means the rest is a post mask */

/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT 	(PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST)

#define PERM_POWEROVERWHELMING ( PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_SPECIAL1 | PERM_SPECIAL2 | PERM_CLOAK | PERM_SEECLOAK | PERM_UPLOAD | PERM_WELCOME | PERM_BOARDS | PERM_ACCOUNTS | PERM_CHATCLOAK | PERM_OVOTE | PERM_SYSOP )

/*
 * These permissions are bitwise ORs of the basic bits. They work that way
 * too. For example, anyone with PERM_SYSOP or PERM_BOARDS or both has
 * PERM_SEEBLEVELS.
 */

#define PERM_ADMINMENU	(PERM_ACCOUNTS | PERM_BOARDS | PERM_OVOTE | PERM_SYSOP)
#define PERM_MULTILOG	PERM_SYSOP
#define PERM_LOGINCLOAK	(PERM_SYSOP | PERM_ACCOUNTS | PERM_BOARDS | PERM_WELCOME)
#define PERM_SEEULEVELS PERM_SYSOP
#define PERM_SEEBLEVELS (PERM_SYSOP | PERM_BOARDS)
#define PERM_MARKPOST   (PERM_SYSOP | PERM_BOARDS)
#define PERM_UCLEAN	(PERM_SYSOP | PERM_ACCOUNTS)
#define PERM_NOTIMEOUT  (PERM_SYSOP | PERM_OVOTE)

#define PERM_SENDMAIL	PERM_BASIC
#define PERM_READMAIL	PERM_BASIC
#define PERM_VOTE	PERM_POST

#define PERM_NORMALUSER (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST)

/* These are used only in Internet Mail Forwarding */
/*
 * You may want to be more restrictive than the default, especially for an
 * open access BBS.
 */

#define PERM_SETADDR	PERM_BASIC	/* to set address for forwarding */
#define PERM_FORWARD	PERM_BASIC	/* to do the forwarding */

/* Don't mess with this. */
#define HAS_PERM(x)	((x)?currentuser.userlevel&(x):1)

#ifndef EXTERN
extern char    *permstrings[];
#else

#define NUMPERMS 15
/*
 * You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
 * depending on how/if you use them.
 */
char           *permstrings[] = {
	"기초 사용자",		/* PERM_BASIC */
	"채팅룸 이용 가능",	/* PERM_CHAT */
	"삐삐조작 가능",	/* PERM_PAGE */
	"포스팅 가능",		/* PERM_POST */
	"스페셜 1 (읽기 제한된 게시판 보기)",	/* PERM_SPECIAL1 */
	"스페셜 2 (Login 기능 사용 가능)",	/* PERM_SPECIAL2 */
	"망토입기 가능",	/* PERM_CLOAK */
	"망토입은 사람 볼 수 있기 가능",	/* PERM_SEECLOAK */
	"파일 업로드 가능",	/* PERM_UPLOAD */
	"환영인사 바꾸기 가능",	/* PERM_WELCOME */
	"게시판 관리자",	/* PERM_BOARDS */
	"계정 관리자",		/* PERM_ACCOUNTS */
	"대화방에서 망토입기 가능",	/* PERM_CHATCLOAK */
	"선거관리자",		/* PERM_VOTE */
	"대 SYSOP"	/* PERM_SYSOP */
};
#endif

#endif /* ifdef _PERMISSIONS_H */
