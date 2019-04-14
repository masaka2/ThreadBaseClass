/**
 * @file   CTcpSocket.h
 * @brief  ＴＣＰソケットクラス
 * 
 * このクラスの詳細は「使用上の注意、及び設計方針」で後述します。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/10/20 渡辺正勝    初版<BR>
 */

#ifndef CTcpSocket_h
#define CTcpSocket_h

#include "CThreadBase.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

////////////////////////////////////////////////////////////////////////////////
// 使用上の注意、及び設計方針
////////////////////////////////////////////////////////////////////////////////
/*
	１．このクラスはＴＣＰ通信機能を有します。リスナ機能は別クラスになります。
		派生クラスのベースとなる他に、そのまま使用することもできます。
		そのまま使用する場合は、setNoticeThread()関数で、
		通知先スレッドを設定してください。

	２．サーバ側とクライアント側双方の機能を持ちます。
		コンストラクタはデフォルトコンストラクタの他に、
		サーバ側用コンストラクタとクライアント側用コンストラクタがあります。
		デフォルトコンストラクタで生成した場合は、クライアント側かサーバ側か
		未定です。その後に setFD()関数（サーバ側）か setServerAddr()関数
		（クライアント側）でタイプを確定してください。

		サーバとクライアントが一体になっているので少し複雑です。
		当初は別クラスとしていましたが、下記の理由で一体にしました。
		・サーバとクライアントは接続方法が異なるだけで通信機能は同じ。
		・サーバとクライアントは一対の機能として実装されることが多く、
		  ベースクラスが一体になっている方が重複が避けられる。

	３．ソケットが切れてもスレッドの再生成は不要です。
		サーバ側は、新しいファイルディスクリプタを設定できます。
		クライアント側は、自動的に再接続を行います。

	４．CThreadBaseクラスから下記メンバ関数をオーバーライドしています。
		当クラスの派生クラスが、これらのメンバ関数をオーバーライドする場合は、
		その関数内で、当クラスのメンバ関数を呼び出してください。

			virtual int  onPreThreadCreate();
			virtual int  onThreadInitiate();
			virtual void onThreadTerminate();
			virtual void onTimer();
			virtual int  onMsg();
			virtual int  onEvent();
*/

////////////////////////////////////////////////////////////////////////////////
// 固定値、及びデフォルト値
////////////////////////////////////////////////////////////////////////////////
namespace EGSOCK_CONFIG {
	const static int MAX_BUF_LEN	= 65535;	// 受信バッファ長（ウィンドウ・サイズと同じ！）

	// 以下、クライアント側で使用
	const static int TIMER_ID	= INT_MAX - 1;	// 接続処理に使用するタイマ
	const static int CONNECT_T1	= 5000;	// 接続を試みる周期
	const static int CONNECT_T2	= 1000;	// エラー等で切断された時から次の接続までの周期
										// ０を指定すれば再接続しません。
}

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰソケットタイプ
////////////////////////////////////////////////////////////////////////////////
namespace EGSOCK_TYPE {
	const static int UNKNOWN	= 0;	// 未定（初期状態）
	const static int SERVER		= 1;	// Server側（Listenerによりacceptされたソケット）
	const static int CLIENT		= 2;	// Client側
}

////////////////////////////////////////////////////////////////////////////////
// 状態
////////////////////////////////////////////////////////////////////////////////
namespace EGSOCK_STS {
	const static int DISCONNECT	= 0;	// 未接続（初期状態）
	const static int CONNECT	= 1;	// 接続
}

////////////////////////////////////////////////////////////////////////////////
// エラーコード
////////////////////////////////////////////////////////////////////////////////
namespace EGSOCK_ERR {
	const static int OK					= 0;	// 正常
	const static int API_CALL			= 1;	// 主にソケット関連API呼び出しでエラー
	const static int AP_ILLEGAL_USE		= 2;	// ユーザ（派生先）の不正使用
	const static int SEND_DATA_WAS_LOST	= 3;	// （切断で）送信データが失われた！
}

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰソケットクラスが使用するメッセージクラス（スレッド間通信用）
////////////////////////////////////////////////////////////////////////////////
/*
	これらのメッセージクラスはデフォルト実装で使用します。
	メッセージが不要であったり、業務に特化したメッセージを使用する場合、
	デフォルト実装部分のメンバ関数をオーバーライドしてください。
*/

// 送信メッセージ（要求元 -> ＴＣＰソケットクラス）
class CTcpSocketSendMsg : public CThreadMsg
{
public:
	int		data_len;
	char	*p_data;

