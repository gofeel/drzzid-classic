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
 #ifndef BBS_CHAT_H
 #define BBS_CHAT_H

/* chat.h - port numbers for the chat rooms -- changing them is wise. */
#define CHATPORT1 7942
#define CHATPORT2 4004
#define CHATPORT3 8282
#define CHATPORT4 8255

/* + by belami */
#define CHATROOM_CNT 4

extern char     chatroom_title[CHATROOM_CNT][100];

#define CHATROOM_TITLE 	".CHATROOM_TITLE"

int change_chatroom_title(int nth, char *title);

#endif
