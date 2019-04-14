#
# Makefile for test 
#
# 2006.7.20 by M.Watanabe
#

CC = g++
CFLAGS = -pthread -Wall -ggdb -I../cmn/
SRCS = \
		../cmn/CThreadBase.cpp \
		../cmn/CLogThread.cpp \
		../cmn/CUdpSocket.cpp \
		main_udp.cpp 


TARGET = tp_udp

${TARGET}: 
	${CC} ${CFLAGS} -o ${TARGET} ${SRCS}

clean:
	rm -f *.o *.map ${TARGET}
