#pragma once
#include "stdheaders.h"
#if !defined PROJECT_NAME
#define PROJECT_NAME ""
#endif

#define CONSOLE_TITLE "YourTitleHere!"

extern Console g_console;
extern bool g_bConsoleInit;
Console consoleInit();

void DllThread();
void DllUnload();