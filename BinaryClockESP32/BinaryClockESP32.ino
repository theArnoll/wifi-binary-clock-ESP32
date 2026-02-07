/*
  Wi-Fi time getting codes based on:
  https://RandomNerdTutorials.com/esp32-date-time-ntp-client-server-arduino/
*/

#include <WiFi.h>
#include <time.h>
#include "WiFiConfig.h"
 
// Wifi ssid, pwd
 
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8*3200;

const uint8_t longMonth[] = {1, 3, 5, 7, 8, 10, 12};

struct current {
  uint8_t month, day, hour, minute, second;
  uint16_t ms;
  bool leapyr, apm,  wifiConnection, timeUpdating,  timeUpdateFailure;
// true= 2/29,  PM, Wi-Fi connected, Updating time, time updating failed
};
struct shftRegPin {
  uint8_t Din, latch, clk;
};

struct current now = {
  .month = 12,
  .day = 31,
  .hour = 12,
  .minute = 59,
  .second = 59,
  .ms = 0,
  .leapyr = true,
  .apm = true,
  .wifiConnection = false,
  .timeUpdating = false,
  .timeUpdateFailure = true
};

struct shftRegPin reg = {
  .Din = 6,
  .latch = 5,
  .clk = 7
};

uint8_t outputPackage[4] = {0, 0, 0, 0}, lastUpdMin = 61;

void wifiWaitingAction()
{
  delay(500);
  // Serial.print(".");
}

void failGetTime()
{
  now.timeUpdateFailure = true;
  // Serial.println("Failed to obtain time");
}

void getTime(uint8_t* pack)
{
  now.timeUpdating = true;
  packAndDisplay(pack);
  configTime(gmtOffset_sec, 0 /*daylightOffset_sec*/, ntpServer);
  now.timeUpdating = false;
  packAndDisplay(pack);
}

void packAndDisplay(uint8_t* pack)
{
  memset(pack, 0, 4);
  /* 
  0b mddh mmss
  mn -d ay hr | minut secnd 
  +8 WC +8 +8 | AP +8 ly +8
  +4 sy +4 +4 | -- +4 31 +4
  +2 sf +2 +2 | 32 +2 32 +2
  +1 16 +1 +1 | 16 +1 16 +1
  ly = leap year, 31 = long month,
  WC = Wi-Fi Connected, sy = syncing, sf = sync fail
  */
  uint8_t location = 7, base = 8;

  // month
  for (uint8_t re = 0; re <= 3; re++)
  {
    if (now.month & base) pack[re] |= (1 << location);
    base /= 2;
  }
  location--;

  // day
  base = 8;
  if (now.day & 16) pack[3] |= (1 << location);
  location--;
  for (uint8_t re = 0; re <= 3; re++)
  {
    if (now.day & base) pack[re] |= (1 << location);
    base /= 2;
  }
  location--;

  // hr
  base = 8;
  for (uint8_t re = 0; re <= 3; re++)
  {
    if (now.hour & base) pack[re] |= (1 << location);
    base /= 2;
  }
  location--;

  // min
  base = 8;
  if (now.minute & 32) pack[2] |= (1 << location);
  if (now.minute & 16) pack[3] |= (1 << location);
  location--;
  for (uint8_t re = 0; re <= 3; re++)
  {
    if (now.minute & base) pack[re] |= (1 << location);
    base /= 2;
  }
  location--;

  // sec
  base = 8;
  if (now.second & 32) pack[2] |= (1 << location);
  if (now.second & 16) pack[3] |= (1 << location); 
  location--;
  for (uint8_t re = 0; re <= 3; re++)
  {
    if (now.second & base) pack[re] |= (1 << location);
    base /= 2;
  }
  location--;

  // status
  pack[0] |= (now.wifiConnection << 6);
  pack[1] |= (now.timeUpdating << 6);
  pack[2] |= (now.timeUpdateFailure << 6);
  pack[0] |= (now.apm << 3);
  pack[0] |= (now.leapyr << 1);
  bool lm = (now.month <= 7 && now.month % 2 == 1) || (now.month >= 8 && now.month % 2 == 0);
  pack[1] |= (lm << 1);

  for (uint8_t re = 0; re < 4; re++)
  {
    digitalWrite(4, re & 2); // Decoder
    digitalWrite(3, re & 1);
    digitalWrite(reg.latch, LOW);
    shiftOut(reg.Din, reg.clk, LSBFIRST, pack[re]);
    digitalWrite(reg.latch, HIGH);
  }
}

void setup(){
  // DO NOT USE pin 2, 8, 9(, 20, 21)
  for (uint8_t selected = 3; selected <= 7; selected++) {
    pinMode(selected, OUTPUT);
  }
  /*
  3, 4, 5, 6, 7
  |DEC|| 74595 |
  */

  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) {
    wifiWaitingAction();
  }
  now.wifiConnection = 1;

  getTime(outputPackage);
}

void loop(){
  // delay(5000);
  now.timeUpdateFailure = false;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    failGetTime();
    return;
  }
  
  now.month = timeinfo.tm_mon + 1;
  now.day = timeinfo.tm_mday;
  now.hour = timeinfo.tm_hour;
  now.minute = timeinfo.tm_min;
  now.second = timeinfo.tm_sec;
  now.ms = 0;
  now.leapyr = timeinfo.tm_year % 4 == 0 && (timeinfo.tm_year % 100 != 0 || timeinfo.tm_year % 400 == 0);
  now.apm = (now.hour >= 12);
  if (now.apm && now.hour != 12) now.hour %= 12;
  // if (now.hour == 0) now.hour = 12;

  packAndDisplay(outputPackage);

  if(lastUpdMin - now.minute >= 5 || lastUpdMin - now.minute <= -5)
  {
    getTime(outputPackage);
    lastUpdMin = now.minute;
  }
}