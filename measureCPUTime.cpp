/*
Written by Sungjun
This code measures CPU cycles and time(for miliseconds) of a module(or function)
*/

void measureSpeedOfCode() {
    TCCR1A = 0;             // Set Timer/Counter Control Register A to 0 for initialization.
    TCCR1B = bit(CS10);     // Set 11st bit to 1 of Timer/Counter Control Register B.
    TCNT1 = 0;              // Initialize register which indicates Timer1's value(or counts).
    unsigned int cycles;

    /* Code under test */
    // do something
    /* End of code under test */

    cycles = TCNT1;
    Serial.print("CPU Cycles: "); Serial.print(cycles - 1);
    Serial.print(", "); Serial.print((float)(cycles - 1) / 16); Serial.println("ms");
}
