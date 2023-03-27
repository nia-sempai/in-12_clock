///////////////Библиотеки////////////////
#include <NTPClient.h>                  //NTP клиент 
#include <ESP8266WiFi.h>                //Предоставляет специальные процедуры Wi-Fi для ESP8266, которые мы вызываем для подключения к сети
#include <WiFiUdp.h>                    //Обрабатывает отправку и получение пакетов UDP
#include <DNSServer.h>                  //Локальный DNS сервер для перенаправления всех запросов на страницу конфигурации
#include <ESP8266WebServer.h>           //Локальный веб сервер для страници конфигурации WiFi
#include <WiFiManager.h>                //Библиотека для удобного подключения к WiFi
#include <ESP8266HTTPClient.h>          //HTTP клиент
#include <ArduinoJson.h>                //Библиотека для работы с JSON
////////////////Настройки/////////////////

//При первом включении создана сеть,
//спомощью которой можно настроить Wi-Fi
//SSID:Connect-WIFI password:PASSWORD

const byte hhOn = 23;                   //Час включения ночного режима
const byte hhOff = 7;                   //Час выключения ночного режима

const byte devMode = 0;                 // 1 - Лог вкл 0 - лог выкл


//API ключ для openweathermap.org
const String appid = "10b7e2870121be19246347d21bc4b4c2";

//////////////////OLED////////////////////


extern uint8_t TinyFont[];
extern uint8_t SmallFont[];             //Шрифты
extern uint8_t BigNumbers[];

//////////////////NTPClient///////////////

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "time.google.com");

///////////////Переменные/////////////////

WiFiManager wifiManager;
HTTPClient http;
DynamicJsonDocument doc(1500);

byte hh, mm, ss;                        //Часы минуты секунды

//Переменные для хранения точек отсчета
unsigned long timing, LostWiFiMillis, lastWeatherUpdate, lastPrintingTime, lastModeSwitch, lastRestoreLampsTime;

String timeStr;                         //Строка с временем с нулями через точку

bool LostWiFi = false;                  //Флаг потери WiFi

int temp, temp_min, temp_max, wID;      //Переменные погоды: Тепература: сейчас, минимальная, максимальная. ID погоды
byte humidity, clouds;                  //Влажность и облака в процентах
String location, weather, description;  //Местоположение, погода, подробное описание погоды
float wind;                             //Ветер в м/с
long timeOffset;                        //Оффсет времени
byte httpErrCount = 0;                  //Счетчик ошибок получения погоды

int nightX, nightY;                     //Координаты надписи в ночном режиме

int currentMode = 2;
int timeMode = 1;
int temperatureMode = 2;
int modeSwitchDelay = 10000;

void setup() {
  unsigned long timer = millis();       //Таймер загрузки
  Serial.println("Starting Serial");        //Отрисовка загрузочного экрана
  Serial.begin(9600);                 //Инициализация последовательного соединения

  wifiManager.setDebugOutput(devMode);  //Вкл выкл лога wifiManager
  //Подключение к сети
  Serial.println("autoConnect");
  wifiManager.autoConnect("Connect-WIFI", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED) {}

  Serial.println("Updating weather");      //Отрисовка загрузочного экрана
  int code = weatherUpdate();           //Обновление погоды
  if ( code != 200) {                   //Если не обновляется
    Serial.println("Could not Access");
    Serial.println("OpenWeatherMap");
    delay(1000);
    ESP.reset();                      //Перезагружаемся
  }

  Serial.println("Starting NTPClient");    //Отрисовка загрузочного экрана
  timeClient.begin();                   //Инициализация NTP клиента
  timeClient.setTimeOffset(timeOffset); //Установка оффсета времени

  //Отрисовка загрузочного экрана
  Serial.println("Done in " + String( millis() - timer ) + "ms");
  delay(700);
}



