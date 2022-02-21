#include "pch.h"
#include "windows.h"
#include "SyncServer.h"


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return TRUE;
}

extern "C" __declspec(dllexport) HANDLE Sync(LPVOID param) {
    return CreateThread(NULL, NULL, SyncServer, param, NULL, NULL);
}

