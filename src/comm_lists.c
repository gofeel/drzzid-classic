/* Modified 2026-11-16. */
/*--
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

	/* added by skji */

#ifdef FILES

#endif
/*-by chopin*/

struct commands cmdlist[] = {
	"Select", Select, 0, "Read", "Select",
	"BBS", BBSMenu, 0, "Select", "Boards",
	"Post", Post, PERM_POST, "Read", "Post",
	"Cabinet", CabinetMenu, PERM_POST, "Read", "Post",
	"Goodbye", Goodbye, 0, "Goodbye", "Goodbye",
	"Read", Read, 0, "Read", "Select",
	"New", New, PERM_BASIC, "Select", "Select",
	"Mail", Mail, 0, "Talk", "Mail",
#ifdef FILES
	"Files", Files, PERM_POST, "Files", "Files",
#endif
	"Talk", Talk, 0, "Talk", "Talk",
	"Xyz", Xyz, 0, "Xyz", "Xyz",
	"Admin", Maintenance, PERM_ADMINMENU, "Admin", "Admin",
	"Login", Login, PERM_SPECIAL2, "Goodbye", "Goodbye",
	"Info", Info, 0, "Info", "Info",
	"Journey", ent_bnet, PERM_POST, "Read", "Exit",
	NULL, NULL, 0, NULL, NULL};

struct commands maillist[] = {
	"Send", m_send, PERM_SENDMAIL, "Send", "Exit",
	"Trans Mybox", m_send_mybox, PERM_POST, "Exit", "Exit",
	"Read", m_read, PERM_READMAIL, "Exit", "Exit",
	"Forward", m_forward, PERM_POST, "Exit", "Exit",
	"Exit", d_exit, 0, "Exit", "Exit",
	NULL, NULL, 0, NULL, NULL};

#ifdef FILES

struct commands filelist[] = {
	"Select", f_select, 0, "List", "Exit",
	"List", f_list, 0, "Download", "Select",
	"Protocol", f_protocol, 0, "Download", "Select",
	"Download", f_download, 0, "Download", "Select",
	"Upload", f_upload, PERM_POST, "Exit", "Select",
	"Text", f_text, 0, "List", "Select",
	"Exit", d_exit, 0, "Exit", "Exit",
NULL, NULL, 0, NULL, NULL};
#endif

struct commands cabinetlist[] = {
	"Mybox Edit", c_edit, PERM_POST, "View", "Exit",
	"View", c_view, PERM_POST, "Exit", "Exit",
	"Remove", c_remove, PERM_POST, "Edit", "Exit",
	"Upload", c_upload, PERM_POST, "View", "Exit",
	"Download", c_download, PERM_POST, "Exit", "Exit",
	"Exit", d_exit, 0, "Exit", "Exit",
NULL, NULL, 0, NULL, NULL};

#ifdef REALINFO

#endif
#ifdef VOTE

#endif

#ifdef INTERNET_EMAIL

#endif
#ifdef BBSDOORS

#endif

struct commands xyzlist[] = {
	"Passwd", x_passwd, PERM_CHAT, "Passwd", "Exit",
	"Name", x_name, PERM_CHAT, "Users", "Exit",
#ifdef INTERNET_EMAIL
	"Address", x_setemail, PERM_SETADDR, "Exit", "Exit",
#endif
	"Users", Users, PERM_POST, "Users", "Users",
	"Find users", x_findusers, PERM_POST, "Users", "Users",
	"Terminal", x_terminal, 0, "Terminal", "Exit",
#ifdef REALINFO
	"Info", x_info, PERM_BASIC, "Exit", "Exit",
#endif
#ifdef VOTE
	"Vote", x_vote, PERM_VOTE, "Exit", "Exit",
	"Results", x_results, PERM_VOTE, "Exit", "Exit",
#endif
	"Welcome", EditWelcome, PERM_WELCOME, "Exit", "Exit",
	"Open", manageNotice, PERM_BASIC, "Open", "Exit",
	"Signature", x_editsig, PERM_CHAT, "Exit", "Exit",
	"QueryEdit", x_editplan, PERM_CHAT, "Exit", "Exit",
	"Date", x_date, 0, "Exit", "Exit",
	"X me", del_me, 0, "Exit", "Exit",
	"Gnu General Public License", Conditions, 0, "G", "G",
	"Memo", x_viewmemo, PERM_POST, "E", "E",
	"Exit", d_exit, 0, "Date", "Exit",
NULL, NULL, 0, NULL, NULL};

