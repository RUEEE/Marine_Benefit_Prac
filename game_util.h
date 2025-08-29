#pragma once

enum KeyOfs
{
    LeftOfs = 0xA733DC,
    UpOfs = 0xA733E4,
    RightOfs = 0xA733E0,
    DownOfs = 0xA733E8,
    ZOfs = 0xA733EC,
    XOfs = 0xA733F0,
    ShiftOfs = 0xA733F4,
    EscOfs = 0xA733F8,
    CtrlOfs = 0xA733FC,
    ENDOfs = 0
};

int GetKeyTime(KeyOfs ofs);