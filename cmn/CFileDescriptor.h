/**
 * @file   CFileDescriptor.h
 * @brief  ファイルディスクリプタクラス
 *
 * このクラスは、select()関数使用時のファイルディスクリプタ操作をまとめています。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/10/19 渡辺正勝    初版<BR>
 */

#ifndef CFileDescriptor_h
#define CFileDescriptor_h

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <list>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// ファイルディスクリプタクラス
////////////////////////////////////////////////////////////////////////////////

class CFileDescriptor
{
public:

	// select()関数使用時、使用後に、下記領域を直接参照します。意味は自明。
	// 尚、select()関数呼び出し前に、必ずrebuild()関数を呼び出してください。

	int		m_maxfd_plus1;

	fd_set	*m_p_readfds;
	fd_set	*m_p_writefds;
	fd_set	*m_p_exceptfds;

	fd_set	m_readfds;
	fd_set	m_writefds;
	fd_set	m_exceptfds;

	//  コンストラクタ
	CFileDescriptor()
	: m_maxfd_plus1(0)
	, m_p_readfds(NULL)
	, m_p_writefds(NULL)
	, m_p_exceptfds(NULL)
	{
		pthread_mutex_init(&m_mutex, NULL);
	}

	//  デストラクタ
	virtual ~CFileDescriptor()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	// ファイルディスクリプタ追加
	int append(int fd, bool bool_read, bool bool_write, bool bool_except=false)
	{
		if (fd < 0) {
			return(-1);
		}
		if (!(bool_read || bool_write || bool_except)) {
			return(-1);
		}
		CControlBlock ControlBlock(fd, bool_read, bool_write, bool_except);
		pthread_mutex_lock(&m_mutex);
		m_list.push_back(ControlBlock);
		pthread_mutex_unlock(&m_mutex);
		return(0);
	}

	// ファイルディスクリプタ削除
	int remove(int fd)
	{
		if (fd < 0) {
			return(-1);
		}
		pthread_mutex_lock(&m_mutex);
		list<CControlBlock>::iterator iter;
		bool bool_erase;
		do {
			bool_erase = false;
			for (iter = m_list.begin(); iter != m_list.end(); ++iter) {
				if (iter->m_fd == fd) {
					m_list.erase(iter);
					bool_erase = true;
					break;
				}
			}
		} while (bool_erase);
		pthread_mutex_unlock(&m_mutex);
		return(0);
	}

	// 再構築
	void rebuild()
	{
		m_maxfd_plus1	= 0;
		m_p_readfds		= NULL;
		m_p_writefds	= NULL;
		m_p_exceptfds	= NULL;
		FD_ZERO(&m_readfds);
		FD_ZERO(&m_writefds);
		FD_ZERO(&m_exceptfds);

		pthread_mutex_lock(&m_mutex);
		list<CControlBlock>::iterator iter;
		for (iter = m_list.begin(); iter != m_list.end(); ++iter) {
			m_maxfd_plus1 = (m_maxfd_plus1 >= (iter->m_fd+1)) ? m_maxfd_plus1: (iter->m_fd+1);
			if (iter->m_bool_read) {
				FD_SET(iter->m_fd, &m_readfds);
				m_p_readfds = &m_readfds;
			}
			if (iter->m_bool_write) {
				FD_SET(iter->m_fd, &m_writefds);
				m_p_writefds = &m_writefds;
			}
			if (iter->m_bool_except) {
				FD_SET(iter->m_fd, &m_exceptfds);
				m_p_exceptfds = &m_exceptfds;
			}
		}
		pthread_mutex_unlock(&m_mutex);
	}

private:
	class CControlBlock
	{
	public:
		int		m_fd;			// ファイルディスクリプタ
		bool	m_bool_read;	// 読み込み監視
		bool	m_bool_write;	// 書き込み監視
		bool	m_bool_except;	// 例外監視

		//  コンストラクタ（デフォルト値付き）
		CControlBlock(int fd=(-1), bool bool_read=false, bool bool_write=false, bool bool_except=false)
		: m_fd(fd)
		, m_bool_read(bool_read)
		, m_bool_write(bool_write)
		, m_bool_except(bool_except)
		{
		};

		//  デストラクタ
		virtual ~CControlBlock()
		{
		};
	};

	//  ファイルディスクリプタ管理ブロックのリスト
	list<CControlBlock>	m_list;

	//  ミューテック（上記リストの排他）
	pthread_mutex_t	m_mutex;
};

#endif
