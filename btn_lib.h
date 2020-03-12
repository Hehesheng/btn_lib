#ifndef __BTN_LIB_H__
#define __BTN_LIB_H__

#include <stdint.h>

#define BTN_NAME_MAX_LEN 10
#define BTN_DEBOUNCE_TIME 20
#define BTN_REPEAT_TIME 40
#define BTN_LONG_PRESSED_TIME 20
#define BTN_HOLD_TIME 10

typedef enum {
    BTN_EVENT_PRESSED = 0,
    BTN_EVENT_RELEASED,
    BTN_EVENT_SINGLE,
    BTN_EVENT_DOUBLE,
    BTN_EVENT_REPEAT,
    BTN_EVENT_LONG,
    BTN_EVENT_HOLD,
    BTN_EVENT_MAX,
    BTN_EVENT_NONE,
} btn_event;

typedef struct btn_handle {
    char name[BTN_NAME_MAX_LEN];
    int tick;
    uint8_t state;
    btn_event event;
    uint8_t clicked;
    uint8_t level;

    int (*read)(struct btn_handle* btn);
    int (*event_callback[BTN_EVENT_MAX])(struct btn_handle* btn);

    struct btn_handle* next;
    void* user_data;
}btn_handle_t;

typedef int (*read_btn_t)(struct btn_handle* btn);
typedef void (*btn_callback_t)(struct btn_handle* btn);

void btn_init(btn_handle_t *btn, char *name, read_btn_t read, uint8_t level);
btn_handle_t *btn_detach(btn_handle_t *btn);
btn_callback_t btn_attach(btn_handle_t*btn, btn_event event, btn_callback_t cb);
void btn_process(int tick);

#endif
