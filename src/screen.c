/* Modified 2026-11-16. */
#include <termcap.h>
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

#include "bbs.h"
#include "screen.h"
#include <stdarg.h>

extern char     clearbuf[];
extern char     cleolbuf[];
extern char     scrollrev[];
extern char     strtstandout[];
extern char     endstandout[];
extern int      clearbuflen;
extern int      cleolbuflen;
extern int      scrollrevlen;
extern int      strtstandoutlen;
extern int      endstandoutlen;

extern int      automargins;
extern int      dumb_term;

#define o_clear()     output(clearbuf,clearbuflen)
#define o_cleol()     output(cleolbuf,cleolbuflen)
#define o_scrollrev() output(scrollrev,scrollrevlen)
#define o_standup()   output(strtstandout,strtstandoutlen)
#define o_standdown() output(endstandout,endstandoutlen)

unsigned char   scr_lns, scr_cols;
unsigned char   cur_ln = 0, cur_col = 0;
unsigned char   roll;
int             scrollcnt;
unsigned char   docls;
unsigned char   downfrom;
int             standing = NA;


struct screenline *big_picture = NULL;

int init_screen(int slns, int scols)
{


	scr_lns = slns;
	scr_cols = MIN(scols, LINELEN);
	big_picture = (struct screenline *) calloc(scr_lns,
						 sizeof(struct screenline));
	for (slns = 0; slns < scr_lns; slns++) {
		big_picture[slns].mode = 0;
		big_picture[slns].len = 0;
		big_picture[slns].oldlen = 0;
	}
	docls = YEA;
	downfrom = 0;
	roll = 0;
	return 0;
}


int initscr(void)
{
	if (!dumb_term && !big_picture)
		init_screen(t_lines, t_columns);
	return 0;
}

int             tc_col, tc_line;

int rel_move(int was_col, int was_ln, int new_col, int new_ln)
{

	extern char    *BC;

	if (new_ln >= t_lines || new_col >= t_columns)
		return 0;
	tc_col = new_col;
	tc_line = new_ln;
	if ((new_col == 0) && (new_ln == was_ln + 1)) {
		ochar('\n');
		if (was_col != 0)
			ochar('\r');
		return 0;
	}
	if ((new_col == 0) && (new_ln == was_ln)) {
		if (was_col != 0)
			ochar('\r');
		return 0;
	}
	if (was_col == new_col && was_ln == new_ln)
		return 0;
	if (new_col == was_col - 1 && new_ln == was_ln) {
		if (BC)
			tputs(BC, 1, ochar);
		else
			ochar(CTRL('H'));
		return 0;
	}
	do_move(new_col, new_ln, ochar);
	return 0;
}


int standoutput(char *buf, int ds, int de, int sso, int eso)
{
	int             st_start, st_end;

	if (eso <= ds || sso >= de) {
		output(buf + ds, de - ds);
		return 0;
	}
	st_start = MAX(sso, ds);
	st_end = MIN(eso, de);
	if (sso > ds)
		output(buf + ds, sso - ds);
	o_standup();
	output(buf + st_start, st_end - st_start);
	o_standdown();
	if (de > eso)
		output(buf + eso, de - eso);
	return 0;
}

