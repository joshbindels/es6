#include <stdio.h>
#include <stdint.h>

int main()
{
    uint32_t* reg = (uint32_t*)0x40024000;
    printf("value: %x", *reg);
    return 0;
}
