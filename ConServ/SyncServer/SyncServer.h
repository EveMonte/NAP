#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Global.h"
#include <ctime>

struct SETSYNCHRO {
	char cmd[5];
	int correction;
};

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


DWORD WINAPI SyncServer(LPVOID lParam)
{
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


	float avgCorrection = 0;
	clock_t Cs = clock();
	SETSYNCHRO* sync = new SETSYNCHRO();
	sync->correction = Cs;
	strcpy(sync->cmd, "Sync");
	int counter = 0;
	// Код возврата
	DWORD rc = 0;
	Contact* client = (Contact*)lParam;
	// Ставим в очередь сообщение о старте обслуживания
	QueueUserAPC(ASStartMessage, client->hAcceptServer, (DWORD)client);
	try
	{
		client->sthread = Contact::WORK;
		int  bytes = 1;
		char ibuf[150], obuf[150] = "Close: finish;", Time[50] = "Sync";
		bool isNTP = false;
		// Принимаем данные из сокета
		while (counter < 10 && client->TimerOff == false)
		{
			// Принимаем данные
			if ((bytes = recv(client->s, ibuf, sizeof(ibuf), NULL)) == SOCKET_ERROR)
			{
				switch (WSAGetLastError())
				{
				case WSAEWOULDBLOCK: Sleep(100); break;
				default: throw  SetErrorMsgText("Recv:", WSAGetLastError());
				}
			}
			else
			{
				// Проверяем метку срабатывания таймера
				if (client->TimerOff != false)
				{
					break;
				}
				if (strcmp(ibuf, "nontp") == 0 || strcmp(ibuf, "ntp") == 0) {
					if (strcmp(ibuf, "ntp") == 0) {
						isNTP = true;
					}
					continue;
				}
				if (!isNTP) {
					//sync->correction = Cs - ((SETSYNCHRO*)&ibuf)->correction;
					sync->correction = clock() - Cs - atoi(ibuf);
					//strcpy(ibuf, (char*)&sync);
					std::cout << "\tcurvalue " << ibuf << "\tcorrection " << sync->correction << "\t#" << counter << std::endl;
					_itoa(sync->correction, ibuf, 10);
					avgCorrection += sync->correction;

				}
				else {
					if ((s = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET) throw WSAGetLastError();
					int lenout = 0, lenin = 0, lensockaddr = sizeof(server);
					if ((lenout = sendto(s, (char*)&out_buf, sizeof(out_buf), NULL, (sockaddr*)&server, sizeof(server))) == SOCKET_ERROR) throw WSAGetLastError();
					if ((lenin = recvfrom(s, (char*)&in_buf, sizeof(in_buf), NULL, (sockaddr*)&server, &lensockaddr)) == SOCKET_ERROR) throw WSAGetLastError();

					in_buf.ReferenceTimestamp[0] = ntohl(in_buf.ReferenceTimestamp[0]) - d;
					in_buf.TransmitTimestamp[0] = ntohl(in_buf.TransmitTimestamp[0]) - d;
					in_buf.TransmitTimestamp[1] = ntohl(in_buf.TransmitTimestamp[1]);

					int ms = (int)1000.0 * ((double)in_buf.TransmitTimestamp[1]) / (double)0xffffffff;
					_itoa(ms, ibuf, 10);
					avgCorrection += ms;
					std::cout << "#" << counter << "\tms: " << ms << std::endl;
					if (closesocket(s) == INVALID_SOCKET) throw WSAGetLastError();

				}
				// Отправляем обратно
				if ((send(client->s, ibuf, sizeof(ibuf), NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("Send:", WSAGetLastError());
				counter++;


			}
		}

		std::cout << "Average correction " << avgCorrection / 10 << std::endl;
		counter = 0;

		if (client->TimerOff == false)
		{
			// Отключаем таймер
			CancelWaitableTimer(client->htimer);
			if ((send(client->s, obuf, sizeof(obuf) + 1, NULL)) == SOCKET_ERROR)
				throw  SetErrorMsgText("Send:", WSAGetLastError());
			// Ставим метку удачного завершения
			client->sthread = Contact::FINISH;
			QueueUserAPC(ASFinishMessage, client->hAcceptServer, (DWORD)client);
		}
	}
	catch (string errorMsgText)
	{
		std::cout << errorMsgText << std::endl;
		CancelWaitableTimer(client->htimer);
		client->sthread = Contact::ABORT;
	}
	// Завершаем поток
	ExitThread(rc);
}

