/**
 * @file   CThreadBase.h
 * @brief  スレッドベースクラス及びその構成要素クラス
 *
 * スレッドベースクラスは、スレッドセーフなメッセージキューと
 * タイマ機能が付いたスレッドクラスの基本クラスです。
 * 
 * このファイルは、スレッドベースクラスとその構成要素である、
 * 下記クラスの定義をします。
 * 
 * ・スレッドメッセージベースクラス<BR>
 * ・スレッド終了メッセージクラス<BR>
 * ・スレッドキュークラス<BR>
 * ・タイマ制御ブロッククラス<BR>
 * ・タイマ制御ブロックリストクラス<BR>
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/09/06 渡辺正勝    初版<BR>
 */

#ifndef CThreadBase_h
#define CThreadBase_h

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <deque>
#include <list>
#include <vector>
#include <algorithm>
#include "CTimeVal.h"
#include "CFileDescriptor.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// スレッドメッセージベースクラスとその派生クラス（スレッド間通信用）
////////////////////////////////////////////////////////////////////////////////

/**
 * @class CThreadMsg CThreadBase.h
 * @brief スレッドメッセージベースクラス
 *
 * スレッド間通信に使用するメッセージは、このクラスから派生させます。
 * 
 */
class CThreadMsg
{
public:
	/// @brief コンストラクタ
	CThreadMsg()
	{};

	/// @brief デストラクタ
	virtual ~CThreadMsg() {};
};

/**
 * @class CStopMsg CThreadBase.h
 * @brief スレッド終了メッセージクラス
 *
 * スレッド終了を要求するメッセージ。<BR>
 * スレッドベースクラスは、このメッセージを受信するとスレッドを終了させます。<BR>
 * スレッド終了メッセージは、通常 CThreadBase::stop()関数内で送信されます。
 * 
 */
class CStopMsg : public CThreadMsg
{
public:
	/// @brief スレッド終了時の返り値
	void*	m_vp_ret;

	/// @brief コンストラクタ（スレッド終了時の返り値付き）
	CStopMsg(void* vp_ret=NULL)
	: m_vp_ret(vp_ret)
	{};

	/// @brief デストラクタ
	virtual ~CStopMsg() {};
};

////////////////////////////////////////////////////////////////////////////////
// スレッドキュークラス
////////////////////////////////////////////////////////////////////////////////

/**
 * @class CThreadQueue CThreadBase.h
 * @brief スレッドキュークラス
 * 
 * スレッドセーフなキュークラスです。<BR>
 * スレッドメッセージベースクラスのポインタをキューイングします。<BR>
 * パラメータの指定により、先頭にもキューイングできます。（優先処理）
 * 
 */
class CThreadQueue
{
public:
	/// @brief コンストラクタ
	CThreadQueue();

	/// @brief デストラクタ
	virtual ~CThreadQueue();

	/**
	 * @brief スレッドメッセージのポインタを登録する。
	 *
	 * スレッドキューの先頭又は末尾に登録する。
	 *
	 * @param	p_msg		スレッドメッセージのポインタ
	 * @param	bool_front	真の場合は先頭に登録
	 * @retval	0			正常
	 * @retval	0以外		異常
	 */
	int  put(CThreadMsg *p_msg, bool bool_front=false);

	/**
	 * @brief スレッドメッセージのポインタを取り出す。
	 *
	 * スレッドキューの先頭からメッセージのポインタを取り出す。
	 *
	 * @param	pp_msg		スレッドメッセージのポインタ
	 * @retval	0			正常
	 * @retval	0以外		異常
	 */
	int  get(CThreadMsg **pp_msg);

	/**
	 * @brief キューイングされている全てのメッセージを削除します。
	 *
	 * @param	なし
	 * @retval	なし
	 */
	void removeAll();

	/**
	 * @brief スレッドキューが空か否かを返す。
	 *
	 * @param	なし
	 * @retval	0		スレッドキューが空でない。
	 * @retval	0以外	スレッドキューが空です。
	 */
	bool empty();

private:
	/// @brief 両頭待ち行列
	deque<CThreadMsg*>	m_dq_p_msg;

	/// @brief ミューテック
	pthread_mutex_t		m_mutex;
};

////////////////////////////////////////////////////////////////////////////////
// タイマ制御ブロッククラス、タイマ制御ブロックリストクラス
////////////////////////////////////////////////////////////////////////////////