	CTcpSocketSendMsg(int _data_len=0, const void *_vp_data=NULL)
	: data_len(_data_len)
	, p_data(NULL)
	{
		if (data_len > 0) {
			p_data = new char[data_len];
			if (p_data && _vp_data) {
				memcpy(p_data, static_cast<const char*>(_vp_data), data_len);
			}
		}
	};
	virtual ~CTcpSocketSendMsg()
	{
		if (p_data) {
			delete [] p_data;
		}
	};
};

// ファイルディスクリプタ設定メッセージ（要求元 -> ＴＣＰソケットクラス）
// クライアント側で使用
class CTcpServerSetFdMsg : public CThreadMsg
{
public:
	int socketFD;

	CTcpServerSetFdMsg()
	:socketFD(-1)
	{};
	virtual ~CTcpServerSetFdMsg() {};
};

// 通知メッセージのベース
// ＴＣＰソケットクラスから通知先へのメッセージのベース
class CTcpSocket;
class CTcpSocketMsg : public CThreadMsg
{
public:
	CTcpSocket	*pTcpSocket;	// 送信元スレッド（自スレッド）

	CTcpSocketMsg()
	: pTcpSocket(NULL)
	{};
	virtual ~CTcpSocketMsg() {};
};

// 受信メッセージ（ＴＣＰソケットクラス -> 通知先）
class CTcpSocketReceiveMsg : public CTcpSocketMsg
{
public:
	int		data_len;
	char	*p_data;

	CTcpSocketReceiveMsg(int _data_len=0, const char *_p_data=NULL)
	: data_len(_data_len)
	, p_data(NULL)
	{
		if (data_len > 0) {
			p_data = new char[data_len];
			if (p_data && _p_data) {
				memcpy(p_data, _p_data, data_len);
			}
		}
	};
	virtual ~CTcpSocketReceiveMsg()
	{
		if (p_data) {
			delete [] p_data;
		}
	};
};

// 状態変化メッセージ（ＴＣＰソケットクラス -> 通知先）
class CTcpSocketChangeStatusMsg : public CTcpSocketMsg
{
public:
	int	new_status;
	int	pre_status;

	CTcpSocketChangeStatusMsg()
	: new_status(EGSOCK_STS::DISCONNECT)
	, pre_status(EGSOCK_STS::DISCONNECT)
	{};
	virtual ~CTcpSocketChangeStatusMsg() {};
};

// エラーメッセージ（ＴＣＰソケットクラス -> 通知先）
class CTcpSocketErrorMsg : public CTcpSocketMsg
{
public:
	int	error;
	int	_errno;	// アンダーバー付けないとエラーになる！

	CTcpSocketErrorMsg()
	: error(EGSOCK_ERR::OK)
	, _errno(0)
	{};
	virtual ~CTcpSocketErrorMsg() {};
};

////////////////////////////////////////////////////////////////////////////////
// ＴＣＰソケットクラス
////////////////////////////////////////////////////////////////////////////////
class CTcpSocket : public CThreadBase
{
public:

	// デフォルトコンストラクタ
	// ソケットタイプが未定になります。
	// クライアント側で使用する場合は、
	// スレッド起動前に setServerAddr()関数でサーバ側アドレスを設定してください。
	CTcpSocket()
	: m_type(EGSOCK_TYPE::UNKNOWN)
	, m_socketFD(-1)
	, m_status(EGSOCK_STS::DISCONNECT)
	, m_data_len(0)
	, m_pNoticeThread(NULL)
	, m_str_server_addr("")
	, m_server_port(0)
	, m_connect_T1(0)
	, m_connect_T2(0)
	, m_flags(0)
	{
		pthread_mutex_init(&m_mutex, NULL);
	};

	// サーバ側用コンストラクタ
	// 生成時に socketFD が未定の場合は -1 を設定してください。
	CTcpSocket(int socketFD, CThreadBase* pNoticeThread=NULL)
	: m_type(EGSOCK_TYPE::SERVER)
	, m_socketFD(socketFD)
	, m_status(EGSOCK_STS::DISCONNECT)
	, m_data_len(0)
	, m_pNoticeThread(pNoticeThread)
	, m_str_server_addr("")
	, m_server_port(0)
	, m_connect_T1(0)
	, m_connect_T2(0)
	, m_flags(0)
	{
		pthread_mutex_init(&m_mutex, NULL);
	};

