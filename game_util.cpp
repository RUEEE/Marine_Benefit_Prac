#include "game_util.h"
#include "3rd/thp/thprac_hook.h"
#include "3rd/thp/thprac_gui_impl_win32.h"
#include "3rd/thp/thprac_gui_impl_dx9.h"

#include <d3d9.h>

#include <format>
using namespace THPrac;

int GetKeyTime(KeyOfs ofs)
{
    return asm_call<0x656C70, Cdecl, int>(*(DWORD*)(ofs));
}