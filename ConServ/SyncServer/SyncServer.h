#pragma once
#include "Global.h"
#include <ctime>

struct SETSYNCHRO {
	char cmd[5];
	int correction;
};

DWORD WINAPI SyncServer(LPVOID lParam)
{
	float avgCorrection = 0;
	clock_t Cs = clock();
	SETSYNCHRO* sync = new SETSYNCHRO();
	SETSYNCHRO* syncClient = new SETSYNCHRO();
	sync->correction = Cs;
	strcpy(sync->cmd, "Sync");
	int counter = 0;
	// ��� ��������
	DWORD rc = 0;
	Contact* client = (Contact*)lParam;
	// ������ � ������� ��������� � ������ ������������
	QueueUserAPC(ASStartMessage, client->hAcceptServer, (DWORD)client);
	try
	{
		client->sthread = Contact::WORK;
		int  bytes = 1;
		char ibuf[150], obuf[150] = "Close: finish;", Time[50] = "Sync";

		// ��������� ������ �� ������
		while (counter < 10 && client->TimerOff == false)
		{
			// ��������� ������
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
				// ��������� ����� ������������ �������
				if (client->TimerOff != false)
				{
					break;
				}
				//sync->correction = Cs - ((SETSYNCHRO*)&ibuf)->correction;
				sync->correction = clock() - Cs - atoi(ibuf);
				//strcpy(ibuf, (char*)&sync);
				std::cout << "\tcurvalue " << ibuf << "\tcorrection " << sync->correction << "\tcounter " << counter << "\tOh\t" << sync->cmd << std::endl;
				_itoa(sync->correction, ibuf, 10);
				avgCorrection += sync->correction;
				// ���������� �������
				if ((send(client->s, ibuf, sizeof(ibuf), NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("Send:", WSAGetLastError());
				counter++;


			}
		}
		std::cout << "Average correction " << avgCorrection / 10 << std::endl;
		counter = 0;

		if (client->TimerOff == false)
		{
			// ��������� ������
			CancelWaitableTimer(client->htimer);
			if ((send(client->s, obuf, sizeof(obuf) + 1, NULL)) == SOCKET_ERROR)
				throw  SetErrorMsgText("Send:", WSAGetLastError());
			// ������ ����� �������� ����������
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
	// ��������� �����
	ExitThread(rc);
}