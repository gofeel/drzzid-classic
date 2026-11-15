/* Modified 2026-11-16. */
#include <termcap.h>
/*    Pirate Bulletin Board System
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
#define LINUX 1

#include "bbs.h"

#if NO_TERMIO

struct sgttyb tty_state, tty_new ;

int get_tty(void)
{
    if(gtty(1,&tty_state) < 0) {
        return(-1) ;
    }
    return 1 ;
}

int init_tty(void)
{
    memmove(&tty_new, &tty_state, sizeof(tty_new)) ;
    tty_new.sg_flags |= RAW ;
    tty_new.sg_flags &= ~(TANDEM|CBREAK|LCASE|ECHO|CRMOD) ;
    stty(1,&tty_new) ;
}

int reset_tty(void)
{
    stty(1,&tty_state) ;
}

int restore_tty(void)
{
    stty(1,&tty_new) ;
}

#else /* ! NO_TERMIO */

struct termio tty_state, tty_new;

int get_tty(void)
{
    if(ioctl(1,TCGETA,&tty_state) < 0) {
        return -1;
    }
#if LINUX == 1 || SOLARIS == 1
    /* Hopefully temporary workaround for buggy login program. */
    tty_state.c_oflag |= OPOST;
    tty_state.c_iflag |= BRKINT;
    tty_state.c_lflag |= ISIG;
    tty_state.c_cc[VMIN] = 1;
#endif
    return 0;
}

int init_tty(void)
{
    memcpy(&tty_new, &tty_state, sizeof(tty_new));
    tty_new.c_iflag &= IXANY;
    tty_new.c_iflag &= ~(ISTRIP);
    tty_new.c_oflag &= ~(OPOST | OLCUC | OCRNL);
    tty_new.c_lflag &= ~(ICANON | ISIG | XCASE | ECHO);
    tty_new.c_lflag |= TOSTOP;
    tty_new.c_cflag &= ~(PARENB | PARODD);
    tty_new.c_cflag |= CS8;
    tty_new.c_cc[VMIN] = 1;
    tty_new.c_cc[VTIME] = 0;
    if (ioctl(1,TCSETA,&tty_new) == -1)
    	printf("ioctl failed\n");
	return 0;
}

int reset_tty(void)
{
    ioctl(1,TCSETA,&tty_state) ;
	return 0;
}

int restore_tty(void)
{
    ioctl(1,TCSETA,&tty_new) ;
	return 0;
}

#endif /* ! NO_TERMIO */

int term_ok(char * term)
{
   char tbuf[1024];
   if (tgetent(tbuf, term) != 1) return 0;
   return 1;
}

#define TERMCOMSIZE (256)

int dumb_term = 1 ;

char clearbuf[TERMCOMSIZE] ;
int clearbuflen ;

char cleolbuf[TERMCOMSIZE] ;
int cleolbuflen ;

char cursorm[TERMCOMSIZE] ;
char *cm ;

char scrollrev[TERMCOMSIZE] ;
int scrollrevlen ;

char strtstandout[TERMCOMSIZE] ;
int strtstandoutlen ;

char endstandout[TERMCOMSIZE] ;
int endstandoutlen ;

int t_lines = 24;
int t_columns = 80 ;

int automargins ;

char *outp ;
int  *outlp ;

int outcf(int ch)
{
    if(*outlp >= TERMCOMSIZE)
        return 0;
    (*outlp)++ ;
    *outp++ = ch ;
	return 0;
}

#include <sys/time.h>
#include <signal.h>

#define	TIMEOUT		30
#define	EMULATIONS	1

static char *getsize[EMULATIONS] =
	{
	"\0337\033[r\033[999;999H\033[6n",
	};

static char *size[EMULATIONS] = { "\033[%d;%dR" };
static char *restore[EMULATIONS] = { "\0338" };

static void generic_abort(int signal_number)
{
	abort_bbs(signal_number);
}

static int readstring(FILE * fp, char * buf, char * str)
{
    int last, c;
    struct itimerval it;

    signal(SIGALRM, generic_abort);
    memset((char *)&it, 0, sizeof(struct itimerval));
    it.it_value.tv_sec = TIMEOUT;
    setitimer(ITIMER_REAL, &it, (struct itimerval *)NULL);
    if ((c = getc(fp)) == 0233) {   /* meta-escape, CSI */
        *buf++ = c = '\033';
        *buf++ = '[';
    } else
        *buf++ = c;
    if(c != *str) {
			generic_abort(SIGALRM);
    }
    last = str[strlen(str) - 1];
    while((*buf++ = getc(fp)) != last) ;
    memset((char *)&it, 0, sizeof(struct itimerval));
    setitimer(ITIMER_REAL, &it, (struct itimerval *)NULL);
    *buf = 0;
	return 0;
}

