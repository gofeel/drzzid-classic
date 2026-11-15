/* Modified 2026-11-16. */
#include "bbs.h"
#include "modes.h"
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

/*
 * rrr - This is separated so I can suck it into the IRC source for use there
 * too
 */

char * ModeType(int mode)
{
	switch (mode) {
	/* 상태 설명은 한글로 5자까지만 해야 됨. 안그럼 Monitor할때 화면의 줄이 안맞음 */
	case IDLE:
		return NULL;
	case ULDL:
		return NULL;
	case TALK:
		return NULL;
	case NEW:
		return NULL;
	case CHAT4:
		return NULL;
	case CHAT1:
		return NULL;
	case CHAT2:
		return NULL;
	case CHAT3:
		return NULL;
	case LAUSERS:
		return NULL;
	case LUSERS:
		return NULL;
	case SMAIL:
		return NULL;
	case RMAIL:
		return NULL;
	case POSTING:
		return NULL;
	case MAIL:
		return NULL;
	case READNEW:
		return NULL;
	case MMENU:
		return NULL;
	case TMENU:
		return NULL;
	case XMENU:
		return NULL;
	case ADMIN:
		return NULL;
	case READING:
		return NULL;
	case PAGE:
		return NULL;
	case LOGIN:
		return NULL;
	case SHOW:
		return NULL;
	case SELECT:
		return NULL;
	case MONITOR:
		return NULL;
	case FLY:
		return NULL;
	case BACK:
		return NULL;
	case SHOUT:
		return NULL;
	case EDITWELC:
		return NULL;
	case ZAP:
		return NULL;
	case EDITSIG:
		return NULL;
	case EDITPLAN:
		return NULL;
	case QUERY:
		return NULL;
	case CNTBRDS:
		return NULL;
	case VOTING:
		return NULL;
	case VISIT:
		return NULL;
	case MEMO:
		return NULL;
	case OPEN:
		return NULL;
	case WAIT:
		return NULL;
	case IRCCHAT:
		return NULL;
	case BBSNET:
		/*
		 * chopin return "BBSNet";
		 */
		return NULL;
	case FOURM:
		return NULL;
	case NLP:
		return NULL;
	case BOARD_MENU:
		return NULL;
	case VIEWMEMO:
		return NULL;
	case QUERYUSERS:
		return NULL;
	case CABINET_MENU:
		return NULL;
	default:
		return NULL;
	}
	return NULL;
}
