//
// test program
//
// 2006.7.20 m.watanabe
//

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "CThreadBase.h"
#include "CLogThread.h"
#include "CTcpListener.h"
#include "CTcpSocket.h"
#include "CTcpEcho.h"
#include "CTcpHealthCheck.h"

using namespace std;

int main(int argc, char* argv[]) {
	string	inputData;
	string	ipAdr;

	do {
		cout << "> server or client ? perhaps quit ? ('s'/'c'/'q')" << endl ;
		cout << "> ";
		getline(cin, inputData);
	} while ((inputData != "s") && (inputData != "c") && (inputData != "q"));
	if (inputData == "q") {
		return(0);
	}
	string server_client = inputData;

	if (server_client == "c") {
		cout << ">please input IP address." << endl ;
		cout << ">";
		getline(cin, ipAdr);
	}

	CLogThread LogThread;
	CLogHandle LogHandle;

	cout << "> log file or cout ? ('L' : \"../log/TCP_*.log\" / other : \"std::cout\")" << endl ;
	cout << "> ";
	getline(cin, inputData);
	if (inputData == "L") {
		LogThread.setParameter("../log", "TCP_", 100);
	} else {
		LogThread.setParameter("", "");	// cout
	}
	LogThread.start();

	LT_MSG(LogHandle, "start!", 0);

	if (server_client == "s") {
		cout << "> please hit 'Enter' if you want to exit." << endl ;
		CTcpEcho TcpEcho(22222);
		TcpEcho.start();
		getline(cin, inputData);
		TcpEcho.stop();
	}

	if (server_client == "c") {
		cout << "> please hit 'Enter' if you want to exit." << endl ;
		CTcpHealthCheck TcpHealthCheck1(ipAdr, 22222);
		CTcpHealthCheck TcpHealthCheck2(ipAdr, 22222);
		CTcpHealthCheck TcpHealthCheck3(ipAdr, 22222);
		CTcpHealthCheck TcpHealthCheck4(ipAdr, 22222);
		CTcpHealthCheck TcpHealthCheck5(ipAdr, 22222);
		TcpHealthCheck1.start();
		TcpHealthCheck2.start();
		TcpHealthCheck3.start();
		TcpHealthCheck4.start();
		TcpHealthCheck5.start();
		getline(cin, inputData);
		TcpHealthCheck1.stop();
		TcpHealthCheck2.stop();
		TcpHealthCheck3.stop();
		TcpHealthCheck4.stop();
		TcpHealthCheck5.stop();
	}

	LT_MSG(LogHandle, "near end!", 0);

	LogThread.stop();

	return 0;
}
