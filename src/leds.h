#ifndef RGB_COLOR_H
#define RGB_COLOR_H

/* RGB colors */
#define C_GREEN 0, 255, 0
#define C_ORANGE 255,140,0
#define C_RED 255,0,0
#define C_YELLOW 255,255,0

/* RBG pin numbers */
#define LED_R 2
#define LED_G 3
#define LED_B 6

#include <string>

void setRgbColor(int code[3]);

#endif