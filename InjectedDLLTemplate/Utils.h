#pragma once
#include "stdheaders.h"
void GetModuleInfo(char* ModuleName, MODULEINFO* ModuleInfo);

// Thanks Rake from Guided Hacking :)
// https://guidedhacking.com/threads/external-internal-pattern-scanning-guide.14112/
uintptr_t ScanBasic(char* pattern, char* mask, char* begin, size_t size);
uintptr_t ScanInternal(char* pattern, char* mask, char* begin, size_t size);
uintptr_t ScanInternalModule(char* pattern, char* mask, char* moduleName);

void NopCodeRange(void* address, size_t range);

// Basic Patching Class
class Patch {
    uintptr_t m_Address;
    BYTE* m_Bytes;
    BYTE* m_OldBytes;
    size_t m_Size;
    bool m_bEnabled;
    
public:
    explicit Patch(uintptr_t address, BYTE* bytes, size_t size);
    ~Patch();
    void Apply();
    void Restore();
};


// Basic Hooking Class
class Hook {
    uintptr_t m_Address;
    uintptr_t m_Target;
    BYTE* m_Bytes;
    BYTE* m_OldBytes;
    size_t m_Size;
    bool m_bEnabled;
public:
    explicit Hook(uintptr_t address, uintptr_t target, size_t BytesNeeded);
    ~Hook();
    void Enable();
    void Disable();
};