#ifndef _MOBIS_FREQ_TIMER_H_
#define _MOBIS_FREQ_TIMER_H_

class FrequencyTimer2 {
private:
    static uint8_t enabled;
public:
    static void enable();
    static void disable();
    static void setPeriod(unsigned long);
    static unsigned long getPeriod();
    static void (* onOverflow)() = 0;
    static void setOnOverflow(void (*)());
};

#endif