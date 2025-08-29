
#include "prac_launch.h"
#include "3rd/thp/thprac_hook.h"
#include "3rd/thp/thprac_load_exe.h"
#include "game_init.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

using namespace THPrac;

struct ExeSig {
    uint32_t timeStamp;
    uint32_t textSize;
};

bool GetExeInfoEx(uintptr_t hProcess, uintptr_t base, ExeSig& exeSigOut)
{
    DWORD bytesRead;
    HANDLE hProc = (HANDLE)hProcess;

    IMAGE_DOS_HEADER dosHeader;
    if (!ReadProcessMemory(hProc, (void*)base, &dosHeader, sizeof(IMAGE_DOS_HEADER), &bytesRead)) {
        return false;
    }
    IMAGE_NT_HEADERS ntHeader;
    if (!ReadProcessMemory(hProc, (void*)(base + dosHeader.e_lfanew), &ntHeader, sizeof(IMAGE_NT_HEADERS), &bytesRead)) {
        return false;
    }

    exeSigOut.timeStamp = ntHeader.FileHeader.TimeDateStamp;
    PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)(base + dosHeader.e_lfanew) + ((LONG)(LONG_PTR) & (((IMAGE_NT_HEADERS*)0)->OptionalHeader)) + ntHeader.FileHeader.SizeOfOptionalHeader);
    for (int i = 0; i < ntHeader.FileHeader.NumberOfSections; i++, pSection++) {
        IMAGE_SECTION_HEADER section;
        if (!ReadProcessMemory(hProc, (void*)(pSection), &section, sizeof(IMAGE_SECTION_HEADER), &bytesRead)) {
            return false;
        }
        if (!strcmp(".text", (char*)section.Name)) {
            exeSigOut.textSize = section.SizeOfRawData;
            return true;
        }
    }

    return false;
}

void InitFunc()
{
    InitGame();
}

void RemoteInit()
{
#define THREAD_RETURN(err, lasterr) ExitThread(err << 16 | lasterr);
    uintptr_t base = (uintptr_t)GetModuleHandleW(NULL);
    if (reinterpret_cast<uintptr_t>(&__ImageBase) == base)
        return;
    ExeSig exeSig;
    if (!GetExeInfoEx((uintptr_t)GetCurrentProcess(), base, exeSig))
        THREAD_RETURN(InjectResult::Ok, 0);
    InitFunc();
    // This has to be ExitThread, because making wWinMain return will destruct any static classes 
    // and uninitialize a lot of C runtime library stuff that we'll need when the game hits our hooks.
    // Using ExitThread here, directly, prevents all that uninitialization code from running.
    // 
    // This also means that every resource allocated by thprac gets leaked when the game exits, but
    // Windows knows how to clean that up on it's own
    // 
    // The value passed to ExitThread is interpreted as an InjectResult { .error = Ok, .lastError = 0 }
    // See thprac_load_exe.cpp, thprac_load_exe.h and inject_shellcode.cpp for more info
    THREAD_RETURN(InjectResult::Ok, 0);
#undef THREAD_RETURN
}


std::vector<std::string> game_names = {
"东方海惠堂rep系统版1.5.exe" ,
"东方海惠堂.exe",
"MarineBenefit.exe"
"MB.exe"
};

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    PWSTR pCmdLine,
    [[maybe_unused]] int nCmdShow)
{
	VEHHookInit();
    RemoteInit();

    std::vector<std::string> files;
    for (auto& game_name : game_names)// enum game first
    {
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            if (entry.is_regular_file()) {
                if (game_name == entry.path().filename().string())
                {
                    MessageBoxW(NULL, entry.path().filename().wstring().c_str(), L"找到了游戏", MB_OK);
                    LaunchGameDirectly(entry.path().wstring().c_str());
                    goto RET;
                }
            }
        }
    }
    // LaunchGameDirectly(L"C:\\disk\\touhou\\2nd\\HHT\\东方海惠堂rep系统版1.5.exe");
    //MessageBoxA(NULL, "没找到有效游戏,请确认\"东方海惠堂rep系统版1.5.exe\"存在", "", MB_OK);
RET:
    return 0;
}