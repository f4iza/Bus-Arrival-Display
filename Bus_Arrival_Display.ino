#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "realmeC33";
const char* pass = "faizal1234";
const char* apiKey = "39ZFM4fjQ6yZH0Xm/ItiLw==";
const char* HalteCode = "70201"; 


long calculateArrivalMinutes(const char* iso8601) {
  if (strlen(iso8601) == 0) return -1;
  struct tm tm;
  strptime(iso8601, "%Y-%m-%dT%H:%M:%S%z", &tm);
  time_t arrivalTime = mktime(&tm);
  time_t now;
  time(&now);
  long diffSeconds = difftime(arrivalTime, now);
  return diffSeconds / 60;
}


void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Bus Arrival"); 
  lcd.setCursor(0, 1);
  lcd.print("Opp Macpherson Stn");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Wifi Terhubung!");
  configTime(8 * 3600, 0, "pool.ntp.org");
  delay(2000); 
}


void loop() {
  HTTPClient http; 
  
  String apiUrl = "https://datamall2.mytransport.sg/ltaodataservice/v3/BusArrival?BusStopCode=" + String(HalteCode);
  http.begin(apiUrl);
  http.addHeader("AccountKey", apiKey);
  int httpCode = http.GET();

  if (httpCode == 200) { 
    String payload = http.getString();

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    JsonObject bus1 = doc["Services"][0]; 
    JsonObject bus2 = doc["Services"][1]; 

    
    String busNo1 = bus1["ServiceNo"];
    String arrivalTime1 = bus1["NextBus"]["EstimatedArrival"];
    long minutes1 = calculateArrivalMinutes(arrivalTime1.c_str());

    String busNo2 = bus2["ServiceNo"];
    String arrivalTime2 = bus2["NextBus"]["EstimatedArrival"];
    long minutes2 = calculateArrivalMinutes(arrivalTime2.c_str());

    
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Bus " + busNo1 + " -> " + String(minutes1) + "min");
    
    lcd.setCursor(0, 1); 
    lcd.print("Bus " + busNo2 + " -> " + String(minutes2) + "min");

  } else {
    Serial.println("Gagal mengambil data, Error: " + String(httpCode));
    lcd.clear();
    lcd.print("Gagal ambil data");
  }

  http.end();
  
  
  Serial.println("Masuk mode Deep Sleep selama 1 menit...");
  ESP.deepSleep(60 * 1000000); 
}