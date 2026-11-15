/* Modified 2026-11-16. */
#ifndef BBS_API_H
#define BBS_API_H

/* Internal Linux BBS interfaces. Keep declarations in sync with their modules. */
#include <stdio.h>
#include "bbs_callbacks.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <time.h>

struct commands;
struct fileheader;
struct help_info;
struct keeploc;
struct one_key;
struct shortfile;
struct textline;
struct user_info;
struct userec;
struct word;

/* bbs.c */
int exit_warning(void);
int BBSMenu(void);
int CabinetMenu(void);
int Maintenance(void);
int Xyz(void);
int Talk(void);
int Files(void);
int Mail(void);
char uleveltochar(unsigned int lvl);
char bleveltochar(unsigned int lvl);
char * Ctime(time_t * clock);
int printuent(void *record_data);
int countusers(void *record_data);
int Users(void);
int Create(void);
int filepr(struct shortfile * fhdrp);
int getstats(char * fname, int * total, int * owned, int * visited, int * unread, char * lastpost);
int update_postcount2(struct shortfile * fhdrp);
int update_postcount(void);
int postcount(struct shortfile * fhdrp);
int Boards(void);
int x_count(void);
char * bfile(const char *fname);
int g_board_names(struct shortfile * fhdrp);
int make_blist(void);
int make_child_blist(void);
char * room_title(int nth_minus_one);
int Zchat(void);
int Select(void);
int receiveFile(char * fn);
int Post(void);
int readtitle(void);
char * readdoent(int num, struct fileheader * ent);
int cmpfilename(const void *record_data);
void mark_flag_as_read(void *record_data);
void mark_flag_as_visit(void *record_data);
int visit_on_readboard(int ent, struct fileheader * fileinfo, char * dirent);
int mark_as_read_on_readboard(int ent, struct fileheader * fileinfo, char * dirent);
int read_post(int ent, struct fileheader * fileinfo, char * dirent);
int read_post_galmuri(int ent, struct fileheader * fileinfo, char * dirent);
int do_select(void);
int do_reply(char * oldtitle, char * oldfilename);
int copy_article_to_reply(char * src, char * dest);
char * make_reply_title(char * title);
int do_post(char * oldfilename);
int Reply(int ent, struct fileheader * fileinfo, char * dirent);
int split_header_and_body(char * fname);
int merge_header_and_new_body(char * fname);
int edit_only_body_of_article(char * infname);
int transferToMybox(int ent, struct fileheader * fileinfo, char * dirent);
int transferToMyboxOnBoard(int ent, struct fileheader * fileinfo, char * dirent);
int edit_post(int ent, struct fileheader * fileinfo, char * dirent);
int mark_post(int ent, struct fileheader * fileinfo, char * dirent);
int del_range(int ent, struct fileheader * fileinfo, char * dirent);
int do_reader_list(int ent, struct fileheader * fileinfo, char * dirent);
int del_post(int ent, struct fileheader * fileinfo, char * dirent);
int sequent_messages(void *record_data);
int sequential_read(int ent, struct fileheader * fileinfo, char * dirent);
int forward_post(int ent, struct fileheader * fileinfo, char * dirent);
int Read(void);
int Login(void);
int Goodbye(void);
void usieslog(char * action, char * id, char * name, char * from);
int report(char * s);
int get_messages(void *record_data);
int get_boards(struct fileheader * fptr);
int get_users(void *record_data);
int UserStatus(void);
int g_messages(void *record_data);
int g_boards(struct shortfile * fptr);
int BoardStatus(void);
int Info(void);
int Conditions(void);
int Welcome(void);
int EditWelcome(void);
int cmpbnames(intptr_t n, const void *record_data);
int Zap(void);
char * time_string_no_newline(time_t t);
char * date_string_no_newline(time_t t);
char * time_string_short_no_newline(time_t t);
char * time_string_short(time_t t);
char * time_string(time_t t);
char * owner_string(void);

