#include "btn_lib.h"
#include <string.h>

#define EVENT_CB(btn, e)             \
    if (btn->event_callback)         \
    {                                \
        btn->event = e;              \
        btn->event_callback[e](btn); \
    }

typedef enum
{
    LEVEL_FALSE,
    LEVEL_TRUE,
} level_t;

typedef enum
{
    STATE_START = 1,
    STATE_WAIT_DEBOUNCE,
    STATE_WAIT_LONG_PRESSED,
    STATE_WAIT_HOLD,
    STATE_MAX_NUM,
} btn_state;

typedef struct state_machine
{
    btn_state state;
    level_t value;
    btn_state next_state;
    void (*state_handle)(btn_handle_t *btn, struct state_machine *s);
} state_m;

static void on_start(btn_handle_t *btn, struct state_machine *s);
static void on_debounce(btn_handle_t *btn, struct state_machine *s);
static void on_wait_long(btn_handle_t *btn, struct state_machine *s);
static void on_wait_hold(btn_handle_t *btn, struct state_machine *s);

static const state_m state_tab[] = {
    {STATE_START, LEVEL_TRUE, STATE_WAIT_DEBOUNCE, on_start},
    {STATE_START, LEVEL_FALSE, STATE_START, on_start},
    {STATE_WAIT_DEBOUNCE, LEVEL_TRUE, STATE_WAIT_LONG_PRESSED, on_debounce},
    {STATE_WAIT_DEBOUNCE, LEVEL_FALSE, STATE_START, on_debounce},
    {STATE_WAIT_LONG_PRESSED, LEVEL_TRUE, STATE_WAIT_HOLD, on_wait_long},
    {STATE_WAIT_LONG_PRESSED, LEVEL_FALSE, STATE_START, on_wait_long},
    {STATE_WAIT_HOLD, LEVEL_TRUE, STATE_WAIT_HOLD, on_wait_hold},
    {STATE_WAIT_HOLD, LEVEL_FALSE, STATE_START, on_wait_hold},
    {0, 0, 0, 0},
};

static btn_handle_t header = {0};

void btn_init(btn_handle_t *btn, char *name, read_btn_t read, uint8_t level)
{
    memset(btn, 0, sizeof(btn_handle_t));
    strncpy(btn->name, name, BTN_NAME_MAX_LEN);

    btn->state = STATE_START;
    btn->read  = read;
    btn->level = level;

    btn->next   = header.next;
    header.next = btn;
}

btn_handle_t *btn_detach(btn_handle_t *btn)
{
    btn_handle_t *node;

    for (node = &header; node->next != NULL; node = node->next)
    {
        if (node->next == btn)
        {
            node->next = btn->next;
            btn->next  = NULL;
            node       = btn;
            break;
        }
    }

    return node;
}

btn_callback_t btn_attach(btn_handle_t *btn, btn_event event, btn_callback_t cb)
{
    btn_callback_t old;

    old                        = btn->event_callback[event];
    btn->event_callback[event] = cb;

    return old;
}

static level_t get_value(btn_handle_t *btn)
{
    level_t value = LEVEL_FALSE;

    if (btn->read) value = (btn->read(btn) == btn->level) ? LEVEL_TRUE : LEVEL_FALSE;

    return value;
}

static void on_start(btn_handle_t *btn, struct state_machine *s)
{
    if (s->value == LEVEL_TRUE)
    {
        btn->state = s->next_state;
        btn->tick  = BTN_DEBOUNCE_TIME;
    }
    else if (s->value == LEVEL_FALSE)
    {
        if (btn->clicked > 0 && btn->tick == 0)
        {
            btn->clicked = 0;
        }
    }
}

static void on_debounce(btn_handle_t *btn, struct state_machine *s)
{
    if (btn->tick != 0) return;

    if (s->value == LEVEL_TRUE)
    {
        EVENT_CB(btn, BTN_EVENT_PRESSED);
        btn->tick = BTN_LONG_PRESSED_TIME;
    }
    btn->state = s->next_state;
}

static void on_wait_long(btn_handle_t *btn, struct state_machine *s)
{
    if (btn->tick == 0 && s->value == LEVEL_TRUE)
    {
        EVENT_CB(btn, BTN_EVENT_LONG);
        btn->tick  = BTN_HOLD_TIME;
        btn->state = s->next_state;
    }
    else if (s->value == LEVEL_FALSE)
    {
        EVENT_CB(btn, BTN_EVENT_RELEASED);
        btn->clicked++;
        EVENT_CB(btn, BTN_EVENT_SINGLE);
        btn->tick = BTN_REPEAT_TIME;
        if (btn->clicked > 1)
        {
            EVENT_CB(btn, BTN_EVENT_REPEAT);
        }
        if (btn->clicked == 2)
        {
            EVENT_CB(btn, BTN_EVENT_DOUBLE);
        }
        btn->state = s->next_state;
    }
}

static void on_wait_hold(btn_handle_t *btn, struct state_machine *s)
{
    if (btn->tick == 0 && s->value == LEVEL_TRUE)
    {
        EVENT_CB(btn, BTN_EVENT_HOLD);
        btn->tick = BTN_HOLD_TIME;
    }
    else if (s->value == s->value == LEVEL_FALSE)
    {
        EVENT_CB(btn, BTN_EVENT_RELEASED);
        btn->clicked = 0;
        btn->tick    = 0;
        btn->state   = s->next_state;
    }
}

void btn_process(int tick)
{
    btn_handle_t *btn;
    level_t value;

    for (btn = header.next; btn != NULL; btn = btn->next)
    {
        value = get_value(btn);
        if (btn->tick > 0)
        {
            btn->tick = btn->tick - tick;
            btn->tick = (btn->tick > 0) ? btn->tick : 0;
        }
        /* state machine */
        for (int i = 0; state_tab[i].state != 0; i++)
        {
            if (btn->state == state_tab[i].state && value == state_tab[i].value && state_tab[i].state_handle != NULL)
            {
                state_tab[i].state_handle(btn, &state_tab[i]);
                break;
            }
        }
    }
}
