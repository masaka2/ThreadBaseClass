/**
 * @file   CLogThread.h
 * @brief  ログスレッドクラス
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/11/21 渡辺正勝    初版<BR>
 * 2006/07/14 渡辺正勝    ２版<BR>
 */

#ifndef CLogThread_h
#define CLogThread_h

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include "CThreadBase.h"
#include "CTimeVal.h"
#include "CStrAid.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// 使用方法など
////////////////////////////////////////////////////////////////////////////////
/*
	１．プライマリスレッド（main関数あたりかな）の先頭で、
		ログスレッドクラス（CLogThread）を生成、起動します。

		起動する前にログ出力先情報を設定します。
		コンストラクタで指定しない場合は、setParameter()関数を使用します。
		「ログ出力先ディレクトリ」と「ログファイル名プレフィックス」が
		共に空の場合は、標準出力に出力します。

	２．ログ出力するプログラムでは、ログハンドルクラス（CLogHandle）を生成して、
		そのインスタンスを第１パラメータにして、ログ出力マクロ（LT_TRACE等）を
		呼び出します。
		ログハンドルクラス（CLogHandle）は、スレッド毎、プログラム毎に、
		複数生成することになります。

		ログハンドルクラスは出力メッセージをログスレッドクラスに送信します。
		実際のログ出力はログスレッドクラスが行います。
		ログスレッドクラスが未起動の場合、エラーにはなりませんが、
		当然、ログ出力はなされません。

	３．プロセス内に複数のログスレッドを立ち上げ、出力先を振り分ける場合、
		２つ目以降はログスレッド番号を変えてください。
		コンストラクタで設定しない（出来ない）場合は、
		setThreadNo()関数を使用します。
		または、ファイル末尾のテンプレートを使用してログスレッド番号を変えます。
*/

////////////////////////////////////////////////////////////////////////////////
// メッセージ
////////////////////////////////////////////////////////////////////////////////

// ログメッセージ（ログスレッドに送る。）
class CLogThreadMsg : public CThreadMsg
{
public:
	string		m_strMsg;	// メッセージ
	int			m_nOption;	// ユーザ使用オプション（エラーレベル等）
	CTimeVal	m_time;		// 時刻
	pthread_t	m_pthread;	// スレッド識別子
	CLogThreadMsg(string strMsg="", int nOption=0, pthread_t pthread=0)
	: m_strMsg(strMsg)
	, m_nOption(nOption)
	, m_pthread(pthread)
	{
		m_time.setCurrent();
		if (m_pthread == 0) {
			m_pthread = pthread_self();
		}
	};
	virtual ~CLogThreadMsg() {};
};

////////////////////////////////////////////////////////////////////////////////
// ログスレッドクラス
////////////////////////////////////////////////////////////////////////////////
const static int DEFAULT_LOG_THREAD_NO = (INT_MAX-1);	// スレッド番号（プロセス内で一意）
class CLogThread : public CThreadBase
{
public:
	CLogThread(	int		nLogThreadNo	= DEFAULT_LOG_THREAD_NO,	// ログスレッド番号
				string	strDirPath		= "./",		// ログ出力先ディレクトリ
				string	strFilePrefix	= "log_",	// ログファイル名プレフィックス
				int		nMaxLine		= 1000,		// １ファイルあたりの最大行数
				int		nMaxFile		= 10)		// 最大ファイル数
	: m_nLogThreadNo	(nLogThreadNo)
	, m_strDirPath		(strDirPath)
	, m_strFilePrefix	(strFilePrefix)
	, m_nMaxLine		(nMaxLine)
	, m_nMaxFile		(nMaxFile)
	, m_nLine			(0)
	, m_pFile			(NULL)
	{};

	virtual ~CLogThread()
	{
		closeFile();
	};

	// ログ出力先情報を設定します。
	// スレッド起動前に設定してください。
	int setParameter(	string	strDirPath		= "./",		// ログ出力先ディレクトリ
						string	strFilePrefix	= "log_",	// ログファイル名プレフィックス
						int		nMaxLine		= 1000,		// １ファイルあたりの最大行数
						int		nMaxFile		= 10)		// 最大ファイル数
	{
		// 起動後か？
		if (get_pthread() != 0) {
			return(ERR_CONTEXT);
		}
		m_strDirPath	= strDirPath	;
		m_strFilePrefix	= strFilePrefix	;
		m_nMaxLine		= nMaxLine		;
		m_nMaxFile		= nMaxFile		;
		return(ERR_OK);
	};

