V_MAYOR=0.7
V_MINOR=xmas
V_DATE=`date +%s`
CC=gcc
LIBS= -ldl -lmysqlclient -lconfig -lpthread
modules = mod/nick.so mod/db.so mod/channel.so \
	  mod/commands.so mod/pserve.so mod/trivia.so
objects = obj/debug.obj obj/io.obj obj/main.obj \
	  obj/server.obj obj/signals.obj obj/helpers.obj \
	  obj/db.obj obj/config.obj obj/modules.obj
headers = src/defines.h
	  
#DEBUG FLAGS 
flags = -g -c -Wall -Werror -Wstrict-prototypes -Wwrite-strings -Wshadow \
        -Wsign-compare -Wno-trigraphs -fno-builtin -fno-common 
#NO DEBUG
#-O2 -fomit-frame-pointer	
flagsmod = -g -Wall -Werror -Wstrict-prototypes -Wwrite-strings -Wshadow \
	-Wsign-compare -Wno-trigraphs -fno-builtin -fno-common \
	-nostartfiles -shared -fPIC -s -export-dynamic 

all: rb

rb: obj ${objects} ${headers} ${modules} ${headers}
	${CC} ${objects} ${LIBS} -o rb
#	strip rb


mod:
	mkdir -p mod
mod/nick.so: src/modules/nick/nick.c ${headers}
	${CC} ${LIBS} ${flagsmod} -o mod/nick.so src/modules/nick/nick.c
mod/db.so: src/modules/db/db.c ${headers}
	${CC} ${LIBS} ${flagsmod} -o mod/db.so src/modules/db/db.c
mod/channel.so: src/modules/channel/channel.c ${headers}
	${CC} ${LIBS} ${flagsmod} -o mod/channel.so src/modules/channel/channel.c
mod/commands.so: src/modules/commands/commands.c ${headers}
	${CC} ${LIBS} ${flagsmod} -o mod/commands.so src/modules/commands/commands.c
mod/pserve.so: src/modules/pserve/pserve.c ${headers}
	${CC} ${LIBS} ${flagsmod} -o mod/pserve.so src/modules/pserve/pserve.c
mod/trivia.so: src/modules/trivia/trivia.c ${headers}
	${CC} ${LIBS} ${flagsmod} -o mod/trivia.so src/modules/trivia/trivia.c

obj:
	mkdir -p obj
obj/debug.obj: src/debug.c ${headers}
	${CC} ${flags} -o obj/debug.obj src/debug.c
obj/io.obj: src/io.c ${headers}
	${CC} ${flags} -o obj/io.obj src/io.c
obj/main.obj: src/main.c ${headers}
	${CC} ${flags} -o obj/main.obj src/main.c -DVERSION_MAYOR='"${V_MAYOR}"' -DVERSION_MINOR='"${V_MINOR}"' -DVERSION_DATE=${V_DATE}
obj/server.obj: src/server.c ${headers}
	${CC} ${flags} -o obj/server.obj src/server.c
obj/signals.obj: src/signals.c ${headers}
	${CC} ${flags} -o obj/signals.obj src/signals.c
obj/helpers.obj: src/helpers.c ${headers}
	${CC} ${flags} -o obj/helpers.obj src/helpers.c
obj/config.obj: src/config.c ${headers}
	${CC} ${flags} -o obj/config.obj src/config.c -DVERSION_MAYOR='"${V_MAYOR}"' -DVERSION_MINOR='"${V_MINOR}"' -DVERSION_DATE=${V_DATE}
obj/db.obj: src/db.c ${headers}
	${CC} ${flags} -o obj/db.obj src/db.c
obj/modules.obj: src/modules.c ${headers}
	${CC} ${flags} -o obj/modules.obj src/modules.c

clean:
	/bin/rm -f obj/*.obj rb rb.log rb.core rb.pid mod/*.so bin/*

utilities: 
	mkdir -p bin
	${CC} ${flags} -o obj/crc32.obj src/utilities/crc32.c -Isrc/utilities
	${CC} ${flags} -o obj/gencrc32.obj src/utilities/gencrc32.c -Isrc/utilities
	${CC} obj/crc32.obj obj/gencrc32.obj -o bin/crc32
	strip bin/crc32

backup: clean
	tar cv ./ | bzip2 -c > ../`date +rb-%Y%m%d_%H%M.tar.bz2`
	
# end