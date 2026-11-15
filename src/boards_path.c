/* Modified 2026-11-16. */
#include <string.h>
#include "bbs.h"
/*-
char path_buf[MAX_DIRNAME];
char *board_path(file)
char *file;
{
	strcpy(path_buf,board_dir);
	if(file[0]!='/')
	    strcat(path_buf,"/");
	strcat(path_buf,file);
	return(path_buf);
}
*/

/*- 주의! 리턴 되는 스트링 포인터는 다른 함수들에의해 마구 변경되어짐
	리턴값은 반드시 curb_buf에 저장해서 전달해
*/

char            curb_buf[MAX_DIRNAME];
char * curbdir(const char *s)
{
	strcpy(curb_buf, board_dir);
	if (s[0] == 0)
		return (curb_buf);	/* curbdir("")의 경우 보드디렉토리 */
	/*
	 * strcpy(curb_buf,"boards");
	 * pushdir(curb_buf,&board_dir[strlen("boards")]);
	 * pushdir(curb_buf,s);
	 */
	pushdir(curb_buf, s);
	return (curb_buf);
}

/* + chopin  */
int popdir(char * dirstack, char * top)
{
	int             i;
	/* 한디렉토리 뺀 스트링을 구함 */
	if (dirstack[strlen(dirstack) - 1] == '/')
		dirstack[strlen(dirstack) - 1] = '\0';
	for (i = strlen(dirstack) - 1; i > 0; i--){
		if (dirstack[i] == '/')
			break;
		}

	dirstack[i] = 0;
	strncpy(top, &dirstack[i + 1], strlen(&dirstack[i + 1]));
	top[strlen(&dirstack[i + 1])] = 0;
	return 0;
}
int pushdir(char *dirstack, const char *s)
{
	if (s[0] != '/' && dirstack[strlen(dirstack) - 1] != '/')
		strcat(dirstack, "/");
	if (s[0] == '/' && dirstack[strlen(dirstack) - 1] == '/')
		dirstack[strlen(dirstack) - 1] = 0;
	strcat(dirstack, s);
	return 0;
}
/* 게시판 디렉토리 위치에서 "boards/"디렉토리를 뺀 나머지를 리턴 */
char * xboard_dir(const char *s)
{
	return (bfile(s) + strlen("boards/"));
}

/*
 * cboard_dir(s): 현재보드의 상대적 위치
 *
 * 각 디렉토리를 recursive하게 돌아다니며 적용되는 프로그램의 경우 상대적인 path를 이 함수에 의해 구할 수
 * file(s)에서 board_dir 만큼을 빼서 리턴함 extern char current_board[]; char
 * *cboard_dir(s) char *s; { return( (char *)(
 * bfile(s)+strlen(current_board)+1)); }
 */