/**
 * @class CTimerCB CThreadBase.h
 * @brief タイマ制御ブロッククラス
 * 
 * タイマ処理に必要な情報をまとめています。
 * 
 */
class CTimerCB
{
public:
	/// @brief 目標時刻
	CTimeVal	m_target_time;

	/// @brief タイマID
	int		m_timer_id;

	/// @brief 周期（周期起動で使用）
	int		m_msec_period;

	/// @brief 削除フラグ
	bool	m_remove;

	/// @brief コンストラクタ（デフォルト値付き）
	CTimerCB(int msec_elapsed_time=0, int timer_id=0, int msec_period=0)
	: m_timer_id(timer_id)
	, m_msec_period(msec_period)
	, m_remove(false)
	{
		m_target_time.setCurrent();
		m_target_time.addMS(msec_elapsed_time);
	};

	/// @brief デストラクタ
	virtual ~CTimerCB()
	{
	};

	/**
	 * @brief 削除の可否を返す。
	 * 
	 * STLのアルゴリズム（remove_if）で使用する。
	 * 
	 * @param	x		タイマ制御ブロック
	 * @retval	0		削除不可
	 * @retval	0以外	削除可
	 */
	static bool is_remove(const CTimerCB& x)
	{
		return(x.m_remove);
	}

	/**
	 * @brief 時刻の比較をする。
	 * 
	 * @param	x		比較対象のタイマ制御ブロック
	 * @retval	0		比較対象と等しいか大きい
	 * @retval	0以外	比較対象より小さい
	 */
	bool operator<(const CTimerCB& x) const
	{
		return(m_target_time < x.m_target_time);
	}
};

/**
 * @class CTimerCBList CThreadBase.h
 * @brief タイマ制御ブロックリストクラス
 * 
 * タイマ制御ブロックをタイムアウト順に繋げて管理する。
 * 
 */
class CTimerCBList
{
public:
	/// @brief コンストラクタ
	CTimerCBList();

	/// @brief デストラクタ
	virtual ~CTimerCBList();

	/**
	 * @brief タイマ制御ブロックを作り、タイムアウト順にリストに登録する。
	 * 
	 * @param	msec_elapsed_time	タイムアウトまでの経過時間（ミリ秒単位）
	 * @param	timer_id			タイマID
	 * @param	msec_period			周期（周期起動で使用／ミリ秒単位）
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  set(int msec_elapsed_time, int timer_id=0, int msec_period=0);

	/**
	 * @brief タイマ制御ブロックを、タイムアウト順にリストに登録する。
	 * 
	 * @param	rTimerCB	タイマ制御ブロック
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  set(CTimerCB &rTimerCB);

	/**
	 * @brief リストからタイマ制御ブロックを削除する。
	 * 
	 * @param	timer_id	削除するタイマID。-1であれば全て削除する。
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  cancel(int timer_id=0);	// timer_id が (-1)であれば全キャンセル

	/**
	 * @brief リスト先頭のタイマ制御ブロックがタイムアウトしているか否かを返す。
	 * 
	 * @param	p_timer_id	タイムアウトしたタイマIDを返す。
	 * @retval	0		タイムアウトしていない
	 * @retval	0以外	タイムアウト
	 */
	bool timeout(int *p_timer_id=NULL);

	/**
	 * @brief 次にタイムアウトする目標時刻を返す。
	 * 
	 * @param	なし
	 * @retval	0		タイマ登録がない
	 * @retval	0以外	次にタイムアウトする目標時刻
	 */
	CTimeVal next_time();

private:
	/// @brief タイマ制御ブロックのリスト
	list<CTimerCB>	m_list_TCB;

	/// @brief ミューテック
	pthread_mutex_t	m_mutex;
};

////////////////////////////////////////////////////////////////////////////////
// スレッドベースクラス（キュー、タイマ付き）
////////////////////////////////////////////////////////////////////////////////

/**
 * @class CThreadBase CThreadBase.h
 * @brief スレッドベースクラス（キュー、タイマ付き）
 * 
 * スレッドベースクラスは、スレッドセーフなメッセージキューと
 * タイマ機能が付いたスレッドクラスの基本クラスです。
 * スレッドベースクラスのリターンコードは原則的に０が正常です。
 * （POSIX pthread 関数にあわせています。）
 * 
 */
