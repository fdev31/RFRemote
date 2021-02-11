// platform settings
#ifdef USE_UNO // UNO NANO & compatibles
#define RF_PIN 2
#else
#define RF_PIN G0 // G0 on M5 stack
#include <M5StickC.h>
#define USE_M5
#define BUTTON_A G39
#define BUTTON_B G37
#define LONGPRESS_DURATION 300
#endif

// RF parameters
#define PULSE_INTERVAL 24
#define PREFIX 0x06ff // first byte is the remote ID (can be changed)
#define PREFIXLEN 14

typedef struct {
    char name[4];
    int code_on;
    int code_off;
    int code_toggle;
} Control;

Control controls[] = {
    {"1", 0x7fe, 0xfb2, 0xf7e},
    {"2", 0xef5, 0x6b9, 0x675},
    {"3", 0xdf4, 0x5b8, 0x574},
    {"4", 0x4fb, 0xcbf, 0xc7b},
    {"All", 0x2f9, 0xabd, 0},
};

const char* buttons[] = {"ON", "OFF", "Toggle"};
#define NB_BUTTONS (sizeof(buttons)/sizeof(char*))
int current_button = 2;

#define NB_CONTROLS (sizeof(controls)/sizeof(Control))
int current_control = 0;

void _pulse(int bitval) {
    digitalWrite(RF_PIN, LOW);
    delay(3);
    digitalWrite(RF_PIN, HIGH);
    delayMicroseconds(bitval?1000:500);
    digitalWrite(RF_PIN, LOW);
}

#define one() _pulse(1)
#define zero() _pulse(0)

void _sendKey(int code, int len) {
    for (int i=0; i<len; i++) {
        code>>(len-1-i)&1 ? one() : zero();
    }
}

void sendKey(int code) {
#ifdef USE_UNO
    digitalWrite(LED_BUILTIN, HIGH);
#endif
    for (int i=0; i<3; i++) {
        _sendKey(PREFIX, PREFIXLEN);
        _sendKey(code, 12);
        delay(PULSE_INTERVAL);
    }
#ifdef USE_UNO
    digitalWrite(LED_BUILTIN, LOW);
#endif
}

void setup() {
#ifdef USE_UNO
    pinMode(LED_BUILTIN, OUTPUT);
#else
    M5.begin();
    M5.Lcd.setRotation(1);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.fillScreen(ORANGE);
#endif
    pinMode(RF_PIN, OUTPUT);
    digitalWrite(RF_PIN, LOW);
}

int ignore_A_release = 0;
int screen_dirty = 1;

void sanity_check() {
    if (current_button == 2 && controls[current_control].code_toggle == 0) current_button = 0;
}

void loop() {
#ifdef USE_M5
    M5.update();
    if (screen_dirty) {
        M5.Lcd.setCursor(10, 10, 2);
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.println(controls[current_control].name);
        M5.Lcd.setCursor(60, 20, 2);
        M5.Lcd.println(buttons[current_button]);
        screen_dirty = 0;
    }
    if (M5.BtnA.wasReleased()) {
        if (ignore_A_release) {
            ignore_A_release = 0;
        } else {
            current_control ++;
            screen_dirty = 1;
            if (current_control >= NB_CONTROLS) current_control = 0;
            sanity_check();
        }
    }
    if (M5.BtnB.wasPressed()) {
        current_button ++;
        screen_dirty = 1;
        if (current_button >= NB_BUTTONS) current_button = 0;
        sanity_check();
    }
    if (M5.BtnA.pressedFor(LONGPRESS_DURATION)) {
        int code = 0;
        switch (current_button) {
            case 0:
                code = controls[current_control].code_on;
                break;
            case 1:
                code = controls[current_control].code_off;
                break;
            case 2:
                code = controls[current_control].code_toggle;
                break;
        }
        sendKey(code);
        ignore_A_release = 1;
    }
#endif
}
