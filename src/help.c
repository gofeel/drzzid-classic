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

#include "bbs.h"

#define HIGHLIGHT 1
#define LOWLIGHT  0

struct help_info {
	int             x, y;
	int             highlight;
	int             level;
	char           *text;
};

#define ycvStr "^^ribbon ( -_-) - The Dr.ZZID's world"

struct help_info RootMenuHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^처음마당",
	-1, -1, LOWLIGHT, 0, "(I)nfo          - 버젼과 저작권 정보\n",
	-1, -1, LOWLIGHT, PERM_ADMINMENU, "(A)dmin         - 관리자마당\n",
	-1, -1, LOWLIGHT, PERM_SPECIAL2,  "(L)ogin         - 다시 로그인 하기\n",
	-1, -1, LOWLIGHT, 0, "(B)BS           - 글상자마당\n",
	-1, -1, LOWLIGHT, PERM_POST, "(C)abinet       - 사물함 마당\n",
#ifdef FILES
	-1, -1, LOWLIGHT, PERM_POST, "(F)iles         - 파일 전송\n",
#endif
#ifdef BBSDOORS
	-1, -1, LOWLIGHT, PERM_POST, "(J)ourney       - 다른 곳으로 나들이 하기\n",
#endif
	-1, -1, LOWLIGHT, 0, "(M)ail          - 편 지 함\n",
	-1, -1, LOWLIGHT, PERM_BASIC, "(N)ew           - 새 글 보기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(P)ost          - 지금 글상자에 글 쓰기(Post)\n",
	-1, -1, LOWLIGHT, 0, "(R)ead          - 지금 글상자의 글 읽기(Read)\n",
	-1, -1, LOWLIGHT, 0, "(S)elect        - 글상자 고르기\n",
	-1, -1, LOWLIGHT, 0, "(T)alk          - 사 랑 방\n",
	-1, -1, LOWLIGHT, 0, "(X)yz           - 개인정보관리\n",
	-1, -1, LOWLIGHT, 0, "(G)oodbye       - BYEBYE^^;\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL,
};

struct help_info MailHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^편지함마당",
	-1, -1, LOWLIGHT, PERM_SENDMAIL, "(S)end          - 편지 보내기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(T)rans MyBox   - 사물함 내용 보내기\n",
	-1, -1, LOWLIGHT, PERM_READMAIL, "(R)ead          - 편지 읽기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(F)orward       - e-mail forwarding 설정\n",
	-1, -1, LOWLIGHT, 0, "(E)xit          - 편지함 떠나기\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};

#ifdef FILES
struct help_info FileHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^파일 전송마당",
	-1, -1, LOWLIGHT, 0, "(S)elect        - 파일 상자 고르기\n",
	-1, -1, LOWLIGHT, 0, "(L)ist          - 지금 파일 상자에 있는 파일 보기\n",
	-1, -1, LOWLIGHT, 0, "(P)rotocol      - 전송 프로토콜 고르기\n",
	-1, -1, LOWLIGHT, 0, "(D)ownload      - 파일 받기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(U)pload        - 파일 올리기\n",
	-1, -1, LOWLIGHT, 0, "(T)ext          - 텍스트 파일 보기\n",
	-1, -1, LOWLIGHT, 0, "(E)xit          - 파일 전송 떠나기\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};
#endif

struct help_info CabinetHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^사물함 마당",
	-1, -1, LOWLIGHT, PERM_POST, "(M)ybox Edit    - 사물함 고치기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(V)iew          - 사물함 내용 보기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(R)emove        - 사물함 지우기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(U)pload        - 사물함 내용 올리기 (Zmodem)\n",
	-1, -1, LOWLIGHT, PERM_POST, "(D)ownload      - 사물함 내용 받기 (Zmodem)\n",
	-1, -1, LOWLIGHT, 0, "(E)xit          - 사물함 마당 떠나기\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};

struct help_info XyzHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^개인정보마당",
	-1, -1, LOWLIGHT, 0, "(G)NU           - GENERAL PUBLIC LICENSE\n",
