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
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/file.h>
#ifndef XINU
#include <sys/stat.h>
#endif

#ifdef BIT8
#define BUFSIZE 8192
#else
#define BUFSIZE 4096
#endif

#ifdef SYSV
#include <sys/ipc.h>
#include <sys/sem.h>
#define SEM_KEY	9236
int             semid;

int get_semaphore(void)
{
	if ((semid = semget(SEM_KEY, 1, 600)) == -1) {
		if ((semid = semget(SEM_KEY, 1, 600 | IPC_CREAT)) == -1)
			return -1;
		semctl(semid, 0, SETVAL, 1);
	}
	return 0;
}

int flock(int fd, int op)
{
	/*- one semaphore for all shared files -> fd is unused */
	struct sembuf   sops;
	sops.sem_num = 0;
	sops.sem_flg = SEM_UNDO;
	switch (op) {
	case LOCK_EX:
		sops.sem_op = -1;
		break;
	case LOCK_UN:
		sops.sem_op = 1;
		break;
	default:
		return -1;
	}
	semop(semid, &sops, 1);
	return 0;
}

#endif

int safewrite(int fd, const void * buf, int size)
{
	int             cc, sz = size, origsz = size;
	const char     *bp = buf;
	/*- #ifdef POSTBUG */
	if (size == sizeof(struct fileheader)) {
		char            foo[80];
		struct stat     stbuf;
		const struct fileheader *fbuf = buf;
		/*
		 * chopin sprintf(foo, "boards/%s", fbuf->filename);
		 */
		sprintf(foo, "%s", bfile(fbuf->filename));
		if (!isalnum2(fbuf->filename[0]) || stat(foo, &stbuf) == -1)
			if (fbuf->filename[0] != 'M' || fbuf->filename[1] != '.') {
				report("safewrite: foiled attempt to write bugged record\n");
				igetch();
				return origsz;
			}
	}
	/*- #endif */
	do {
		cc = write(fd, bp, sz);
		if ((cc < 0) && (errno != EINTR)) {
			report("safewrite failed!");
			igetch();
			return -1;
		}
		if (cc > 0) {
			bp += cc;
			sz -= cc;
		}
	}
	while (sz > 0);
	return origsz;
}

#ifdef POSTBUG

char            bigbuf[5120];
int             numtowrite;
int             bug_possible = 0;

int saverecords(char *filename, int size, int pos)
{
	int             fd;
	if (!bug_possible)
		return 0;
	if ((fd = open(filename, O_RDONLY)) == -1)
		return -1;
	if (pos > 5)
		numtowrite = 5;
	else
		numtowrite = 4;
	lseek(fd, (pos - numtowrite - 1) * size, L_SET);
	read(fd, bigbuf, numtowrite * size);
}

