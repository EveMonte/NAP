#pragma once
#include "Global.h"
#include "ServiceWorkTime.h"

const string currentDateTime();
int* getHoursAndMinutes(string str);
bool isBetween(string now, string from, string to);

DWORD WINAPI DispathServer(LPVOID pPrm)
{
	DWORD rc = 0;
	try
	{
		while (*((TalkersCmd*)pPrm) != Exit)
		{
			try
			{
				if (WaitForSingleObject(Event, 300) == WAIT_OBJECT_0)
				{
					//подключ.в тек. момент
					if (&Work > 0)
					{
						Contact* client = NULL;
						int libuf = 1;
						char CallBuf[50] = "", SendError[50] = "ErrorInquiry";
						EnterCriticalSection(&scListContact);

						//цикл сканирования списка клиентов
						for (ListContact::iterator p = Contacts.begin(); p != Contacts.end(); p++)
						{
							if (p->type == Contact::ACCEPT) //если статус accept (подключен)
							{
								client = &(*p);
								bool Check = false;

								while (Check == false)
								{
									if ((libuf = recv(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
									{
										switch (WSAGetLastError())
										{
										case WSAEWOULDBLOCK: Sleep(100); break;
										default: throw  SetErrorMsgText("Recv:", WSAGetLastError());
										}
									}
									else Check = true;
								}
								//анализ запроса
								//изменяет check в случае не соответствия
								if (strcmp(CallBuf, "Rand") == 0 || strcmp(CallBuf, "Echo") == 0 || strcmp(CallBuf, "Time") == 0 || strcmp(CallBuf, "Sync") == 0)
									Check == true;
								else Check == false;
								if (Check == true)
								{
									string time_error;
									if (false) {
										client->type = Contact::CONTACT;
										strcpy_s(client->srvname, CallBuf);
										client->htimer = CreateWaitableTimer(NULL, false, NULL);
										_int64 time = -600000000;
										SetWaitableTimer(client->htimer, (LARGE_INTEGER*)&time, 0, ASWTimer, client, false);

										cout << CallBuf << endl;
										
										if (strcmp(CallBuf, "Rand") == 0) {
											if (execRand != NULL) {
												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execRand(client);
											}
											else {
												if ((libuf = send(client->s, "Rand server not avaliable", sizeof("Rand server not avaliable"), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = NULL;
											}
										}
										else if (strcmp(CallBuf, "Echo") == 0)
										{
											if (execEcho != NULL) {
												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execEcho(client);
											}
											else {
												if ((libuf = send(client->s, "Echo server not avaliable", sizeof("Echo server not avaliable"), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = NULL;
											}
										}
										else if (strcmp(CallBuf, "Time") == 0) {
											if (execTime!=NULL) {
												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execTime(client);
											}
											else {
												if ((libuf = send(client->s, "Time server not avaliable", sizeof("Time server not avaliable"), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = NULL;
											}
										}
										else if (strcmp(CallBuf, "Sync") == 0) {
											std::cout << "SyncServer Dispath" << std::endl;
											if (execSync!=NULL) {
												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execSync(client);
											}
											else {
												if ((libuf = send(client->s, "Sync server not avaliable", sizeof("Sync server not avaliable"), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = NULL;
											}
										}
										else {
											client->hthread = NULL;
										}
									}
									else {


										if (strcmp(CallBuf, "Rand") == 0) {
											string time_now = currentDateTime();

											cout << time_now << endl;

											if (randServer != NULL && client->hthread!=NULL) {
												client->type = Contact::CONTACT;
												strcpy_s(client->srvname, CallBuf);
												client->htimer = CreateWaitableTimer(NULL, false, NULL); //создание ожидающего таймера
												_int64 time = -600000000; //установка времени (15 секунд)
												SetWaitableTimer(client->htimer, (LARGE_INTEGER*)&time, 0, ASWTimer, client, false);

												cout << CallBuf << endl;

												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execRand(client);
											}
											else {
												time_error = "RandService unavailable\0";

												EnterCriticalSection(&scListContact);
												client->CloseConn = true;
												client->sthread = Contact::FINISH;
												LeaveCriticalSection(&scListContact);

												if ((libuf = send(client->s, time_error.c_str(), time_error.length(), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
											}
										}
										else if (strcmp(CallBuf, "Echo") == 0) {
											string time_now = currentDateTime();
											cout << time_now << endl;

											if (echoServer != NULL && client->hthread != NULL)
											{
												client->type = Contact::CONTACT;
												strcpy_s(client->srvname, CallBuf);
												client->htimer = CreateWaitableTimer(NULL, false, NULL);
												_int64 time = -600000000;
												SetWaitableTimer(client->htimer, (LARGE_INTEGER*)&time, 0, ASWTimer, client, false);

												cout << CallBuf << endl;

												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execEcho(client);
											}
											else {
												time_error = "EchoService unavailable\0";

												EnterCriticalSection(&scListContact);
												client->CloseConn = true;
												client->sthread = Contact::FINISH;
												LeaveCriticalSection(&scListContact);

												if ((libuf = send(client->s, time_error.c_str(), time_error.length(), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());

												SYSTEMTIME stt;
												GetLocalTime(&stt);
												printf("%d.%d.%d %d:%02d эхо сервер недоступен ", stt.wDay, stt.wMonth, stt.wYear, stt.wHour, stt.wMinute);
												cout << ";" << endl;

											}
										}
										else if (strcmp(CallBuf, "Sync") == 0) {

											string time_now = currentDateTime();
											cout << time_now << endl;

											if (echoServer != NULL && client->hthread != NULL)
											{
												client->type = Contact::CONTACT;
												strcpy_s(client->srvname, CallBuf);
												client->htimer = CreateWaitableTimer(NULL, false, NULL);
												_int64 time = -60000000000;
												SetWaitableTimer(client->htimer, (LARGE_INTEGER*)&time, 0, ASWTimer, client, false);

												cout << CallBuf << endl;

												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execSync(client);
											}
											else {
												time_error = "SyncService unavailable\0";

												EnterCriticalSection(&scListContact);
												client->CloseConn = true;
												client->sthread = Contact::FINISH;
												LeaveCriticalSection(&scListContact);

												if ((libuf = send(client->s, time_error.c_str(), time_error.length(), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());

												SYSTEMTIME stt;
												GetLocalTime(&stt);
												printf("%d.%d.%d %d:%02d синхро сервер недоступен ", stt.wDay, stt.wMonth, stt.wYear, stt.wHour, stt.wMinute);
												cout << ";" << endl;

											}
										}
										else if (strcmp(CallBuf, "Time") == 0) {
											string time_now = currentDateTime();
											cout << time_now << endl;

											if (timeServer != NULL && client->hthread!=NULL) {
												client->type = Contact::CONTACT;
												strcpy_s(client->srvname, CallBuf);
												client->htimer = CreateWaitableTimer(NULL, false, NULL);
												_int64 time = -600000000;
												SetWaitableTimer(client->htimer, (LARGE_INTEGER*)&time, 0, ASWTimer, client, false);

												cout << CallBuf << endl;

												if ((libuf = send(client->s, CallBuf, sizeof(CallBuf), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());
												client->hthread = execTime(client);
											}
											else {
												time_error.append("Service");
												time_error.append(CallBuf);
												time_error.append("недоступен\0");

												EnterCriticalSection(&scListContact);
												client->CloseConn = true;
												client->sthread = Contact::FINISH;
												LeaveCriticalSection(&scListContact);

												if ((libuf = send(client->s, time_error.c_str(), time_error.length(), NULL)) == SOCKET_ERROR)
													throw SetErrorMsgText("Send:", WSAGetLastError());

								

											}
										}
									}
								}

								else //в случае ошибочного запроса
								{
									if ((libuf = send(client->s, SendError, sizeof(SendError) + 1, NULL)) == SOCKET_ERROR)
										throw SetErrorMsgText("Send:", WSAGetLastError());
									closesocket(client->s);
									client->sthread = Contact::ABORT;
									CancelWaitableTimer(client->htimer);
								}
							}
						}
						LeaveCriticalSection(&scListContact);
					}
					SleepEx(0, true);
				}
				SleepEx(0, true);
			}
			catch (string errorMsgText)
			{
				std::cout << errorMsgText << endl;
			}
		}
	}
	catch (string errorMsgText)
	{
		std::cout << errorMsgText << endl;
	}
	ExitThread(rc);
}

int* getHoursAndMinutes(string str) {
	string str_1, str_2;
	if (str[2] == ':') {
		str_1 += str.substr(0, 2);
		str_2 += str.substr(3, 2);
		int ihh = atoi(str_1.c_str());
		int imm = atoi(str_2.c_str());
		return new int[2]{ ihh, imm };
	}
	return NULL;
}

const string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%H:%M", &tstruct);

	return buf;
}

bool isBetween(string now, string from, string to) {
	return true;
}