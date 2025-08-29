#include "utils.h"

void memswap(void* buf1_, void* buf2_, unsigned int len)
{
    unsigned char* buf1 = (unsigned char*)buf1_;
    unsigned char* buf2 = (unsigned char*)buf2_;

    for (unsigned i = 0; i < len; i++) {
        unsigned char temp = buf1[i];
        buf1[i] = buf2[i];
        buf2[i] = temp;
    }
}