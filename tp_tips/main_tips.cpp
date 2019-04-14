// strTips.h  fsTips.h 等のテストプログラムです。
//
// 2007.4.4 M.Watanabe
//

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "strTips.h"
#include "fsTips.h"

using namespace std;


int main(int argc, char* argv[]) {
	string	strInputData;
	int	nNo;

	while (true) {
		cout << ">  1:int" << endl ;
		cout << ">  2:long" << endl ;
		cout << ">  3:float" << endl ;
		cout << ">  4:double" << endl ;
		cout << ">  5:int(template test)" << endl ;
		cout << "> 11:hex int" << endl ;
		cout << "> 12:hex long" << endl ;
		cout << "> 13:hex unsigned int" << endl ;
		cout << "> 14:hex unsigned long" << endl ;
		cout << "> 21:format" << endl ;
		cout << "> 31:CREATE_DIR" << endl ;
		cout << "> 32:MOVE_DIR" << endl ;
		cout << "> 9:quit" << endl ;
		cout << ">";
		getline(cin, strInputData);
		nNo = strTips::STR_TO_INT(strInputData, -1);
		if (nNo < 0) {
			continue;
		}
		if (nNo == 9) {
			return(0);
		}
		if (nNo == 1) {
			cout << "int test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				int n;
				if (!strTips::FROM_STR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::STR_TO_INT(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (n == 99999) {
					break;
				}
			}
		}
		if (nNo == 2) {
			cout << "long test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				long n;
				if (!strTips::FROM_STR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::STR_TO_LONG(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (n == 99999) {
					break;
				}
			}
		}
		if (nNo == 3) {
			cout << "float test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				float n;
				if (!strTips::FROM_STR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::STR_TO_FLOAT(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (n == 99999) {
					break;
				}
			}
		}
		if (nNo == 4) {
			cout << "double test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				double n;
				if (!strTips::FROM_STR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::STR_TO_DOUBLE(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (n == 99999) {
					break;
				}
			}
		}
		if (nNo == 5) {
			cout << "int test(template test)" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				int n;
				if (!strTips::FROM_STR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::STR_TO_INT(strInputData);
//				string str = strTips::TO_STR_TEMPLATE(n);	// 省略可？
//				string str = strTips::TO_STR_TEMPLATE<>(n);	// 省略可？
//				string str = strTips::TO_STR<>(n);	// 省略可？
				string str = strTips::TO_STR(n);	// 省略可？
				cout << str << endl ;
				if (n == 99999) {
					break;
				}
			}
		}
		if (nNo == 11) {
			cout << "hex int test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				int n;
				if (!strTips::FROM_HEXSTR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::HEXSTR_TO_INT(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (strInputData == "99999") {
					break;
				}
			}
		}
		if (nNo == 12) {
			cout << "hex long test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				long n;
				if (!strTips::FROM_HEXSTR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::HEXSTR_TO_LONG(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (strInputData == "99999") {
					break;
				}
			}
		}
		if (nNo == 13) {
			cout << "hex unsigned int test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				unsigned int n;
				if (!strTips::FROM_HEXSTR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::HEXSTR_TO_UINT(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (strInputData == "99999") {
					break;
				}
			}
		}
		if (nNo == 14) {
			cout << "hex unsigned long test" << endl ;
			cout << "99999 : quit" << endl ;
			while (true) {
				getline(cin, strInputData);
				unsigned long n;
				if (!strTips::FROM_HEXSTR(n, strInputData)) {
					cout << "error!" << endl ;
				}
				n = strTips::HEXSTR_TO_ULONG(strInputData);
				string str = strTips::TO_STR(n);
				cout << str << endl ;
				if (strInputData == "99999") {
					break;
				}
			}
		}
		if (nNo == 21) {
			int x;
			cin >> x;
			string strFormat;
			for (int i=0; i < x; i++) {
				strFormat += '.';
			}
			strFormat += "%d:%s:%f";
			int d = 12345;
			string s = "string";
			float f = 987.654;
			string strOut = strTips::FORMAT(strFormat.c_str(), d, s.c_str(), f);
			cout << strOut << endl ;
			getline(cin, strInputData);
		}
		if (nNo == 31) {
			fsTips::CREATE_DIR("./temp/a/b/c");
		}
		if (nNo == 32) {
			fsTips::MOVE_DIR("./temp/a","./temp/x");
		}
	}
	return 0;
}
