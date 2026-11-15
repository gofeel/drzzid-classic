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
#include <string.h>
#include "bbs.h"

struct word {
	char           *str;
	struct word    *next;
};

/*
int gputs(char *s)
{
	FILE *fp=fopen("/tmp/alog", "a");
	fprintf(fp, "%s\n", s);
	fclose(fp);
}

int gprintf(char *s, int i)
{
	FILE *fp=fopen("/tmp/alog", "a");
	fprintf(fp, s, i);
	fclose(fp);
}

int gprintf2(char *s, int i, int j)
{
	FILE *fp=fopen("/tmp/alog", "a");
	fprintf(fp, s, i, j);
	fclose(fp);
}
*/
struct word    *toplev = NULL, *current = NULL;
extern int      numboards;
int FreeNameList(void)
{
	struct word    *p, *tmp;

	for (p = toplev; p; ) {
	/*
		gprintf2("Sub Node %d ready to free. (Next = %d)\n", p, p->next);
		*/
		tmp = p->next;
		free(p->str);
		free(p);
		p = tmp;
	}
	/*
	gputs("Top Level Freed");
	*/
	return 0;
}

int CreateNameList(void)
{
	if (toplev)
		FreeNameList();
	toplev = NULL;
	current = NULL;
	return 0;
}

int AddNameList(char * name)
{

	struct word    *node;

	node = (struct word *) malloc(sizeof(struct word));
	node->next = NULL;
	node->str = (char *) malloc(strlen(name) + 1);
	strcpy(node->str, name);
	if (toplev) {
	/*
		gprintf2("Sub Node %d Allocated (Parent %d)\n", node, current);
		*/
		current->next = node;
		current = node;
		return 0;
	}
	toplev = node;
	current = node;
	/*
	gprintf("Top Level Node %d Allocated\n", toplev);
	*/
	return 0;
}

int NumInList(struct word * list)
{
	int    i;

	for (i = 0; list != NULL; i++, list = list->next)
		 /* Null Statement */ ;
	return i;
}

int ApplyToNameList(bbs_name_visitor fptr)
{
	struct word    *p;

	for (p = toplev; p != NULL; p = p->next)
		(*fptr) (p->str);
	return 0;
}

int chkstr(char * tag, char * name, int ignCase)
{
	char           *otag = tag;
	char           *oname = name;
	while (*tag != '\0') {
		if (!ignCase) {
			if (*tag != *name)
				return 0;
		} else {
			if (toupper(*tag) != toupper(*name))
				return 0;
		}
		tag++;
		name++;
	}

	if (*name == '\0')
		strcpy(otag, oname);

	return 1;
}

struct word * GetSubList(char * tag, struct word * list, int ignorecase)
{
	struct word    *wlist, *wcurr;

	wlist = NULL;
	wcurr = NULL;
	while (list != NULL) {
		if (tag == NULL || chkstr(tag, list->str, ignorecase)) {
			struct word *node;

			node = (struct word *) malloc(sizeof(struct word));
			node->str = list->str;
			node->next = NULL;
			if (wlist)
				wcurr->next = node;
			else
				wlist = node;
			wcurr = node;
		}
		list = list->next;
	}
	/*
	gprintf("SubList %d allocated\n", wlist);
	*/
	return wlist;
}

int ClearSubList(struct word * list)
{
	struct word *tmp;
	/*
	gprintf("SubList %d deallocated\n", list);
	*/
	while (list) {
		tmp = list->next;
		free(list);
		list = tmp;
	}
	return 0;
}

int MaxLen(struct word * list, int count)
{
	int             len = strlen(list->str);

	while (list != NULL && count) {
		int             t = strlen(list->str);
		if (t > len)
			len = t;
		list = list->next;
		count--;
	}
	return len;
}

/*
 * chopin get_common_prefix(wordlist): wordlist에 있는 스트링중에서 공통으�
 * tring sequence의 길이를 리�
 *
 */
int get_common_prefix(struct word * list)
{
	struct word    *p;
	int             prefix, i;

	prefix = 99999;
	for (p = list->next; p != NULL; p = p->next)
		if (prefix > (i = match_len(p->str, list->str)))
			prefix = i;
	return prefix;
}
/*
 * chopin match_len(s1,s2) s1과 s2의 maximum prefix의 길이를 반�
 */
int match_len(char * s1, char * s2)
{
	int             i;
	for (i = 0; i <= strlen(s1) && s1[i] == s2[i]; i++);
	return i;
}

#define NUMLINES (19)
/*
 * chopin namecomplete():bcache[]의 보드 번호를 리턴. 0:맞는 보드가
 * int namecomplete(char *프롬프트,char *이름) (      input   ,  output  )
 *
 * 리턴값: bcache[]내에 읽어 들인 위치(index)+1 0일때 : 해당되는 값이 없음.
 *
 *
 * 보드이름을 선택하는 루틴을 짤때: make_blist(); // 또는   make_child_blist();
 * // bnum = namecomplete((char *) NULL, bname); if(bnum==0) printf("잘못된 게시�
 * );
 *
 *
 * 유저 이름을 선택하는 루틴을 짤때: u_namelist(); namecomplete(NULL, uident);
 * if(!getuser(uident)) printf("아이디가 이상합니다 \n");
 *
 * 아니면 if( (unum=namecomplete(NULL,uident)) ==0 ) printf("
 * n"); 해보지 않아서 모르겠는데..이렇게 해도 될지 모르겠군요.
 * e()함수는 return 값이 없었는데 chopin이 만들었음.
 */
