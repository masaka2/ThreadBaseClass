/**
 * @file   CLogThread.cpp
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

#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <dirent.h>

#include "CLogThread.h"

////////////////////////////////////////////////////////////////////////////////
// ログスレッドクラス
////////////////////////////////////////////////////////////////////////////////
int CLogThread::onPreThreadCreate()
{
	// パス指定の調整（後々の便宜のため！）
	// ファイル名プレフィックスがあり、パス指定がなければカレントパスにする。
	// パス指定の最後に'/'がなければ付加する。
	if (m_strDirPath == "") {
		if (m_strFilePrefix != "") {
			m_strDirPath = "./";
		}
	} else {
		if (m_strDirPath[m_strDirPath.length()-1] != '/') {
			m_strDirPath += '/';
		}
	}
	// ログスレッドは１つなので、スレッド番号を自分で登録する。
	return(setAttribute(m_nLogThreadNo));
}

int CLogThread::onMsg(CThreadMsg *p_msg)
{
	if (CLogThreadMsg* pLogThreadMsg = dynamic_cast<CLogThreadMsg*>(p_msg)) {
		struct tm *ptm = localtime(&pLogThreadMsg->m_time.tv_sec);
		if (m_nLine == 0) {
			openFile(ptm);
		}
		out()	<< m_StrAid.Format(	"%04d.%02d.%02d %02d:%02d:%02d %06d ",
									ptm->tm_year+1900,
									ptm->tm_mon+1,
									ptm->tm_mday,
									ptm->tm_hour,
									ptm->tm_min,
									ptm->tm_sec,
									pLogThreadMsg->m_time.tv_usec)
				<< pLogThreadMsg->m_strMsg
				<< endl;
		if ((++m_nLine) == m_nMaxLine) {
			closeFile();
			m_nLine = 0;
		}
	}
	return(ERR_OK);
}

// ログファイルのオープン処理
// ファイル名には日付を加える。
// 指定ファイル数を超えたら、古い順から削除する。
void CLogThread::openFile(struct tm *ptm)
{
	if (m_strDirPath == "") {
		return;
	}

	string strNewFile = 
		m_StrAid.Format("%s%02d%02d%02d_%02d%02d%02d.log",	// 形式変えたら下も！
						m_strFilePrefix.c_str(),
						ptm->tm_year+1900-2000,
						ptm->tm_mon+1,
						ptm->tm_mday,
						ptm->tm_hour,
						ptm->tm_min,
						ptm->tm_sec);

	m_pFile = new ofstream((m_strDirPath+strNewFile).c_str(), ofstream::out | ofstream::trunc);
	if (m_pFile == NULL) {
		return;
	}

	DIR* pDir = ::opendir(m_strDirPath.c_str());
	if (pDir == NULL) {
		return;
	}

	string strTmpFile = 
		m_StrAid.Format("%s******_******.log",	// 形式変えたら上も！
						m_strFilePrefix.c_str());

	string			strOldFile;
	list<string>	strOldFileList;
	struct dirent	*pEnt;

	while ((pEnt = ::readdir(pDir)) != NULL) {
		strOldFile = pEnt->d_name;
		if (strOldFile == ".") continue;
		if (strOldFile == "..") continue;
		if (strOldFile.length() != strTmpFile.length()) continue;
		unsigned int i;
		for (i=0; i < strOldFile.length(); i++) {
			if (strOldFile[i] != strTmpFile[i]) {
				if ((strOldFile[i] < '0') ||
					(strOldFile[i] > '9') ||
					(strTmpFile[i] != '*')) {
					break;
				}
			}
		}
		if (i == strOldFile.length()) {
			strOldFileList.push_back(strOldFile);
		}
	}
	::closedir(pDir);

	int nDelete = strOldFileList.size() - m_nMaxFile;
	if (nDelete > 0) {
		strOldFileList.sort();
		list<string>::iterator iter;
		for (iter = strOldFileList.begin(); iter != strOldFileList.end(); ++iter) {
			::unlink((m_strDirPath+(*iter)).c_str());
			if ((--nDelete) <= 0) {
				break;
			}
		}
	}
}

void CLogThread::closeFile()
{
	if (m_pFile) {
		m_pFile->close();
		delete m_pFile;
		m_pFile = NULL;
	}
}

CLogThread* CLogThread::getLogThread(int nLogThreadNo)
{
	int thread_status;
	CThreadBase* pThreadBase = CThreadBase::getInstance(nLogThreadNo, &thread_status);
	// 当初、稼働中以外は NULL を返していたが、
	// 起動直後に書き込んだログが失われるので、レディ状態もＯＫとする。
	if ((thread_status == STS_RUNNING) || (thread_status == STS_READY)) {
		return(dynamic_cast<CLogThread*>(pThreadBase));
	}
	return(NULL);
}

