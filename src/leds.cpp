/* Status led library */
#include <bits/stdc++.h>
#include <vector>
using namespace std;

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
