#include "stdafx.h"
#include "Global.h"
#include "AcceptServer.h"
#include "DispathServer.h"
#include "GarbageCleaner.h"
#include "ConsolePipe.h"
#include "ResponseServer.h"
#include "ServiceWorkTime.h"

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Russian"); 
	SetConsoleTitle("Concurrent Server");
	
	//устанавливаем номер tcp-порта, взяв его из параметров среды или оставляю неизменным значение по умолчанию
	if (argc > 1)
	{
		int tmp = atoi(argv[1]);
		if (tmp >= 0 && tmp <= 65535)
		{
			port = atoi(argv[1]);
			cout << "Порт с консоли:   " << port << ";" << endl;
		}
		else
		{
			cout << "недопустимый порт" << endl;
		}
	}
	else
	{
		cout << "Порт по умолчанию:   " << port  << endl;
	}

	
	if (argc > 2)
	{
		int tmp = atoi(argv[2]);
		if (tmp >= 0 && tmp <= 65535)
		{
			uport = atoi(argv[2]);
		}
	}


	//устанавливаем имя загружаемой библиотеки
	if (argc > 3)
	{
		dllname = argv[3];
	}

	//устанваливаем имя именованного канала
	if (argc > 4)
	{
		nnapme = argv[4];
		cout << "Pipe Name:   " << nnapme << ";" << endl;
	}
	else
	{
		cout << "Pipe Name:   " << nnapme << "  (default);" << endl;
	}

	//устанваливаем позывной сервера
	if (argc > 5)
	{
		ucall = argv[5];
		cout << "Call:   " << ucall << ";" << endl;
	}
	else
	{
		cout << "Call:        " << ucall << "  (default);" << endl;
	}

	srand((unsigned)time(NULL));

	volatile TalkersCmd cmd = Start;			//команда управления сервером

	InitializeCriticalSection(&scListContact);

	echoServer = LoadLibrary("EchoServer.dll");
	execEcho = (HANDLE(*)(LPVOID))GetProcAddress(echoServer, "Echo");
	cout << "Echo server:" << echoServer<<endl;

	randServer = LoadLibrary("RandServer.dll");
	execRand = (HANDLE(*)(LPVOID))GetProcAddress(randServer, "Rand");
	cout << "Rand server:" << randServer << endl;

	timeServer = LoadLibrary("TimeServer.dll");
	execTime = (HANDLE(*)(LPVOID))GetProcAddress(timeServer, "Time");
	cout << "Time server:" << timeServer << endl;

	syncServer = LoadLibrary("SyncServer.dll");
	execSync = (HANDLE(*)(LPVOID))GetProcAddress(syncServer, "Sync");
	cout << "Sync server:" << syncServer << endl;


	//st1 = LoadLibrary("Servicelibrary.dll"); 
	//ts1 = (HANDLE(*)(char*, LPVOID))GetProcAddress(st1, "SSS"); //импортируем функцию
	//if (st1 == NULL) cout << "Fail DLL;" << endl;
	//else cout << "Loaded DLL:      " << dllname << ";" << endl << endl;

	//создаем потоки
	hAcceptServer = CreateThread(NULL, NULL, AcceptServer, (LPVOID)&cmd, NULL, NULL);
	HANDLE hDispathServer = CreateThread(NULL, NULL, DispathServer, (LPVOID)&cmd, NULL, NULL);
	HANDLE hGarbageCleaner = CreateThread(NULL, NULL, GarbageCleaner, (LPVOID)&cmd, NULL, NULL);
	HANDLE hConsolePipe = CreateThread(NULL, NULL, ConsolePipe, (LPVOID)&cmd, NULL, NULL);
	HANDLE hResponseServer = CreateThread(NULL, NULL, ResponseServer, (LPVOID)&cmd, NULL, NULL);
	//HANDLE hServiceWorkTime = CreateThread(NULL, NULL, ServiceWorkTime, (LPVOID)&cmd, NULL, NULL);

	//устанавливаем приоритеты
	SetThreadPriority(hGarbageCleaner, THREAD_PRIORITY_BELOW_NORMAL);
	SetThreadPriority(hDispathServer, THREAD_PRIORITY_NORMAL);
	SetThreadPriority(hConsolePipe, THREAD_PRIORITY_NORMAL);
	SetThreadPriority(hResponseServer, THREAD_PRIORITY_ABOVE_NORMAL);
	SetThreadPriority(hAcceptServer, THREAD_PRIORITY_HIGHEST);
	//SetThreadPriority(hServiceWorkTime, THREAD_PRIORITY_HIGHEST);


	//ждем завершения потоков
	WaitForSingleObject(hAcceptServer, INFINITE);
	WaitForSingleObject(hDispathServer, INFINITE);
	WaitForSingleObject(hGarbageCleaner, INFINITE);
	WaitForSingleObject(hConsolePipe, INFINITE);
	WaitForSingleObject(hResponseServer, INFINITE);
	//WaitForSingleObject(hServiceWorkTime, INFINITE);

	//закрываем дескрипторы (освобождаем ресурсы)
	CloseHandle(hAcceptServer);
	CloseHandle(hDispathServer);
	CloseHandle(hGarbageCleaner);
	CloseHandle(hConsolePipe);
	CloseHandle(hResponseServer);
	//CloseHandle(hServiceWorkTime);

	DeleteCriticalSection(&scListContact);

	FreeLibrary(echoServer);
	FreeLibrary(timeServer);
	FreeLibrary(randServer);
	FreeLibrary(syncServer);

	system("pause");
	return 0;
};