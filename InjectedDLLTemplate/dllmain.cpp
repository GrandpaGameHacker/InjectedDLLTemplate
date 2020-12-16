#include "dllmain.h"

Console consoleInit()
{
    if (!g_bConsoleInit) {
        g_bConsoleInit = true;
        return Console(CONSOLE_TITLE);
    }
}

bool g_bConsoleInit = false;
Console g_console = consoleInit();



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
    g_console.Write("[+] Dll Injected");

    // Make sure to close all threads/all are done before calling DllUnload
    Sleep(6000);
    DllUnload();
}

void DllUnload()
{
    HMODULE self;
    self = GetModuleHandleA(PROJECT_NAME);
    if(self)
        FreeLibraryAndExitThread(self, 0);
}
