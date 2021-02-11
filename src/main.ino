#ifdef USE_UNO
#define RF_PIN 2
#else
#define RF_PIN G0 // G0 on M5 stack
#include <M5StickC.h>
#define USE_MENU
#define BUTTON_A G39
#define BUTTON_B G37
#endif
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

const char* buttons[] = {"ON", "OFF", "TOGGLE"};
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
  M5.Lcd.setTextColor(WHITE,BLACK);
  M5.Lcd.setCursor(40, 10, 2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.println("RCU");
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
#ifdef USE_MENU
  M5.update();
  if (screen_dirty) {
      M5.Lcd.setCursor(40, 10, 2);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println(controls[current_control].name);
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
  if (M5.BtnA.pressedFor(500)) {
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

/*
    sendKey(0xf7e); // 1 - toggle
    sendKey(0x7fe); // 1 - on
    sendKey(0xfb2); // 2 - off

    sendKey(0x675); // 2 - toggle
    sendKey(0xef5); // 2 - on
    sendKey(0x6b9); // 2 - off

    sendKey(0x574); // 3 - toggle
    sendKey(0xdf4); // 3 - on
    sendKey(0x5b8); // 3 - off

    sendKey(0xc7b); // 4 - toggle
    sendKey(0x4fb); // 4 - on
    sendKey(0xcbf); // 4 - off

    sendKey(0x2f9); // all on
    sendKey(0xabd); // all off

    if (pingpong++ %2 == 0) {
        sendKey(0x2f9);
        } else {
        sendKey(0xabd);
        }
    */


}
