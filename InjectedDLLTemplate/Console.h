#pragma once
#include "stdheaders.h"

#define ESC "\x1b"
#define CSI "\x1b["

#define CONSTYLE_ERROR BACKGROUND_RED | BACKGROUND_INTENSITY
#define CONSTYLE_DEFAULT FOREGROUND_GREEN | FOREGROUND_INTENSITY

class Console
{
	FILE* fpin, *fpout, *fperr;
	HANDLE hstdin, hstdout;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
public:
	Console();
	~Console();

	void Init();
	void Free();
	void Write(const std::string& str);
	void FWrite(const char* format, ...);
	void WriteErr(const std::string& str);
	void SetAttribute(WORD attribute);
};

