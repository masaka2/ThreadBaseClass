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
#include "CUdpSocket.h"
#include "CUdpEcho.h"
#include "CUdpHealthCheck.h"

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

	cout << "> log file or cout ? ('L' : \"../log/UDP_*.log\" / other : \"std::cout\")" << endl ;
	cout << "> ";
	getline(cin, inputData);
	if (inputData == "L") {
		LogThread.setParameter("../log", "UDP_", 100);
	} else {
		LogThread.setParameter("", "");	// cout
	}
	LogThread.start();

	LT_MSG(LogHandle, "start!", 0);

	if (server_client == "s") {
		cout << "> please hit 'Enter' if you want to exit." << endl ;
		CUdpEcho UdpEcho(22222);
		UdpEcho.start();
		getline(cin, inputData);
		UdpEcho.stop();
	}

	if (server_client == "c") {
		cout << "> please hit 'Enter' if you want to exit." << endl ;
		CUdpHealthCheck UdpHealthCheck1(ipAdr, 22222);
		CUdpHealthCheck UdpHealthCheck2(ipAdr, 22222);
		CUdpHealthCheck UdpHealthCheck3(ipAdr, 22222);
		CUdpHealthCheck UdpHealthCheck4(ipAdr, 22222);
		CUdpHealthCheck UdpHealthCheck5(ipAdr, 22222);
		UdpHealthCheck1.start();
		UdpHealthCheck2.start();
		UdpHealthCheck3.start();
		UdpHealthCheck4.start();
		UdpHealthCheck5.start();
		getline(cin, inputData);
		UdpHealthCheck1.stop();
		UdpHealthCheck2.stop();
		UdpHealthCheck3.stop();
		UdpHealthCheck4.stop();
		UdpHealthCheck5.stop();
	}

	LT_MSG(LogHandle, "near end!", 0);

	LogThread.stop();

	return 0;
}
