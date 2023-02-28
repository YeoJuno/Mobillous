#ifndef _MOBIS_FREQ_TIMER_H_
#define _MOBIS_FREQ_TIMER_H_

class mobis_timer {
private:
    static uint8_t enabled;
    static unsigned long value;
public:
    static void enable();
    static void disable();
    static void setPeriod(unsigned long);
    static unsigned long getPeriod();
    static void (* onOverflow)() = 0;
    static void setOnOverflow(void (*)());
    static unsigned long mobisMillis(void);
};

#endif