	// クライアント側用コンストラクタ
	CTcpSocket(	string str_server_addr,
				uint16_t server_port,
				int connect_T1=EGSOCK_CONFIG::CONNECT_T1,
				int connect_T2=EGSOCK_CONFIG::CONNECT_T2,
				CThreadBase* pNoticeThread=NULL)
	: m_type(EGSOCK_TYPE::CLIENT)
	, m_socketFD(-1)
	, m_status(EGSOCK_STS::DISCONNECT)
	, m_data_len(0)
	, m_pNoticeThread(pNoticeThread)
	, m_str_server_addr(str_server_addr)
	, m_server_port(server_port)
	, m_connect_T1(connect_T1)
	, m_connect_T2(connect_T2)
	, m_flags(0)
	{
		pthread_mutex_init(&m_mutex, NULL);
	};

	virtual ~CTcpSocket()
	{
		closeSocket();	// stop()が呼び出されずにデストラクタが走った時のため。
		pthread_mutex_destroy(&m_mutex);
	};

	/*
		ファイルディスクリプタを設定します。
		サーバ側で使用します。
		ソケットタイプが未定の場合、この関数を呼び出した時点でサーバタイプとなります。
		起動後でもファイルディスクリプタを変更できます。
		ソケットが接続と切断を繰り返しても、その度に再起動する必要はありません。
	*/
	int setFD(int socketFD);

	/*
		サーバ側アドレスを設定します。
		クライアント側で使用します。
		ソケットタイプが未定の場合、この関数を呼び出した時点でクライアントタイプとなります。
		スレッド起動前に設定してください。
		コンストラクタで設定する場合は不要です。
	*/
	int setServerAddr(	string str_server_addr,
						uint16_t server_port,
						int connect_T1=EGSOCK_CONFIG::CONNECT_T1,
						int connect_T2=EGSOCK_CONFIG::CONNECT_T2);

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
	virtual int Send(const void *vp_data, int data_len);

	// 状態問い合わせ
	int getStatus()
	{
		int	status;
		pthread_mutex_lock(&m_mutex);
		status = m_status;
		pthread_mutex_unlock(&m_mutex);
		return(status);
	};

protected:
	/*
		カスタマイズを容易にするため、ほとんどのメンバ関数はオーバーライド可としてます。
	*/
	virtual int  onPreThreadCreate();
	virtual int  onThreadInitiate();
	virtual void onThreadTerminate();
	/*
		再接続にタイマを使用しています。
		派生先で onTimer() を使用する場合、
		派生元（このクラス）の onTimer() を呼び出さないと再接続しません。
	*/
	virtual void onTimer(int timer_id);
	virtual int onMsg(CThreadMsg *p_msg);
	virtual int onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds);
	/*
		onReceive()関数の *p_accept_len について！

		*p_accept_len はonReceive()内で受け付けたデータ長を意味します。
		onReceive()呼び出し前に、data_len と同じ値に初期化されます。
		そのため、onReceive()で意識的に更新しなければ、
		全データ受け付けたことになります。
		data_len より小さい値が返って来たら、
		残りのデータで onReceive()を再呼び出しします。
		ゼロが返って来たら、新たにソケットからデータ受信するまで、
		onReceive()呼び出しは発生しません。
		新たな受信が発生したときは、前回の残りデータと新データを連結して
		onReceive()を呼び出します。

		この仕様は、固定長読み出しを容易にするためです。

		例として、１０バイト毎に処理したいケースで説明します。
		ソケットから２５バイト受信したとします。
		data_len と *p_accept_len が ２５で onReceive()が呼ばれます。
		ここで、先頭の１０バイトを受け付け、*p_accept_len を１０にして復帰します。
		すると、data_len と *p_accept_len が １５で onReceive()が再度呼ばれます。
		次も、先頭の１０バイトを受け付け、*p_accept_len を１０にして復帰します。
		すると、data_len と *p_accept_len が ５で onReceive()が呼ばれます。
		１０バイトに満たないので、*p_accept_len を０にして復帰します。
		次に onReceive()が呼び出されるのは、新たなデータを受信してからとなります。
	*/
	virtual int onReceive(const char *p_data , int data_len, int *p_accept_len);
	virtual int onChangeStatus(int new_status, int pre_status);
	virtual int onError(int error, int _errno=0);

	int openSocket();
	int openSocketServer();
	int openSocketClient();
	int closeSocket(int timer=0);
	int changeStatus(int new_status);

	virtual int sendSocket(const char *p_data, int data_len);

	int		m_type;
	int		m_socketFD;
	int		m_status;

	char	m_buf[EGSOCK_CONFIG::MAX_BUF_LEN];
	int		m_data_len;

	CThreadBase*	m_pNoticeThread;	// 通知先スレッド

	// 以下、クライアント側で使用
	string		m_str_server_addr;
	uint16_t	m_server_port;
	int			m_connect_T1;
	int			m_connect_T2;

private:
	pthread_mutex_t	m_mutex;

	// 以下、クライアント側で使用
	int		m_flags;

};

#endif
