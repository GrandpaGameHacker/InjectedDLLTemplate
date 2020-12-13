#include "dllmain.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&DllThread, NULL, NULL, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void DllThread()
{
    Console con;
    // Insert Your Code Here
    
    // Make sure to close all threads/all are done before calling DllUnload
    DllUnload();
}

void DllUnload()
{
    HMODULE self;
    self = GetModuleHandleA(PROJECT_NAME);
    if(self)
        FreeLibraryAndExitThread(self, 0);
}
