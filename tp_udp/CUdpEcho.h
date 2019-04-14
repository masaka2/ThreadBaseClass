/**
 * @file   CUdpEcho.h
 * @brief  ＵＤＰエコークラス（テスト用）
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2006/06/06 渡辺正勝    初版<BR>
 */

#ifndef CUdpEcho_h
#define CUdpEcho_h

#include "CUdpSocket.h"
#include "CLogThread.h"
#include "CStrAid.h"

////////////////////////////////////////////////////////////////////////////////
// ＵＤＰエコークラス（テスト用）
////////////////////////////////////////////////////////////////////////////////
class CUdpEcho : public CUdpSocket
{
public:
	CUdpEcho(uint16_t bind_port=0)
	: CUdpSocket(bind_port)
	{
	};

	virtual ~CUdpEcho()
	{
	};

protected:
	virtual int onReceive(	const char	*p_data,
							int			data_len,
							string		str_peer_addr,
							uint16_t	peer_port)
	{
		LT_MSG(	m_LogHandle,
				(m_StrAid.Format("receive from %s %d", str_peer_addr.c_str(), peer_port)).c_str(), 0);
		sendTo(p_data, data_len, str_peer_addr, peer_port);
		return(ERR_OK);
	};

private:
	CLogHandle	m_LogHandle;
	CStrAid		m_StrAid;
};

#endif
