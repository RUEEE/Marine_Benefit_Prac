#include "prac_launch.h"
#include "3rd/thp/utils/wininternal.h"
#include "3rd/thp/thprac_load_exe.h"

using namespace THPrac;

static bool LocalApplyTHPrac(HANDLE process)
{
    void* extraData = nullptr;
    size_t extraSize = 0;
    return THPrac::LoadSelf(process, extraData, extraSize);
}

HANDLE __stdcall LaunchGameDirectly(std::filesystem::path game_path)
{
    bool result = true;

    STARTUPINFOW startup_info;
    PROCESS_INFORMATION proc_info;
    memset(&startup_info, 0, sizeof(STARTUPINFOW));
    startup_info.cb = sizeof(STARTUPINFOW);

    CreateProcessW(game_path.wstring().c_str(), nullptr, nullptr, nullptr, false, CREATE_SUSPENDED, nullptr, game_path.parent_path().wstring().c_str(), &startup_info, &proc_info);

    uintptr_t base = GetGameModuleBase(proc_info.hProcess);
    result = (WriteTHPracSig(proc_info.hProcess, base) && LocalApplyTHPrac(proc_info.hProcess));
    if (!result) {
        TerminateThread(proc_info.hThread, ERROR_FUNCTION_FAILED);
    }
    else {
        ResumeThread(proc_info.hThread);
    }

    CloseHandle(proc_info.hThread);
    if (result) {
        return proc_info.hProcess;
    }
    else {
        CloseHandle(proc_info.hProcess);
        return 0;
    }
}