#ifdef MMMM
	-1, -1, LOWLIGHT, PERM_CHAT, "(4)m RC         Edit 4m Chat Config File\n",
#endif

	-1, -1, LOWLIGHT, PERM_POST, "(U)sers         - 모든 사용자 보기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(F)ind users    - 사용자 찾아보기\n",
	-1, -1, LOWLIGHT, 0, "(D)ate          - 시간 보기\n",
#ifdef REALINFO
	-1, -1, LOWLIGHT, PERM_BASIC, "(I)nfo          - 자기 정보 보기\n",
#endif
#ifdef INTERNET_EMAIL
	-1, -1, LOWLIGHT, PERM_SETADDR, "(A)ddress       - 이메일 주소 변경\n",
#endif
	-1, -1, LOWLIGHT, PERM_CHAT, "(N)ame          - 사용자 별명 변경\n",
	-1, -1, LOWLIGHT, PERM_CHAT, "(P)asswd        - 암호 변경\n",
	-1, -1, LOWLIGHT, PERM_CHAT, "(Q)ueryEdit     - 소개말 변경\n",
#ifdef VOTE
	-1, -1, LOWLIGHT, PERM_VOTE, "(R)esults       - 투표 결과 보기\n",
#endif
	-1, -1, LOWLIGHT, PERM_POST, "(M)emo          - 지난 메모보기\n",
	-1, -1, LOWLIGHT, PERM_CHAT, "(S)ignature     - 서명 변경\n",
	-1, -1, LOWLIGHT, 0, "(T)erminal      - 터미널 타입 변경\n",
#ifdef VOTE
	-1, -1, LOWLIGHT, PERM_VOTE, "(V)ote          - 투표장\n",
#endif

	/*-- moved from mainmenu by chopin*/
	-1, -1, LOWLIGHT, PERM_WELCOME, "(W)elcome       - 환영 인사 고치기\n",
	-1, -1, LOWLIGHT, PERM_BASIC, "(O)pen          - 공지 사항\n",
	-1, -1, LOWLIGHT, 0, "(X) Me          - 아이디 삭제하기\n",
	-1, -1, LOWLIGHT, 0, "(E)xit          - 떠나기\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};

struct help_info TalkHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^이야기마당",
	-1, -1, LOWLIGHT, 0, "(U)sers         - 지금 누구누구 놀구있나\n",
#ifdef REALINFO
	-1, -1, LOWLIGHT, PERM_CHAT, "(R)ealname      - 현재 유저 실명으로 보기\n",

#endif

	-1, -1, LOWLIGHT, 0,          "(L)ist          - 지금 있는 사람 요약해보기\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(M)onitor       - 사용자 구경하기\n",
	-1, -1, LOWLIGHT, 0,          "(Q)uery         - 사용자 궁금해\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(T)alk          - 1대1 대담\n",
	-1, -1, LOWLIGHT, PERM_BASIC, "(C)hat          - 왁자지껄\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(F)ly           - 쪽지 날려 보내기\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(B)ack          - 방금 전 쪽지보기\n",
	-1, -1, LOWLIGHT, PERM_CLOAK, "(S)hout         - 모든 사용자에게 소리치기\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(P)ager         - 삐삐 켜기/끄기\n",
        -1, -1, LOWLIGHT, PERM_PAGE,  "(A)ttach        - 메모 붙여놓기\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(O)verride      - 특정 사용자에게 삐삐 켜기\n",
	-1, -1, LOWLIGHT, PERM_PAGE,  "(I)MissYou      - 특정 사용자 기다리기\n",
	-1, -1, LOWLIGHT, PERM_CLOAK, "(W)earCloak     - 마법의 외투 입기/벗기\n",
#ifdef IRC
	-1, -1, LOWLIGHT, PERM_CHAT, "(I)RC           IRC 대화 하기\n",
