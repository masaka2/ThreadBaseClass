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
		../cmn/CTcpListener.cpp \
		../cmn/CTcpSocket.cpp \
		CTcpEcho.cpp \
		CTcpHealthCheck.cpp \
		main_tcp.cpp 

TARGET = tp_tcp

${TARGET}: 
	${CC} ${CFLAGS} -o ${TARGET} ${SRCS}

clean:
	rm -f *.o *.map ${TARGET}