	// ログスレッド番号を設定（変更）します。
	// スレッド起動前に設定してください。
	int setThreadNo(int nLogThreadNo)
	{
		// 起動後か？
		if (get_pthread() != 0) {
			return(ERR_CONTEXT);
		}
		m_nLogThreadNo = nLogThreadNo;
		return(ERR_OK);
	};

	// ログスレッドのポインタを返す。
	// 一度取れたら、その後は呼ばないで！
	static CLogThread* getLogThread(int nLogThreadNo=DEFAULT_LOG_THREAD_NO);

protected:
	virtual int onPreThreadCreate();
	virtual int onMsg(CThreadMsg *p_msg);

	virtual void openFile(struct tm *ptm);
	virtual void closeFile();

	int		m_nLogThreadNo;
	string	m_strDirPath;
	string	m_strFilePrefix;
	int		m_nMaxLine;
	int		m_nMaxFile;

	int			m_nLine;
	ofstream	*m_pFile;

	// 出力先の切り替え
	// 毎回呼ばず、予め設定する方法を模索したがうまくいかない。
	// 標準出力は排他処理があり時間がかかるらしい。
	// そのため、ファイルを標準出力に統合すると、ファイル出力が遅くなるらしい。
	virtual ostream& out() {
		// キャストなしだとエラーにするコンパイラもある。
		// return(m_pFile ? *m_pFile : std::cout);
		return(m_pFile ? *(static_cast<ostream*>(m_pFile)) : std::cout);
	};

	CStrAid	m_StrAid;

private:
};

////////////////////////////////////////////////////////////////////////////////
// ログハンドルクラス（ユーザはこれを使う！）
////////////////////////////////////////////////////////////////////////////////
class CLogHandle
{
public:
	CLogHandle(int nLogThreadNo = DEFAULT_LOG_THREAD_NO)	// ログスレッド番号
	: m_nLogThreadNo(nLogThreadNo)
	, m_pLogThread(NULL)
	{};
	virtual ~CLogHandle() {};

	// ログスレッド番号を設定（変更）します。
	// ポインタ取得前に設定してください。
	int setThreadNo(int nLogThreadNo)
	{
		// ポインタ取得後か？
		if (m_pLogThread != NULL) {
			return(CThreadBase::ERR_CONTEXT);
		}
		m_nLogThreadNo = nLogThreadNo;
		return(CThreadBase::ERR_OK);
	};

	// 書込要求
	void write(string strMsg, int nOption=0)
	{
		if (m_pLogThread == NULL) {
			m_pLogThread = CLogThread::getLogThread(m_nLogThreadNo);
			if (m_pLogThread == NULL) {
				return;
			}
		}
		CLogThreadMsg *pLogThreadMsg = new CLogThreadMsg(strMsg, nOption);
		m_pLogThread->postMsg(pLogThreadMsg);
	};

	CStrAid	m_StrAid;

protected:
	int	m_nLogThreadNo;
	CLogThread*	m_pLogThread;

private:
};

////////////////////////////////////////////////////////////////////////////////
// ユーザ使用マクロ（デフォルト）
// デフォルトで足りない場合、独自にマクロを定義してください。
// __FILE__マクロと__LINE__マクロを有効にするには #define を使うしかありません。
// クラス内で inline 関数にしたが駄目でした。
////////////////////////////////////////////////////////////////////////////////
#define LT_TRACE(handle){\
	handle.write(handle.m_StrAid.Format(\
	"%s[%d]", __FILE__,__LINE__));\
}
#define LT_MSG(handle, str, option){\
	handle.write(handle.m_StrAid.Format(\
	"%s[%d] %s", __FILE__,__LINE__, str), option);\
}
#define LT_DEC(handle, name, val, option){\
	handle.write(handle.m_StrAid.Format(\
	"%s[%d] %s=%d", __FILE__,__LINE__, name, val), option);\
}
#define LT_HEX(handle, name, val, option){\
	handle.write(handle.m_StrAid.Format(\
	"%s[%d] %s=%x", __FILE__,__LINE__, name, val), option);\
}
#define LT_STR(handle, name, str, option){\
	handle.write(handle.m_StrAid.Format(\
	"%s[%d] %s=%s", __FILE__,__LINE__, name, str), option);\
}

////////////////////////////////////////////////////////////////////////////////
// 独自ログスレッドクラスとログハンドルクラスのテンプレート
////////////////////////////////////////////////////////////////////////////////
template <int N>
class CMyLogThread : public CLogThread
{
public:
	CMyLogThread()
	: CLogThread(N)
	{};
	virtual ~CMyLogThread() {};
};

template <int N>
class CMyLogHandle : public CLogHandle
{
public:
	CMyLogHandle()
	: CLogHandle(N)
	{};
	virtual ~CMyLogHandle() {};
};

#endif

