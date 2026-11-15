/* Modified 2026-11-16. */
#include "bbs.h"
/*- 지순기 */

extern int      selboard;
extern int      talkrequest;
extern char    *page_requestor;

static char     buf[100];
int print_current_boardname_NOT_USED(void)
{
	return 0;
	sprintf(buf, "%s", (char *) boardmargin());
	move(0, 79 - strlen(buf) - 5);
	clrtoeol();
	prints(buf);
}

int print_newmail_message(void)
{
	int             newmail;
	/*
	 * - by bugman newmail = chkmail(); sprintf(buf, "%s", newmail ?
	 * "[편지왔어요!!]" : "[새편지없음]"); move(0, 80-strlen(buf)-5); if
	 * (newmail) standout(); prints("%s", buf); if (newmail) standend();
	 */
	newmail = chkmail();
	move(0, 80 - 15 - 2);
	if (newmail) {
		standout();
		prints("[편지 왔어요!!]");
		standend();
	} else {
		prints("               ");
	}
	return 0;
}

int print_talkreq_message_NOT_USED(void)
{
	return 0;
	if (talkrequest)
		sprintf(buf, "Talk:%-9s", page_requestor);
	else
		sprintf(buf, "-------");
	if (talkrequest)
		standout();
	move(0, 65);
	prints("%s", buf);
	if (talkrequest)
		standend();
	return 0;
}
