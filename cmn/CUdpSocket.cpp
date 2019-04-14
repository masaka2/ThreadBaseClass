/**
 * @file   CUdpSocket.cpp
 * @brief  ＵＤＰソケットクラス
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2006/06/02 渡辺正勝    初版<BR>
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include "CUdpSocket.h"
#include <strings.h>

////////////////////////////////////////////////////////////////////////////////
// ＵＤＰソケットクラス
////////////////////////////////////////////////////////////////////////////////

int CUdpSocket::onThreadInitiate()
{
	return(openSocket());
}

void CUdpSocket::onThreadTerminate()
{
	closeSocket();
}

int CUdpSocket::onMsg(CThreadMsg *p_msg)
{
	if (CUdpSocketSendMsg* pUdpSocketSendMsg = dynamic_cast<CUdpSocketSendMsg*>(p_msg)) {
		sendTo(	pUdpSocketSendMsg->p_data,
				pUdpSocketSendMsg->data_len,
				pUdpSocketSendMsg->str_peer_addr,
				pUdpSocketSendMsg->peer_port);
	}
	return(ERR_OK);
}

// ::sendto()のラッパ（エラー処理等の統一のため）
int CUdpSocket::sendTo(	const void	*vp_data,
						int			data_len,
						string		str_peer_addr,
						uint16_t	peer_port)
{
	struct sockaddr_in peer_sock;
	bzero(&peer_sock, sizeof peer_sock);
	peer_sock.sin_family = AF_INET;
	peer_sock.sin_addr.s_addr = inet_addr(str_peer_addr.c_str());
	peer_sock.sin_port = htons(peer_port);
	int n = sendto(	m_socketFD,
					vp_data,
					data_len,
					0,
					(struct sockaddr *)&peer_sock,
					sizeof peer_sock);
	if (n != data_len) {
		// エラー処理をきちんとやるべきか？
		// Linux ではバッファ不足は起こらないらしい！（ＵＤＰも？）
		onError(EGUDP_ERR::API_CALL, errno, str_peer_addr, peer_port);
	}
	// FDを登録してなければ、はじめての送信後に登録する。
	// bind()してなければ、はじめての送信で自ポートが確定する。
	if (m_bool_append == false) {
		appendFD(m_socketFD, true, false);
		m_bool_append = true;
	}
	return(n);
}

int CUdpSocket::onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds)
{
	// m_socketFDが(-1)ならガードする。（落ちるぞ！）
	// 来ない筈だが派生されたときのことも考慮する。
	if (m_socketFD == (-1)) {
		return(ERR_OK);
	}
	if (FD_ISSET(m_socketFD, p_readfds)) {
		struct sockaddr_in peer_sock;
		socklen_t peer_sock_len = sizeof peer_sock;
		int n = recvfrom(	m_socketFD,
							m_buf,
							sizeof m_buf,
							0,
							(struct sockaddr *)&peer_sock,
							&peer_sock_len);
		if (n == 0) {
			return(ERR_OK);
		}
		if (n < 0) {
			onError(EGUDP_ERR::API_CALL, errno);
			return(ERR_OK);
		}
		string str_peer_addr(inet_ntoa(peer_sock.sin_addr));
		onReceive(	m_buf,
					n,
					str_peer_addr,
					ntohs(peer_sock.sin_port));
	}
	return(ERR_OK);
}

int CUdpSocket::onReceive(	const char	*p_data,
							int			data_len,
							string		str_peer_addr,
							uint16_t	peer_port)
{
	if (m_pNoticeThread) {
		CUdpSocketReceiveMsg *pUdpSocketReceiveMsg = 
			new CUdpSocketReceiveMsg(data_len, p_data, str_peer_addr, peer_port);
		pUdpSocketReceiveMsg->pUdpSocket = this;
		m_pNoticeThread->postMsg(pUdpSocketReceiveMsg);
	} else {
		cout << "data receive: " << data_len << " bytes from " << str_peer_addr << " " << peer_port << endl;
	}
	return(ERR_OK);
}

int CUdpSocket::onError(int			error,
						int			_errno,
						string		str_peer_addr,
						uint16_t	peer_port)
{
	if (m_pNoticeThread) {
		CUdpSocketErrorMsg *pUdpSocketErrorMsg = 
			new CUdpSocketErrorMsg(str_peer_addr, peer_port);
		pUdpSocketErrorMsg->pUdpSocket = this;
		pUdpSocketErrorMsg->error = error;
		pUdpSocketErrorMsg->_errno = _errno;
		m_pNoticeThread->postMsg(pUdpSocketErrorMsg);
	} else {
		cout << "error: " << error << " " << _errno << endl;
	}
	return(ERR_OK);
}

int CUdpSocket::openSocket()
{
	assert(m_socketFD == (-1));
	if ((m_socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == (-1)) {
		perror("socket");
		return(-1);
	}
	if (m_bind_port) {
		struct sockaddr_in my_addr;
		bzero(&my_addr, sizeof my_addr);
		my_addr.sin_family = AF_INET;
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		my_addr.sin_port = htons(m_bind_port);
		if ((-1) == bind(m_socketFD, (struct sockaddr *)&my_addr, sizeof my_addr)) {
			perror("bind");
			closeSocket();
			return(-1);
		}
		appendFD(m_socketFD, true, false);
		m_bool_append = true;
	}
	return(ERR_OK);
}

int CUdpSocket::closeSocket()
{
	if (m_socketFD != (-1)) {
		removeFD(m_socketFD);
		close(m_socketFD);
		m_socketFD = (-1);
	}
	return(ERR_OK);
}

