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

#ifndef _MODES_H
#define _MODES_H

/* Lots o' modes! */

#define IDLE    0		/* Modes a user can be in */
#define ULDL    1		/* see mode in struct user_info in bbs.h */
#define TALK    2
#define NEW     3
#define CHAT1   4
#define READNEW 5
#define POSTING 6
#define MAIL    7
#define CHAT2   8
#define CHAT4   9
#define CHAT3   10
#define LAUSERS 11
#define LUSERS  12
#define SMAIL   13
#define RMAIL   14
#define MMENU	15
#define TMENU	16
#define XMENU	17
#define READING	18
#define PAGE	19
#define ADMIN	20
#define SHOW	21
#define SELECT	22
#define LOGIN	23
#define MONITOR 24
#define EDITWELC 25
#define ZAP	26
#define EDITSIG	27
#define EDITPLAN 28
#define QUERY	29
#define CNTBRDS	30
#define VOTING	31
#define VISIT	32
#define IRCCHAT	33
#define BBSNET	34
#define FOURM	35
#define NLP     36		/*- by chopin */
#define BOARD_MENU   37
#define FLY   38
#define BACK 39
#define SHOUT 40
#define MEMO 41
#define OPEN 42
#define VIEWMEMO 43
#define QUERYUSERS 44
#define WAIT 45
#define CABINET_MENU 46

#endif