class CThreadBase
{
public:
	/// @brief エラーコード
	enum {
		ERR_OK			=  0,	///< 正常
		ERR_PARAM		= -1,	///< パラメータエラー
		ERR_CONTEXT		= -2,	///< コンテキストエラー
		ERR_BUSY		= -3,	///< ビジー状態
		ERR_TERMINATE	= -4,	///< 終了状態
		ERR_RESOURCE	= -5,	///< リソース不足
		ERR_SYSTEM		= -6	///< システムコールで異常
	};

	/// @brief コンストラクタ
	CThreadBase();

	/// @brief デストラクタ
	virtual ~CThreadBase();

	/**
	 * @brief スレッド属性を設定する。
	 * 
	 * start()実行前に呼び出して下さい。
	 * スレッド番号はプロセス内で重複なく指定してください。
	 * 
	 * @param	thread_no		スレッド番号（インスタンス管理で使用）
	 * @param	parent			親スレッドのポインタ（親スレッドへの通知で使用）
	 * @param	p_pthread_attr	スレッド属性（pthread_createのパラメータ）
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  setAttribute(const int thread_no=(-1), CThreadBase* parent=NULL, pthread_attr_t* p_pthread_attr=NULL);

	/**
	 * @brief スレッドを起動する。
	 * 
	 * @param	なし
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  start();

	/**
	 * @brief スレッドを終了する。
	 * 
	 * @param	bool_join			スレッド終了を待つ（pthread_join()実行）か否か
	 * @param	bool_immediately	キューイングされたメッセージを棄て、直ちに終了するか否か
	 * @param	vp_ret				スレッド終了時の返り値
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  stop(bool bool_join=true, bool bool_immediately=false, void* vp_ret=NULL);

	/**
	 * @brief スレッド終了を待つ。
	 * 
	 * stop()でスレッド終了を待たなかった時に使用します。
	 * 以下の様な用途があります。
	 * １．複数のスレッドを速やかに終了させるために、
	 * 　　個々のstop()呼び出しでスレッド終了を待たず、後でjoin()を呼び出す。
	 * ２．自律的に終了する子スレッドの終了を、親スレッドが待ち合わせる場合に使用する。
	 * 
	 * @param	なし
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  join();

	/**
	 * @brief スレッドにメッセージをキューイングする。
	 * 
	 * ここで渡されたメッセージは、本クラスが責任を持って解放（delete）します。
	 * 異常で復帰した場合も、本関数内で解放します。
	 * これは使用者の負荷を低減しメモリリークを防ぐための仕様です。
	 * 
	 * キューイングされたメッセージは、 onMsg()関数で順番に通知されます。
	 * 
	 * @param	pp_msg			スレッドメッセージのポインタ
	 * @param	bool_high_prior	高優先（先頭にキューイング）
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	virtual int  postMsg(CThreadMsg *p_msg, bool bool_high_prior=false);

	/**
	 * @brief スレッド識別子を返す。
	 * 
	 * @param	なし
	 * @retval	スレッド識別子
	 */
	pthread_t get_pthread() { return(m_pthread_copy); };

	/**
	 * @brief スレッドが活動状態か否かを返す。
	 *
	 * @param	なし
	 * @retval	0		非活動状態
	 * @retval	0以外	活動状態
	 */
	bool active() { return((m_pthread != 0) && (!m_bool_shutdown)); };

	/**
	 * @brief スレッド番号を返す。
	 * 
	 * スレッド番号は本クラスのインスタンス管理で使用する番号です。
	 * setAttribute()で明示的に指定しない場合は、-1が返ります。
	 * 
	 * @param	なし
	 * @retval	スレッド番号
	 */
	int  get_thread_no() { return(m_thread_no); };

	/**
	 * @brief 親スレッドのポインタを返す。
	 * 
	 * setAttribute()で明示的に指定しない場合は、NULLが返ります。
	 * 
	 * @param	なし
	 * @retval	スレッド番号
	 */
	CThreadBase* get_parent() { return(m_parent); };

	/// @brief スレッド状態
	enum {
		STS_UNKNOWN		= 0,	///< 不明・スレッド管理テーブルに未登録（スレッド番号未設定）
		STS_READY		= 1,	///< レディ状態
		STS_RUNNING		= 2,	///< 動作中
		STS_SHUTDOWN	= 3,	///< シャットダウン中
		STS_STOP		= 4,	///< 停止状態
		STS_DESTROY		= 5		///< 削除済み
	};

