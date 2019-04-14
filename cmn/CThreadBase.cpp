/**
 * @file   CThreadBase.cpp
 * @brief  スレッドベースクラス及びその構成要素クラス
 *
 * このファイルは、スレッドベースクラスと、その構成要素である下記クラスの実装です。
 * 
 * ・スレッドキュークラス<BR>
 * ・タイマ制御ブロックリストクラス<BR>
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/09/06 渡辺正勝    初版<BR>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>	// デバッグで使う。
#include <pthread.h>
#include <exception>
#include <assert.h>
#include "CThreadBase.h"

////////////////////////////////////////////////////////////////////////////////
// スレッドキュークラス
////////////////////////////////////////////////////////////////////////////////

// コンストラクタ
CThreadQueue::CThreadQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
}

// デストラクタ
CThreadQueue::~CThreadQueue()
{
	removeAll();
	pthread_mutex_destroy(&m_mutex);
}

// スレッドメッセージのポインタを登録する。
int CThreadQueue::put(CThreadMsg *p_msg, bool bool_front)
{
	int ret = 0;
	pthread_mutex_lock(&m_mutex);
	if (bool_front) {
		m_dq_p_msg.push_front(p_msg);
	} else {
		m_dq_p_msg.push_back(p_msg);
	}
	pthread_mutex_unlock(&m_mutex);
	return(ret);
}

// スレッドメッセージのポインタを取り出す。
int CThreadQueue::get(CThreadMsg **pp_msg)
{
	int ret = 0;
	pthread_mutex_lock(&m_mutex);
	if (m_dq_p_msg.empty()) {
		*pp_msg = NULL;
		ret = (-1);
	} else {
		*pp_msg = m_dq_p_msg.front();
		m_dq_p_msg.pop_front();
	}
	pthread_mutex_unlock(&m_mutex);
	return(ret);
}

// キューイングされている全てのメッセージを削除します。
void CThreadQueue::removeAll()
{
	pthread_mutex_lock(&m_mutex);
	while (!m_dq_p_msg.empty()) {
		delete m_dq_p_msg.front();
		m_dq_p_msg.pop_front();
	}
	pthread_mutex_unlock(&m_mutex);
	return;
}

// スレッドキューが空か否かを返す。
bool CThreadQueue::empty()
{
	bool bool_ret = false;
	pthread_mutex_lock(&m_mutex);
	if (m_dq_p_msg.empty()) {
		bool_ret = true;
	}
	pthread_mutex_unlock(&m_mutex);
	return(bool_ret);
}

////////////////////////////////////////////////////////////////////////////////
// タイマ制御ブロックリストクラス
////////////////////////////////////////////////////////////////////////////////

// コンストラクタ
CTimerCBList::CTimerCBList()
{
	pthread_mutex_init(&m_mutex, NULL);
}

// デストラクタ
CTimerCBList::~CTimerCBList()
{
	pthread_mutex_destroy(&m_mutex);
}

// タイマ制御ブロックを作り、タイムアウト順にリストに登録する。
int CTimerCBList::set(int msec_elapsed_time, int timer_id, int msec_period)
{
	if (msec_elapsed_time <= 0)	{	return(CThreadBase::ERR_PARAM);	}
	if (timer_id < 0)			{	return(CThreadBase::ERR_PARAM);	}
	if (msec_period < 0)		{	return(CThreadBase::ERR_PARAM);	}
	CTimerCB TimerCB(msec_elapsed_time, timer_id, msec_period);
	return(set(TimerCB));
}

// タイマ制御ブロックを、タイムアウト順にリストに登録する。
int CTimerCBList::set(CTimerCB &rTimerCB)
{
	pthread_mutex_lock(&m_mutex);
	list<CTimerCB>::iterator iter;
	for (iter = m_list_TCB.begin(); iter != m_list_TCB.end(); ++iter) {
		if (rTimerCB < (*iter)) {
			break;
		}
	}
	m_list_TCB.insert(iter, rTimerCB);
	pthread_mutex_unlock(&m_mutex);
	return(CThreadBase::ERR_OK);
}

// リストからタイマ制御ブロックを削除する。
int CTimerCBList::cancel(int timer_id)
{
	// timer_id が (-1)であれば全キャンセル
	if (timer_id < (-1)) {
		return(CThreadBase::ERR_PARAM);
	}
	pthread_mutex_lock(&m_mutex);

	list<CTimerCB>::iterator iter;
	for (iter = m_list_TCB.begin(); iter != m_list_TCB.end(); ++iter) {
		if ((timer_id == (-1)) || (iter->m_timer_id == timer_id)) {
			iter->m_remove = true;
		}
	}
	m_list_TCB.remove_if(CTimerCB::is_remove);

	pthread_mutex_unlock(&m_mutex);
	return(CThreadBase::ERR_OK);
}

// リスト先頭のタイマ制御ブロックがタイムアウトしているか否かを返す。
bool CTimerCBList::timeout(int *p_timer_id)
{
	bool bool_ret = false;
	CTimerCB TimerCB;
	pthread_mutex_lock(&m_mutex);
	if (!m_list_TCB.empty()) {
		CTimeVal current_time(CTimeVal::CURRENT);
		if (m_list_TCB.begin()->m_target_time <= current_time) {
			TimerCB = m_list_TCB.front();
			m_list_TCB.pop_front();
			if (p_timer_id) {
				*p_timer_id = TimerCB.m_timer_id;
			}
			bool_ret = true;
		}
	}
	pthread_mutex_unlock(&m_mutex);
	if (TimerCB.m_msec_period > 0) {
		TimerCB.m_target_time.addMS(TimerCB.m_msec_period);
		set(TimerCB);
	}
	return(bool_ret);
}

// 次にタイムアウトする目標時刻を返す。
CTimeVal CTimerCBList::next_time()
{
	CTimeVal target_time(CTimeVal::CLEAR);
	pthread_mutex_lock(&m_mutex);
	if (!m_list_TCB.empty()) {
		target_time = m_list_TCB.begin()->m_target_time;
	}
	pthread_mutex_unlock(&m_mutex);
	return(target_time);
}

////////////////////////////////////////////////////////////////////////////////
// スレッドベースクラス（キュー、タイマ付き）
////////////////////////////////////////////////////////////////////////////////

// 例外のハンドラ
void callback_unexpected()
{
	cerr << "callback unexpected handler." << endl;
	cerr << "this thread = " << pthread_self() << endl;
}

// スタートルーチン
void* start_routine(void* arg)
{
	set_unexpected(callback_unexpected);
	return((reinterpret_cast<CThreadBase*>(arg))->run());
}

// コンストラクタ
CThreadBase::CThreadBase()
: m_pthread(0)
, m_pthread_copy(0)
, m_bool_shutdown(false)
, m_thread_no(-1)
, m_parent(NULL)
, m_p_pthread_attr(NULL)
{
	pthread_mutex_init(&m_mutex, NULL);
	assert(pipe(m_pipe)==0);
}

// デストラクタ
CThreadBase::~CThreadBase()
{
	stop();	// もし生きてたら止める。
	setInstanceInfo(STS_DESTROY);
	close(m_pipe[PIPE_READ]);
	close(m_pipe[PIPE_WRITE]);
	pthread_mutex_destroy(&m_mutex);
}

// スレッド属性を設定する。
int CThreadBase::setAttribute(const int thread_no, CThreadBase* parent, pthread_attr_t* p_pthread_attr)
{
	if (m_pthread != 0) {
		return(ERR_CONTEXT);
	}
	m_thread_no	= thread_no;
	m_parent	= parent;
	if (p_pthread_attr) {
		m_pthread_attr   = *p_pthread_attr;
		m_p_pthread_attr = &m_pthread_attr;
	}
	int ret;
	ret = setInstanceInfo(STS_READY);
	return(ret);
}

// スレッドを起動する。
int CThreadBase::start()
{
	if (m_pthread != 0) {
		return(ERR_CONTEXT);
	}
	int ret;
	ret = onPreThreadCreate();
	if (ret) {
		return(ret);
	}
	pthread_mutex_lock(&m_mutex);
	ret = pthread_create(&m_pthread, m_p_pthread_attr, start_routine, (void*)this);
	m_pthread_copy = m_pthread;
	pthread_mutex_unlock(&m_mutex);
	if (ret) {
		return(ret);
	}
	return(ret);
}

// スレッドを終了する。
int CThreadBase::stop(bool bool_join, bool bool_immediately, void* vp_ret)
{
	return(stop_join(true, bool_join, bool_immediately, vp_ret));
}

// スレッド終了を待つ。
int CThreadBase::join()
{
	return(stop_join(false, true));
}

// スレッド終了、スレッド終了待ち合わせ。
int CThreadBase::stop_join(bool bool_stop, bool bool_join, bool bool_immediately, void* vp_ret)
{
	if (m_pthread == 0) {
		return(ERR_CONTEXT);
	}
	if (m_pthread == pthread_self()) {
		return(ERR_CONTEXT);
	}
	if (bool_stop) {
		shutdown(bool_immediately, vp_ret);
	}
	if (bool_join) {
		pthread_join(m_pthread, NULL);
		m_pthread = 0;
		onPostThreadJoin();
	}
	return(ERR_OK);
}

// 自スレッドにスレッド終了メッセージをキューイングする。
void CThreadBase::shutdown(bool bool_immediately, void* vp_ret)
{
	if (active() == false) {
		return;
	}
	CStopMsg *p_stop_msg = new CStopMsg(vp_ret);
	postMsg(p_stop_msg, bool_immediately);

	m_bool_shutdown = true;
	setInstanceInfo(STS_SHUTDOWN);
}

// スレッドにメッセージをキューイングする。
int CThreadBase::postMsg(CThreadMsg *p_msg, bool bool_high_prior)
{
	int ret = ERR_OK;
	if (active() == false) {
		if (m_pthread == 0) {
			ret = ERR_CONTEXT;
		} else {
			ret = ERR_TERMINATE;
		}
		// エラーの時はメッセージを削除する。
		// postMsg()したメッセージは、責任を持って削除する。
		// 呼び出し側に負担をかけない。
		delete p_msg;
		return(ret);
	}
	ret = m_queue.put(p_msg, bool_high_prior);
	if (ret) {
		return(ret);
	}
	// スレッドへの通知（内容は無意味）
	if (write(m_pipe[PIPE_WRITE], "!", 1) != 1) {
		ret = ERR_SYSTEM;
	}
	return(ret);
}

// スレッドのrun関数。
void* CThreadBase::run()
{
	// 起動側スレッドがスレッド識別子を取り込むのを待ち合わせる。
	pthread_mutex_lock(&m_mutex);
	pthread_mutex_unlock(&m_mutex);
	setInstanceInfo(STS_RUNNING);
	void* vp_ret = NULL;
	bool bool_stop = false;
	if (onThreadInitiate()) {
		bool_stop = true;
	} else {
		appendFD(m_pipe[PIPE_READ], true, false);
	}

	while (!bool_stop) {
		int ret;

		int timer_id;
		while (m_TimerCBList.timeout(&timer_id)) {
			onTimer(timer_id);
		}

		if (m_queue.empty()) {
			CTimeVal time_span(CTimeVal::CLEAR);
			CTimeVal next_time = m_TimerCBList.next_time();
			if (next_time.isSet()) {
				CTimeVal current_time(CTimeVal::CURRENT);
				if (next_time > current_time) {
					time_span = next_time.getSpan(current_time);
				}
			} else {
				time_span.tv_sec	= 60*60*24*365;	// １年待ち（実装の都合）
				time_span.tv_usec	= 0;
			}

			m_FDs.rebuild();

			int	result = select(m_FDs.m_maxfd_plus1,
								m_FDs.m_p_readfds,
								m_FDs.m_p_writefds,
								m_FDs.m_p_exceptfds,
								&time_span);
			if (result > 0) {
				if (FD_ISSET(m_pipe[PIPE_READ], &m_FDs.m_readfds)) {
					FD_CLR  (m_pipe[PIPE_READ], &m_FDs.m_readfds);	// 派生先に渡すときゴミは残さない！
					result--;
					char buf;
					assert(read(m_pipe[PIPE_READ], &buf, 1) == 1);
					assert(buf == '!');
				}
				if (result > 0) {
					// 派生先が登録したファイルディスクリプタにイベントが発生した時に呼び出す。
					ret = onEvent(m_FDs.m_p_readfds, m_FDs.m_p_writefds, m_FDs.m_p_exceptfds);
				}
			} else {
				// タイムアウト
				if (result == 0) {
					continue;
				} else {
					assert(false);
				}
			}
		}

		CThreadMsg *p_msg;
		ret = m_queue.get(&p_msg);
		if (ret) {
			// これはないはずだが！
			continue;
		}

		// 終了メッセージ
		if (CStopMsg *p_stop_msg = dynamic_cast<CStopMsg*>(p_msg)) {
			vp_ret = p_stop_msg->m_vp_ret;
			bool_stop = true;
		} else {
			// 以下、派生先定義メッセージ
			ret = onMsg(p_msg);
			// ret によって何かする？
		}

		// メッセージの削除は、ここで一括して行う。
		if (p_msg) {
			delete p_msg;
		}
	}
	onThreadTerminate();
	setInstanceInfo(STS_STOP);	// 正確にはまだSTOPしてないが。
	return(vp_ret);
}

////////////////////////////////////////////////////////////////////////////////
// 状態収集スレッド クラス インスタンス管理
////////////////////////////////////////////////////////////////////////////////
pthread_mutex_t	CThreadBase::g_class_mutex = PTHREAD_MUTEX_INITIALIZER;
vector<CThreadBase::_ThreadCB> CThreadBase::g_vectorThreadCB;

// スレッド番号からインスタンスを返す。
CThreadBase* CThreadBase::getInstance(const int thread_no, int* p_thread_status)
{
	pthread_mutex_lock(&g_class_mutex);
	size_t index = 0;
	for ( ; index < g_vectorThreadCB.size(); index++) {
		if (g_vectorThreadCB[index].thread_no == thread_no) {
			break;
		}
	}
	pthread_mutex_unlock(&g_class_mutex);
	return(getInstanceByIndex(index, p_thread_status));
}

// 登録位置からインスタンスを返す。
CThreadBase* CThreadBase::getInstanceByIndex(const size_t index, int* p_thread_status)
{
	CThreadBase* p_thread_base = NULL;
	int thread_status = STS_UNKNOWN;
	pthread_mutex_lock(&g_class_mutex);
	if ((index >= 0) && (index < g_vectorThreadCB.size())) {
		p_thread_base = g_vectorThreadCB[index].p_thread_base;
		thread_status = g_vectorThreadCB[index].thread_status;
	}
	pthread_mutex_unlock(&g_class_mutex);
	if (p_thread_status) {
		*p_thread_status = thread_status;
	}
	return(p_thread_base);
}

// スレッド状態を設定する。
int CThreadBase::setInstanceInfo(const int thread_status) const
{
	int ret = ERR_OK;
	// スレッド番号が(-1)は管理対象外。
	if (m_thread_no == (-1)) {
		return(ERR_OK);
	}
	pthread_mutex_lock(&g_class_mutex);
	size_t index = 0;
	for ( ; index < g_vectorThreadCB.size(); index++) {
		if (g_vectorThreadCB[index].thread_no == m_thread_no) {
			// 状態変更！
			g_vectorThreadCB[index].thread_status = thread_status;
			break;
		}
	}
	if (index == g_vectorThreadCB.size()) {
		CThreadBase::_ThreadCB ThreadCB;
		ThreadCB.thread_no		= m_thread_no;
		ThreadCB.p_thread_base	= const_cast<CThreadBase*>(this);
		ThreadCB.thread_status	= thread_status;
		g_vectorThreadCB.push_back(ThreadCB);
	}
	pthread_mutex_unlock(&g_class_mutex);
	return(ret);
}

