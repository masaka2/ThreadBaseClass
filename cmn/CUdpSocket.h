/**
 * @file   CUdpSocket.h
 * @brief  ＵＤＰソケットクラス
 * 
 * このクラスの詳細は「使用上の注意、及び設計方針」で後述します。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2006/06/02 渡辺正勝    初版<BR>
 */

#ifndef CUdpSocket_h
#define CUdpSocket_h

#include "CThreadBase.h"
#include <sys/socket.h>
#include <arpa/inet.h>

////////////////////////////////////////////////////////////////////////////////
// 使用上の注意、及び設計方針
////////////////////////////////////////////////////////////////////////////////
/*
	１．このクラスはＵＤＰ通信機能を有します。
		派生クラスのベースとなる他に、そのまま使用することもできます。
		そのまま使用する場合は、setNoticeThread()関数で、
		通知先スレッドを設定してください。

	２．ポートをバインドできます。
		コンストラクタ、または setBindPort()関数で指定してください。

	３．CThreadBaseクラスから下記メンバ関数をオーバーライドしています。
		当クラスの派生クラスが、これらのメンバ関数をオーバーライドする場合は、
		その関数内で、当クラスのメンバ関数を呼び出してください。

			virtual int  onThreadInitiate();
			virtual void onThreadTerminate();
			virtual int  onMsg();
			virtual int  onEvent();
*/

////////////////////////////////////////////////////////////////////////////////
// 固定値、及びデフォルト値
////////////////////////////////////////////////////////////////////////////////
namespace EGUDP_CONFIG {
	const static int MAX_BUF_LEN	= 65535;	// 受信バッファ長（ウィンドウ・サイズと同じ！）
}

////////////////////////////////////////////////////////////////////////////////
// エラーコード
////////////////////////////////////////////////////////////////////////////////
namespace EGUDP_ERR {
	const static int OK			= 0;	// 正常
	const static int API_CALL	= 1;	// 主にソケット関連API呼び出しでエラー
}

////////////////////////////////////////////////////////////////////////////////
// ＵＤＰソケットクラスが使用するメッセージクラス（スレッド間通信用）
////////////////////////////////////////////////////////////////////////////////
/*
	これらのメッセージクラスはデフォルト実装で使用します。
	メッセージが不要であったり、業務に特化したメッセージを使用する場合、
	デフォルト実装部分のメンバ関数をオーバーライドしてください。
	派生したクラス内で直接入出力する場合は、これらのメッセージは不要になります。
*/

// ＵＤＰメッセージのベース
class CUdpSocketMsg : public CThreadMsg
{
public:
	string		str_peer_addr;	// 相手アドレス
	uint16_t	peer_port;		// 相手ポート

	CUdpSocketMsg(string _str_peer_addr="", uint16_t _peer_port=0)
	: str_peer_addr(_str_peer_addr)
	, peer_port(_peer_port)
	{};
	virtual ~CUdpSocketMsg() {};
};

// 送信メッセージ（要求元 -> ＵＤＰソケットクラス）
class CUdpSocketSendMsg : public CUdpSocketMsg
{
public:
	int		data_len;
	char	*p_data;

	CUdpSocketSendMsg(int _data_len=0, const void *_vp_data=NULL, string _str_peer_addr="", uint16_t _peer_port=0)
	: CUdpSocketMsg(_str_peer_addr, _peer_port)
	, data_len(_data_len)
	, p_data(NULL)
	{
		if (data_len > 0) {
			p_data = new char[data_len];
			if (p_data && _vp_data) {
				memcpy(p_data, static_cast<const char*>(_vp_data), data_len);
			}
		}
	};
	virtual ~CUdpSocketSendMsg()
	{
		if (p_data) {
			delete [] p_data;
		}
	};
};

// 通知メッセージのベース
// ＵＤＰソケットクラスから通知先へのメッセージのベース
class CUdpSocket;
class CUdpSocketNoticeMsg : public CUdpSocketMsg
{
public:
	CUdpSocket	*pUdpSocket;	// 送信元スレッド（自スレッド）

	CUdpSocketNoticeMsg(string _str_peer_addr="", uint16_t _peer_port=0)
	: CUdpSocketMsg(_str_peer_addr, _peer_port)
	, pUdpSocket(NULL)
	{};
	virtual ~CUdpSocketNoticeMsg() {};
};

// 受信メッセージ（ＵＤＰソケットクラス -> 通知先）
class CUdpSocketReceiveMsg : public CUdpSocketNoticeMsg
{
public:
	int		data_len;
	char	*p_data;

