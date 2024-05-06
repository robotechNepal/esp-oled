#include <OLED_I2C.h>
 #include <ChronosESP32.h>
#include "graphics.h"
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUILTINLED 2
#define BUTTON 0 // ESP32 BOOT button

OLED  myOLED(21, 22); //(SDA, SCL)
ChronosESP32 watch("RoboTech Watch");

extern uint8_t SmallFont[], MediumNumbers[];

static bool deviceConnected = false;
static int id = 0;
long timeout = 10000, timer = 0, scrTimer = 0;
bool rotate = false, flip = false, hr24 = true, notify = true, screenOff = false, scrOff = false, b1;
int scroll = 0, bat = 0, lines = 0, msglen = 0;
String msg;
String msg0, msg1, msg2, msg3, msg4, msg5;


void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
    case CF_USER:
w
      flip = ((b >> 24) & 0xFF) == 1; // imperial normal, metric flipped
      rotate = ((b >> 8) & 0xFF) == 1; // celsuis normal, fahrenheit rotated
      timeout = ((b >> 16) & 0xFF) * 1000; //

      break;
  }
}


void notificationCallback(Notification notification)
{
  Serial.print("Notification received at ");
  Serial.println(notification.time);
  Serial.print("From: ");
  Serial.print(notification.app);
  Serial.print("\tIcon: ");
  Serial.println(notification.icon);
  Serial.println(notification.message);

  timer = millis();
  msg = notification.message.substring(0, 126);
  msglen = msg.length();
  lines = ceil(float(msglen) / 21);
  scroll = 0;
  scrOff = false;

  Serial.println(msglen);

}

void ringerCallback(String caller, bool state)
{
  if (state)
  {
    Serial.print("Ringer: Incoming call from ");
    Serial.println(caller);

    timer = millis() + 15000;
    msg = "Incoming call        " + caller;
    msglen = msg.length();
    lines = ceil(float(msglen) / 21);
    scroll = 0;

    scrOff = false;
  }
  else
  {
    Serial.println("Ringer dismissed");
    timer = millis() - timeout;
    scrOff = true;
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting watch");
  
  pinMode(BUILTINLED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  if (!myOLED.begin(SSD1306_128X64)) {
    Serial.println("Failed to allocate enough RAM");
    while (1);  // In case the library failed to allocate enough RAM for the display buffer...
  }
  myOLED.setFont(SmallFont);


  watch.setConfigurationCallback(configCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setRingerCallback(ringerCallback);
  
  watch.begin();

  watch.set24Hour(true);
  watch.setBattery(80);

  timer = millis() - timeout;

}

void loop() {
  watch.loop();

  button(digitalRead(BUTTON) == LOW);   // input pin
  myOLED.clrScr();

  myOLED.flipMode(flip ^ rotate); // custom function for HUD display (mirrored)
  myOLED.rotateDisplay(rotate);
  myOLED.sleepMode(scrOff & screenOff);

  digitalWrite(BUILTINLED, watch.isConnected());

  long cur = millis();
  if (cur < timer + timeout) {
    scrTimer = cur;
    long rem = (timer + timeout - cur);
    if (lines > 3 && rem < timeout - (timeout / 3) && rem > (timeout / 3)) {
      scroll = map(rem, timeout - (timeout / 3), (timeout / 3), 0, (lines - 3) * 11); //scroll if longer than 3 lines
    }
    showNotification();   // show notification
  } else {
    if (!scrOff && screenOff && cur > scrTimer + timeout) {
      scrOff = true;
    }

    printLocalTime();   // display time
    if (scrOff & screenOff) {
      myOLED.clrScr();
    }
  }

  myOLED.update();

}

void showNotification() {
  myOLED.setFont(SmallFont);
  copyMsg(msg);
  if (msglen == 0) {
    msg0 = "No messsage";
  }
  myOLED.print(msg0, LEFT, 1 - scroll);
  myOLED.print(msg1, LEFT, 12 - scroll);
  myOLED.print(msg2, LEFT, 23 - scroll);
  myOLED.print(msg3, LEFT, 34 - scroll);
  myOLED.print(msg4, LEFT, 45 - scroll);
  myOLED.print(msg5, LEFT, 56 - scroll);


}

void printLocalTime() {
  myOLED.print(watch.getAmPmC(true),  RIGHT, 10);

  myOLED.setFont(MediumNumbers);
  myOLED.print(watch.getHourZ() + watch.getTime(":%M:%S"), CENTER, 0);
  myOLED.setFont(SmallFont);
  myOLED.print(watch.getTime("%a %d %b"), CENTER, 21);

  if (watch.isConnected()) {
    myOLED.drawBitmap(0, 15, bluetooth, 16, 16);
    myOLED.drawRect(110, 23, 127, 30);
    myOLED.drawRectFill(108, 25, 110, 28);
    myOLED.drawRectFill(map(watch.getPhoneBattery(), 0, 100, 127, 110), 23, 127, 30);
  }


}

void copyMsg(String ms) {
  switch (lines) {
    case 1:
      msg0 = ms.substring(0, msglen);
      msg1 = "";
      msg2 = "";
      msg3 = "";
      msg4 = "";
      msg5 = "";
      break;
    case 2:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, msglen);
      msg2 = "";
      msg3 = "";
      msg4 = "";
      msg5 = "";
      break;
    case 3:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, msglen);
      msg3 = "";
      msg4 = "";
      msg5 = "";
      break;
    case 4:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, 63);
      msg3 = ms.substring(63, msglen);
      msg4 = "";
      msg5 = "";
      break;
    case 5:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, 63);
      msg3 = ms.substring(63, 84);
      msg4 = ms.substring(84, msglen);
      msg5 = "";
      break;
    case 6:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, 63);
      msg3 = ms.substring(63, 84);
      msg4 = ms.substring(84, 105);
      msg5 = ms.substring(105, msglen);
      break;
  }
}

void button(bool b) {
  if (b) {
    if (!b1) {
      // button debounce, code to be executed between this and the next comment

      long current = millis();
      if (scrOff) {
        scrOff = false;
        scrTimer = current;
      } else {
        if (current < timer + timeout) {
          timer = current - timeout;
        } else {
          timer = current;
        }
        scroll = 0;
        scrOff = false;
      }


      // end button debounce
      b1 = true;
    }
  } else {
    b1 = false;
  }
}
