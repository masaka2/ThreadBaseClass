/**
 * @file   fsTips.h
 * @brief  ファイル（ディレクトリ）関連のチップス関数
 * 
 * クラスにするまでもないが、毎回書くのは冗長なマクロ的な関数を集める。
 * 
 * @author  渡辺正勝
 *
 * 変更履歴<BR>
 * 日付　　　 担当　　　　記事<BR>
 * ---------------------------------------------------------------------------<BR>
 * 2007/04/10 渡辺正勝    初版<BR>
 */
/*
	以前作成した共通のチップス関数群から、
	ファイル関連（ディレクトリ）のみ抜き出して変更しました。
	boostを使用します。
*/

#ifndef fsTips_h
#define fsTips_h

#include <sys/types.h>
#include <string>
#include <boost/filesystem/operations.hpp>

using namespace std;

namespace fsTips {

// ２つのパス（ファイル名）を結合する。
// 結合部分に'/'が有ったり無かったりでエラーになるストレスから解放されたい。
inline string PATH_CAT(const string &strPath1st, const string &strPath2nd)
{
	if (strPath1st.empty() || strPath2nd.empty()) {
		return(strPath1st + strPath2nd);
	}
	if (strPath1st[strPath1st.length()-1] == '/') {
		if (strPath2nd[0] == '/') {
			return(strPath1st + strPath2nd.substr(1));
		} else {
			;
		}
	} else {
		if (strPath2nd[0] == '/') {
			;
		} else {
			return(strPath1st + '/' + strPath2nd);
		}
	}
	return(strPath1st + strPath2nd);
}

// パス内からファイル名のみを取り出す。
// 一番最後の'/'以降を返す。'/'が無かったら全て返す。
inline string GET_FILE_NAME(const string &strPath)
{
	unsigned int pos = strPath.rfind('/');
	if (pos != string::npos) {
		return(strPath.substr(pos+1));
	}
	return(strPath);
}

// ファイルの存在チェック
inline int CHECK_EXISTS_FILE(const string &strFilePath)
{
	// string を boost::filesystem::path に変換するとき例外が発生することがある。
	try {
		boost::filesystem::path Path(strFilePath);
		if (!boost::filesystem::exists(Path)) {	// does not exist
			return(1);
		} else {
			if (boost::filesystem::is_directory(Path)) {	// is a directory
				return(2);
			}
		}
		return(0);
	} catch ( ... ) {
		return(3);
	}
}

// ディレクトリの存在チェック
inline int CHECK_EXISTS_DIR(const string &strDirPath)
{
	// string を boost::filesystem::path に変換するとき例外が発生することがある。
	try {
		boost::filesystem::path Path(strDirPath);
		if (!boost::filesystem::exists(Path)) {	// does not exist
			return(1);
		} else {
			if (!boost::filesystem::is_directory(Path)) {	// is a not directory
				return(2);
			}
		}
		return(0);
	} catch ( ... ) {
		return(3);
	}
}

// ディレクトリを一気に作成する。
// 親ディレクトリが存在しなければ、その親とたどり、存在するところから作る。
// 一番大本のディレクトリが存在しなければエラーとなる。
// また、既に存在していた場合はエラーにはしない。
// 不正なパス名を指定すると呼び出し時に例外が発生します。
// 基本的にCREATE_DIR()を使ってください。
inline int CREATE_DIR_PATH(boost::filesystem::path Path)
{
	if (boost::filesystem::exists(Path)) {
		if (boost::filesystem::is_directory(Path)) {
			return(0);
		}
		return(1);
	} else {
		if (Path.has_branch_path()) {
			int nResult = CREATE_DIR_PATH(Path.branch_path());
			if (nResult) {
				return(nResult);
			}
		}
		try {
			boost::filesystem::create_directory(Path);
		} catch ( ... ) {
			return(2);
		}
	}
	return(0);
}

// ディレクトリを一気に作成する。（文字列指定／パス変換例外をハンドル）
inline int CREATE_DIR(const string &strPath)
{
	try {
		boost::filesystem::path Path(strPath);
		return(CREATE_DIR_PATH(Path));
	} catch ( ... ) {
		return(3);
	}
}

// ディレクトリを一旦削除して再作成する。
// ディレクトリのクリアを簡単に行う。
inline int RE_CREATE_DIR(const string &strDir)
{
	if (!boost::filesystem::exists(strDir)) {	// does not exist
		return(1);
	}
	try {
		boost::filesystem::remove_all(strDir);
		boost::filesystem::create_directory(strDir);
	} catch ( ... ) {
		return(2);
	}
	return(0);
}

// 指定ディレクトリ内の空ファイル（サイズ０）を削除する。
inline int REMOVE_EMPTY_FILES(const string &strDir)
{
	int nResult;
	nResult = CHECK_EXISTS_DIR(strDir);
	if (nResult) {
		return(nResult);
	}
	boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
	for (boost::filesystem::directory_iterator itr(strDir); itr != end_itr; ++itr) {
		if (!boost::filesystem::is_directory(*itr)) {
			if (boost::filesystem::file_size(*itr) == 0) {
				try {
					boost::filesystem::remove(*itr);
				} catch ( ... ) {
					return(4);
				}
			}
		}
	}
	return(0);
}

// ディレクトリを移動する。
// ターゲットディレクトリが存在したらエラー、
// ターゲットディレクトリの親が存在してなければ作成する。
inline int MOVE_DIR(const string &strSourceDir, const string &strTargetDir)
{
	int nResult;
	nResult = CHECK_EXISTS_DIR(strSourceDir);
	if (nResult) {
		return(10+nResult);
	}
	boost::filesystem::path TargetPath;
	try {
		TargetPath = strTargetDir;
	} catch ( ... ) {
		return(20);
	}
	if (boost::filesystem::exists(TargetPath)) {
		return(21);
	}
	nResult = CREATE_DIR_PATH(TargetPath.branch_path());
	if (nResult) {
		return(30+nResult);
	}
	try {
		boost::filesystem::rename(strSourceDir, TargetPath);
	} catch ( ... ) {
		return(1);
	}
	return(0);
}

};	// end of fsTips

#endif
