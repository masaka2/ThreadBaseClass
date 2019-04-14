/**
 * @file   CStrAid.h
 * @brief  std::string用 補助クラス
 *
 * std::stringクラスにフォーマット関数がないので作りました。
 * boost等を使用する場合は不要になります。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2006/07/11 渡辺正勝    初版<BR>
 */

#ifndef CStrAid_h
#define CStrAid_h

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

class CStrAid
{
	public:

		CStrAid()
		: m_size(1024)
		{
			m_p = new char[m_size];
		}

		~CStrAid()
		{
			delete [] m_p;
		}

		std::string Format(const char * format, ...)
		{
			va_list args;
			va_start(args, format);

			FILE *fp;
			if ((fp = fopen("/dev/null", "w")) == NULL){
				return (std::string(""));
			}
			int size = vfprintf(fp, format, args);
			if (m_size < ((size_t)size + 1)) {
				m_size = ((size_t)size + 1);
				delete [] m_p;
				m_p = new char[m_size];
			}
			vsprintf(m_p, format, args); 
			fclose(fp);

			va_end(args);
			return (std::string(m_p));
		}

	private:
		size_t	m_size;
		char	*m_p;
};

#endif

