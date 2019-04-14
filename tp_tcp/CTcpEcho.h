/**
 * @file   CTcpEcho.h
 * @brief  ＴＣＰエコークラス（テスト用）
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/10/20 渡辺正勝    初版<BR>
 */

#ifndef CTcpEcho_h
#define CTcpEcho_h

#include "CTcpListener.h"
#include "CTcpSocket.h"
#include "CLogThread.h"
#include "CStrAid.h"

////////////////////////////////////////////////////////////////////////////////
// 固定値
////////////////////////////////////////////////////////////////////////////////
namespace EGSOCK_ECHO {
	const static int MAX_PORT	= 5;	// 最大ポート数
} // namespace EGSOCK_ECHO

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰエコークラス（テスト用）
////////////////////////////////////////////////////////////////////////////////
class CTcpEcho : public CTcpListener
{
public:
	CTcpEcho(uint16_t port=0)
	: CTcpListener(port)
	{
	};

	virtual ~CTcpEcho()
	{
	};

protected:
	virtual int  onThreadInitiate();
	virtual void onThreadTerminate();
	virtual int  onMsg(CThreadMsg *p_msg);
	virtual int  onConnect(int connectFD, struct sockaddr_in &client_addr);

private:
	CTcpSocket	m_TcpSocket[EGSOCK_ECHO::MAX_PORT];
	CLogHandle	m_LogHandle;
	CStrAid		m_StrAid;
};

#endif
