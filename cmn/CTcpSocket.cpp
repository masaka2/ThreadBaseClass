/**
 * @file   CTcpSocket.cpp
 * @brief  ＴＣＰソケットクラス
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
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include "CTcpSocket.h"
#include <strings.h>

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰソケットクラス
////////////////////////////////////////////////////////////////////////////////

int CTcpSocket::setFD(int socketFD)
{
	if (socketFD < 0) {
		return(ERR_PARAM);
	}
	if (m_type == EGSOCK_TYPE::CLIENT) {
		return(ERR_CONTEXT);
	}
	if (m_type == EGSOCK_TYPE::UNKNOWN) {
		// 未定であれば、サーバ側にする。（この関数を呼び出したことで確定）
		m_type = EGSOCK_TYPE::SERVER;
	}
	int ret = ERR_OK;
	// 起動前か？
	if (get_pthread() == 0) {
		closeSocket();	// 起動前に２度以上呼び出された時の対応。
		m_socketFD = socketFD;
		// openSocket()は起動時に行われる。
	} else {
		CTcpServerSetFdMsg *pTcpServerSetFdMsg = new CTcpServerSetFdMsg;
		pTcpServerSetFdMsg->socketFD = socketFD;
		ret = postMsg(pTcpServerSetFdMsg);
	};
	return(ret);
}

int CTcpSocket::setServerAddr(	string str_server_addr,
								uint16_t server_port,
								int connect_T1,
								int connect_T2)
{
	if (str_server_addr == "") {
		return(ERR_PARAM);
	}
	if (m_server_port == 0) {
		return(ERR_PARAM);
	}
	if (connect_T1 < 0) {
		return(ERR_PARAM);
	}
	if (connect_T2 < 0) {
		return(ERR_PARAM);
	}
	// 起動後か？
	if (get_pthread() != 0) {
		return(ERR_CONTEXT);
	}
	if (m_type == EGSOCK_TYPE::SERVER) {
		return(ERR_CONTEXT);
	}
	if (m_type == EGSOCK_TYPE::UNKNOWN) {
		// 未定であれば、クライアント側にする。（この関数を呼び出したことで確定）
		m_type = EGSOCK_TYPE::CLIENT;
	}
	m_str_server_addr	= str_server_addr;
	m_server_port		= server_port;
	m_connect_T1		= connect_T1;
	m_connect_T2		= connect_T2;
	return(ERR_OK);
}

int CTcpSocket::Send(const void *vp_data, int data_len)
{
	if (vp_data == NULL) {
		return(ERR_PARAM);
	}
	if (data_len <= 0) {
		return(ERR_PARAM);
	}
	CTcpSocketSendMsg *pTcpSocketSendMsg = new CTcpSocketSendMsg(data_len, vp_data);
	int ret = postMsg(pTcpSocketSendMsg);
	return(ret);
}

int CTcpSocket::onPreThreadCreate()
{
	if (m_type == EGSOCK_TYPE::SERVER) {
		;	// サーバであれば、起動後にファイルディスクリプタを設定できる。
	} else {
		// クライアント側は起動前に相手側が確定してないとＮＧ！
		if (m_type == EGSOCK_TYPE::CLIENT) {
			if (m_str_server_addr == "") {
				return(ERR_CONTEXT);
			}
			if (m_server_port == 0) {
				return(ERR_CONTEXT);
			}
		} else {
			// 未定であれば、サーバ側にしてしまう。
			m_type = EGSOCK_TYPE::SERVER;
		}
	}
	return(ERR_OK);
}

int CTcpSocket::onThreadInitiate()
{
	return(openSocket());
}

void CTcpSocket::onThreadTerminate()
{
	closeSocket();
}

void CTcpSocket::onTimer(int timer_id)
{
	if (timer_id == EGSOCK_CONFIG::TIMER_ID) {
		openSocket();
	}
}

int CTcpSocket::onMsg(CThreadMsg *p_msg)
{
	if (CTcpSocketSendMsg* pTcpSocketSendMsg = dynamic_cast<CTcpSocketSendMsg*>(p_msg)) {
		if (m_status == EGSOCK_STS::CONNECT) {
			sendSocket(pTcpSocketSendMsg->p_data, pTcpSocketSendMsg->data_len);
		} else {
			onError(EGSOCK_ERR::SEND_DATA_WAS_LOST);
		}
	}
	if (CTcpServerSetFdMsg* pTcpServerSetFdMsg = dynamic_cast<CTcpServerSetFdMsg*>(p_msg)) {
		closeSocket();
		m_socketFD = pTcpServerSetFdMsg->socketFD;
		openSocket();
		return(ERR_OK);
	}
	return(ERR_OK);
}

// ::send()のラッパ（エラー処理等の統一のため）
int CTcpSocket::sendSocket(const char *p_data, int data_len)
{
	// cout << "pre send." << endl;
	int n = send(m_socketFD, p_data, data_len, 0);
	// cout << "post send." << endl;
	if (n != data_len) {
		// エラー処理をきちんとやるべきか？
		// Linux ではバッファ不足は起こらないらしい！
		onError(EGSOCK_ERR::API_CALL, errno);
	}
	return(n);
}

int CTcpSocket::onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds)
{
	if (m_status == EGSOCK_STS::CONNECT) {
		if (FD_ISSET(m_socketFD, p_readfds)) {
again:
			int n = 0;
			if ((n = recv(	m_socketFD,
							&m_buf[m_data_len],
							EGSOCK_CONFIG::MAX_BUF_LEN - m_data_len,
							0)) < 0) {
				if (errno == EINTR) {
					goto again;
				} else {
					onError(EGSOCK_ERR::API_CALL, errno);
					closeSocket(m_connect_T2);
					return(ERR_OK);
				}
			}
			if (n == 0) {
				closeSocket(m_connect_T2);
				return(ERR_OK);
			} else {
				m_data_len += n;
				int pos = 0;
				int accept_len;
				do {
					accept_len = m_data_len - pos;
					onReceive(	&m_buf[pos],
								accept_len,
								&accept_len);
					if ((accept_len > (m_data_len - pos)) || (accept_len < 0)) {
						accept_len = m_data_len - pos;
						onError(EGSOCK_ERR::AP_ILLEGAL_USE);
					}
					pos += accept_len;
				} while ((m_data_len != pos) && (accept_len != 0));
				m_data_len -= pos;
				if (m_data_len != 0) {
					if (m_data_len < EGSOCK_CONFIG::MAX_BUF_LEN) {
						memmove(&m_buf[0], &m_buf[pos], m_data_len);
					} else {
						onError(EGSOCK_ERR::AP_ILLEGAL_USE);
					}
				}
			}
		}
	} else {
		// m_socketFDが(-1)ならガードする。（落ちるぞ！）
		// 来ない筈だが派生されたときのことも考慮する。
		if ((m_type == EGSOCK_TYPE::CLIENT) && (m_socketFD != (-1))) {
			if (FD_ISSET(m_socketFD, p_readfds) || FD_ISSET(m_socketFD, p_writefds)) {
				// 非ブロッキングは移植性に問題あり。
				int error = 0;
				socklen_t len = sizeof(error);
				if (getsockopt(m_socketFD, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
					error = errno;	// Solaris は errno に設定？
				} else {
					errno = error;
				}
				removeFD(m_socketFD);
				if ((m_flags = fcntl(m_socketFD, F_SETFL, m_flags)) < 0) {
					perror("fcntl");
					closeSocket(m_connect_T2);
					return(-1);
				}
				if (!error) {
					appendFD(m_socketFD, true, false);
					changeStatus(EGSOCK_STS::CONNECT);
					// cout << "connect completed" << endl;
				} else {
					closeSocket(m_connect_T1);	// タイマ値が違う！
					// cout << "connect error" << endl;
				}
				return(ERR_OK);
			}
		}
	}
	return(ERR_OK);
}

int CTcpSocket::onReceive(const char *p_data , int data_len, int *p_accept_len)
{
	if (m_pNoticeThread) {
		CTcpSocketReceiveMsg *pTcpSocketReceiveMsg = new CTcpSocketReceiveMsg(data_len, p_data);
		pTcpSocketReceiveMsg->pTcpSocket = this;
		m_pNoticeThread->postMsg(pTcpSocketReceiveMsg);
	} else {
		cout << "data receive: " << data_len << " bytes" << endl;
	}
	return(ERR_OK);
}

int CTcpSocket::onChangeStatus(int new_status, int pre_status)
{
	if (m_pNoticeThread) {
		CTcpSocketChangeStatusMsg *pTcpSocketChangeStatusMsg = new CTcpSocketChangeStatusMsg;
		pTcpSocketChangeStatusMsg->pTcpSocket = this;
		pTcpSocketChangeStatusMsg->new_status = new_status;
		pTcpSocketChangeStatusMsg->pre_status = pre_status;
		m_pNoticeThread->postMsg(pTcpSocketChangeStatusMsg);
	} else {
		cout << "change status : " << pre_status << " to " << new_status << endl;
	}
	return(ERR_OK);
}

int CTcpSocket::onError(int error, int _errno)
{
	if (m_pNoticeThread) {
		CTcpSocketErrorMsg *pTcpSocketErrorMsg = new CTcpSocketErrorMsg;
		pTcpSocketErrorMsg->pTcpSocket = this;
		pTcpSocketErrorMsg->error = error;
		pTcpSocketErrorMsg->_errno = _errno;
		m_pNoticeThread->postMsg(pTcpSocketErrorMsg);
	} else {
		cout << "error: " << error << " " << _errno << endl;
	}
	return(ERR_OK);
}

int CTcpSocket::openSocket()
{
	if (m_type == EGSOCK_TYPE::SERVER) {
		openSocketServer();
	} else {
		if (m_type == EGSOCK_TYPE::CLIENT) {
			openSocketClient();
		} else {
			assert(false);
		}
	}
	return(ERR_OK);
}

int CTcpSocket::openSocketServer()
{
	if (m_socketFD != (-1)) {
		appendFD(m_socketFD, true, false);
		changeStatus(EGSOCK_STS::CONNECT);
	}
	return(ERR_OK);
}

int CTcpSocket::openSocketClient()
{
	assert(m_socketFD == (-1));
	if ((m_socketFD = socket(AF_INET, SOCK_STREAM, 0)) == (-1)) {
		perror("socket");
		return(-1);
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port   = htons(m_server_port);
	if (inet_pton(AF_INET, m_str_server_addr.c_str(), &server_addr.sin_addr) <= 0) {
		// perror("inet_pton");
		struct hostent *hp;
		hp = gethostbyname(m_str_server_addr.c_str());
		if (!hp) {
			perror("gethostbyname");
			closeSocket(m_connect_T2);
			return(-1);
		}
		server_addr.sin_addr = *reinterpret_cast<struct in_addr *>(hp->h_addr);
	}
	// 非ブロッキングに！
	if ((m_flags = fcntl(m_socketFD, F_GETFL, 0)) < 0) {
		perror("fcntl");
		closeSocket(m_connect_T2);
		return(-1);
	}
	if ((m_flags = fcntl(m_socketFD, F_SETFL, m_flags | O_NONBLOCK)) < 0) {
		perror("fcntl");
		closeSocket(m_connect_T2);
		return(-1);
	}
	int ret = connect(m_socketFD, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret == 0) {
		// cout << "connect completed immediately" << endl;
		if ((m_flags = fcntl(m_socketFD, F_SETFL, m_flags)) < 0) {
			perror("fcntl");
			closeSocket(m_connect_T2);
			return(-1);
		}
		appendFD(m_socketFD, true, false);
		changeStatus(EGSOCK_STS::CONNECT);
	} else {
		if (errno != EINPROGRESS) {
			perror("connect");
			closeSocket(m_connect_T2);
			return(-1);
		} else {
			appendFD(m_socketFD, true, true);
		}
	}
	return(ERR_OK);
}

int CTcpSocket::closeSocket(int timer)
{
	if (m_socketFD != (-1)) {
		removeFD(m_socketFD);
		close(m_socketFD);
		m_socketFD = (-1);
	}
	if (m_status == EGSOCK_STS::CONNECT) {
		changeStatus(EGSOCK_STS::DISCONNECT);
	}
	if (m_type == EGSOCK_TYPE::CLIENT) {
		if (active()) {	// スレッド終了時は再接続しない。
			if (timer) {
				setTimer(timer, EGSOCK_CONFIG::TIMER_ID);
			};
		};
	}
	return(ERR_OK);
}

int CTcpSocket::changeStatus(int new_status)
{
	// EGSOCK_STS::DISCONNECT で呼び出すのは closeSocket() のみ！
	onChangeStatus(new_status, m_status);
	pthread_mutex_lock(&m_mutex);
	m_status = new_status;
	pthread_mutex_unlock(&m_mutex);
	return(ERR_OK);
}

