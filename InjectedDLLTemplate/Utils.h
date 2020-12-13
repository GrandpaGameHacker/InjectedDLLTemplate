#pragma once
#include "stdheaders.h"

//const BYTE farJumpInstruction[] = { 0xff, 0x25, 0,0,0,0 };
//const BYTE jumpInstruction = { 0xEB };

void GetModuleInfo(char* ModuleName, MODULEINFO* ModuleInfo);

intptr_t ScanBasic(char* pattern, char* mask, char* begin, intptr_t size);
intptr_t ScanInternal(char* pattern, char* mask, char* begin, intptr_t size);
intptr_t ScanInternalModule(char* pattern, char* mask, char* moduleName);
