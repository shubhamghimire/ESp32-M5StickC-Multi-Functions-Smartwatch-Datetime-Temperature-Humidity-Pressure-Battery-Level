#include <M5StickC.h>

#include <WiFi.h>

#include "time.h"

#include <Wire.h>

#include <Adafruit_Sensor.h>

#include <Adafruit_BME280.h>



Adafruit_BME280 bme;



const char* ssid = "put_your_wifi_ssid_here";

const char* password = "put_your_wifi_password_here";

const char* ntpServer = "ntp.jst.mfeed.ad.jp";



RTC_TimeTypeDef RTC_TimeStruct;

RTC_DateTypeDef RTC_DateStruct;



int battery = 0;

float b = 0;

int state = 1;



void setup() {

  // put your setup code here, to run once:



  M5.begin();

  Wire.begin(0, 26); //Wire.begin(sda, scl);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 0); // using home button for wakeup, second parameter is the state of the button that will trigger the wakeup (0 or 1)

  M5.Lcd.setRotation(3);

  M5.Axp.ScreenBreath(11);  //screen brightness 7-15

  M5.Lcd.fillScreen(BLACK);

  setTime();

}



void loop() {

  // put your main code here, to run repeatedly:



  int modeButton = digitalRead(M5_BUTTON_HOME);

  int sleepButton = digitalRead(M5_BUTTON_RST);  //prefered using a button for sleep instead of a timeframe



  switch (state) {

    case 1:



      if (modeButton == HIGH && sleepButton == HIGH) {



        loadDisplay();



        batteryPercent();



        delay(500);





      } else if (modeButton == LOW && sleepButton == HIGH) {

        M5.Lcd.fillScreen(BLACK);    //Added visual indicator to add time between the button press and state change.

        M5.Lcd.setCursor(2, 25);

        M5.Lcd.setTextSize(1);

        M5.Lcd.printf("Changing Modes");

        delay(1000);

        state = 2;

      } else if (modeButton == HIGH && sleepButton == LOW) {

        M5.Lcd.fillScreen(BLACK);

        M5.Axp.DeepSleep();

      }

      break;



    case 2:

      if (modeButton == HIGH && sleepButton == HIGH) {



        if (!bme.begin()) {

          Serial.println("Could not find a valid BMP280 sensor, check wiring!");

          state = 1;

        }



        float tmp = bme.readTemperature();

        float hum = bme.readHumidity();

        float pressure = bme.readPressure();

        Serial.printf("Temperature: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);



        M5.Lcd.fillScreen(BLACK);

        M5.Lcd.setCursor(0, 15);

        M5.Lcd.setTextColor(WHITE, BLACK);

        M5.Lcd.setTextSize(1);

        M5.Lcd.printf("Temp: %2.1f  \r\nHumi: %2.0f%%  \r\nPressure:%2.0fPa\r\n", tmp, hum, pressure);



        delay(1000);



      } else if (modeButton == LOW && sleepButton == HIGH) {

        M5.Lcd.fillScreen(BLACK);   //Added visual indicator to add time between the button press and state change.

        M5.Lcd.setCursor(2, 25);

        M5.Lcd.setTextSize(1);

        M5.Lcd.printf("Changing Modes");

        delay(1000);

        state = 1;

      } else if (modeButton == HIGH && sleepButton == LOW) {

        M5.Lcd.fillScreen(BLACK);

        M5.Axp.DeepSleep();

      }

      break;

  }



}



void batteryPercent() {

  M5.Lcd.setCursor(115, 2);

  M5.Lcd.setTextSize(1);

  b = M5.Axp.GetVbatData() * 1.1 / 1000;

  battery = ((b - 3.0) / 1.2) * 100;



  if (battery > 100)

    battery = 100;

  else if (battery < 100 && battery > 9)

    M5.Lcd.print(" ");

  else if (battery < 9)

    M5.Lcd.print("  ");

  if (battery < 10)

    M5.Axp.DeepSleep();

  M5.Lcd.print(battery);

  M5.Lcd.print("%");



}



void loadDisplay() {



  static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};



  M5.Rtc.GetTime(&RTC_TimeStruct);

  M5.Rtc.GetData(&RTC_DateStruct);

  if (RTC_TimeStruct.Hours > 12) {  //change to 12h format

    RTC_TimeStruct.Hours -= 12;

  }

  M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setTextSize(1);

  M5.Lcd.setCursor(2, 2);

  M5.Lcd.printf("%s   %04d-%02d-%02d", wd[RTC_DateStruct.WeekDay], RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date);

  M5.Lcd.drawLine(0, 15, 159, 15, TFT_WHITE);

  M5.Lcd.setCursor(2, 25);

  M5.Lcd.setTextSize(3);

  M5.Lcd.printf("%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);

  M5.Lcd.setCursor(100, 32);

  M5.Lcd.setTextSize(2);

  M5.Lcd.printf("%02d", RTC_TimeStruct.Seconds);

}



void setTime() {



  // connect to wifi

  Serial.printf("Connecting to %s ", ssid);

  WiFi.begin(ssid, password);

  delay(500);

  while (WiFi.status() == WL_CONNECTED) {  //if wifi does connect go ahead and set the time with NTP

    Serial.println(" CONNECTED, SETTING TIME");



    // Set NTP time to local

    configTime(9 * 3600, 3600, ntpServer);  // 9 is for Japan's timezone. (utc + 9)



    // Get local time

    struct tm timeInfo;

    if (getLocalTime(&timeInfo)) {



      // Set RTC time

      RTC_TimeTypeDef TimeStruct;

      TimeStruct.Hours   = timeInfo.tm_hour;

      TimeStruct.Minutes = timeInfo.tm_min;

      TimeStruct.Seconds = timeInfo.tm_sec;

      M5.Rtc.SetTime(&TimeStruct);



      RTC_DateTypeDef DateStruct;

      DateStruct.WeekDay = timeInfo.tm_wday;

      DateStruct.Month = timeInfo.tm_mon + 1;

      DateStruct.Date = timeInfo.tm_mday;

      DateStruct.Year = timeInfo.tm_year + 1900;

      M5.Rtc.SetData(&DateStruct);

    }

    // Disconnect Wifi if it did connect

    WiFi.disconnect(true);

    WiFi.mode(WIFI_OFF);

  }

}
