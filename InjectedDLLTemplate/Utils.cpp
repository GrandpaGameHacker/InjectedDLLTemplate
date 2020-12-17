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
    if (range > 0) {
        DWORD oldProtect;
        if (VirtualProtect(address, range, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memset(address, 0x90, range);
            VirtualProtect(address, range, oldProtect, &oldProtect);
        }
    }
}

Patch::Patch(uintptr_t address, BYTE* bytes, size_t size) :
    m_Address(address), m_Bytes(nullptr), m_OldBytes(nullptr), m_Size(size), m_bEnabled(false)
{
    m_Bytes = (BYTE*) malloc(m_Size);
    m_OldBytes = (BYTE*)malloc(m_Size);
    memcpy_s((void*)m_Bytes, m_Size, bytes, m_Size);
    memcpy_s((void*)m_OldBytes, m_Size, (void*)m_Address, m_Size);
}

Patch::~Patch()
{
    if (m_bEnabled) {
        Restore();
    }
    free(m_Bytes);
    free(m_OldBytes);
}

void Patch::Apply()
{
    if (!m_bEnabled)
    {
        m_bEnabled = true;
        DWORD oldProtect;
        VirtualProtect((void*)m_Address, m_Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy_s((void*)m_Address, m_Size, (void*)m_Bytes, m_Size);
        VirtualProtect((void*)m_Address, m_Size, oldProtect, &oldProtect);
    }
};

void Patch::Restore()
{
    if (m_bEnabled)
    {
        m_bEnabled = false;
        DWORD oldProtect;
        VirtualProtect((void*)m_Address, m_Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy_s((void*)m_Address, m_Size, (void*)m_OldBytes, m_Size);
        VirtualProtect((void*)m_Address, m_Size, oldProtect, &oldProtect);
    }
};

Hook::Hook(uintptr_t address, uintptr_t target, int BytesNeeded) :
    m_Address(address), m_Target(target), m_Bytes(nullptr), m_OldBytes(nullptr), m_Size(BytesNeeded), m_bEnabled(false)
{

#if _WIN64

    BYTE X64Jmp[] = "\xFF\x25\x00\x00\x00\x00"; // Bytes needed minimum 14
    if (m_Size < 14)
        return;
    m_Bytes = (BYTE*) malloc(m_Size);
    m_OldBytes = (BYTE*) malloc(m_Size);

    // Setting up the hook buffer
    memcpy_s((void*)m_Bytes, m_Size, (void*)X64Jmp, sizeof(X64Jmp));

    uintptr_t* jmpAbsolute = (uintptr_t*)((uintptr_t)m_Bytes + 6);
    *jmpAbsolute = m_Target;

    void* nops = (void*)((uintptr_t)m_Bytes + 14);
    int numberOfNops = m_Size - 14;
    NopCodeRange(nops, numberOfNops);

    // Copying original bytes to the internal buffer
    memcpy_s((void*)m_OldBytes, m_Size, (void*)m_Address, m_Size);
#else
    BYTE X86Jmp[] = "\xE9"; // Bytes needed minimum 5
    if (m_Size < 5)
        return;
    m_Bytes = (BYTE*)malloc(m_Size);
    m_OldBytes = (BYTE*)malloc(m_Size);

    memcpy_s((void*)m_Bytes, m_Size, (void*)X86Jmp, sizeof(X86Jmp));
    uintptr_t* jmpRelative = (uintptr_t*)((uintptr_t)m_Bytes + 1);
    *jmpRelative = (m_Target - m_Address - 5);

    void* nops = (void*)((uintptr_t)m_Bytes + 5);
    int numberOfNops = m_Size - 5;
    NopCodeRange(nops, numberOfNops);

    memcpy_s((void*)m_OldBytes, m_Size, (void*)m_Address, m_Size);
#endif
}

Hook::~Hook()
{
    if (m_bEnabled) {
        Disable();
    }
    free(m_Bytes);
    free(m_OldBytes);
}

void Hook::Enable()
{
    if (!m_bEnabled)
    {
        m_bEnabled = true;
        DWORD oldProtect;
        VirtualProtect((void*)m_Address, m_Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy_s((void*)m_Address, m_Size, (void*)m_Bytes, m_Size);
        VirtualProtect((void*)m_Address, m_Size, oldProtect, &oldProtect);
    }
}

void Hook::Disable()
{
    if (m_bEnabled)
    {
        m_bEnabled = false;
        DWORD oldProtect;
        VirtualProtect((void*)m_Address, m_Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy_s((void*)m_Address, m_Size, (void*)m_OldBytes, m_Size);
        VirtualProtect((void*)m_Address, m_Size, oldProtect, &oldProtect);
    }
}