void loop() {
  timeClient.update();                //Обновление времени

  if (millis() - lastWeatherUpdate > 120000) {  //Обновление погоды раз в 2 минуты
    weatherUpdate();
    lastWeatherUpdate = millis();
  }

  if (millis() - lastPrintingTime > 1000) {
    if (currentMode == timeMode){
      printTime();
    }else if(currentMode == temperatureMode){
      printTemperature();
    }
    lastPrintingTime = millis();
  }

  if (millis() - lastModeSwitch > modeSwitchDelay) {  //Обновление погоды раз в 2 минуты
    lastModeSwitch = millis();
    if (currentMode == timeMode) {
      currentMode = temperatureMode;
      modeSwitchDelay = 3000;

    }else if (currentMode == temperatureMode) {
      currentMode = timeMode;
      modeSwitchDelay = 15000;
    }

  }


  if (WiFi.status() != WL_CONNECTED) {//Если нет подключения
    if (LostWiFi == 0) {
      LostWiFi = 1;
      LostWiFi = 1;                   //Ставим флаг
      LostWiFiMillis = millis();
    } else if (millis() - LostWiFiMillis > 180000) {
      Serial.println("0000");
      delay(1000);
      ESP.reset();                    //Перезагружаемся
    }
  }

  hh = timeClient.getHours();
  mm = timeClient.getMinutes();       //Запись времени без нулей
  ss = timeClient.getSeconds();       //в переменные
}



int weatherUpdate() {                  //Функция обновления погоды
  if (WiFi.status() == WL_CONNECTED) { //Выполняется только если WiFi подключен

    logIf("Updating weather");
    //Формирование строки запроса, знаю, что тупо сделал
    http.begin("http://api.openweathermap.org/data/2.5/weather?lat=64.56&lon=39.82&appid=10b7e2870121be19246347d21bc4b4c2&units=metric&lang=en");


    int httpCode = http.GET();         //Запрос + получение http кода
    String json = http.getString();    //Получение строки ответа
    logIf("Http Code: " + String(httpCode) );
    logIf("Got JSON: " + json);
    http.end();

    if (httpCode != 200) {             //Если код не 200
      httpErrCount++;                  //Инкремент счетчика ошибок
      logIf( "httpErrCount: " + String(httpErrCount) );
      return httpCode;                 //Возвращаем код
    }
    //Парсинг JSON
    DeserializationError error = deserializeJson(doc, json);

    if (error) {                       //Если не парсится
      if (devMode) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
      return httpCode;                 //Возвращаем http код
    }

    temp = doc["main"]["temp"];        //Запись значений в переменные
    temp_min = doc["main"]["temp_min"];
    temp_max = doc["main"]["temp_max"];
    wind = doc["wind"]["speed"];
    description = doc["weather"][0]["description"].as<String>();
    weather = doc["weather"][0]["main"].as<String>();
    humidity = doc["main"]["humidity"];
    clouds = doc["clouds"]["all"];
    location = doc["name"].as<String>();
    timeOffset = doc["timezone"];
    wID = doc["weather"][0]["id"];

    httpErrCount = 0;                  //Обнуляем счетчик ошибок
    Serial.println(temp);
    Serial.println(weather);
    Serial.println(location);
    return httpCode;                   //Возвращаем http код
  }
}


//Вывод лога если включен режим отладки
void logIf(String msg) {
  if (devMode) {
    Serial.println(msg);
  }
}



void printTime() {                       //Функция обновления и смены экранов
  timeStr = "";                       //Форматирование строки времени
  if (hh <= 9) {                      //Добавляем точки и нули где нужно
    timeStr = timeStr + "0" + String(hh);
  } else {
    timeStr = timeStr + String(hh);
  }
  if (mm <= 9) {
    timeStr = timeStr + "0" + String(mm);
  } else {
    timeStr = timeStr + String(mm);
  }

  Serial.println(timeStr);
}

void printTemperature() {
  String result = "";
  int absTemp = abs(temp);
  // отрицательная температура будет с единицы начинаться
  if (temp >= 0) {
    result = "00";
  } else {
    result = "10";
  }
  if (absTemp <= 9) {
    result = result + "0" + absTemp;
  } else {
    result = result + absTemp;
  }
  Serial.println(result);
}
