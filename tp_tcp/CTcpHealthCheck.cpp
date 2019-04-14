/**
 * @file   CTcpHealthCheck.cpp
 * @brief  TCPヘルスチェックスレッドクラス（テスト用）
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/11/22 渡辺正勝    初版<BR>
 */

#include <errno.h>
#include <stdio.h>
#include <iostream>
#include "CTcpHealthCheck.h"

////////////////////////////////////////////////////////////////////////////////
// 独自のログ用マクロ
////////////////////////////////////////////////////////////////////////////////
#define MY_LOG(str){\
	m_LogHandle.write(m_StrAid.Format(\
	"%s[%d] %s %s", __FILE__,__LINE__, m_strMyID.c_str(), str), 0);\
}

////////////////////////////////////////////////////////////////////////////////
// TCPヘルスチェックスレッドクラス
////////////////////////////////////////////////////////////////////////////////
int CTcpHealthCheck::onThreadInitiate()
{
	int ret = CTcpSocket::onThreadInitiate();	// これ重要！
	if (ret != ERR_OK) {
		return(ret);
	}
	m_strMyID = m_StrAid.Format("pthread(%d)", static_cast<int>(get_pthread()));
	setTimer(m_msec_elapsed_time, TIMER_ID_PERIOD, m_msec_period);
	return(ERR_OK);
}

void CTcpHealthCheck::onTimer(int timer_id)
{
	if (timer_id == TIMER_ID_PERIOD) {
		if (getStatus() == EGSOCK_STS::CONNECT) {
			m_strData = m_strMyID + " " + strCurrentTime();
			sendSocket(m_strData.c_str(), m_strData.length());
			setTimer(m_msec_timeout, TIMER_ID_TIMEOUT);
		}
		return;
	}
	if (timer_id == TIMER_ID_TIMEOUT) {
		MY_LOG("timeout.");
		return;
	}
	CTcpSocket::onTimer(timer_id);	// これ重要！
}

int CTcpHealthCheck::onReceive(const char *p_buf , int data_len, int *p_accept_len)
{
	if (data_len < m_strData.length()) {
		*p_accept_len = 0;
		return(ERR_OK);
	}
	*p_accept_len = m_strData.length();
	cancelTimer(TIMER_ID_TIMEOUT);
	if (0 == strncmp(p_buf, m_strData.c_str(), m_strData.length())) {
		MY_LOG("ok.");
	} else {
		MY_LOG("unmatched.");
	}
	return(ERR_OK);
}

int CTcpHealthCheck::onChangeStatus(int new_status, int pre_status)
{
	if (new_status == EGSOCK_STS::CONNECT) {
		MY_LOG("connect.");
	} else {
		MY_LOG("disconnect.");
	}
	return(ERR_OK);
}

#undef MY_LOG

