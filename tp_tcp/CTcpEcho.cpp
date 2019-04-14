/**
 * @file   CTcpEcho.cpp
 * @brief  ＴＣＰエコークラス（テスト用）
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
#include "CTcpEcho.h"

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰエコークラス（テスト用）
////////////////////////////////////////////////////////////////////////////////

int CTcpEcho::onThreadInitiate()
{
	for (int i=0; i < EGSOCK_ECHO::MAX_PORT; i++) {
		m_TcpSocket[i].setAttribute(i);
		m_TcpSocket[i].setNoticeThread(this);
		m_TcpSocket[i].start();
	}
	return(CTcpListener::onThreadInitiate());
}

void CTcpEcho::onThreadTerminate()
{
	CTcpListener::onThreadTerminate();
	for (int i=0; i < EGSOCK_ECHO::MAX_PORT; i++) {
		m_TcpSocket[i].stop();
	}
}

int CTcpEcho::onMsg(CThreadMsg *p_msg)
{
	if (CTcpSocketReceiveMsg* pTcpSocketReceiveMsg = dynamic_cast<CTcpSocketReceiveMsg*>(p_msg)) {
		pTcpSocketReceiveMsg->pTcpSocket->Send(pTcpSocketReceiveMsg->p_data, pTcpSocketReceiveMsg->data_len);
//		cout << "data receive: " << pTcpSocketReceiveMsg->data_len << " bytes" << endl;
		return(ERR_OK);
	}
	if (CTcpSocketChangeStatusMsg* pTcpSocketChangeStatusMsg = dynamic_cast<CTcpSocketChangeStatusMsg*>(p_msg)) {
		int i = pTcpSocketChangeStatusMsg->pTcpSocket->get_thread_no();
		if (pTcpSocketChangeStatusMsg->new_status == EGSOCK_STS::DISCONNECT) {
			LT_MSG(m_LogHandle, (m_StrAid.Format("disconnect.(%d)", i)).c_str(), 0);
		} else {
			LT_MSG(m_LogHandle, (m_StrAid.Format(   "connect.(%d)", i)).c_str(), 0);
		}
		return(ERR_OK);
	}
	return(ERR_OK);
}

int CTcpEcho::onConnect(int connectFD, struct sockaddr_in &client_addr)
{
	int i = 0;
	for (; i < EGSOCK_ECHO::MAX_PORT; i++) {
		if (m_TcpSocket[i].getStatus() == EGSOCK_STS::DISCONNECT) {
			m_TcpSocket[i].setFD(connectFD);
			break;
		}
	}
	if (i == EGSOCK_ECHO::MAX_PORT) {
		close(connectFD);
	}
	return(ERR_OK);
}