/* bcache.c */
int fillcache(void *record_data);
int apply_childboards(bbs_board_visitor func);
int nop(struct shortfile *board);
int apply_child_dir(bbs_board_visitor func);
int apply_childboards2(bbs_board_visitor func, char * startdir, char * curdir, int dir_flag, int zap_flag);
int apply_boards(bbs_board_visitor func);
int is_dir(int bnum);
int getbnum(const char *bname);
int haspostperm(char * bname);
int isNonameBoard(char * bname);
int fillucache(void *record_data);
int resolve_ucache(void);
int u_namelist(void);
int searchuser(char * userid);
int apply_users(bbs_user_visitor func);
int getuser(char * userid);

/* boards_path.c */
char * curbdir(const char *s);
int popdir(char * dirstack, char * top);
int pushdir(char *dirstack, const char *s);
char * xboard_dir(const char *s);

/* cabinet.c */
int c_edit(void);
int c_view(void);
int c_remove(void);
int c_upload(void);
int c_download(void);

/* chat.c */
int addColon(char * chatid);
int fixchatid(char * chatid);
int currenttime(void);
int askLimit(void);
int t_chat(void);
char * advance(char * ptr);
int printchatline(char * str);
int printprivmsg(char * to_ptr);
int printinfoline(char * cmd);
int printchatent(void *record_data);
int dowho(void);
int printuserent(void *record_data);
int dousers(void);
int printlongent(void *record_data);
int count_chatters(int * chatters, char ** chatters_list);
int dotitle(void);
int dolong(void);
int doreal(void);
int chatlookup(void *record_data);
int doback(void);
int dofly(char * id);
int doinvite(char * id);
int domsg(char * id, int fd);
int chat_query(char * id, int fd);
int donick(char * newname, int fd);
int dochatcommand(char * cmd, int fd, char * goodbye_msg);
int ent_chat1(void);
int ent_chat2(void);
int ent_chat3(void);
int ent_chat4(void);

/* chat_change_title.c */
int change_chatroom_title(int nth_chatroom, char * newtitle);

/* chat_edit_title.c */
int edit_chatroom_title(int nth_chatroom);

/* delete.c */
int d_exit(void);
int boardcheck(const void *record_data);
int d_board(void);
int duentbds(struct shortfile * fptr);
int del_me(void);
int d_user_by_id(int id);
int d_user(void);
int kick_user_by_id(char * kickuser);
int kick_user(void);

/* edit.c */
void indigestion(int i);
struct textline * back_line(struct textline * pos, int num);
struct textline * forward_line(struct textline * pos, int num);
int getlineno(void);
char * killsp(char * s);
struct textline * alloc_line(void);
void append(struct textline * p, struct textline * line);
void delete_line(struct textline * line);
void split(struct textline * line, int pos);
int join(struct textline * line);
void insert_char(int ch);
void delete_char(void);
void vedit_init(void);
void read_file(char * filename);
void write_header(FILE * fp, int type);
int addsignature(FILE * fp);
int vedit_from_file(char * srcfile, char * dstfile, int saveheader, int type);
int write_file(char * filename, int saveheader, int type);
void display_buffer(void);
int do_fly_on_line(int msgLine, int promptend);
int back_during_edit(void);
int vedit(char * filename, int saveheader, int type);
int vedit_help(void);
int editline(int y, int x, char * buf, int limitlen);
int call_helvis(char * filename, int saveheader, int type);
void dumpEditing(void);

/* fastnew.c */
int print_fileheader(struct fileheader * fptr);
int init_newcount(char * board);
int newstart(int numfiles, char * board);
int update_new_pointer(int new_position, char * board);
int decrease_new_pointer(char * board, int deleted);
int read_boards(struct shortfile * fptr);
int New(void);
int visit_boards(struct shortfile * fptr);
int Visit(void);
int chkmail(void);
int getnewuserid(void);
int getnewutmpent(struct user_info * up);
int genuarray(char * s);
int mass_delete(struct shortfile * fptr);
int mass_delmail(char * s);
int mass_delusers(char * s);
int newclean(char * s, int id);
int fastdelete(void);

