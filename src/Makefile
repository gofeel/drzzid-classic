# Modified 2026-11-16.
# Linux C17 build. Runtime record structures are deliberately kept unchanged.
.DEFAULT_GOAL := all
CC ?= cc
BBSACCOUNT ?= $(shell id -un)
CPPFLAGS += -DLINUX -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700
CFLAGS ?= -O0 -g
STRICT_CFLAGS = -std=c17 -Werror=implicit-function-declaration -Werror=implicit-int \
    -Werror=strict-prototypes -Werror=old-style-definition -Werror=missing-prototypes \
    -Werror=incompatible-pointer-types -Werror=int-conversion -Werror=return-type
CRYPTLIB ?= -lcrypt
LIBS = -ltermcap $(CRYPTLIB)

CFILE = bbs.c record.c delete.c main.c stuff.c mail.c files.c \
    xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
    maintain.c edit.c more.c pass.c help.c bcache.c fastnew.c \
    screen.c io.c term.c codetab.c hanlib.c modetype.c \
    chat_change_title.c chat_edit_title.c messages.c boards_path.c head_notice.c cabinet.c
COBJS = $(CFILE:.c=.o)
PROGRAMS = bbs bbs.chatd bbsrf passwdent SaveToCabinet cntuser adduser replace_utmp

.PHONY: all clean check dep depend install installbbs installchatd installother a i
all: $(PROGRAMS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT_CFLAGS) -MMD -MP -c $< -o $@

bbs: $(COBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

bbs.chatd: station.o chat_change_title.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

bbsrf: bbsrf.c readutmp.o passwdent
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT_CFLAGS) $(LDFLAGS) \
	    -DBBSHOME=\"`./passwdent $(BBSACCOUNT) homedir`\" \
	    -DBBSUID=`./passwdent $(BBSACCOUNT) uid` -o $@ bbsrf.c readutmp.o $(CRYPTLIB)

passwdent: passwdent.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

SaveToCabinet: SaveToCabinet.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

cntuser: cntuser.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

adduser: adduser.o pass.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(CRYPTLIB)

replace_utmp: replace_utmp.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Syntax-check every translation unit with the same strict declaration checks.
check:
	@set -e; for source in *.c; do \
	    $(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT_CFLAGS) \
	        -DBBSHOME='"/data"' -DBBSUID=0 -fsyntax-only "$$source"; \
	done

# Install under the selected account without removing build outputs.
a: installbbs
i: installbbs
installbbs: bbs passwdent
	install -d "`./passwdent $(BBSACCOUNT) homedir`/bin"
	install -m 0755 bbs "`./passwdent $(BBSACCOUNT) homedir`/bin/"
installchatd: bbs.chatd passwdent
	install -d "`./passwdent $(BBSACCOUNT) homedir`/bin"
	install -m 0755 bbs.chatd "`./passwdent $(BBSACCOUNT) homedir`/bin/"
installother: bbsrf SaveToCabinet cntuser adduser passwdent
	install -d "`./passwdent $(BBSACCOUNT) homedir`/bin"
	install -m 0755 bbsrf SaveToCabinet cntuser adduser "`./passwdent $(BBSACCOUNT) homedir`/bin/"
install: installchatd installother installbbs


clean:
	rm -f $(PROGRAMS) *.o *.d .depend tags a.out core *~

dep depend:
	$(MAKE) $(COBJS)

-include $(wildcard *.d)