#ifdef IRC

#endif

#ifdef REALINFO

#endif

struct commands talklist[] = {
	"Users", t_users, 0, "Users", "Exit",
#ifdef REALINFO
	"Realname", t_rusers, PERM_CHAT, "Users", "Exit",
#endif
	"List", t_list, 0, "List", "Exit",
	"Monitor", t_monitor, PERM_PAGE, "Monitor", "Exit",
	"Query", t_query, 0, "Query", "Exit",
	"Talk", t_talk, PERM_PAGE, "Talk", "Exit",
	"Pager", t_pager, PERM_PAGE, "Users", "Exit",
	"Attach", t_attach, PERM_PAGE, "Users", "Exit",
	"Override", t_override, PERM_PAGE, "Users", "Exit",
	"IMissYou", t_wait_user, PERM_PAGE, "IMissYou", "IMissYou",
	"WearCloak", x_cloak, PERM_CLOAK, "Users", "Exit",
	"Chat", Zchat, PERM_BASIC, "Chat", "Chat",
#ifdef IRC
	"IRC", t_irc, PERM_CHAT, "IRC", "Exit",
#endif
	"Date", x_date, 0, "Query", "Exit",
	"Fly", t_fly, PERM_PAGE, "Fly", "Attach",
	"Back", t_back, PERM_PAGE, "Users", "Exit",
	"Shout", t_shout, PERM_CLOAK, "Users", "Exit",
	"Kick", kick_user, PERM_SYSOP, "Users", "Exit",
	"Exit", d_exit, 0, "Users", "Exit",
NULL, NULL, 0, NULL, NULL};

#ifdef VOTE

#endif

struct commands maintlist[] = {
	"User Clean", m_uclean, PERM_UCLEAN, "User Clean", "Exit",
	"Xempt", m_xempt, PERM_UCLEAN, "Xempt", "Exit",
	"Level", x_level, PERM_SYSOP, "Level", "Exit",
	"Shell", x_csh, PERM_SYSOP, "Exit", "Exit",
	"Info", m_info, PERM_ACCOUNTS, "Info", "Info",
	"Add User", m_adduser, PERM_ACCOUNTS, "Add User", "Exit",
	"Delete User", d_user, PERM_ACCOUNTS, "Delete", "Exit",
	"Mail Clean", m_mclean, PERM_SYSOP, "Exit", "Exit",
	"New Board", Create, PERM_BOARDS, "Exit", "Exit",
	"Board Delete", d_board, PERM_BOARDS, "Board", "Exit",
	"Change Board", m_editbrd, PERM_BOARDS, "Change Board", "Exit",
	"Trace", m_trace, PERM_SYSOP, "Trace", "Exit",
#ifdef VOTE
	"Vote", m_vote, PERM_OVOTE, "Exit", "Exit",
#endif
	"Exit", d_exit, 0, "Exit", "Exit",
NULL, NULL, 0, NULL, NULL};

struct commands BBSlist[] = {
	"Boards", Boards, 0, "Select", "Boards",
	"Select", Select, 0, "Read", "Boards",
	"New", New, PERM_BASIC, "Select", "Select",
	"Post", Post, PERM_POST, "Read", "Post",
	"Read", Read, PERM_BASIC, "Select", "Post",
	"Count", x_count, 0, "Exit", "Count",
	"Zap", Zap, PERM_POST, "Zap", "Zap",
	"Visit", Visit, PERM_POST, "Help", "Visit",
	"Exit", d_exit, 0, "Exit", "Exit",
NULL, NULL, 0, NULL, NULL};