/* files.c */
char * NameProtocol(int id);
char * CurrentProtocol(void);
char * filemargin(void);
int get_dir(void);
int f_list(void);
int f_create_namelist(void);
int f_download(void);
int f_upload(void);
int f_protocol(void);
int f_select(void);
int f_text(void);

/* hanlib.c */
void ks2tgstr(unsigned char * s, unsigned char * d);
void tg2ksstr(unsigned char * s, unsigned char * d);
void rev16(char * s, char * t);
int comp16(char a1, char a2, char b1, char b2);
int ustrcmp(unsigned char * s, unsigned char * t);
void tg2instr(unsigned char * s, char * t);
void ks2instr(unsigned char * s, char * t);
void in2ksstr(char * s, unsigned char * t);
void in2tgstr(char * s, unsigned char * t);
char * josa(char * src, char * what);

/* head_notice.c */
void setNoticeToNew(void);
void dispNotice(int isLoginTime);
int manageNotice(void);

/* help.c */
char * makeCenterStr(char * src);
int do_the_help(struct help_info * p);
int Help(void);
int m_help(void);
int f_help(void);
int x_help(void);
int t_help(void);
int a_help(void);
int BBS_help(void);
int Cabinet_help(void);
int mainreadhelp(void);
int mailreadhelp(void);

/* io.c */
void hit_alarm_clock(int dummy);
int init_alarm(void);
int oflush(void);
int output(char * s, int len);
int ochar(int c);
int add_io(int fd, int timeout);
int add_flush(bbs_action flushfunc);
int num_in_buf(void);
int igetch(void);
int getdata(int line, int col, char * prompt, char * buf, int len, int echo, bbs_action complete);
int getone(int line, int col, char * prompt, char * buf, int len, int echo, bbs_action complete);

/* mail.c */
int m_init(void);
int send_each_mail(char * userid, char * filename, char * mail_subject);
int do_send(char ** userids, char * title, char * srcFile);
int m_send(void);
int m_send_mybox(void);
int print_mailerror_message(char * userid, int errcode);
int read_mail(struct fileheader * fptr);
int read_new_mail(void *record_data);
int m_new(void);
int m_forward(void);
int mailtitle(void);
char * maildoent(int num, struct fileheader * ent);
int set_new_title_NOT_USED(int y, int x, struct fileheader * fh);
int mail_read(int ent, struct fileheader * fileinfo, char * dirent);
int mail_read_galmuri(int ent, struct fileheader * fileinfo, char * dirent);
int mail_reply(int ent, struct fileheader * fileinfo, char * dirent);
int mail_del(int ent, struct fileheader * fileinfo, char * dirent);
int mail_forward(int ent, struct fileheader * fileinfo, char * dirent);
int mail_del_range(int ent, struct fileheader * fileinfo, char * dirent);
int mail_mark(int ent, struct fileheader * fileinfo, char * dirent);
int mail_adduser(int ent, struct fileheader * fileinfo, char * dirent);
int mail_manage_user(int ent, struct fileheader * fileinfo, char * dirent, int isEnable);
int mail_disableuser(int ent, struct fileheader *fileinfo, char *dirent);
int mail_enableuser(int ent, struct fileheader *fileinfo, char *dirent);
int mail_deluser(int ent, struct fileheader * fileinfo, char * dirent);
int m_read(void);
int invalidaddr(char * src);
int spacestozeros(char * s);
int getqsuffix(char * s);
int convert_tz(int local, int gmt, char * buf);
int createqf(char * title, char * qsuffix);
int doforward(char * dirent, struct shortfile * fh);

