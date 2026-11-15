/* Modified 2026-11-16. */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "bbs_api.h"
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

#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdio.h>
#include <crypt.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include <strings.h>
#include <ctype.h>

char * genpasswd(char * pw)
{
	char            saltc[3] = {0};
	long            salt;
	int             i, c;
	static char     pwbuf[14];

	if (strlen(pw) == 0)
		return "";
	time(&salt);
	salt = 9 * getpid();
#ifndef lint
	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
#endif
	for (i = 0; i < 2; i++) {
		c = saltc[i] + '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		saltc[i] = c;
	}
	strcpy(pwbuf, pw);
	return crypt(pwbuf, saltc);
}

int checkpasswd(char * passwd, char * test)
{
	static char     pwbuf[14];
	char           *pw;

	strncpy(pwbuf, test, 14);
	pw = crypt(pwbuf, passwd);
	return (!strcmp(pw, passwd));
}
