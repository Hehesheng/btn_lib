#include <stdio.h>
#include <stdlib.h>
#include "btn_lib.h"

btn_handle_t btn;

int get_pin(btn_handle_t* btn)
{
    return read_pin();
}

void callback(btn_handle_t* btn)
{
    printf("btn [%s] get event:%d\n", btn->name, btn->event);
}

int main(int argc, char** argv)
{
    btn_init(&btn, "btn", get_pin, 1);

    for (int i = 0; i < BTN_EVENT_MAX; i++)
    {
        btn_attach(&btn, i, callback);
    }

    while (1)
    {
        btn_process(10);
        delay_ms(10);
    }
    
    return 0;
}