int _namecomplete(char * prompt, char * data, int ignCase)
{
/*	char		dummy[2]="";*/
	char           *temp;
	int             ch;
	int             count = 0;
	int             clearbot = NA;
	int             i;
	if (scrint) {
		struct word    *cwlist, *morelist;
		int             x, y;
		int             origx, origy;
		if (prompt != NULL) {
			prints("%s", prompt);
			clrtoeol();
		}
		temp = data;

		if (toplev == NULL)
			AddNameList("");
		cwlist = GetSubList(NULL,  toplev, ignCase);
		morelist = NULL;
		getyx(&y, &x);
		getyx(&origy, &origx);
		while ((ch = igetch()) != EOF) {
			if (ch == '\n' || ch == '\r') {
				*temp = '\0';
				prints("\n");
				/*
				 * + chopin 보드가 하나있을때는 .. 보드뿐. 이때는 그냥 리턴하면 안됨
				 */
				/*
				 * if (count==0) { ClearSubList(cwlist);
				 * return 0;// .. 디렉토리의 bcache[]
				 * / }
				 */
				if (NumInList(cwlist) == 1)
					strcpy(data, cwlist->str);
				ClearSubList(cwlist);
				break;
			}
			if (ch == ' ') {
				int             col, len;
				char            prefix[255];
				/* + chopin */
				if (NumInList(cwlist) == 1) {
					strcpy(data, cwlist->str);
					move(y, x);
					prints("%s", data + count);
					count = strlen(data);
					temp = data + count;
					getyx(&y, &x);
					continue;
				}
				clearbot = YEA;
				col = 0;
				if (!morelist)
					morelist = cwlist;
				len = MaxLen(morelist, NUMLINES);
				move(3, 0);
				clrtobot();
				standout();
				prints(
				       "------------------------------- 아래를 참고하세요------------------------------");
				standend();
				while (len + col < 80) {
					int             i;
					int             k;
					for (i = NUMLINES; (morelist) && (i > 0);
					   i--, morelist = morelist->next) {
						move(4 + (NUMLINES - i), col);
						prints("%s", morelist->str);

						/* + chopin 디렉토리이면 "/"를 추가 */
						/*
						 *
						 * if(is_dir(getbnum(morelist->
						 * word))) prints("/");
						 */
						/*- chopin
						for(k=0;k<numboards;k++)
							if(bcache[k].isdirectory)
							    if(!strcmp(bcache[k].filename,morelist->str))
								{
								    prints("/");
								    break;
								}
						*/
					}
					col += len + 2;
					if (!morelist)
						break;
					len = MaxLen(morelist, NUMLINES);
				}
				if (morelist) {
					move(23, 0);
					standout();
					prints("-- 계속 --");
					standend();
				} {
					/*
					 * chopin 현재 선택된 보드들 중  가장
					 * refix를 찾아서  이것이 data
					 * x로 하는 더 큰 문자열이면 data로 넣음
					 */
					int             i, j;

					if ((i = get_common_prefix(cwlist)) > 0 && i > strlen(data)) {
						j = strlen(data);
						strncpy(data, cwlist->str, i);
						data[i] = '\0';
						move(y, x);
						prints("%s", &data[j]);
						count = strlen(data);
						temp = data + count;
						getyx(&y, &x);
						continue;

					}
				}

				move(y, x);
				continue;
			}
			if (ch == '\177' || ch == '\010') {

				if (temp == data)
					continue;
				temp--;
				count--;
				*temp = '\0';
				ClearSubList(cwlist);
				cwlist = GetSubList(data, toplev, ignCase);
				morelist = NULL;
				x--;
				move(y, x);
				addch(' ');
				move(y, x);
				continue;
			}
			if (count < STRLEN) {
				struct word    *node;

				*temp++ = ch;
				count++;
				*temp = '\0';
				node = GetSubList(data, cwlist, ignCase);
				if (node == NULL) {
					bell();
					temp--;
					*temp = '\0';
					count--;
					continue;
				}
				ClearSubList(cwlist);
				cwlist = node;
				morelist = NULL;
				move(y, x);
				addch(ch);
				x++;
			}
		}
		if (ch == EOF)
			longjmp(byebye, -1);
		prints("\n");
		refresh();
		if (clearbot) {
			move(3, 0);
			clrtobot();
		}
		/*- chopin
			if (*data) {
			    move(origy, origx);
			    prints("%s\n", data);
			    /* for (x=1; x<500; x++);  delay *
			}
		*/
		/* 디렉토리를 선택할때 마지막의 '/'문자를 입력한 경우 없앰 */
		/*
		 * if(strlen(data)>1 && data[strlen(data)-1]=='/')
		 * data[strlen(data)-1]='\0';
		 */

		/* chopin  bcache[]의보드 번호를 리턴 */
		return (getbnum(data));
	}
	if (prompt != NULL) {
		printf("%s", prompt);
		fflush(stdout);
	}
	if (!fgets(data, STRLEN, stdin))
		longjmp(byebye, -1);
	if (temp = strchr(data, '\n'))
		*temp = '\0';
	return 0;
}
int namecomplete(char * prompt, char * data)
{
	return _namecomplete(prompt, data, 0);
}
int namecomplete_ignorecase(char * prompt, char * data)
{
	return _namecomplete(prompt, data, 1);
}
