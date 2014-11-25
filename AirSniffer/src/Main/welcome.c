#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"

int main()
{
    display_init();
    printf("[AirSniffer][Welcome]LCD init finished\n");
    
    display_welcome();
    
    return 0;
}