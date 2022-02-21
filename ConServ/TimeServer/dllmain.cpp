#include "pch.h"
#include "Windows.h"
#include "TimeServer.h"

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return TRUE;
}

extern "C" __declspec(dllexport) HANDLE Time(LPVOID param) {
    return CreateThread(NULL, NULL, TimeServer, param, NULL, NULL);
}