	/**
	 * @brief スレッド番号からインスタンスを返す。
	 * 
	 * スレッド番号をキーに、
	 * スレッド管理テーブルから該当インスタンスのポインタを返す。
	 * 
	 * @param	thread_no		スレッド番号
	 * @param	p_thread_status	スレッド状態
	 * @retval	インスタンスのポインタ
	 */
	static CThreadBase *getInstance(const int thread_no, int* p_thread_status=NULL);

	/**
	 * @brief 登録位置からインスタンスを返す。
	 * 
	 * スレッド管理テーブルの登録位置からインスタンスのポインタを返す。
	 * 
	 * @param	index			登録位置
	 * @param	p_thread_status	スレッド状態
	 * @retval	インスタンスのポインタ
	 */
	static CThreadBase *getInstanceByIndex(const size_t index, int* p_thread_status=NULL);

protected:
	/**
	 * @brief スレッド生成前に呼び出される。
	 * 
	 * start()関数内から、pthread_create()実行前に呼び出されます。
	 * スレッド生成前にやるべき処理があれば、本関数をオーバーライドします。
	 * 本関数が異常で復帰した場合、pthread_create()実行を止め、
	 * start()関数も異常で復帰します。
	 * 尚、本関数はスレッド生成前なので、親スレッドの実行権で動作します。
	 * 
	 * @param	なし
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	virtual int  onPreThreadCreate()	{ return(ERR_OK); }

	/**
	 * @brief 自スレッド起動直後に呼び出される。
	 * 
	 * 自スレッド起動直後にやるべき処理があれば、本関数をオーバーライドします。
	 * 本関数が異常で復帰した場合、自スレッドを終了させます。
	 * 
	 * @param	なし
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	virtual int  onThreadInitiate()		{ return(ERR_OK); }

	/**
	 * @brief 自スレッド終了直前に呼び出される。
	 * 
	 * 自スレッド終了直前にやるべき処理があれば、本関数をオーバーライドします。
	 * 
	 * @param	なし
	 * @retval	なし
	 */
	virtual void onThreadTerminate()	{ return; }

	/**
	 * @brief スレッド終了後に呼び出される。
	 * 
	 * stop()関数又は join()関数内から、pthread_join()実行後に呼び出されます。
	 * スレッド終了後にやるべき処理があれば、本関数をオーバーライドします。
	 * stop()関数又は join()関数で「スレッド終了待ち」を行わない場合、
	 * 本関数は呼び出されません。
	 * 尚、本関数はスレッド終了後なので、親スレッドの実行権で動作します。
	 * 
	 * @param	なし
	 * @retval	なし
	 */
	virtual void onPostThreadJoin()		{ return; }

	/**
	 * @brief メッセージ受信時に呼び出される。
	 * 
	 * メッセージ受信時にやるべき処理があれば、本関数をオーバーライドします。
	 * スレッドメッセージの解放（delete）はベースクラスで行います。
	 * 本関数内で解放（delete）しないで下さい。
	 * 
	 * 尚、スレッド終了メッセージ（ CStopMsg::）は
	 * ベースクラスで処理するため、本関数は呼び出されません。
	 * 
	 * @param	p_msg	スレッドメッセージのポインタ
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	virtual int onMsg(CThreadMsg *p_msg)	{ return(ERR_OK); }

	/**
	 * @brief タイマを設定する。
	 * 
	 * @param	msec_elapsed_time	タイムアウトまでの経過時間（ミリ秒単位）
	 * @param	timer_id			タイマID
	 * @param	msec_period			周期（周期起動で使用／ミリ秒単位）
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int  setTimer(int msec_elapsed_time, int timer_id=0, int msec_period=0)
	{
		return(m_TimerCBList.set(msec_elapsed_time, timer_id, msec_period));
	}

	/**
	 * @brief タイマをキャンセルする。
	 * 
	 * @param	timer_id	キャンセルするタイマID。-1であれば全てキャンセルする。
	 * @retval	0			正常
	 * @retval	0以外		異常
	 */
	int  cancelTimer(int timer_id=0)	// timer_id が (-1)であれば全キャンセル
	{
		return(m_TimerCBList.cancel(timer_id));
	}