#endif
#ifdef MMMM
	-1, -1, LOWLIGHT, PERM_CHAT, "(F)our M        Enter 4m Intercom Chat\n",
#endif
	-1, -1, LOWLIGHT, PERM_SYSOP, "(K)ick          - 사용자 추방\n",
	-1, -1, LOWLIGHT, 0, "(D)ate          - 지금 시간 보기\n",
	-1, -1, LOWLIGHT, 0, "(E)xit          - 이야기마당 떠나기\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};

struct help_info AdminHelp[] = {
	4, 0, HIGHLIGHT, 0, "^^관리자마당",
	-1, -1, LOWLIGHT, PERM_UCLEAN, "(U)ser Clean    - 30일 동안 안 쓴 사용자 지우기\n",
	-1, -1, LOWLIGHT, PERM_UCLEAN, "(X)empt         - 안 지울 사용자 표시\n",
	-1, -1, LOWLIGHT, PERM_SYSOP, "(L)evel         - 사용자 권한 설정\n",
	-1, -1, LOWLIGHT, PERM_SYSOP, "(S)hell         - UNIX 잠깐 쓰기\n",
	-1, -1, LOWLIGHT, PERM_ACCOUNTS, "(I)nfo          - 사용자 정보 알기/바꾸기\n",
	-1, -1, LOWLIGHT, PERM_ACCOUNTS, "(A)dd User      - 사용자 계정 주기\n",
	-1, -1, LOWLIGHT, PERM_ACCOUNTS, "(D)elete User   - 사용자 지우기\n",
	-1, -1, LOWLIGHT, PERM_BOARDS, "(N)ew Board     - 새로운 글상자 만들기\n",
	-1, -1, LOWLIGHT, PERM_BOARDS, "(B)oard Delete  - 글상자 지우기\n",
	-1, -1, LOWLIGHT, PERM_BOARDS, "(C)hange Board  - 글상자 정보 바꾸기\n",
	-1, -1, LOWLIGHT, PERM_SYSOP, "(M)ail Clean    - 오래되고 마크 안된 편지 지우기\n",
	-1, -1, LOWLIGHT, PERM_SYSOP, "(T)race         - 추적 옵션 설정\n",
#ifdef VOTE
	-1, -1, LOWLIGHT, PERM_OVOTE, "(V)ote          - 투표를 위한 투표함 열기\n",
#endif
	-1, -1, LOWLIGHT, 0, "(E)xit          - 관리자마당 떠나기\n",
	/*
	 * by bugman -1, -1, LOWLIGHT, 0, "(H)elp          도움 화면 보기\n",
	 */
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};

struct help_info BBShelp[] = {
	4, 0, HIGHLIGHT, 0, "^^글상자마당",
	-1, -1, LOWLIGHT, PERM_BASIC, "(N)ew           - 새 글 보기\n",
	-1, -1, LOWLIGHT, PERM_BASIC, "(B)oards        - 글상자 풀이말 보기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(P)ost          - 지금 글상자에 글 쓰기(Post)\n",
	-1, -1, LOWLIGHT, 0, "(R)ead          - 지금 글상자의 글 읽기(Read)\n",
	-1, -1, LOWLIGHT, 0, "(S)elect        - 글상자 고르기\n",
	-1, -1, LOWLIGHT, 0, "(C)ount         - 글상자에 올라간 글수 세 기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(Z)ap           - 안 볼 글상자 고르기\n",
	-1, -1, LOWLIGHT, PERM_POST, "(V)isit         - 새글 읽은 걸로 하기\n",
	-1, -1, LOWLIGHT, 0, "(E)xit          - 글상자마당 떠나기\n",
	-1, -1, HIGHLIGHT, 0, ycvStr,
	-1, -1, LOWLIGHT, 0, NULL
};

