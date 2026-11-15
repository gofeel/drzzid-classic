/* Modified 2026-11-16. */
/*-    Pirate Bulletin Board System
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
#include <sys/param.h>
#include <stdlib.h>
//extern char    *getenv();

int pressanykey(void)
{
	char            buf[2];
	int		ch;
	move(t_lines - 1, 0);
	clrtoeol();
	prints("[아무글쇠나 누르세요]");
	ch = igetch();
	if (ch == CTRL('O')){
		do_fly_on_line( t_lines -1, YEA);
	}
	refresh();
	return 0;
}

int presskey(char * prompt)
{
	int             key;
	extern	int msgLine;
	move(msgLine = t_lines - 1, 0);
	clrtoeol();
	if (prompt)
		prints("%s", prompt);
	else
		prints("[아무글쇠나 누르세요]");
	key = igetch();
	if (key == CTRL('O')){
		do_fly_on_line( t_lines -1, YEA);
	}
	refresh();
	return key;
}

int pressreturn(void)
{
	pressanykey();
	return 0;		/*- skji */
	/*- skji
	    char buf[2] ;
	    getdata(t_lines-1,0,"[RETURN]을 누르면 계속됩니다.",buf,2,NOECHO,NULL) ;
	    move(t_lines-1,0) ;
	    clrtoeol() ;
	    refresh() ;
	    return 0 ;
	*/
}

int dumbreturn(void)
{
	char            ch;
	prints("[RETURN]을 누르면 계속됩니다.");
	while ((ch = igetch()) != EOF)
		if (ch == '\n' || ch == '\r')
			break;
	return 0;
}

int bell(void)
{
	fprintf(stderr, "%c", CTRL('G'));
	return 0;
}

int Getyn(char * s)
{
	CreateNameList();
	AddNameList("Yes");
	AddNameList("yes");
	AddNameList("No");
	AddNameList("no");
	namecomplete("(Y/N)[N] ? ", s);	/* [N]+ by 박재민 */
	return 0;
}

int touchnew(void)
{
	int fdd;
	fdd = open(FLUSH, O_WRONLY | O_CREAT, 0664);
	if (fdd == -1)
		return 0;
	write(fdd, "^^;\n", 22);
	close(fdd);
	return 0;
}


/*- rrr - Snagged from pbbs 1.8 */

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (40)
/*- chopin
#define BINDIR "/bin/"
*/
#define BINDIR ""

char           *bbsenv[MAXENVS];
int             numbbsenvs = 0;

/*- Case Independent strncmp */

int ci_strncmp(const char *s1, const char *s2, int n)
{
	for (; n; s1++, s2++, n--) {
		if (*s1 == '\0' && *s2 == '\0')
			break;
/* - bugman : 아이디 체크할 때 대소문자를 구별 않는 것을 없앰..
		if ((isalpha(*s1) ? *s1 | 0x20 : *s1) != (isalpha(*s2) ? *s2 | 0x20 : *s2))
*/
		if (*s1 != *s2)
			return YEA;
	}
	return NA;
}

int bbssetenv(char * env, char * val)
{
	int    i, len;

	if (numbbsenvs == 0)
		bbsenv[0] = NULL;
	len = strlen(env);
	for (i = 0; bbsenv[i]; i++)
		if (!ci_strncmp(env, bbsenv[i], len))
			break;
	if (i >= MAXENVS)
		return -1;
	if (bbsenv[i])
		free(bbsenv[i]);
	else
		bbsenv[++numbbsenvs] = NULL;
	bbsenv[i] = malloc(strlen(env) + strlen(val) + 2);
	strcpy(bbsenv[i], env);
	strcat(bbsenv[i], "=");
	strcat(bbsenv[i], val);
	return 0;
}

int do_exec(char * com, char * wd)
{
	char            sbuf[40];
	char            path[MAXPATHLEN];
	char            pcom[MAXCOMSZ];
	char           *arglist[MAXARGS];
	char           *tz;
	int    i, len;
	int    argptr;
	char  *lparse;
	int             status, pid, w;
	int             pmode;
	void (*isig)(int), (*qsig)(int);
	extern char home_dir[];

	strncpy(path, BINDIR, MAXPATHLEN);
	strncpy(pcom, com, MAXCOMSZ);
	len = MIN(strlen(com) + 1, MAXCOMSZ);
	pmode = LOOKFIRST;
	for (i = 0, argptr = 0; i < len; i++) {
		if (pcom[i] == '\0')
			break;
		if (pmode == QUOTEMODE) {
			if (pcom[i] == '\001') {
				pmode = LOOKFIRST;
				pcom[i] = '\0';
				continue;
			}
			continue;
		}
		if (pcom[i] == '\001') {
			pmode = QUOTEMODE;
			arglist[argptr++] = &pcom[i + 1];
			if (argptr + 1 == MAXARGS)
				break;
			continue;
		}
		if (pmode == LOOKFIRST) {
			if (pcom[i] != ' ') {
				arglist[argptr++] = &pcom[i];
				if (argptr + 1 == MAXARGS)
					break;
				pmode = LOOKLAST;
			} else
				continue;
 		}
		if (pcom[i] == ' ') {
			pmode = LOOKFIRST;
			pcom[i] = '\0';
		}
	}
	arglist[argptr] = NULL;
	if (argptr == 0)
		return -1;
	if (*arglist[0] == '/')
		strncpy(path, arglist[0], MAXPATHLEN);
	else
		strncat(path, arglist[0], MAXPATHLEN);
	reset_tty();
	alarm(0);
	if ((pid = vfork()) == 0) {
		char buf[10];
		if (wd)
			if (chdir(wd)) {
				fprintf(stderr, "Unable to chdir to '%s'\n", wd);
				exit(-1);
			}
		bbssetenv("BBSHOME", home_dir);
		bbssetenv("PATH", "/bin:.:./bin");
		bbssetenv("TERM", currentuser.termtype);
		bbssetenv("USER", currentuser.userid);
		bbssetenv("USERNAME", currentuser.username);
		sprintf(sbuf, "%d", utmpent);
		bbssetenv("UTMPENT", sbuf);
		sprintf(sbuf, "%d", MAX_SUIT_ACTIVE);
		bbssetenv("MAX_SUIT_ACTIVE", sbuf);
		bbssetenv("EMAILADDR", currentuser.email);
		if ((tz = getenv("TZ")) != NULL)
			bbssetenv("TZ", tz);
		if (numbbsenvs == 0)
			bbsenv[0] = NULL;
		execve(path, arglist, bbsenv);
		fprintf(stderr, "\nEXECV FAILED... path = '%s'\n", path);
		perror(path);
		igetch();	/*--chopin*/
		exit(-1);
	}
	isig = signal(SIGINT, SIG_IGN);
	qsig = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != 1)
		 /*- NULL STATEMENT */ ;
	signal(SIGINT, isig);
	signal(SIGQUIT, qsig);
	restore_tty();
#ifdef DOTIMEOUT
	alarm(IDLE_TIMEOUT);
#endif
	return ((w == -1) ? w : status);
}
