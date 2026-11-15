/* Modified 2026-11-16. */
#include <stdlib.h>
#include <stdio.h>
/* #include <stdlib.h> */
#include <string.h>
#include "hanlib.h"

extern unsigned short KStbl[2350];
extern unsigned short Single[51];

static int codecmp(const void *left, const void *right)
{
    const unsigned short *p = left;
    const unsigned short *q = right;
	return (*p - *q);
}

static unsigned short TG2KS(unsigned short code)
{
	short           index;
	unsigned short *p;

	if ((p = (unsigned short *) bsearch((char *) &code, (char *) KStbl,
				     sizeof(KStbl) / sizeof(unsigned short),
				sizeof(unsigned short), codecmp)) != NULL) {
		index = p - KStbl;
		return (((index / 94) + 0xb0) << 8) + (index % 94) + 0xa1;
	}
	for (index = 0; index < 51; index++) {
		if (Single[index] == code)
			return (0xa4a1 + index);
	}
	return 0;
}

static unsigned short KS2TG(unsigned short code)
{
	int             index;
	unsigned short  c1, c2;

	c1 = code >> 8;
	c2 = code & 0x00ff;
	if (c1 == 0xa4)
		return (c2 >= 0xa1 && c2 <= 0xd3) ? Single[c2 - 0xa1] : 0;
	else if (c1 < 0xb0 || c1 > 0xc8 || c2 < 0xa1 || c2 > 0xfe)
		return 0;

	index = ((code >> 8) - 0xb0) * 94 + (code & 0x00ff) - 0xa1;
	return KStbl[index];
}

void ks2tgstr(unsigned char * s, unsigned char * d)
{
	unsigned short  c, c2, t;
	while (c = (unsigned short) (*s++)) {
		if (c >= 0x80 && (c2 = (unsigned short) (*s++))) {
			t = KS2TG((c << 8) + c2);
			*d++ = (unsigned char) (t >> 8);
			*d++ = (unsigned char) (t & 0xff);
		} else
			*d++ = (unsigned char) c;
	}
	*d = 0;
}

void tg2ksstr(unsigned char * s, unsigned char * d)
{
	unsigned short  c, c2, t;
	while (c = (unsigned short) (*s++)) {
		if (c >= 0x80 && (c2 = (unsigned short) (*s++))) {
			t = TG2KS((c << 8) + c2);
			*d++ = (unsigned char) (t >> 8);
			*d++ = (unsigned char) (t & 0xff);
		} else
			*d++ = (unsigned char) c;
	}
	*d = 0;
}

void rev16(char * s, char * t)
{
	int             i, len = strlen(s);
	if (len % 2) {
		fprintf(stderr, "rev16: invalid input\n");
		strcpy(t, "");
		return;
	}
	for (i = len - 2; i >= 0; i -= 2) {
		*t = s[i];
		*(t + 1) = s[i + 1];
		t += 2;
	}
	*t = 0;
}

int comp16(char a1, char a2, char b1, char b2)
{
	short  f, m, l;
	f = (short) ((a1 & 0x7f) >> 2) - (short) ((b1 & 0x7f) >> 2);
	m = (short) ((a1 & 0x03) << 3) + (short) ((a2 & 0xe0) >> 5)
		- (short) ((b1 & 0x03) << 3) - (short) ((b2 & 0xe0) >> 5);
	l = (short) (a2 & 0x1f) - (short) (b2 & 0x1f);
	return f ? f : (m ? m : l);
}

int ustrcmp(unsigned char * s, unsigned char * t)
{
	while (*s == *t) {
		if (*s == 0)
			return 0;
		s++;
		t++;
	}
	return *s - *t;
}

void tg2instr(unsigned char * s, char * t)
{
	int             len = strlen(s);
	if (len == 0) {
		*t = 0;
		return;
	}
	while (*s) {
		*t++ = (*s & 0x7f) >> 2;
		*t = ((*s++) & 0x03) << 3;
		*t++ += (*s & 0xe0) >> 5;
		*t++ = (*s++) & 0x1f;
	}
	*t = 0;
}

void ks2instr(unsigned char * s, char * t)
{
	int             len = strlen(s);
	unsigned char  *tg;
	if (len == 0) {
		*t = 0;
		return;
	}
	tg = (unsigned char *) malloc(len + 1);
	ks2tgstr(s, tg);
	tg2instr(tg, t);
}

void in2ksstr(char * s, unsigned char * t)
{
	unsigned char   tg[80];
	in2tgstr(s, tg);
	tg2ksstr(tg, t);
}

void in2tgstr(char * s, unsigned char * t)
{
	int             len = strlen(s);

	if (len % 3) {
		*t = 0;
		return;
	}
	while (*s) {
		*t = ((*s++) << 2) | 0x80;
		*t++ |= (*s & 0x18) >> 3;
		*t = ((*s++) & 0x07) << 5;
		*t++ |= *s++;
	}
	*t = 0;
}
char           *list[] = {
"은", "는", "이", "가", "을", "를", "으로", "로", "과", "와"};
char * josa(char * src, char * what)
{
	char            tmps[100];
	int             widx = -1;
	if (strcmp(what, "은") == 0)
		widx = 0;
	else if (strcmp(what, "는") == 0)
		widx = 0;
	else if (strcmp(what, "이") == 0)
		widx = 2;
	else if (strcmp(what, "가") == 0)
		widx = 2;
	else if (strcmp(what, "을") == 0)
		widx = 4;
	else if (strcmp(what, "를") == 0)
		widx = 4;
	else if (strcmp(what, "으로") == 0)
		widx = 6;
	else if (strcmp(what, "로") == 0)
		widx = 6;
	else if (strcmp(what, "과") == 0)
		widx = 8;
	else if (strcmp(what, "와") == 0)
		widx = 8;

	ks2tgstr(src, tmps);
	if (widx == -1)
		return what;
	else if (widx == 6 && (tmps[strlen(tmps) - 1] & 0x1f) == 9)
		return list[widx + 1];
	else if ((tmps[strlen(tmps) - 1] & 0x1f) == 1)
		return list[widx + 1];
	else
		return list[widx];
}

/* EOF  */