/* main.c */
void u_enter(void);
int setflags(int mask, int value);
int u_exit(void);
int cmpuids(intptr_t argument, const void *record_data);
int dosearchuser(char * userid);
void print_talk_request(void);
char * getLastMessageSender(void);
void print_msg_request(void);
void talk_request(int dummy);
void msg_request(int dummy);
_Noreturn void abort_bbs(int dummy);
int cmpuids2(intptr_t unum, const void *record_data);
int multi_user_check(void);
int account_disabled(char * userid);
int isGuest(char * userid);
int dispMiscInfo(void);
void enableTalkFly(void);
void disableTalkFly(void);
char * boardmargin(void);
int egetch(void);
void viewScreen(char * cmdtitle, char * cmdprompt, struct commands * cmdtable, bbs_margin marginnote, bbs_action helpFn);
int docmd(char * cmdtitle, char * cmdprompt, char * firstcmds, struct commands * cmdtable, bbs_margin marginnote, bbs_action helpFn);
int m_adduser(void);
int logattempt(char * uid, char * frm, int badid);
int countattempts(void);
int showattempts(int really);
void chkWaiters(void);
void chkMemo(void);

/* maintain.c */
int do_user_check(void *record_data);
int d_u_list(char * s);
int m_uclean(void);
int m_xempt(void);
int m_info(void);
int m_editbrd(void);
int user_display(struct userec * u);
int totalDay(int y, int m, int d);
int isOlder(struct fileheader * fhdrp, int day);
int domailclean(void *record_data);
int cleanmail(void *record_data);
int m_mclean(void);
int m_trace(void);
int clearflags(void);
int suckinfile(FILE * fp, char * fname);
int closepolls(void);
int m_vote(void);

/* messages.c */
int print_current_boardname_NOT_USED(void);
int print_newmail_message(void);
int print_talkreq_message_NOT_USED(void);

/* modetype.c */
char * ModeType(int mode);

/* more.c */
int readln2(FILE * fp, char * buf, int columns);
int readln(FILE * fp, char * buf);
int more(char * filename, int promptend, int type);
int lmore(char * filename, int promptend, int type);
void cat(char * filename);

/* namecomplete.c */
int FreeNameList(void);
int CreateNameList(void);
int AddNameList(char * name);
int NumInList(struct word * list);
int ApplyToNameList(bbs_name_visitor fptr);
int chkstr(char * tag, char * name, int ignCase);
struct word * GetSubList(char * tag, struct word * list, int ignorecase);
int ClearSubList(struct word * list);
int MaxLen(struct word * list, int count);
int get_common_prefix(struct word * list);
int match_len(char * s1, char * s2);
int _namecomplete(char * prompt, char * data, int ignCase);
int namecomplete(char * prompt, char * data);
int namecomplete_ignorecase(char * prompt, char * data);

/* pass.c */
char * genpasswd(char * pw);
int checkpasswd(char * passwd, char * test);

/* read.c */
struct keeploc * getkeep(char * s, int def_topline, int def_cursline);
int fixkeep(char * s, int first, int last);
char * genSearchString(char * s);
int i_read(char * dirent, bbs_action dotitle, bbs_entry_formatter doentry, struct one_key * rcmdlist);

/* record.c */
int safewrite(int fd, const void * buf, int size);
long get_num_records(char * filename, int size);
int append_record(char * filename, const void * record, int size);
int apply_record(char * filename, bbs_record_visitor fptr, int size);
int search_record(char * filename, void * rptr, int size, bbs_record_predicate fptr, intptr_t farg);
int get_record(char * filename, void * rptr, int size, int id);
int get_records(char * filename, void * rptr, int size, int id, int number);
int substitute_record(char * filename, void * rptr, int size, int id);
char * slashToUnderscore(char * s);
int delete_records(char * filename, int size, int * ids, int idcnt);
int delete_record(char * filename, int size, int id);
int delete_range(char * filename, int id1, int id2);
int update_file(char * dirname, int size, int ent, bbs_record_match filecheck, bbs_record_update fileupdate);
int delete_file(char * dirname, int size, int ent, bbs_record_match filecheck);

