#pragma once
#include "Global.h"
//для вы-ния процедуры подключ.клиентов
static void WaitClients()	 
{
	bool ListEmpty = false;
	while(!ListEmpty) 
	{
		EnterCriticalSection(&scListContact);
		ListEmpty = Contacts.empty();
		LeaveCriticalSection(&scListContact);
		SleepEx(0, TRUE); // 0 - по умолч.; true - для запуска асинх.процедур
	}
}

//для подключения клиентов
bool AcceptCycle(int squirt, SOCKET* s)
{	
	bool rc = false;
	Contact c(Contact::ACCEPT,"AcceptServer");
	
	c.hAcceptServer=hAcceptServer;

	if ((c.s = accept(*s,(sockaddr*)&c.prms, &c.lprms)) == INVALID_SOCKET)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK) 
			throw  SetErrorMsgText("Accept:",WSAGetLastError());
	}
	else //если подключился
	{ 
		rc = true;               
		InterlockedIncrement(&Accept);
		InterlockedIncrement(&Work);  
		EnterCriticalSection(&scListContact);
		Contacts.push_front(c);
		LeaveCriticalSection(&scListContact);
		SetEvent(Event); // Переводим событие в сигнальное состояние
	}
	return rc;  
};

// цикл обработки команд
void CommandsCycle(TalkersCmd& cmd, SOCKET* s)
{
	int counter = 0;
	while (cmd!=Exit) 
	{	
		switch (cmd)   
		{
		case Start:  //возобновить подключение клиентов
			cmd = Getcommand; 
			counter = MAXCLIENTS;
			break; 
		case Stop:  //остановить подключение клиентов 
			cmd = Getcommand;   
			counter = 0;
			break;
		case Wait:  //обработать текущих, а после разрешить подключение новых клиентов
			WaitClients();
			cmd = Getcommand;
			counter = MAXCLIENTS;
			break;	
		case Shutdown:  //обработать текущих, а после завершить сервер
			WaitClients();
			cmd = Exit;
			break;			
		}; 

		if(cmd!=Exit&&counter>Work)
		{
			if (AcceptCycle(counter, s))   //цикл  запрос/подключение (accept) 
			{
				cmd = Getcommand;      
			}    
			SleepEx(0, TRUE);
		} 	

	}
};

// Потоковая функция обработки запросов на подключение клиентов
DWORD WINAPI AcceptServer (LPVOID pPrm)
{
	DWORD rc = 0; //код возврата  
	SOCKET  ServerSocket;
	WSADATA wsaData;  

	try
	{
		if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
			throw  SetErrorMsgText("Startup:",WSAGetLastError());

		if ((ServerSocket = socket(AF_INET, SOCK_STREAM, NULL))== INVALID_SOCKET) 
			throw  SetErrorMsgText("Socket:",WSAGetLastError());

		SOCKADDR_IN Server_IN;  
		Server_IN.sin_family  = AF_INET;
		Server_IN.sin_port = htons(port);		//из Global.h (2000 по умолч.)
		Server_IN.sin_addr.s_addr =ADDR_ANY;
		
		if (bind(ServerSocket,(LPSOCKADDR)&Server_IN, sizeof(Server_IN))== SOCKET_ERROR)
			throw  SetErrorMsgText("Bind:",WSAGetLastError());

		if (listen(ServerSocket,SOMAXCONN)== SOCKET_ERROR)  
			throw  SetErrorMsgText("Listen:",WSAGetLastError());

		// Переводим сокет в неблокирующий режим
		//accept не приостан.вып-ние потока, а возвращ.знач.нового сокета, если 
		// обнаружен запрос на создание канала
		u_long nonblk;  
		if (ioctlsocket( ServerSocket, FIONBIO, &(nonblk = 1)) == SOCKET_ERROR)
			throw SetErrorMsgText("Ioctlsocket:",WSAGetLastError());

		TalkersCmd* command = (TalkersCmd*)pPrm;
		CommandsCycle(*((TalkersCmd*)command),&ServerSocket);

		if (closesocket(ServerSocket)== SOCKET_ERROR)
			throw  SetErrorMsgText("Сlosesocket:",WSAGetLastError());

		if (WSACleanup()== SOCKET_ERROR)         
			throw  SetErrorMsgText("Cleanup:",WSAGetLastError());
	}
	catch (string errorMsgText)
	{ 
		std::cout<<errorMsgText<<endl;
	}
	ExitThread(rc);  
}