int redoscr(void)
{
	register int    i, j;

	register struct screenline *bp = big_picture;

	if (dumb_term)
		return 0;
	o_clear();
	tc_col = 0;
	tc_line = 0;
	for (i = 0; i < scr_lns; i++) {
		j = (i + roll) % scr_lns;
		if (bp[j].len == 0)
			continue;
		rel_move(tc_col, tc_line, 0, i);
		if (bp[j].mode & STANDOUT)
			standoutput(bp[j].data, 0, bp[j].len, bp[j].sso, bp[j].eso);
		else
			output(bp[j].data, bp[j].len);
		tc_col += bp[j].len;
		if (tc_col >= t_columns) {
			if (!automargins) {
				tc_col -= t_columns;
				tc_line++;
				if (tc_line >= t_lines)
					tc_line = t_lines - 1;
			} else
				tc_col = t_columns - 1;
		}
		bp[j].mode &= ~(MODIFIED);
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	docls = NA;
	scrollcnt = 0;
	oflush();
	return 0;
}

int refresh(void)
{
	register int    i, j;
	register struct screenline *bp = big_picture;
	extern int      automargins;
	extern int      scrollrevlen;

	if (dumb_term)
		return 0;
	if (num_in_buf() != 0)
		return 0;
	if ((docls) || (abs(scrollcnt) >= (scr_lns - 3))) {
		redoscr();
		return 0;
	}
	if (scrollcnt < 0) {
		if (!scrollrevlen) {
			redoscr();
			return 0;
		}
		rel_move(tc_col, tc_line, 0, 0);
		while (scrollcnt < 0) {
			o_scrollrev();
			scrollcnt++;
		}
	}
	if (scrollcnt > 0) {
		rel_move(tc_col, tc_line, 0, t_lines - 1);
		while (scrollcnt > 0) {
			ochar('\n');
			scrollcnt--;
		}
	}
	for (i = 0; i < scr_lns; i++) {
		j = (i + roll) % scr_lns;
		if (bp[j].mode & MODIFIED && bp[j].smod < bp[j].len) {
			bp[j].mode &= ~(MODIFIED);
			if (bp[j].emod >= bp[j].len)
				bp[j].emod = bp[j].len - 1;
			rel_move(tc_col, tc_line, bp[j].smod, i);
			if (bp[j].mode & STANDOUT)
				standoutput(bp[j].data, bp[j].smod, bp[j].emod + 1,
					    bp[j].sso, bp[j].eso);
			else
				output(&bp[j].data[bp[j].smod], bp[j].emod - bp[j].smod + 1);
			tc_col = bp[j].emod + 1;
			if (tc_col >= t_columns) {
				if (automargins) {
					tc_col -= t_columns;
					tc_line++;
					if (tc_line >= t_lines)
						tc_line = t_lines - 1;
				} else
					tc_col = t_columns - 1;
			}
		}
		if (bp[j].oldlen > bp[j].len) {
			rel_move(tc_col, tc_line, bp[j].len, i);
			o_cleol();
		}
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	oflush();
	return 0;
}

int clear(void)
{
	register int    i;

	if (dumb_term)
		return 0;
	roll = 0;
	docls = YEA;
	downfrom = 0;
	for (i = 0; i < scr_lns; i++) {
		big_picture[i].mode = 0;
		big_picture[i].len = 0;
		big_picture[i].oldlen = 0;
	}
	move(0, 0);
	return 0;
}

int clrtoeol(void)
{
	register struct screenline *slp;

	if (dumb_term)
		return 0;
	standing = NA;
	slp = &big_picture[((cur_ln + roll) % scr_lns)];
	if (cur_col <= slp->sso)
		slp->mode &= ~STANDOUT;
	if (cur_col > slp->oldlen) {
		register int    i;
		for (i = slp->len; i <= cur_col; i++)
			slp->data[i] = ' ';
	}
	slp->len = cur_col;
	return 0;
}

int clrtobot(void)
{
	register int    i;

	if (dumb_term)
		return 0;
	for (i = cur_ln; i < scr_lns; i++) {
		big_picture[(i + roll) % scr_lns].mode = 0;
		big_picture[(i + roll) % scr_lns].len = 0;
		if (big_picture[(i + roll) % scr_lns].oldlen > 0)
			big_picture[(i + roll) % scr_lns].oldlen = 255;
	}
	return 0;
}

int clrstandout(void)
{
	register int    i;
	if (dumb_term)
		return 0;
	for (i = 0; i < scr_lns; i++)
		big_picture[i].mode &= ~(STANDOUT);
	return 0;
}

static char     nullstr[] = "(null)";

int move(int y, int x)
{
	cur_col = x;
	cur_ln = y;
	return 0;
}

int getyx(int *y, int *x)
{
	*y = cur_ln;
	*x = cur_col;
	return 0;
}

int outc(unsigned char c)
{
	register struct screenline *slp;

#ifndef BIT8
	c &= 0x7f;
#endif
	if (dumb_term) {
#ifdef BIT8
		if (!isprint2(c)) {
#else
		if (!isprint(c)) {
#endif
			if (c == '\n') {
				ochar('\r');
				ochar('\n');
				return 0;
			}
			ochar('*');
			return 0;
		}
		ochar(c);
		return 0;
	}
	slp = &big_picture[((cur_ln + roll) % scr_lns)];
#ifdef BIT8
	if (!isprint2(c)) {	/*- deal with non-printables */
#else
	if (!isprint(c)) {	/*- deal with non-printables */
#endif
		if (c == '\n' || c == '\r') {	/*- do the newline thing */
			if (standing) {
				slp->eso = MAX(slp->eso, cur_col);
				standing = NA;
			}
			if (cur_col > slp->len) {
				register int    i;
				for (i = slp->len; i <= cur_col; i++)
					slp->data[i] = ' ';
			}
			slp->len = cur_col;
			cur_col = 0;
			if (cur_ln < scr_lns)
				cur_ln++;
			return 0;
		}
		c = '*';	/*- else substitute a '*' for non-printable */
	}
	if (cur_col >= slp->len) {
		register int    i;
		for (i = slp->len; i < cur_col; i++)
			slp->data[i] = ' ';
		slp->data[cur_col] = '\0';
		slp->len = cur_col + 1;
	}
	if (slp->data[cur_col] != c) {
		if ((slp->mode & MODIFIED) != MODIFIED)
			slp->smod = (slp->emod = cur_col);
		slp->mode |= MODIFIED;
		if (cur_col > slp->emod)
			slp->emod = cur_col;
		if (cur_col < slp->smod)
			slp->smod = cur_col;
	}
	slp->data[cur_col] = c;
	cur_col++;
	if (cur_col >= scr_cols) {
		if (standing && slp->mode & STANDOUT) {
			standing = NA;
			slp->eso = MAX(slp->eso, cur_col);
		}
		cur_col = 0;
		if (cur_ln < scr_lns)
			cur_ln++;
	}
	return 0;
}

int outs(char *str)
{
	while (*str != '\0')
		outc(*str++);
	return 0;
}

int outns(char *str, int n)
{
	for (; n > 0; n--)
		outc(*str++);
	return 0;
}


int             dec[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

void
prints(const char *fmt, ...)
{
    va_list ap, copy;
    char *buf;
    int len;
    va_start(ap, fmt);
    va_copy(copy, ap);
    len = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (len >= 0 && (buf = malloc((size_t)len + 1)) != NULL) {
        vsnprintf(buf, (size_t)len + 1, fmt, ap);
        outns(buf, len);
        free(buf);
    }
    va_end(ap);
}

int addch(int ch)
{
	outc(ch);
	return 0;
}

int scroll(void)
{
	if (dumb_term) {
		prints("\n");
		return 0;
	}
	scrollcnt++;
	roll = (roll + 1) % scr_lns;	/*- subtract one from roll mod scr_lns*/
	move(scr_lns - 1, 0);
	clrtoeol();
	return 0;
}

int rscroll(void)
{
	if (dumb_term) {
		prints("\n\n");
		return 0;
	}
	scrollcnt--;
	roll = (roll + (scr_lns - 1)) % scr_lns;
	move(0, 0);
	clrtoeol();
	return 0;
}

int standout(void)
{
	register struct screenline *slp;

	if (dumb_term || !strtstandoutlen)
		return 0;
	if (!standing) {
		slp = &big_picture[((cur_ln + roll) % scr_lns)];
		standing = YEA;
		slp->sso = cur_col;
		slp->eso = cur_col;
		slp->mode |= STANDOUT;
	}
	return 0;
}

int standend(void)
{
	register struct screenline *slp;

	if (dumb_term || !strtstandoutlen)
		return 0;
	if (standing) {
		slp = &big_picture[((cur_ln + roll) % scr_lns)];
		standing = NA;
		slp->eso = MAX(slp->eso, cur_col);
	}
	return 0;
}

#ifdef BIT8
int isprint2(int ch)
{

	return (ch & 0x80 ? 1 : isprint(ch));
/*- bugman : ANSI 코드를 사용할수 있도록 하려면 아래 코드를 사용할것..
	return ((ch & 0x80 ? 1 : isprint(ch)) || ch == '');
*/
}

int isalpha2(int ch)
{
	return (ch & 0x80 ? 1: isalpha(ch));
}

int isalnum2(int ch)
{
	return (ch & 0x80 ? 1: isalnum(ch));
}
#endif
