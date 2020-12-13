#include "Console.h"



Console::Console() :
	fpin(nullptr),
	fpout(nullptr),
	fperr(nullptr),
	hstdin(NULL),
	hstdout(NULL),
	csbi()
{
	Init();
}

Console::~Console()
{
}

void Console::Init()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen_s(&fpin, "CONIN$", "r", stdin);
	freopen_s(&fpout, "CONOUT$", "w", stdout);
	freopen_s(&fperr, "CONOUT$", "w", stderr);

	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hstdout, &csbi);
	SetAttribute(CONSTYLE_DEFAULT);
}

void Console::Free()
{
	fclose(fpin);
	fclose(fpout);
	fclose(fperr);
	FreeConsole();
}

void Console::SetTitle(const std::string& str)
{
	SetConsoleTitle(str.c_str());
}

void Console::Cls()
{
	SMALL_RECT scrollRect;
	COORD scrollTarget;
	CHAR_INFO fill;
	if (!GetConsoleScreenBufferInfo(hstdout, &csbi))
	{
		return;
	}

	scrollRect.Left = 0;
	scrollRect.Top = 0;
	scrollRect.Right = csbi.dwSize.X;
	scrollRect.Bottom = csbi.dwSize.Y;

	scrollTarget.X = 0;
	scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

	fill.Char.UnicodeChar = TEXT(' ');
	fill.Attributes = csbi.wAttributes;

	ScrollConsoleScreenBuffer(hstdout, &scrollRect, NULL, scrollTarget, &fill);

	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;

	SetConsoleCursorPosition(hstdout, csbi.dwCursorPosition);
}

void Console::Write(const std::string& str)
{
	std::cout << str << std::endl;
}

void Console::FWrite(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void Console::WriteErr(const std::string& str)
{
	SetAttribute(CONSTYLE_ERROR);
	Write(str);
	SetAttribute(CONSTYLE_DEFAULT);
}

void Console::FWriteErr(const char* format, ...)
{
	SetAttribute(CONSTYLE_ERROR);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	SetAttribute(CONSTYLE_DEFAULT);
}

void Console::SetAttribute(WORD attribute)
{
	SetConsoleTextAttribute(hstdout, attribute);
}
