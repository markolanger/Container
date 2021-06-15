//GPIO 16 mit REST | GPIO 2 & 15 auf GND | CH_PD auf VCC
#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC);
const char* ssid = "couchsurfer";           // WiFi SSID
const char* password = "#";    // WiFi password
const char* server = "maker.ifttt.com";
const char* resource = "/trigger/ESP8266PIR/with/key/XXXXXXXXXXXXXXXXXXXXX";
uint64_t uS_TO_S_FACTOR = 1000000; //Umrechner
uint64_t TIME_TO_SLEEP = 0; //in sek.
const char * relaisIP = "192.168.180.111";
String url = "/?r2=1";
void setup() {
  Serial.begin(115200);  Serial.println();  Serial.println();
  initWifi();
  triggerWifiRelais();
  makeIFTTTRequest();
  Serial.println("");
  Serial.println("==========================");
  Serial.println("");
  Serial.println("Going to sleep now");
  Serial.println("");
  Serial.println("==========================");
  Serial.println("");
  ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}
void loop() {
}
void initWifi() {
  Serial.println("");
  Serial.println("==========================");
  Serial.print("Connecting to SSID: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  int timeout = 20 * 4; // 20 seconds
  while (WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(" .");
  }
  Serial.println(" ");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect, going back to sleep");
    ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  }
  Serial.print("WiFi connected in: ");
  Serial.print(millis());
  Serial.print("seconds, IP address: ");
  Serial.println(WiFi.localIP());
}

void triggerWifiRelais()
{ Serial.println("");
  Serial.println("==========================");
  Serial.println("");
  Serial.println("Connect to Wifi-Relais: ");
  Serial.print(relaisIP);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(relaisIP, 80)) {
    Serial.println("connection failed");
    return;
  }
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + relaisIP + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  Serial.print(url);
  Serial.println(" --> done!");
  delay(50);
  Serial.println("");
}

// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
  Serial.println("");
  Serial.println("==========================");
  Serial.print("Connecting to: ");
  Serial.print(server);
  WiFiClient client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  Serial.println("");
  Serial.println("==========================");
  Serial.print("Request resource: ");
  Serial.println(resource);
  String jsonObject = String("{\"value1\":\"ESP8266_2") + "\",\"value2\":\"" + ESP.getVcc() + "\",\"value3\":\"" + WiFi.RSSI() + "\"}";
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  int timeout = 10 * 10; // 10 seconds
  while (!!!client.available() && (timeout-- > 0)) {
    delay(100);
  }
  if (!!!client.available()) {
    Serial.println("No response...");
  }
  while (client.available()) {
    Serial.write(client.read());
  }
  Serial.println("");
  client.stop();
  Serial.println("");
}