int restorerecords(char *filename, int size, int pos)
{
	int             fd;
	if (!bug_possible)
		return 0;
	if ((fd = open(filename, O_WRONLY)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	lseek(fd, (pos - numtowrite - 1) * size, L_SET);
	safewrite(fd, bigbuf, numtowrite * size);
	report("post bug poison set out!");
	flock(fd, LOCK_UN);
	bigbuf[0] = '\0';
	close(fd);
}

#endif

long get_num_records(char * filename, int size)
{
	struct stat     st;

	if (stat(filename, &st) == -1)
		return 0;
	return (st.st_size / size);
}

int append_record(char * filename, const void * record, int size)
{
	int             fd;
#ifdef POSTBUG
	int             numrecs = (int) get_num_records(filename, size);
	bug_possible = 1;
	if (size == sizeof(struct fileheader) && numrecs && (numrecs % 4 == 0))
		saverecords(filename, size, numrecs + 1);
#endif
	/*
	 * + chopin prints("file:%s,size:%d\n",filename,size);
	 */
	if ((fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0664)) == -1) {
		perror("open");
		igetch();
		return -1;
	}
	flock(fd, LOCK_EX);
	if (safewrite(fd, record, size) == -1) {
		report("append record failed (safewrite)!!!!");
		igetch();
	}
	flock(fd, LOCK_UN);
	close(fd);
#ifdef POSTBUG
	if (size == sizeof(struct fileheader) && numrecs && (numrecs % 4 == 0))
		restorerecords(filename, size, numrecs + 1);
	bug_possible = 0;
#endif
	return 0;
}

_Alignas(max_align_t) char gb[BUFSIZE];

int apply_record(char * filename, bbs_record_visitor fptr, int size)
{
	int             fd;

	if (size > BUFSIZE) {
		fprintf(stderr, "size to big in apply record\n");
		return -1;
	}
	if ((fd = open(filename, O_RDONLY, 0)) == -1) {
		return -1;
	}
	while (read(fd, gb, size) == size) {
		if ((*fptr) (gb) == QUIT) {
			close(fd);
			return QUIT;
		}
	}
	close(fd);
	return 0;
}

int search_record(char * filename, void * rptr, int size, bbs_record_predicate fptr, intptr_t farg)
{
	int             fd;
	int             id = 1;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return 0;
	while (read(fd, rptr, size) == size) {
		if ((*fptr) (farg, rptr)) {
			close(fd);
			return id;
		}
		id++;
	}
	close(fd);
	return 0;
}

int get_record(char * filename, void * rptr, int size, int id)
{
	int             fd;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	if (lseek(fd, size * (id - 1), L_SET) == -1) {
		close(fd);
		return -1;
	}
	if (read(fd, rptr, size) != size) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int get_records(char * filename, void * rptr, int size, int id, int number)
{
	int             fd;
	int             n;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	if (lseek(fd, size * (id - 1), L_SET) == -1) {
		close(fd);
		return 0;
	}
	if ((n = read(fd, rptr, size * number)) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return (n / size);
}

int substitute_record(char * filename, void * rptr, int size, int id)
{
	int             fd;
#ifdef POSTBUG
	if (size == sizeof(struct fileheader) && (id > 1) && ((id - 1) % 4 == 0))
		saverecords(filename, size, id);
#endif
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0664)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	if (lseek(fd, size * (id - 1), L_SET) == -1)
		report("substitute_record failed!!! (lseek)");
	if (safewrite(fd, rptr, size) != size)
		report("substitute_record failed!!! (safewrite)");
	flock(fd, LOCK_UN);
	close(fd);
#ifdef POSTBUG
	if (size == sizeof(struct fileheader) && (id > 1) && ((id - 1) % 4 == 0))
		restorerecords(filename, size, id);
#endif
	return 0;
}

char * slashToUnderscore(char * s)
{
	static char retbuf[100];
	char *p=&(retbuf[0]);
	for(;*s; s++, p++){
		if (*s != '/') {
			*p = *s;
		} else {
			*p = '_';
		}
	}
	*p = '\0';
	return retbuf;
}

#ifdef _DEBUG__
#define REP prints("%d ", __LINE__), refresh()
#else
#define REP NULL
#endif
int delete_records(char * filename, int size, int * ids, int idcnt)
{
	int             fdr, fdw;
	int             fd;
	int             count;
	char 	tmpfile[50];
	char 	dellock[150];
	char 	deleted[150];

	if (strstr(filename, "boards/") == NULL){
		sprintf(tmpfile, "tmp/.tmpfile.%d", getpid());
		sprintf(deleted, "tmp/.deleted.mail");
	} else {
		sprintf(tmpfile, "boards/tmp/.tmpfile.%d", getpid());
		sprintf(deleted, "boards/tmp/.deleted.%s", slashToUnderscore(filename));
	}
	sprintf(dellock, "tmp/.dellock.of.%s", slashToUnderscore(filename));

	if ((fd = open(dellock, O_RDWR | O_CREAT | O_APPEND, 0664)) == -1)
		return -1;
	flock(fd, LOCK_EX);

	if ((fdr = open(filename, O_RDONLY, 0)) == -1) {
		report("delete_record failed!!! (open)");
		flock(fd, LOCK_UN);
		close(fd);
		unlink(dellock);
		return -1;
	}
	if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0664)) == -1) {
		flock(fd, LOCK_UN);
		report("delete_record failed!!! (open tmpfile)");
		close(fd);
		unlink(dellock);
		close(fdr);
		return -1;
	}
	count = 1;
	while (read(fdr, gb, size) == size){
		int tosave, i;

#ifdef _DEBUG__
		prints("count=%d ", count);
		refresh();
#endif
		for ( i = 0, tosave = YEA; i< idcnt; i++){
			if (ids[i] == count){
				tosave = NA;
				break;
			}
		}
		count ++;
		if ( !tosave ) continue;
		if ( safewrite(fdw, gb, size) == -1) {
			unlink(tmpfile);
			close(fdr);
			close(fdw);
			report("delete_record failed!!! (safewrite)");
			flock(fd, LOCK_UN);
			close(fd);
			unlink(dellock);
			return -1;
		}
	}
	close(fdr);
	close(fdw);
	unlink(deleted);
	if (rename(filename, deleted) == -1) {
		flock(fd, LOCK_UN);
		report("delete_record failed!!! (rename)");
		close(fd);
		unlink(dellock);
		return -1;
	}
	if (rename(tmpfile, filename) == -1) {
		flock(fd, LOCK_UN);
		report("delete_record failed!!! (rename)");
		close(fd);
		unlink(dellock);
		return -1;
	}
	flock(fd, LOCK_UN);
	close(fd);
	unlink(dellock);
	return 0;
}

int delete_record(char * filename, int size, int id)
{
	delete_records(filename, size, &id, 1);
	return 0;
}