	CUdpSocketReceiveMsg(int _data_len=0, const char *_p_data=NULL, string _str_peer_addr="", uint16_t _peer_port=0)
	: CUdpSocketNoticeMsg(_str_peer_addr, _peer_port)
	, data_len(_data_len)
	, p_data(NULL)
	{
		if (data_len > 0) {
			p_data = new char[data_len];
			if (p_data && _p_data) {
				memcpy(p_data, _p_data, data_len);
			}
		}
	};
	virtual ~CUdpSocketReceiveMsg()
	{
		if (p_data) {
			delete [] p_data;
		}
	};
};

// エラーメッセージ（ＵＤＰソケットクラス -> 通知先）
// エラーによっては、相手アドレスと相手ポートが設定されないこともあります。
class CUdpSocketErrorMsg : public CUdpSocketNoticeMsg
{
public:
	int	error;
	int	_errno;	// アンダーバー付けないとエラーになる！

	CUdpSocketErrorMsg(string _str_peer_addr="", uint16_t _peer_port=0)
	: CUdpSocketNoticeMsg(_str_peer_addr, _peer_port)
	, error(EGUDP_ERR::OK)
	, _errno(0)
	{};
	virtual ~CUdpSocketErrorMsg() {};
};

////////////////////////////////////////////////////////////////////////////////
// ＵＤＰソケットクラス
////////////////////////////////////////////////////////////////////////////////
class CUdpSocket : public CThreadBase
{
public:

	// バインドするポートと通知先スレッドを指定できます。
	CUdpSocket(uint16_t bind_port=0, CThreadBase* pNoticeThread=NULL)
	: m_socketFD(-1)
	, m_bind_port(bind_port)
	, m_pNoticeThread(pNoticeThread)
	, m_bool_append(false)
	{
	};

	virtual ~CUdpSocket()
	{
		closeSocket();	// stop()が呼び出されずにデストラクタが走った時のため。
	};

	/*
		バインドするポートを設定します。
		スレッド起動前に設定してください。
		コンストラクタで設定する場合は不要です。
	*/
	int setBindPort(uint16_t bind_port)
	{
		if (bind_port == 0) {
			return(ERR_PARAM);
		}
		// 起動後か？
		if (get_pthread() != 0) {
			return(ERR_CONTEXT);
		}
		m_bind_port = bind_port;
		return(ERR_OK);
	};

	/*
		通知先スレッドを設定する。
		スレッド起動前に設定してください。
		コンストラクタで設定する場合は不要です。
	*/
	int setNoticeThread(CThreadBase* pNoticeThread)
	{
		// 起動前か？
		if (get_pthread() != 0) {
			return(ERR_CONTEXT);
		}
		m_pNoticeThread = pNoticeThread;
		return(ERR_OK);
	};

	// データ送信要求
	// 派生したクラス内で直接入出力する場合は、
	// sendTo()関数を使用してください。メッセージ経由より効率的です。
	virtual int Send(	const void	*vp_data,
						int			data_len,
						string		str_peer_addr,
						uint16_t	peer_port)
	{
		if (vp_data == NULL) {
			return(ERR_PARAM);
		}
		if (data_len <= 0) {
			return(ERR_PARAM);
		}
		if (str_peer_addr == "") {
			return(ERR_PARAM);
		}
		if (peer_port == 0) {
			return(ERR_PARAM);
		}
		CUdpSocketSendMsg *pUdpSocketSendMsg = 
			new CUdpSocketSendMsg(data_len, vp_data, str_peer_addr, peer_port);
		int ret = postMsg(pUdpSocketSendMsg);
		return(ret);
	};

protected:
	/*
		カスタマイズを容易にするため、ほとんどのメンバ関数はオーバーライド可としてます。
	*/
	virtual int  onThreadInitiate();
	virtual void onThreadTerminate();
	virtual int  onMsg(CThreadMsg *p_msg);
	virtual int  onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds);

	// 派生したクラス内で直接入出力する場合は、
	// Send()関数ではなく、直接これを使用してください。メッセージ経由より効率的です。
	virtual int sendTo(	const void	*vp_data,
						int			data_len,
						string		str_peer_addr,
						uint16_t	peer_port);

	// 派生したクラス内で直接入出力する場合は、
	// この関数をオーバーライドして使用してください。メッセージ経由より効率的です。
	virtual int onReceive(	const char	*p_data,
							int			data_len,
							string		str_peer_addr,
							uint16_t	peer_port);

	virtual int onError(int			error,
						int			_errno=0,
						string		str_peer_addr="",
						uint16_t	peer_port=0);

	int openSocket();
	int closeSocket();

	int			m_socketFD;
	uint16_t	m_bind_port;
	CThreadBase*	m_pNoticeThread;	// 通知先スレッド
	bool	m_bool_append;
	char	m_buf[EGUDP_CONFIG::MAX_BUF_LEN];

private:

};

#endif
