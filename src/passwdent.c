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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

int main(int ac, char ** av)
{
	struct passwd  *pw;

	if (ac != 3) {
		fprintf(stderr, "bad arg count\n");
		exit(-1);
	}
	pw = getpwnam(av[1]);
	if (!pw) {
		fprintf(stderr, "%s not in passwd file\n", av[1]);
		exit(-1);
	}
	if (!strcmp(av[2], "uid")) {
		printf("%d", pw->pw_uid);
		exit(0);
	}
	if (!strcmp(av[2], "gid")) {
		printf("%d", pw->pw_gid);
		exit(0);
	}
	if (!strcmp(av[2], "homedir")) {
		printf("%s", pw->pw_dir);
		exit(0);
	}
	if (!strcmp(av[2], "shell")) {
		printf("%s", pw->pw_shell);
		exit(0);
	}
	if (!strcmp(av[2], "check")) {
		exit(0);
	}
	fprintf(stderr, "second option unknown\n");
	fprintf(stderr, "try: uid, gid, homedir, or shell.\n");
	exit(-1);
	return 0;
}
