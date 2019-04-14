/**
 * @file   CTcpListener.h
 * @brief  ＴＣＰリスナベースクラス
 * 
 * このクラスはＴＣＰソケットのリスナです。
 * 派生クラスのベースとなる他に、そのまま使用することもできます。
 * 
 * 実際の通信はＴＣＰソケットクラスを使用してください。
 * 
 * CThreadBaseクラスから下記メンバ関数をオーバーライドしています。
 * 当クラスの派生クラスが、これらのメンバ関数をオーバーライドする場合は、
 * その関数内で、当クラスのメンバ関数を呼び出してください。
 *   
 *   virtual int  onPreThreadCreate();
 *   virtual int  onThreadInitiate();
 *   virtual void onThreadTerminate();
 *   virtual int  onEvent();
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/10/20 渡辺正勝    初版<BR>
 */

#ifndef CTcpListener_h
#define CTcpListener_h

#include "CThreadBase.h"
#include <sys/socket.h>
#include <arpa/inet.h>

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰリスナ接続メッセージクラス（スレッド間通信用）
////////////////////////////////////////////////////////////////////////////////
/*
	デフォルトの実装では、通知先スレッドが登録されていると、
	接続時にこのメッセージで通知します。
*/
class CTcpListener;
class CTcpListenerConnectMsg : public CThreadMsg
{
public:
	CTcpListener	*pTcpListener;	// 通知元リスナスレッド
	uint16_t		listen_port;	// リスナのポート
					// 上の２つは複数リスナ使用時にどこから来たか識別するため！
	int	connectFD;
	struct sockaddr_in	client_addr;
	string	ip_addr;

	CTcpListenerConnectMsg()
	{};

	virtual ~CTcpListenerConnectMsg() {};
};

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰリスナベースクラス
////////////////////////////////////////////////////////////////////////////////
class CTcpListener : public CThreadBase
{
public:
	CTcpListener(uint16_t port=0, CThreadBase* pNoticeThread=NULL)
	: m_port(port)
	, m_listenFD(-1)
	, m_pNoticeThread(pNoticeThread)
	{
	};

	virtual ~CTcpListener()
	{
		closeSocket();	// stop()が呼び出されずにデストラクタが走った時のため。
	};

	// スレッド起動前に設定してください。
	// コンストラクタで設定する場合は不要です。
	int setPort(uint16_t port)
	{
		m_port = port;
		return(ERR_OK);
	};

	// スレッド起動前に設定してください。
	// コンストラクタで設定する場合は不要です。
	int setNoticeThread(CThreadBase* pNoticeThread)
	{
		m_pNoticeThread = pNoticeThread;
		return(ERR_OK);
	};

protected:
	virtual int  onPreThreadCreate();
	virtual int  onThreadInitiate();
	virtual void onThreadTerminate();
	virtual int  onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds);

	// 接続時のデフォルトの実装です。
	virtual int  onConnect(int connectFD, struct sockaddr_in &client_addr);

	int openSocket();
	int closeSocket();

	uint16_t	m_port;
	int			m_listenFD;
	CThreadBase*	m_pNoticeThread;	// 通知先スレッド

private:

};

#endif