	/**
	 * @brief タイムアウト時に呼び出される。
	 * 
	 * タイムアウト時にやるべき処理があれば、本関数をオーバーライドします。
	 * 
	 * @param	timer_id			タイマID
	 * @retval	なし
	 */
	virtual void onTimer(int timer_id)	{ return; }

	/**
	 * @brief 自スレッドにスレッド終了メッセージをキューイングする。
	 * 
	 * スレッド内部から自発的に終了する場合に使用する。
	 * 
	 * @param	bool_immediately	先頭にキューイングするか否か
	 * @param	vp_ret				スレッド終了時の返り値
	 * @retval	なし
	 */
	void shutdown(bool bool_immediately=false, void* vp_ret=NULL); 

	// ファイルディスクリプタ追加
	int appendFD(int fd, bool bool_read, bool bool_write, bool bool_except=false)
	{
		return(m_FDs.append(fd, bool_read, bool_write, bool_except));
	}

	// ファイルディスクリプタ削除
	int removeFD(int fd)
	{
		return(m_FDs.remove(fd));
	}

	// 登録したファイルディスクリプタにイベントが発生した時に呼び出す。
	virtual int onEvent(fd_set *p_readfds, fd_set *p_writefds, fd_set *p_exceptfds)
	{
		return(ERR_OK);
	}

	// 起動側との排他に使用する。
	// 起動前後に一度だけ使用している。
	// もったいないので、派生先でも使用可能とする。
	pthread_mutex_t	m_mutex;

private:
	pthread_t		m_pthread;			///< スレッド識別子
	pthread_t		m_pthread_copy;		///< スレッド識別子のコピー
										// m_pthread は終了しても必要となる。
	bool			m_bool_shutdown;	///< 終了処理中フラグ
	int				m_thread_no;		///< スレッド番号
	CThreadBase*	m_parent;			///< 親スレッド
	pthread_attr_t	m_pthread_attr;		///< スレッド属性（pthread_createのパラメータ）
	pthread_attr_t*	m_p_pthread_attr;	///< スレッド属性のポインタ

	/// @brief パイプのファイルディスクリプタ
	enum {
		PIPE_READ	= 0,	///< 読み込み用
		PIPE_WRITE	= 1,	///< 書き込み用
		PIPE_SIZE	= 2		///< サイズ
	};
	int				m_pipe[PIPE_SIZE];	///< スレッド間通知用パイプ

	CFileDescriptor	m_FDs;				///< ファイルディスクリプタ

	CThreadQueue	m_queue;			///< スレッドキュー

	CTimerCBList	m_TimerCBList;		///< タイマ制御ブロックリスト

	// スレッドクラスのインスタンス管理
	/*
		スレッド番号が(-1)は管理対象外
	*/
	static pthread_mutex_t	g_class_mutex;	///< インスタンス管理テーブルのミューテック
	struct _ThreadCB {
		int				thread_no;			///< スレッド番号
		CThreadBase*	p_thread_base;		///< スレッドのポインタ
		int				thread_status;		///< スレッド状態
	};
	static vector<_ThreadCB> g_vectorThreadCB;

	/**
	 * @brief スレッド終了、スレッド終了待ち合わせ。
	 * 
	 * stop()と join()の実装関数です。
	 * 
	 * @param	bool_stop			スレッド終了メッセージをキューイングするか否か
	 * @param	bool_join			スレッド終了を待つ（pthread_join()実行）か否か
	 * @param	bool_immediately	先頭にキューイングするか否か
	 * @param	vp_ret				スレッド終了時の返り値
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int stop_join(bool bool_stop, bool bool_join, bool bool_immediately=false, void* vp_ret=NULL);

	/**
	 * @brief スレッド状態を設定する。
	 * 
	 * @param	thread_status	スレッド状態
	 * @retval	0		正常
	 * @retval	0以外	異常
	 */
	int setInstanceInfo(const int thread_status) const;

public:
	/**
	 * @brief スレッドのrun関数。
	 * 
	 * 実装の都合で publicになっていますが、直接呼び出さないで下さい。
	 * 
	 * @param	なし
	 * @retval	なし
	 */
	void* run();	// public じゃないとダメなので。直接呼ぶなよ。
};

#endif
