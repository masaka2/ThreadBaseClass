#
# Makefile for test 
#
# 2006.7.20 by M.Watanabe
#

CC = g++
CFLAGS = -pthread -Wall -ggdb -lboost_filesystem -I../cmn/
SRCS = \
		main_tips.cpp 

TARGET = tp_tips

${TARGET}: 
	${CC} ${CFLAGS} -o ${TARGET} ${SRCS}

clean:
	rm -f *.o *.map ${TARGET}
