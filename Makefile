LOCAL = ../
LIBS_PATH = -L /usr/lib64 -L /usr/local/lib -L ${LOCAL}/libcurl++
INCS = -I /usr/local/include -I ${LOCAL}
LIBS = -l curl

SRC_LIBCURL = curl.cpp
OBJ_LIBCURL = ${SRC_LIBCURL:.cpp=.o}

SRC_TEST0 = easy_example.cpp
OBJ_TEST0 = ${SRC_TEST0:.cpp=.o}
SRC_TEST1 = multi_example.cpp
OBJ_TEST1 = ${SRC_TEST1:.cpp=.o}

CC = c++
CFLAGS = -std=c++14 -c -Wall -Werror -fPIE -fPIC -pedantic -O3 ${INCS}
LDFLAGS = ${LIBS_PATH} ${LIBS}

all: libcurl++.so easy_example multi_example

.cpp.o:
		@echo CC $<
		@${CC} ${CFLAGS} $<

libcurl++.so: ${OBJ_LIBCURL}
		@echo CC -o $@
		@${CC} -shared -o $@ ${OBJ_LIBCURL} ${LDFLAGS}

easy_example: ${OBJ_TEST0}
		@echo CC -o $@
		@${CC} -o $@ ${OBJ_TEST0} ${LIBS_PATH} -l curl++

multi_example: ${OBJ_TEST1}
		@echo CC -o $@
		@${CC} -o $@ ${OBJ_TEST1} ${LIBS_PATH} -l curl++

clean:
		@echo Cleaning
		@rm -f ${OBJ_LIBCURL} ${OBJ_TEST0} ${OBJ_TEST1}
		@rm -f *example
