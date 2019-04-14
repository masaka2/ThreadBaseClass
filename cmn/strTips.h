/**
 * @file   strTips.h
 * @brief  文字列関連のチップス関数
 * 
 * クラスにするまでもないが、毎回書くのは冗長なマクロ的な関数を集める。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2007/03/30 渡辺正勝    初版<BR>
 */
/*
	以前作成した共通のチップス関数群から、文字列関連のみ抜き出して変更しました。
*/

#ifndef strTips_h
#define strTips_h

#include <sys/types.h>
#include <stdio.h>	// FORMAT
#include <stdlib.h>	// FORMAT
#include <stdarg.h>	// FORMAT
#include <sstream>	// ostringstream istringstream
#include <string>
#include <iostream>
#include <climits>	// INT_MIN,LONG_MIN
#include <cfloat>	// FLT_MIN,DBL_MIN

using namespace std;

namespace strTips {

// リサイズ機能付きバッファ確保クラス
// わざわざ作らなくとも何処かにあるような気がするが...
template<typename T> class CBuf
{
	public:
		CBuf(int size=256) : m_size(size) {m_p = new T[m_size];}
		~CBuf() {delete [] m_p;}
		operator T *() const {return(m_p);}
		void resize(int size)
		{
			if (m_size < size) {
				m_size = size;
				delete [] m_p;
				m_p = new T[m_size];
			}
		}
	private:
		int	m_size;
		T	*m_p;
};

// フォーマット関数
inline string FORMAT(const char * format, ...)
{
	int max_size = 256;
	CBuf<char> Buf(max_size);
	va_list args;
	while (true) {
		va_start(args, format);
		int n = vsnprintf((char*)Buf, max_size, format, args); 
		va_end(args);
		// 返値がバージョンによって変わる！（注意！）
		if ((n > -1) && (n < max_size)) {
			return(string((char*)Buf));
		}
		if (n > -1) {		// glibc 2.1
			max_size = n+1; // precisely what is needed
		} else {			// glibc 2.0
			max_size *= 2;	// twice the old size
		}
		Buf.resize(max_size);
	}
	return(string(""));	// ここ来ないけど！
}

// 数値から文字列への変換テンプレート
// 使用方法を以下に示します。
// string str = TO_STR(n);
// 型は引数からコンパイラが自動的に判断します。
// "<int>"と言ったテンプレート引数は省略出来ます。
template<typename T> string TO_STR(const T &n)
{
	ostringstream strm; 
	strm << n;
	return(strm.str());
}

// 文字列から数値への変換テンプレート群

// 文字列から数値への変換テンプレート（変換の成否を復帰情報で返すバージョン）
template<typename T>
bool FROM_STR_TEMPLATE(	T &t,
						const string &s,
						ios_base& (*f)(ios_base&))
{
	istringstream iss(s);
	return(!(iss >> f >> t).fail());
}

// 文字列から数値への変換テンプレート（エラーであればデフォルト値を返すバージョン）
template<typename T>
T FROM_STR_DEFAULT(	const string &s,
					T d,
					ios_base& (*f)(ios_base&))
{
	T t;
	return((FROM_STR_TEMPLATE<T>(t, s, f)) ? t : d);
}

// 十進数文字列から数値への変換テンプレート（変換の成否を復帰情報で返すバージョン）
// 使用方法を以下に示します。
// if (!FROM_STR(n, s)) {
// 型は引数からコンパイラが自動的に判断します。
// "<int>"と言ったテンプレート引数は省略出来ます。
template<typename T>
bool FROM_STR(T &t, const string &s){return(FROM_STR_TEMPLATE<T>(t, s, std::dec));}

// １６進数文字列から数値への変換テンプレート（変換の成否を復帰情報で返すバージョン）
// 使用方法を以下に示します。
// if (!FROM_HEXSTR(n, s)) {
// 型は引数からコンパイラが自動的に判断します。
// "<int>"と言ったテンプレート引数は省略出来ます。
template<typename T>
bool FROM_HEXSTR(T &t, const string &s){return(FROM_STR_TEMPLATE<T>(t, s, std::hex));}

// 十進数文字列から数値への変換テンプレート（エラーであればデフォルト値を返すバージョン）
template<typename T>
T FROM_DEC_STR_DEFAULT(const string &s, T d){return(FROM_STR_DEFAULT<T>(s, d, std::dec));}

// １６進数文字列から数値への変換テンプレート（エラーであればデフォルト値を返すバージョン）
template<typename T>
T FROM_HEX_STR_DEFAULT(const string &s, T d){return(FROM_STR_DEFAULT<T>(s, d, std::hex));}

// 以下、主に使用するタイプを関数化する。

// 十進数文字列から数値への変換関数（エラーであればデフォルト値を返すバージョン）
inline int    STR_TO_INT   (const string &s, int    d= INT_MIN){return(FROM_DEC_STR_DEFAULT<int>(s, d));}
inline long   STR_TO_LONG  (const string &s, long   d=LONG_MIN){return(FROM_DEC_STR_DEFAULT<long>(s, d));}
inline float  STR_TO_FLOAT (const string &s, float  d= FLT_MIN){return(FROM_DEC_STR_DEFAULT<float>(s, d));}
inline double STR_TO_DOUBLE(const string &s, double d= DBL_MIN){return(FROM_DEC_STR_DEFAULT<double>(s, d));}

// １６進数文字列から数値への変換関数（エラーであればデフォルト値を返すバージョン）
inline int  HEXSTR_TO_INT (const string &s, int  d= INT_MIN){return(FROM_HEX_STR_DEFAULT<int>(s, d));}
inline long HEXSTR_TO_LONG(const string &s, long d=LONG_MIN){return(FROM_HEX_STR_DEFAULT<long>(s, d));}
inline unsigned int  HEXSTR_TO_UINT (const string &s, unsigned int  d= UINT_MAX){return(FROM_HEX_STR_DEFAULT<unsigned int>(s, d));}
inline unsigned long HEXSTR_TO_ULONG(const string &s, unsigned long d=ULONG_MAX){return(FROM_HEX_STR_DEFAULT<unsigned long>(s, d));}

};	// end of strTips

#endif
