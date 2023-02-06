/* Status led library */
#include "Arduino.h"
#include "leds.h"

int rgbPin[] = {LED_R, LED_G, LED_B};

void setRgbColor(int colorCode[3]) {
    for (int i = 0; i < 3; i++) {
        analogWrite(rgbPin[i], colorCode[i]);
    }
}