int delete_range(char * filename, int id1, int id2)
{
	int             fdr, fdw;
	int             fd;
	int             count;
	struct fileheader fhdr;
	char 	tmpfile[50];
	char 	dellock[150];
	char 	deleted[150];

	if (strstr(filename, "boards/") == NULL){
		sprintf(tmpfile, "tmp/.tmpfile.%d", getpid());
		sprintf(deleted, "tmp/.deleted.%s", slashToUnderscore(filename));
	} else {
		sprintf(tmpfile, "boards/tmp/.tmpfile.%d", getpid());
		/*
		sprintf(deleted, "boards/tmp/.deleted.%s", slashToUnderscore(filename));
		*/
		sprintf(deleted, "boards/tmp/.deleted.mail");
	}
	sprintf(dellock, "tmp/.del_lock.%s", slashToUnderscore(filename));

	if ((fd = open(dellock, O_RDWR | O_CREAT | O_APPEND, 0664)) == -1)
		return -1;
	flock(fd, LOCK_EX);

	if ((fdr = open(filename, O_RDONLY, 0)) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		unlink(dellock);
		return -1;
	}
	if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0664)) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		unlink(dellock);
		close(fdr);
		return -1;
	}
	count = 1;
	while (read(fdr, &fhdr, sizeof fhdr) == sizeof fhdr) {
		if (count < id1 || count > id2 || fhdr.accessed[0] & FILE_MARKED) {
			if ((safewrite(fdw, &fhdr, sizeof fhdr) == -1)) {
				unlink(tmpfile);
				close(fdr);
				close(fdw);
				flock(fd, LOCK_UN);
				close(fd);
				unlink(dellock);
				return -1;
			}
		} else {
			char           *t;
			char            buf[256];
			strcpy(buf, filename);
			if (t == strrchr(buf, '/'))
				*t = '\0';
			sprintf(gb, "%s/%s", buf, fhdr.filename);
			unlink(gb);
		}
		count++;
	}

	close(fdr);
	close(fdw);

	if (rename(filename, deleted) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		unlink(dellock);
		return -1;
	}
	if (rename(tmpfile, filename) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		unlink(dellock);
		return -1;
	}
	flock(fd, LOCK_UN);
	close(fd);
	unlink(dellock);
	return 0;
}

_Alignas(max_align_t) char abuf[4096];

int update_file(char * dirname, int size, int ent, bbs_record_match filecheck, bbs_record_update fileupdate)
{
	int             fd;

	if ((fd = open(dirname, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	if (lseek(fd, size * (ent - 1), L_SET) != -1) {
		if (read(fd, abuf, size) == size)
			if ((*filecheck) (abuf)) {
				lseek(fd, -size, L_INCR);
				(*fileupdate) (abuf);
				if (safewrite(fd, abuf, size) != size) {
					report("update_file failed!!! (safewrite)");
					flock(fd, LOCK_UN);
					close(fd);
					return -1;
				}
				flock(fd, LOCK_UN);
				close(fd);
				return 0;
			}
	}
	lseek(fd, 0, L_SET);
	while (read(fd, abuf, size) == size) {
		if ((*filecheck) (abuf)) {
			lseek(fd, -size, L_INCR);
			(*fileupdate) (abuf);
			if (safewrite(fd, abuf, size) != size) {
				report("update_file failed!!! (safewrite)");
				flock(fd, LOCK_UN);
				close(fd);
				return -1;
			}
			flock(fd, LOCK_UN);
			close(fd);
			return 0;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
	return -1;
}

int delete_file(char * dirname, int size, int ent, bbs_record_match filecheck)
{
	int             fd;
	struct stat     st;
	long            numents;

	if ((fd = open(dirname, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	fstat(fd, &st);
	numents = ((long) st.st_size) / size;
	if (((long) st.st_size) % size != 0)
		fprintf(stderr, "Irregular File Size, Truncating\n");
	if (lseek(fd, size * (ent - 1), L_SET) != -1) {
		if (read(fd, abuf, size) == size)
			if ((*filecheck) (abuf)) {
				int             i;
				for (i = ent; i < numents; i++) {
					if (lseek(fd, (i) * size, L_SET) == -1)
						break;
					if (read(fd, abuf, size) != size)
						break;
					if (lseek(fd, (i - 1) * size, L_SET) == -1)
						break;
					if (safewrite(fd, abuf, size) != size)
						break;
				}
				ftruncate(fd, size * (numents - 1));
				flock(fd, LOCK_UN);
				close(fd);
				return 0;
			}
	}
	lseek(fd, 0, L_SET);
	ent = 1;
	while (read(fd, abuf, size) == size) {
		if ((*filecheck) (abuf)) {
			int             i;
			for (i = ent; i < numents; i++) {
				if (lseek(fd, (i + 1) * size, L_SET) == -1)
					break;
				if (read(fd, abuf, size) != size)
					break;
				if (lseek(fd, (i) * size, L_SET) == -1)
					break;
				if (safewrite(fd, abuf, size) != size)
					break;
			}
			ftruncate(fd, size * (numents - 1));
			flock(fd, LOCK_UN);
			close(fd);
			return 0;
		}
		ent++;
	}
	flock(fd, LOCK_UN);
	close(fd);
	return -1;
}
