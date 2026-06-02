#include <stdio.h>

int fixed_value __attribute__((section(".my_fixed_section"))) = 42;

int main()
{
    printf("Value = %d\n", fixed_value);

    printf("Address of fixed_value = %p\n", (void*)&fixed_value);

    fixed_value = 100;

    printf("Updated Value = %d\n", fixed_value);

    return 0;
}
