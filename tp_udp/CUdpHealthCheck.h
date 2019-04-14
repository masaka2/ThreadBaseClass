/**
 * @file   CUdpHealthCheck.h
 * @brief  UDPヘルスチェックスレッドクラス（テスト用）
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2006/06/07 渡辺正勝    初版<BR>
 */

#ifndef CUdpHealthCheck_h
#define CUdpHealthCheck_h

#include <sys/types.h>
#include <string>
#include "CUdpSocket.h"
#include "CLogThread.h"
#include "CStrAid.h"

const static int UDP_TIMER_ID_PERIOD	= 1;	// ヘルスチェック周期に使用するタイマ
const static int UDP_TIMER_ID_TIMEOUT	= 2;	// タイムアウトに使用するタイマ

////////////////////////////////////////////////////////////////////////////////
// 独自のログ用マクロ
////////////////////////////////////////////////////////////////////////////////
#define MY_LOG(str){\
	m_LogHandle.write(m_StrAid.Format(\
	"%s[%d] %s %s", __FILE__,__LINE__, m_strMyID.c_str(), str), 0);\
}

////////////////////////////////////////////////////////////////////////////////
// UDPヘルスチェックスレッドクラス
////////////////////////////////////////////////////////////////////////////////
class CUdpHealthCheck : public CUdpSocket
{
public:

	CUdpHealthCheck(
		string str_peer_addr="127.0.0.1",
		uint16_t peer_port=22222,
		int msec_elapsed_time=1000,
		int msec_period=10000,
		int msec_timeout=5000
	)
	: m_str_peer_addr(str_peer_addr)
	, m_peer_port(peer_port)
	, m_msec_elapsed_time(msec_elapsed_time)
	, m_msec_period(msec_period)
	, m_msec_timeout(msec_timeout)
	{
	};

	virtual ~CUdpHealthCheck()
	{
	};

protected:

	virtual int onThreadInitiate()
	{
		int ret = CUdpSocket::onThreadInitiate();	// これ重要！
		if (ret != ERR_OK) {
			return(ret);
		}
		m_strMyID = m_StrAid.Format("pthread(%d)", static_cast<int>(get_pthread()));
		setTimer(m_msec_elapsed_time, UDP_TIMER_ID_PERIOD, m_msec_period);
		return(ERR_OK);
	};

	virtual void onTimer(int timer_id)
	{
		if (timer_id == UDP_TIMER_ID_PERIOD) {
			m_strData = m_strMyID + " " + strCurrentTime();
			sendTo(m_strData.c_str(), m_strData.length(), m_str_peer_addr, m_peer_port);
			setTimer(m_msec_timeout, UDP_TIMER_ID_TIMEOUT);
			return;
		}
		if (timer_id == UDP_TIMER_ID_TIMEOUT) {
			MY_LOG("timeout.");
			return;
		}
	};

	virtual int onReceive(	const char	*p_data,
							int			data_len,
							string		str_peer_addr,
							uint16_t	peer_port)
	{
		cancelTimer(UDP_TIMER_ID_TIMEOUT);
		if (data_len != m_strData.length()) {
			MY_LOG("length unmatched.");
			return(ERR_OK);
		}
		if (str_peer_addr != m_str_peer_addr) {
			MY_LOG("peer addr unmatched.");
			return(ERR_OK);
		}
		if (peer_port != m_peer_port) {
			MY_LOG("peer port unmatched.");
			return(ERR_OK);
		}
		if (0 == strncmp(p_data, m_strData.c_str(), m_strData.length())) {
			MY_LOG("ok.");
		} else {
			MY_LOG("data unmatched.");
		}
		return(ERR_OK);
	};

	string strCurrentTime()
	{
		CTimeVal current_time(CTimeVal::CURRENT);
		return(m_StrAid.Format("%d:%d",
								static_cast<int>(current_time.tv_sec),
								static_cast<int>(current_time.tv_usec)));
	};

private:
	string		m_str_peer_addr;
	uint16_t	m_peer_port;

	int		m_msec_elapsed_time;
	int		m_msec_period;
	int		m_msec_timeout;

	string	m_strMyID;
	string	m_strData;

	CLogHandle	m_LogHandle;
	CStrAid		m_StrAid;
};

#undef MY_LOG

#endif
