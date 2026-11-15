/* Modified 2026-11-16. */
/* 이것은 가주비 형이 만든 겁니다. */
/* 카이스트 전산과 시스렙          */
/* 1993년 4월 5일 식목일에         */
/* 나무를 심읍시다...............  */

#ifndef BBS_HANLIB_H
#define BBS_HANLIB_H

#ifdef __cplusplus
extern          "C" {
#endif

	void ks2tgstr   (unsigned char *, unsigned char *);
	void tg2ksstr   (unsigned char *, unsigned char *);
	void tg2instr   (unsigned char *, char *);
	void ks2instr   (unsigned char *, char *);
	void in2tgstr   (char *, unsigned char *);
	void in2ksstr   (char *, unsigned char *);
	char           *josa (char *, char *);

	void rev16      (char *, char *);
	int comp16      (char, char, char, char);
	int ustrcmp     (unsigned char *, unsigned char *);
	void show3      (char *);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
