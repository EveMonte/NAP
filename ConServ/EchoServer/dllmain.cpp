// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "Windows.h"
#include "EchoServer.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    return TRUE;
}

extern "C" __declspec(dllexport) HANDLE Echo(LPVOID param) {
    return CreateThread(NULL, NULL, EchoServer, param, NULL, NULL);
}