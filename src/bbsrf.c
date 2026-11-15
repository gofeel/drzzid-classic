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
#include <utmp.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

#include "readutmp.h"
#include <stdlib.h>
int getfromhost(char *buf, int size);
static const STRUCT_UTMP *search_entries(int n, const STRUCT_UTMP *utmp_buf, const char *line);

char           *env[10];

int main(void)
{
	int	uid;
	char	hostname[80],host[16];
	int		len;
	char buf[256];

	uid = getuid();
	if( uid != BBSUID ){
		printf("UID DOES NOT MATCH\n");
		exit(-1);
	}

	sprintf(buf, "BBSHOME=%s", BBSHOME);
	env[0] = buf;
	env[1] = NULL;

	len = getfromhost( hostname, 80 );
	if( len ){
		execle("bin/bbs", "bbs", "h", hostname, NULL, env);
	} else{
		execle("bin/bbs", "bbs",  "h", "몰라요", NULL, env);
	}
	printf("BBSHOME:%s\n", BBSHOME);
	printf("execl(%s); failed\n", "bin/bbs");
	exit(-1);
	return 0;
}

int getfromhost ( char* buf, int size)
{
  const STRUCT_UTMP *utmp_entry;
  STRUCT_UTMP *utmp_buf;
  char *tty;
  int fail;
  int n_users;

  tty = ttyname (0);
  if (tty == NULL)
    return 0;
  tty += 5;			/* Remove "/dev/".  */

  fail = read_utmp (UTMP_FILE, &n_users, &utmp_buf);
  if (fail)
    return 0;
  utmp_entry = search_entries (n_users, utmp_buf, tty);
  if (utmp_entry == NULL)
    return 0;
if( utmp_entry->ut_host[0] )
   strncpy( buf, utmp_entry->ut_host, size );
else
   sprintf( buf, "%d.%d.%d.%d", (utmp_entry->ut_addr&0xff),(utmp_entry->ut_addr&0xff00)>>8,(utmp_entry->ut_addr&0xff0000)>>16,(utmp_entry->ut_addr&0xff000000)>>24);
    buf[size-1]=0;

return strlen(buf);

}

static const STRUCT_UTMP *
search_entries (int n, const STRUCT_UTMP *utmp_buf, const char *line)
{
  while (n--)
    {
      if (utmp_buf->ut_name[0]
#ifdef USER_PROCESS
	  && utmp_buf->ut_type == USER_PROCESS
#endif
	  && !strncmp (line, utmp_buf->ut_line, sizeof (utmp_buf->ut_line)))
	return utmp_buf;
      utmp_buf++;
    }
  return NULL;
}
