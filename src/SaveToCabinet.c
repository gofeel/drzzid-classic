/* Modified 2026-11-16. */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define BUFSZ 10240
#define true 1
#define false 0
int main(void)
{
	int ch;
	FILE *fp;
	char buf[512];
#ifdef RMCR
	int wasCR = false;
#endif
/*
	char abuf[BUFSZ];
*/

	sprintf(buf, "tmp/%s--", getenv("USER"));
	fp = fopen(buf, "wb");
        while ((ch = getc(stdin)) != EOF) {
#ifdef RMCR
                if (ch == 0x0d)
                        wasCR = true;
                else if (ch != 0x0a && wasCR)
                        putc(0x0d, fp);
                if (ch != 0x0d) {
                        wasCR = false;
                        putc(ch, fp);
                }
#else
		putc(ch, fp);
#endif
        }
/*
	while ( (ch = fread(abuf, 1, BUFSZ, stdin)) > 0){
		fwrite(abuf, 1, ch, fp);
	}
*/
        fclose(fp);
	return 0;
}
