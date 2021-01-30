LOCAL = ../
LIBS_PATH = -L /usr/lib64 -L /usr/local/lib -L ${LOCAL}/
INCS = -I /usr/local/include -I ${LOCAL}
LIBS = -l curl -Wl,-rpath=${LOCAL_LIB}/

SRC_LIBCURL = curl.cpp
OBJ_LIBCURL = ${SRC_LIBCURL:.cpp=.o}

SRC_TEST = example.cpp
OBJ_TEST = ${SRC_TEST:.cpp=.o}

CC = g++
CFLAGS = -std=c++11 -c -g -Wall -Werror -fPIE -fPIC -pedantic ${INCS}
LDFLAGS = ${LIBS}

all: libcurl++.so example

.cpp.o:
		@echo CC $<
		@${CC} ${CFLAGS} $<

libcurl++.so: ${OBJ_LIBCURL}
		@echo CC -o $@
		@${CC} -shared -o $@ ${OBJ_LIBCURL} ${LDFLAGS}

example: ${OBJ_TEST}
		@echo CC -o $@
		@${CC} -o $@ ${OBJ_TEST} ${LDFLAGS} -L $(CURDIR) -l curl++ -Wl,-rpath,$(CURDIR)

clean:
		@echo Cleaning
		@rm -f ${OBJ_LIBCURL} ${OBJ_TEST}
		@rm -f example
