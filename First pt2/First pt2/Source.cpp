#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdafx.h"
#include <iostream>
#include <ctime>
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib")

struct NTP_packet {
	CHAR head[4];
	DWORD32 RootDelay;
	DWORD32 RootDispersion;
	CHAR ReferenceIndetifier[4];
	DWORD ReferenceTimestamp[2];
	DWORD64 OriginateTimeStamp;
	DWORD32 TransmitTimestamp[2];
	DWORD32 KeyIdentifier;
	DWORD64 MessageDigest[2];
};

int _tmain(int args, _TCHAR* argv[]) {
	int h = CLOCKS_PER_SEC;
	clock_t t = clock();
	int d = 613608 * 3600;
	time_t ttime;
	time(&ttime);

	WSAData wsaData;
	SOCKET s;
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("88.147.254.232");
	server.sin_port = htons(123);

	NTP_packet out_buf, in_buf;
	ZeroMemory(&out_buf, sizeof(out_buf));
	ZeroMemory(&in_buf, sizeof(in_buf));

	out_buf.head[0] = 0x1B;
	out_buf.head[1] = 0x00;
	out_buf.head[2] = 4;
	out_buf.head[3] = 0xEC;

	try {
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) throw WSAGetLastError();
		if ((s = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET) throw WSAGetLastError();
		int lenout = 0, lenin = 0, lensockaddr = sizeof(server);
		for (int i = 0; i < 10; i++) {
			Sleep(10000);
			if ((lenout = sendto(s, (char*)&out_buf, sizeof(out_buf), NULL, (sockaddr*)&server, sizeof(server))) == SOCKET_ERROR) throw WSAGetLastError();
			if ((lenin = recvfrom(s, (char*)&in_buf, sizeof(in_buf), NULL, (sockaddr*)&server, &lensockaddr)) == SOCKET_ERROR) throw WSAGetLastError();

			in_buf.ReferenceTimestamp[0] = ntohl(in_buf.ReferenceTimestamp[0]) - d;
			in_buf.TransmitTimestamp[0] = ntohl(in_buf.TransmitTimestamp[0]) - d;
			in_buf.TransmitTimestamp[1] = ntohl(in_buf.TransmitTimestamp[1]);

			int ms = (int)1000.0 * ((double)in_buf.TransmitTimestamp[1]) / (double)0xffffffff;

			std::cout << "#" << i << "\tms: " << ms << "\tReferenceTimestamp[0]: " << in_buf.ReferenceTimestamp[0] << "\tTransmitTimestamp[0]: " << in_buf.TransmitTimestamp[0] << std::endl;
		}
		if (closesocket(s) == INVALID_SOCKET) throw WSAGetLastError();
		if (WSACleanup() == SOCKET_ERROR) throw WSAGetLastError();
	}
	catch (int e) {
		std::cout << "error:" << e << std::endl;
	}
}