struct help_info MainRead[] = {
	/*-chopin
	    2, 0, HIGHLIGHT, 0, "움직임 명령어\n",
	*/
	0, 0, HIGHLIGHT, 0, "^^읽기마당 도움 화면",
	2, 0, HIGHLIGHT, 0, "움직임 명령어\n",
	-1, -1, LOWLIGHT, 0, "p,k,n,j      글 이동 (p/k : 이전글, n/j : 다음글)\n",
	-1, -1, LOWLIGHT, 0, "P,K,N,J      페이지 이동 (P/K : 이전 페이지, N/J : 다음 페이지)\n",
	-1, -1, LOWLIGHT, 0, "숫자 <cr>    지정한 번호의 글로 가기\n",
	-1, -1, LOWLIGHT, 0, "$            맨 마지막 글로 가기\n",
	-1, -1, HIGHLIGHT, 0, "그 외 명령어\n",
	/*- <cr> reding removed by chopin
	  Since,it makes unwilling reading by double return typing when exit reading

	    -1, -1,  LOWLIGHT,   0, "r,<sp>,<cr>  글 읽기\n",
	*/
	-1, -1, LOWLIGHT, 0, "r,<sp>,g     글 읽기 (g: 갈무리 모드 읽기)\n",
	-1, -1, LOWLIGHT, PERM_POST, "CTRL-P,w     글 올리기\n",
	-1, -1, LOWLIGHT, PERM_BASIC, "d            자기 글 지우기\n",
	-1, -1, LOWLIGHT, PERM_BOARDS, "D            범위 설정으로 글 많이 지우기\n",
	-1, -1, LOWLIGHT, PERM_MARKPOST, "m            글에 지우기 방지 표시\n",
	/*- chopin
	    -1, -1,  LOWLIGHT,   PERM_SYSOP, "E             지금 글 고침\n",
	*/
	-1, -1, LOWLIGHT, PERM_BASIC, "E            자기 글 고침\n",
	-1, -1, LOWLIGHT, 0, "s            새로운 글상자 고르기\n",
	-1, -1, LOWLIGHT, 0, "S            차례대로 안본 글 보기\n",
	/*- chopin */
	-1, -1, LOWLIGHT, 0, "V            글 읽은 걸로 만들기 \n",
	-1, -1, LOWLIGHT, 0, "v            글 안 읽은 걸로 만들기 \n",

#ifdef INTERNET_EMAIL
	-1, -1, LOWLIGHT, PERM_FORWARD, "F,CTRL-T     글 보내기(F : 전자우편 주소로, CTRL-T : 사물함으로)\n",
#endif
	-1, -1, LOWLIGHT, 0,            "CTRL-L       화면 다시 그리기\n",
	-1, -1, LOWLIGHT, 0, "/,?,.,<,>    찾기(\"/\" : 아래로 찾기, \"?\" : 위로 찾기, \".\" : 다시 찾기)\n",
	-1, -1, LOWLIGHT, 0, "ESC, q       찾기도중 취소하기\n",
	-1, -1, LOWLIGHT, 0, "CTRL-R       글 읽은 사람들 보기\n",
	-1, -1, LOWLIGHT, 0, "h            도움 화면 보기\n",
/*
	-1, -1, LOWLIGHT, 0, "h            도움 화면 보기\n",
	-1, -1, HIGHLIGHT, 0, "e            읽기마당 떠나기\n",
*/
	-1, -1, LOWLIGHT, 0, NULL
};

