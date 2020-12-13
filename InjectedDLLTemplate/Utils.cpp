#include "Utils.h"

void GetModuleInfo(char* ModuleName, MODULEINFO* ModuleInfo) {
	HMODULE Module{ 0 };
	Module = GetModuleHandle(ModuleName);
	if(Module != NULL)
		GetModuleInformation(GetCurrentProcess(), Module, ModuleInfo, sizeof(MODULEINFO));
}

intptr_t ScanBasic(char* pattern, char* mask, char* begin, intptr_t size)
{
    intptr_t patternLen = strlen(mask);

    for (intptr_t i = 0; i < size; i++)
    {
        bool found = true;
        for (intptr_t j = 0; j < patternLen; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(char*)((intptr_t)begin + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return ((intptr_t)begin + i);
        }
    }
    return 0;
}

intptr_t ScanInternal(char* pattern, char* mask, char* begin, intptr_t size)
{
    intptr_t match{ 0 };
    MEMORY_BASIC_INFORMATION mbi{};

    for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
    {
        if (!VirtualQuery(curr, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

        match = ScanBasic(pattern, mask, curr, mbi.RegionSize);

        if (match != 0)
        {
            break;
        }
    }
    return match;
}

intptr_t ScanInternalModule(char* pattern, char* mask, char* moduleName)
{
    MODULEINFO modinfo;
    GetModuleInfo(moduleName, &modinfo);
    intptr_t size = modinfo.SizeOfImage;
    char* begin = (char*)modinfo.lpBaseOfDll;
    return ScanInternal(pattern, mask, begin, size);
}

void NopCodeRange(void* address, size_t range)
{
    DWORD oldProtect;
    if (VirtualProtect(address, range, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memset(address, 0x90, range);
        VirtualProtect(address, range, oldProtect, &oldProtect);
    }
}

