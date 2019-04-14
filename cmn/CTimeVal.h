/**
 * @file   CTimeVal.h
 * @brief  timevalのラッパークラス
 *
 * このクラスはtimevalから派生しているので、timevalと同様に使用できます。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2005/10/14 渡辺正勝    初版<BR>
 */

#ifndef CTimeVal_h
#define CTimeVal_h

#include <sys/types.h>
#include <sys/time.h>

////////////////////////////////////////////////////////////////////////////////
// timevalのラッパークラス
////////////////////////////////////////////////////////////////////////////////

class CTimeVal : public timeval
{
public:
	// 初期化フラグ
	typedef	enum {
				CLEAR,		// クリア
				CURRENT		// 現在時刻
			} INITIALIZE;

	CTimeVal(INITIALIZE init=CLEAR)
	{
		if (init == CURRENT) {
			gettimeofday(this, NULL);
		} else {
			timerclear(this);
		}
	};

	CTimeVal(const CTimeVal& x)
	{
		*this = x;
	};

	CTimeVal(const struct timeval tv)
	{
		tv_sec  = tv.tv_sec;
		tv_usec = tv.tv_usec;
	};

	virtual ~CTimeVal()
	{
	};

	CTimeVal& operator=(const CTimeVal& x)
	{
		tv_sec  = x.tv_sec;
		tv_usec = x.tv_usec;
		return *this;
	}

	bool operator==(const CTimeVal& x) const
	{
		return(timercmp(this, &x, ==));
	}

	bool operator!=(const CTimeVal& x) const
	{
		return(timercmp(this, &x, !=));
	}

	bool operator>(const CTimeVal& x) const
	{
		return(timercmp(this, &x, >));
	}

	bool operator<(const CTimeVal& x) const
	{
		return(timercmp(this, &x, <));
	}

	bool operator<=(const CTimeVal& x) const
	{
		return(!timercmp(this, &x, >));
	}

	bool operator>=(const CTimeVal& x) const
	{
		return(!timercmp(this, &x, <));
	}

	bool isSet(void)
	{
		return(this->tv_sec || this->tv_usec);
	}

	void clear(void)
	{
		timerclear(this);
	}

	void setCurrent(void)
	{
		gettimeofday(this, NULL);
	}

	CTimeVal getSpan(const CTimeVal& x) const
	{
		CTimeVal big   = (*this > x) ? *this: x;
		CTimeVal small = (*this > x) ? x: *this;
		if (big.tv_usec < small.tv_usec) {
			big.tv_usec += (1000 * 1000);
			big.tv_sec--;
		}
		big.tv_usec -= small.tv_usec;
		big.tv_sec  -= small.tv_sec;
		return big;
	}

	CTimeVal add(const CTimeVal& x)
	{
		this->tv_sec  += x.tv_sec;
		this->tv_usec += x.tv_usec;
		this->tv_sec  += (this->tv_usec / (1000 * 1000));
		this->tv_usec %= (1000 * 1000);
		return *this;
	}

	CTimeVal addMS(const int msec)
	{
		CTimeVal temp;
		temp.tv_sec  = (msec / 1000);
		temp.tv_usec = (msec % 1000) * 1000;
		return(this->add(temp));
	}

};

#endif
