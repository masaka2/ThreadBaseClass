/**
 * @file   CTcpHealthCheck.h
 * @brief  TCPヘルスチェックスレッドクラス（テスト用）
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/11/22 渡辺正勝    初版<BR>
 */

#ifndef CTcpHealthCheck_h
#define CTcpHealthCheck_h

#include <sys/types.h>
#include <string>
#include "CTcpSocket.h"
#include "CLogThread.h"
#include "CStrAid.h"

const static int TIMER_ID_PERIOD	= 1;	// ヘルスチェック周期に使用するタイマ
const static int TIMER_ID_TIMEOUT	= 2;	// タイムアウトに使用するタイマ

////////////////////////////////////////////////////////////////////////////////
// TCPヘルスチェックスレッドクラス
////////////////////////////////////////////////////////////////////////////////
class CTcpHealthCheck : public CTcpSocket
{
public:

	CTcpHealthCheck(
		string str_server_addr="127.0.0.1",
		uint16_t server_port=22222,
		int msec_elapsed_time=1000,
		int msec_period=10000,
		int msec_timeout=5000,
		int connect_T1=EGSOCK_CONFIG::CONNECT_T1,
		int connect_T2=EGSOCK_CONFIG::CONNECT_T2
	)
	: CTcpSocket(str_server_addr, server_port, connect_T1, connect_T2, this)
	, m_msec_elapsed_time(msec_elapsed_time)
	, m_msec_period(msec_period)
	, m_msec_timeout(msec_timeout)
	{
	};

	virtual ~CTcpHealthCheck()
	{
	};

protected:
	virtual int  onThreadInitiate();
	virtual void onTimer(int timer_id);

	virtual int onReceive(const char *p_buf , int data_len, int *p_accept_len);
	virtual int onChangeStatus(int new_status, int pre_status);

	string strCurrentTime()
	{
		CTimeVal current_time(CTimeVal::CURRENT);
		return(m_StrAid.Format("%d:%d",
								static_cast<int>(current_time.tv_sec),
								static_cast<int>(current_time.tv_usec)));
	};

private:
	int		m_msec_elapsed_time;
	int		m_msec_period;
	int		m_msec_timeout;

	string	m_strMyID;
	string	m_strData;

	CLogHandle	m_LogHandle;
	CStrAid		m_StrAid;
};

#endif
