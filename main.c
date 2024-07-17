#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "bass.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

const char KEYS [] = {
    'Z', 'X', 'C', 'V', 'B', 'N', 'M',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'
};

float keys_vol[sizeof(KEYS)];
float keys_phase[sizeof(KEYS)];
float VOLUME = 0.4;
int BASE_FREQ = 200;

DWORD CALLBACK StreamProc(HSTREAM handle, float* buffer, DWORD length, void* user)
{
    float omega;
    memset(buffer, 0, length);
    for (int i = 0; i < sizeof(KEYS); i++) {
        if (!keys_vol[i])
            continue;

        omega = 2.0 * PI * pow(2.0, (i+2)/12.0) * (BASE_FREQ) / 44100.0;
        for (int c = 0; c < length / sizeof(float); c += 2) {
            buffer[c] += keys_vol[i] * sin(keys_phase[i]);
            buffer[c + 1] = buffer[c]; // stereo
            keys_phase[i] += omega;
            if (keys_vol[i] < VOLUME) {
                keys_vol[i] -= 0.003;
                if (keys_vol[i] <= 0) {
                    keys_vol[i] = 0;
                    break;
                }
            }
        }
    }
    return length;
}

void ViewInfo() {
    system("cls");
    printf("Press these keys to play:\n    ");
    for (int i = 0; i < sizeof(KEYS); i++)
    {
        printf("%c ", KEYS[i]);
    }
    printf("\n\nIncrease BASE Sound Frequency - NumPad+\n");
    printf("Decrease BASE Sound Frequency - NumPad-\n");
    printf("Escape - Exit\n");
}

int main() {
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        int code = BASS_ErrorGetCode();
        printf("BASS initialization error: %d\n", code);
        return code;
    }

    HSTREAM stream = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, (STREAMPROC*)StreamProc, 0);
    BASS_ChannelSetAttribute(stream, BASS_ATTRIB_BUFFER, 0);
    BASS_ChannelPlay(stream, FALSE);

    INPUT_RECORD console_event;
    DWORD rr; // receives the number of input records read

    ViewInfo();
    char command_buffer[150];

    while (ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &console_event, 1, &rr)) {
        if (console_event.EventType != KEY_EVENT)
            continue; // read only keyboard events
        else if (console_event.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) // exit
            break;

        sprintf_s(command_buffer, sizeof(command_buffer), "title WMKeyboard v1.0.0.0 by rzc0d3r ^| BASE Sound Frequency: %dhz", BASE_FREQ);
        system(command_buffer);

        if (console_event.Event.KeyEvent.wVirtualKeyCode == 107 && console_event.Event.KeyEvent.bKeyDown) { // NumPad+ (increase BASE_FREQ)
            if (BASE_FREQ + 5 != 1005)
                BASE_FREQ += 5;
        }
        if (console_event.Event.KeyEvent.wVirtualKeyCode == 109 && console_event.Event.KeyEvent.bKeyDown) { // NumPad- (decrease BASE_FREQ)
            if (BASE_FREQ - 5 != 55)
                BASE_FREQ -= 5;
        }
        for (int i=0; i < sizeof(KEYS); i++)
        {
            if ((char)console_event.Event.KeyEvent.wVirtualKeyCode == KEYS[i]) { // detect press on sound-key
                if (console_event.Event.KeyEvent.bKeyDown && keys_vol[i] < VOLUME)
                    keys_vol[i] = VOLUME + 0.002;
                else if (!console_event.Event.KeyEvent.bKeyDown && keys_vol)
                    keys_vol[i] -= 0.003;
                break;
            }
        }
    }

    BASS_ChannelStop(stream);
    BASS_StreamFree(stream);
    BASS_Free();

    return 0;
}