int term_init(char * term)
{
    extern char PC, *UP, *BC ;
    extern short ospeed ;
    static char UPbuf[TERMCOMSIZE] ;
    static char BCbuf[TERMCOMSIZE] ;
    static char buf[4096] ;
    char sbuf[4096] ;
    char *sbp, *s ;

#if NO_TERMIO
    ospeed = tty_state.sg_ospeed ;
#else
    ospeed = tty_state.c_cflag & CBAUD ;
#endif
    if(tgetent(buf, term) != 1)
        return NA;
    sbp = sbuf ;
    s = tgetstr("pc", &sbp) ; /* get pad character */
    if(s)
        PC = *s ;

    if (strcmp(term, "xterm") == 0 ||
	strcmp(term, "vt220") == 0 ||
	strcmp(term, "vt200") == 0 ||
	strcmp(term, "vt102") == 0 ||
	strcmp(term, "iyagi") == 0 ||
	strcmp(term, "vt52") == 0) { /* let's try this */

#if LINUX == 1 || SOLARIS == 1
	struct winsize ws;
#else
	struct ttysize ts;
#endif
	int tty;
	int emu;
	int rows, cols;
	FILE *ttyfp;
	char buf[BUFSIZ];

	emu = 0;	/* currently only for xterm */
	ttyfp = fopen("/dev/tty", "r+");
	tty = fileno(ttyfp);
	write (tty, getsize[0], strlen(getsize[0]));
	readstring(ttyfp, buf, size[0]);
	if (strcmp(term, "iyagi") == 0)
		sscanf (buf, size[emu], &cols, &rows);
	else
		sscanf (buf, size[emu], &rows, &cols);
	write(tty, restore[emu], strlen(restore[emu]));
#if LINUX == 1 || SOLARIS == 1
	if (ioctl (tty, TIOCGWINSZ, &ws) != -1) {
	  ws.ws_row = rows;
	  ws.ws_col = cols;
	  ioctl (tty, TIOCSWINSZ, &ws);
	}
#else
	if (ioctl (tty, TIOCGSIZE, &ts) != -1) {
	  ts.ts_lines = rows;
	  ts.ts_cols = cols;
	  ioctl (tty, TIOCSSIZE, &ts);
	}
#endif
	t_lines = rows;
	t_columns = cols;
    } else {
      t_lines = tgetnum("li") ;
      t_columns = tgetnum("co") ;
    }
    automargins = tgetflag("am") ;
    outp = clearbuf ;         /* fill clearbuf with clear screen command */
    outlp = &clearbuflen ;
    clearbuflen = 0 ;
    sbp = sbuf ;
    s = tgetstr("cl", &sbp) ;
    if(s)
        tputs(s,t_lines, outcf) ;
    outp = cleolbuf ;         /* fill cleolbuf with clear to eol command */
    outlp = &cleolbuflen ;
    cleolbuflen = 0 ;
    sbp = sbuf ;
    s = tgetstr("ce",&sbp) ;
    if(s)
        tputs(s,1,outcf) ;
    outp = scrollrev ;
    outlp = &scrollrevlen ;
    scrollrevlen = 0 ;
    sbp = sbuf ;
    s = tgetstr("sr",&sbp) ;
    if(s)
        tputs(s,1,outcf) ;
    outp = strtstandout ;
    outlp = &strtstandoutlen ;
    strtstandoutlen = 0 ;
    sbp = sbuf ;
    s = tgetstr("so",&sbp) ;
    if(s)
        tputs(s,1,outcf) ;
    outp = endstandout ;
    outlp = &endstandoutlen ;
    endstandoutlen = 0 ;
    sbp = sbuf ;
    s = tgetstr("se",&sbp) ;
    if(s)
        tputs(s,1,outcf) ;
    sbp = cursorm ;
    cm = tgetstr("cm",&sbp) ;
    if(cm)
        dumb_term = 0 ;
    else
        dumb_term = 1 ;
    sbp = UPbuf ;
    UP = tgetstr("up",&sbp) ;
    sbp = BCbuf ;
    BC = tgetstr("bc",&sbp) ;
    if(dumb_term) {
        t_lines = 24 ;
        t_columns = 80 ;
    }
    return YEA;
}

int do_move(int destcol, int destline, bbs_output_char outc)
{
    tputs(tgoto(cm,destcol,destline), 1, outc) ;
	return 0;
}