struct help_info MailRead[] = {
	0, 0, HIGHLIGHT, 0, "^^편지 읽기 도움 화면",
	2, 0, HIGHLIGHT, 0, "움직임 명령어\n",
	-1, -1, LOWLIGHT, 0, "p,k          이전 편지\n",
	-1, -1, LOWLIGHT, 0, "n,j          다음 편지\n",
	-1, -1, LOWLIGHT, 0, "P,K          이전 페이지\n",
	-1, -1, LOWLIGHT, 0, "N,J          전 페이지\n",
	-1, -1, LOWLIGHT, 0, "## <cr>      ##번 째 편지로 가기\n",
	-1, -1, LOWLIGHT, 0, "$            맨 마지막 편지로 가기\n",
	-1, -1, LOWLIGHT, 0, "\n",
	-1, -1, HIGHLIGHT, 0, "그 외 명령어\n",
	/*- <cr> reding removed by chopin
	  Since,it makes unwilling reading by double return typing when exit reading
	    -1, -1,  LOWLIGHT,   0, "r,<sp>,<cr>  편지 읽기\n",
	*/
	-1, -1, LOWLIGHT, 0, "r,<sp>,g     편지 읽기 (g: 갈무리 모드 읽기)\n",
	-1, -1, LOWLIGHT, 0, "R            답장 하기\n",
#ifdef INTERNET_EMAIL
/*
	-1, -1, LOWLIGHT, PERM_FORWARD, "F            전자 우편 주소로 글 보내기\n",
*/
	-1, -1, LOWLIGHT, PERM_FORWARD, "F,CTRL-T     글 보내기(F : 전자우편 주소로, CTRL-T : 사물함으로)\n",
#endif
	-1, -1, LOWLIGHT, 0, "d            지금 편지 지우기\n",
	-1, -1, LOWLIGHT, 0, "D            범위 안에 있는 편지 지우기\n",
	-1, -1, LOWLIGHT, 0, "m            지우기 방지를 위한 표시/표시지우기\n",
/*
	-1, -1, LOWLIGHT, 0, "z            Zmodem 으로 다운 받기\n",
*/
	-1, -1, LOWLIGHT, 0, "CTRL-L       화면 다시 그리기\n",
	-1, -1, LOWLIGHT, 0, "h            도움 화면 보기\n",
	-1, -1, LOWLIGHT, 0, "e            읽기마당 떠나기\n",
	-1, -1, LOWLIGHT, 0, NULL
};

/* by bugman ---------------> */
char * makeCenterStr(char * src)
{
	static char     buf[90];
	int    i;
	int             srcLen, startPos;
	if (src[0] != '^' || src[1] != '^')
		return src;
	for (i = 0; i < 78; i++)
		buf[i] = ' ';
	buf[i] = '\n';
	buf[i + 1] = '\0';
	srcLen = strlen(src) - 2;
	startPos = (78 - srcLen) / 2;
	for (i = 2; src[i]; i++) {
		buf[i + startPos] = src[i];
	}
	return buf;

}
/* <-------------- by bugman */
int do_the_help(struct help_info * p)
{
	extern int      msgLine;

	while (p->text) {
		if (p->x != -1) {
			move(p->x, p->y);
		}
#ifdef PERMS
		if (HAS_PERM(p->level))
#else
		if (currentuser.userlevel >= p->level)
#endif
			if (p->highlight) {
				standout();
				/*
				 * - by bugman if
				 * (p->text[strlen(p->text)-1]=='\n')
				 * p->text[strlen(p->text)-1] = 0;
				 */
				prints("%s", makeCenterStr(p->text));
				standend();
			} else
				prints("%s", p->text);
		p++;
	}
	msgLine = t_lines -1;
	return 0;
}

int Help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(RootMenuHelp);
	clrtobot();
	return 0;
}

int m_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(MailHelp);
	clrtobot();
	return 0;
}

#ifdef FILES
int f_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(FileHelp);
	clrtobot();
	return 0;
}
#endif

int x_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(XyzHelp);
	clrtobot();
	return 0;
}

int t_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(TalkHelp);
	clrtobot();
	return 0;
}

int a_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(AdminHelp);
	clrtobot();
	return 0;
}

int BBS_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(BBShelp);
	clrtobot();
	return 0;
}

int Cabinet_help(void)
{
	move(3, 0);
	clrtoeol();
	do_the_help(CabinetHelp);
	clrtobot();
	return 0;
}

int mainreadhelp(void)
{
	clear();
	do_the_help(MainRead);
	pressreturn();
	clear();
	return FULLUPDATE;
}

int mailreadhelp(void)
{
	clear();
	do_the_help(MailRead);
	pressreturn();
	clear();
	return FULLUPDATE;
}
