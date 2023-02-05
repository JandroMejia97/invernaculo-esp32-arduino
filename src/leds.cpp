/* Status led library */
#include <bits/stdc++.h>
#include <vector>
using namespace std;

/* RGB colors */
#define C_GREEN "0,255,0"
#define C_ORANGE "255,140,0"
#define C_RED "255,0,0"
#define C_YELLOW "255,255,0"
#define C_SEPARATOR ','

/* RBG pin numbers */
#define LED_R 2
#define LED_G 3
#define LED_B 6

int rpgPin[] = {LED_R, LED_G, LED_B}

vector<string> splitCodes (const string &s) {
    vector<string> result;
    stringstream ss (s);
    string item;

    while (getline (ss, item, C_SEPARATOR)) {
        result.push_back (item);
    }

    return result;
}

void setRgbColor(string colorCode) {
    vector<string> colorCodes = splitCodes(colorCode);
    int i = 0;

    for (int p : rpgPin) {
        analogWrite(p, colorCodes[i])
        i++;
    }
}
