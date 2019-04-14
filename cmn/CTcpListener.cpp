/**
 * @file   CTcpListener.cpp
 * @brief  ＴＣＰリスナベースクラス
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/10/20 渡辺正勝    初版<BR>
 */

#include <errno.h>
#include <stdio.h>
#include <iostream>
#include "CTcpListener.h"
#include <strings.h>	// for bzero()

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰリスナベースクラス
////////////////////////////////////////////////////////////////////////////////

int CTcpListener::onPreThreadCreate()
{
	if (m_port == 0) {
		return(ERR_CONTEXT);
	}
	return(ERR_OK);
}

int CTcpListener::onThreadInitiate()
{
	return(openSocket());
}

void CTcpListener::onThreadTerminate()
{
	closeSocket();
}

int CTcpListener::onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds)
{
	if (FD_ISSET(m_listenFD, p_readfds)) {
		struct sockaddr_in	client_addr;
		socklen_t			client_len = sizeof(client_addr);
		int connectFD = accept(m_listenFD, (struct sockaddr*)&client_addr, &client_len);
		if (connectFD == (-1)) {
			perror("accept");
			closeSocket();
		} else {
			client_addr.sin_port = ntohs(client_addr.sin_port);
			onConnect(connectFD, client_addr);
		}
	}
	return(ERR_OK);
}

int CTcpListener::onConnect(int connectFD, struct sockaddr_in &client_addr)
{
	char cAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, cAddr, INET_ADDRSTRLEN);
	// 通知スレッドあり？
	if (m_pNoticeThread) {
		CTcpListenerConnectMsg *pTcpListenerConnectMsg = new CTcpListenerConnectMsg;
		pTcpListenerConnectMsg->pTcpListener = this;
		pTcpListenerConnectMsg->listen_port	 = m_port;
		pTcpListenerConnectMsg->connectFD	 = connectFD;
		pTcpListenerConnectMsg->client_addr	 = client_addr;
		pTcpListenerConnectMsg->ip_addr		 = cAddr;
		m_pNoticeThread->postMsg(pTcpListenerConnectMsg);
	} else {
		cout << "new client_ent: ";
		cout << cAddr;
		cout << ", port ";
		cout << client_addr.sin_port << endl;
		close(connectFD);
	}
	return(ERR_OK);
}

int CTcpListener::openSocket()
{
	if ((m_listenFD = socket(AF_INET, SOCK_STREAM, 0)) == (-1)) {
		perror("socket");
		return(-1);
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port        = htons(m_port);
	if ((bind(m_listenFD, (struct sockaddr*)&server_addr, sizeof(server_addr))) == (-1)) {
		perror("bind");
		closeSocket();
		return(-1);
	}
	if ((listen(m_listenFD, SOMAXCONN)) == (-1)) {
		perror("listen");
		closeSocket();
		return(-1);
	}
	appendFD(m_listenFD, true, false);
	return(ERR_OK);
}

int CTcpListener::closeSocket()
{
	if (m_listenFD != (-1)) {
		removeFD(m_listenFD);
		close(m_listenFD);
		m_listenFD = (-1);
	}
	return(ERR_OK);
}

