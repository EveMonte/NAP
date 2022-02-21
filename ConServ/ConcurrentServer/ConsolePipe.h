#pragma once
#include "Global.h"
//С ИК, связь RC и PS
DWORD WINAPI ConsolePipe(LPVOID pPrm)
{
	HANDLE hPipe;
	DWORD rc = 0;
	try
	{
		BOOL fSuccess;

		//установка атрибутов безопасности
		SECURITY_ATTRIBUTES SecurityAttributes;
		SECURITY_DESCRIPTOR SecurityDescriptor;

		fSuccess = InitializeSecurityDescriptor(
			&SecurityDescriptor,
			SECURITY_DESCRIPTOR_REVISION);

		if (!fSuccess) {
			throw new string("InitializeSecurityDescriptor():");
		}

		fSuccess = SetSecurityDescriptorDacl(
			&SecurityDescriptor,
			TRUE,
			NULL,
			FALSE);

		if (!fSuccess) {
			throw new string("SetSecurityDescriptorDacl():");
		}

		SecurityAttributes.nLength = sizeof(SecurityAttributes);
		SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;
		SecurityAttributes.bInheritHandle = FALSE;

		//создание именованного канала
		char rnnapme[50];
		strcpy(rnnapme, "\\\\.\\pipe\\");
		strcat(rnnapme, nnapme);
		if ((hPipe = CreateNamedPipe(rnnapme,
			PIPE_ACCESS_DUPLEX,				//дуплексный канал 
			PIPE_TYPE_MESSAGE | PIPE_WAIT,	// сообщения|синхронный
			1, NULL, NULL,					// максимум 1 экземпляр
			INFINITE, &SecurityAttributes)) == INVALID_HANDLE_VALUE)
			throw SetErrorMsgText("Create:", GetLastError());

		while (*((TalkersCmd*)pPrm) != Exit) //цикл работы
		{
			if (!ConnectNamedPipe(hPipe, NULL))
				// ожидать клиента   
				throw SetErrorMsgText("Connect:", GetLastError());
			char ReadBuf[50], WriteBuf[50];
			DWORD nBytesRead = 0, nBytesWrite = 0;
			TalkersCmd SetCommand;
			bool Check;
			while (*((TalkersCmd*)pPrm) != Exit)
			{
				if (*((TalkersCmd*)pPrm) == Getcommand) {

					if (!ReadFile(hPipe, ReadBuf, sizeof(ReadBuf), &nBytesRead, NULL))
						break;
					if (nBytesRead > 0)
					{
						Check = true;
						int n = atoi(ReadBuf);
						switch (n)
						{
						case 0:
							sprintf(WriteBuf, "%s", "Start");
							SetCommand = TalkersCmd::Start;
							break;
						case 1:
							sprintf(WriteBuf, "%s", "Stop");
							SetCommand = TalkersCmd::Stop;
							break;
						case 2:
							sprintf(WriteBuf, "%s", "Exit");
							SetCommand = TalkersCmd::Exit;
							break;
						case 3:
							sprintf(WriteBuf, "\nAccept: %i\nFail: %i\nFinished: %i\nWork: %i\n", Accept, Fail, Finished, Work);
							Check = false;
							break;
						case 4:
							sprintf(WriteBuf, "%s", "Wait");
							SetCommand = TalkersCmd::Wait;
							break;
						case 5:
							sprintf(WriteBuf, "%s", "Shutdown");
							SetCommand = TalkersCmd::Shutdown;
							break;
						case 8:
							std::cout << 8 << std::endl;

							Check = false;
							if (echoServer != NULL) {
								*((TalkersCmd*)pPrm) = TalkersCmd::Wait;
								FreeLibrary(echoServer);
								sprintf(WriteBuf, "%s", "Echo unloaded");
								echoServer = NULL;
								execEcho = NULL;
								
							}
							else {

								echoServer = LoadLibrary("EchoServer.dll");
								sprintf(WriteBuf, "%s", "Echo loaded");
								execEcho = (HANDLE(*)(LPVOID))GetProcAddress(echoServer, "Echo");
								
							}
							break;
						case 9:
							std::cout << 9 << std::endl;

							Check = false;
							if (timeServer != NULL) {
								*((TalkersCmd*)pPrm) = TalkersCmd::Wait;
								FreeLibrary(timeServer);
								sprintf(WriteBuf, "%s", "Time unloaded");
								timeServer = NULL;
								execTime = NULL;
							}
							else {
								Sleep(500);
								timeServer = LoadLibrary("TimeServer.dll");
								sprintf(WriteBuf, "%s", "Time loaded");
								execTime = (HANDLE(*)(LPVOID))GetProcAddress(timeServer, "Time");

							}
							break;
						case 10:
							std::cout << 10 << std::endl;

							Check = false;
							if (randServer != NULL) {
								*((TalkersCmd*)pPrm) = TalkersCmd::Wait;
								FreeLibrary(randServer);
								sprintf(WriteBuf, "%s", "Rand unloaded");
								randServer = NULL;
								execTime = NULL;
							}
							else {
								randServer = LoadLibrary("RandServer.dll");
								sprintf(WriteBuf, "%s", "Rand loaded");
								execRand = (HANDLE(*)(LPVOID))GetProcAddress(randServer, "Rand");
							}
							break;
						case 11:
							std::cout << 11 << std::endl;
							Check = false;
							if (syncServer != NULL) {
								*((TalkersCmd*)pPrm) = TalkersCmd::Wait;
								FreeLibrary(syncServer);
								sprintf(WriteBuf, "%s", "Sync unloaded");
								syncServer = NULL;
								execTime = NULL;
							}
							else {
								syncServer = LoadLibrary("SyncServer.dll");
								sprintf(WriteBuf, "%s", "Sync loaded");
								execSync = (HANDLE(*)(LPVOID))GetProcAddress(syncServer, "Sync");
							}
							break;
						default:
							sprintf(WriteBuf, "%s", "NoCmd");
							Check = false;
							break;
						}
						if (Check == true)
						{
							*((TalkersCmd*)pPrm) = SetCommand;
						}

						printf("ConsolePipe: команда %s\n", WriteBuf);
						if (!WriteFile(hPipe, WriteBuf, sizeof(WriteBuf), &nBytesRead, NULL))
							throw new string("CP WRITE ERROR");
					}
				}
				else Sleep(1000);
			}
			if (!DisconnectNamedPipe(hPipe))           // отключить клиента   
				throw SetErrorMsgText("disconnect:", GetLastError());
		}
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);
	}
	catch (string ErrorPipeText)
	{
		cout << ErrorPipeText << endl;
	}
	ExitThread(rc);
}