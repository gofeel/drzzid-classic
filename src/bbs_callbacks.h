/* Modified 2026-11-16. */
#ifndef BBS_CALLBACKS_H
#define BBS_CALLBACKS_H

#include <stdint.h>
struct fileheader;
struct shortfile;

typedef int (*bbs_action)(void);
typedef char *(*bbs_margin)(void);
typedef int (*bbs_record_visitor)(void *record);
typedef int (*bbs_record_match)(const void *record);
typedef void (*bbs_record_update)(void *record);
typedef int (*bbs_record_predicate)(intptr_t argument, const void *record);
typedef int (*bbs_board_visitor)(struct shortfile *board);
typedef int (*bbs_user_visitor)(char *userid, int index);
typedef int (*bbs_entry_action)(int index, struct fileheader *entry, char *directory);
typedef char *(*bbs_entry_formatter)(int index, struct fileheader *entry);
typedef int (*bbs_output_char)(int ch);
typedef int (*bbs_name_visitor)(char *name);

#endif