/* screen.c */
int init_screen(int rows, int cols);
int initscr(void);
int move(int y, int x);
int getyx(int * y, int * x);
int rel_move(int oldx, int oldy, int x, int y);
int refresh(void);
int redoscr(void);
int clear(void);
int clrtoeol(void);
int clrtobot(void);
int clrstandout(void);
int scroll(void);
int rscroll(void);
int outc(unsigned char c);
int outs(char * str);
int outns(char * str, int n);
void prints(const char * fmt, ...);
int addch(int ch);
int standout(void);
int standend(void);
int isprint2(int ch);
int isalpha2(int ch);
int isalnum2(int ch);

/* stuff.c */
int pressanykey(void);
int presskey(char * prompt);
int pressreturn(void);
int dumbreturn(void);
int bell(void);
int Getyn(char * s);
int touchnew(void);
int ci_strncmp(const char *s1, const char *s2, int n);
int bbssetenv(char * env, char * val);
int do_exec(char * com, char * wd);

/* talk.c */
int ishidden(char * user);
char * modestring(int mode, int towho, int complete, char * chatid, char * action);
char pagerchar(char * me, char * them, int pager);
int printcuent(void *record_data);
int listcuent(void *record_data);
int dumb_list(void *record_data);
int creat_list(void);
int t_users(void);
int t_rusers(void);
int t_pager(void);
char * getLastMessageReceiver(void);
int writeFlyingMessage(char * id, char * msg);
int t_fly(void);
int t_back(void);
int shoutent(void *record_data);
int t_shout(void);
int t_attach(void);
int t_wait_user(void);
int t_query(void);
int dumb_user_list(void);
int do_all_list(void *record_data);
int list_all_users(void);
int count_active(void *record_data);
int num_active_users(void);
int tooManyUsers(void);
int t_cmpuids(intptr_t uid, const void *record_data);
int t_talk(void);
int cmpunums(intptr_t unum, const void *record_data);
int searchuserlist(int unum);
int setpagerequest(void);
int servicepage(int arg);
int talkreply(void);
char * splitStr(char * src);
int dotalkent(struct user_info * uentp, char * buf);
int dotalkuserlist(int sline, int eline, int * curln, int * curcol, char * wordbuf, int * wordbuflen);
int do_talk_string(int sline, int eline, int * curln, int * curcol, char * wordbuf, int * wordbuflen, char * s);
int do_talk_char(int sline, int eline, int * curln, int * curcol, char * wordbuf, int * wordbuflen, int ch);
int talkflush(void);
int getdata_during_talk_fly(int line, int col, char * prompt, char * buf, int len, int echo, bbs_action complete);
int do_talk_fly(void);
int do_talk_query(void);
int do_talk(int fd);
int shortulist(void *record_data);
int do_list(void);
int t_list(void);
void sig_catcher(int dummy);
int t_monitor(void);
int can_override(char * userid, int uid, char * whoasks);
int listfriends(void);
int addtooverride(char * uident);
int deleteoverride(char * uident);
int t_override(void);

/* term.c */
int get_tty(void);
int init_tty(void);
int reset_tty(void);
int restore_tty(void);
int term_ok(char * term);
int outcf(int ch);
int term_init(char * term);
int do_move(int destcol, int destline, bbs_output_char outc);

/* xyz.c */
int x_passwd(void);
int x_name(void);
int x_realname(void);
int x_setemail(void);
int x_csh(void);
int x_terminal(void);
unsigned int setperms(unsigned int pbits);
int x_level(void);
int x_cloak(void);
int x_date(void);
int x_info(void);
int x_editsig(void);
int x_editplan(void);
int x_vote(void);
int x_results(void);
int ent_bnet(void);
int nlp(void);
int x_viewmemo(void);
int printQueryResult(void *record_data);
int x_findusers(void);

int standoutput(char *buf, int ds, int de, int sso, int eso);
int isalpha2(int ch);
int isalnum2(int ch);
#endif
