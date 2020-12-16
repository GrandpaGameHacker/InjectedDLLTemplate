#include "Utils.h"

void GetModuleInfo(char* ModuleName, MODULEINFO* ModuleInfo) {
	HMODULE Module{ 0 };
	Module = GetModuleHandle(ModuleName);
	if(Module != NULL)
		GetModuleInformation(GetCurrentProcess(), Module, ModuleInfo, sizeof(MODULEINFO));
}

uintptr_t ScanBasic(char* pattern, char* mask, char* begin, size_t size)
{
    uintptr_t patternLen = strlen(mask);

    for (uintptr_t i = 0; i < size; i++)
    {
        bool found = true;
        for (uintptr_t j = 0; j < patternLen; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(char*)((uintptr_t)begin + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return ((uintptr_t)begin + i);
        }
    }
    return 0;
}

uintptr_t ScanInternal(char* pattern, char* mask, char* begin, size_t size)
{
    uintptr_t match{ 0 };
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

uintptr_t ScanInternalModule(char* pattern, char* mask, char* moduleName)
{
    MODULEINFO modinfo;
    GetModuleInfo(moduleName, &modinfo);
    uintptr_t size = modinfo.SizeOfImage;
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

Patch::Patch(uintptr_t address, BYTE* bytes, size_t size) :
    m_Address(address), m_Bytes(nullptr), m_OldBytes(nullptr), m_Size(size), m_bPatchEnabled(false)
{
    m_Bytes = (BYTE*) malloc(m_Size);
    m_OldBytes = (BYTE*)malloc(m_Size);
    memcpy_s((void*)m_Bytes, m_Size, bytes, m_Size);
    memcpy_s((void*)m_OldBytes, m_Size, (void*)m_Address, m_Size);
}

Patch::~Patch()
{
    if (m_bPatchEnabled) {
        Restore();
    }
    free(m_Bytes);
    free(m_OldBytes);
}

void Patch::Apply()
{
    if (!m_bPatchEnabled)
    {
        m_bPatchEnabled = true;
        DWORD oldProtect;
        VirtualProtect((void*)m_Address, m_Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy_s((void*)m_Address, m_Size, (void*)m_Bytes, m_Size);
        VirtualProtect((void*)m_Address, m_Size, oldProtect, &oldProtect);
    }
};

void Patch::Restore()
{
    if (m_bPatchEnabled)
    {
        m_bPatchEnabled = false;
        DWORD oldProtect;
        VirtualProtect((void*)m_Address, m_Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy_s((void*)m_Address, m_Size, (void*)m_OldBytes, m_Size);
        VirtualProtect((void*)m_Address, m_Size, oldProtect, &oldProtect);
    }
};
