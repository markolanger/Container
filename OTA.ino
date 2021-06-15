/*
   Offen:
   + AJAX - Schalter An/Aus besser umsetzen
   NTP mittels Zeitfenster begrenzen
   Sek. An einstellen
   Sek. Aus einstellen
   Taster als Generell an oder automodus.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "index_html.h"  // In dieser Datei befindet sich die Steuerung Webseite

const char* ssid = "couchsurfer";           // WiFi SSID
const char* password = "#HKS.-hp-7";        // WiFi password
int active_low = 0;                         // "0" Relais schaltet bei +5V , "1" Relais beim schaltet bei 0V
int intervalR1 = 500;                       // Zeit fürs Schalter halten
#define R1Pin 13                            // Relais 
int i1 = 0;


ESP8266WebServer server(80);        // Serverport  hier einstellen
int val1 = 0;                       // Hier wird der Schaltzustand gespeichert
int val2 = 0;
int val3 = 1;


void setup()
{
  Serial.begin(115200);             // Serielle Schnittstelle initialisieren
  pinMode(R1Pin, OUTPUT);
  digitalWrite(R1Pin, active_low);
  Serial.println("");
  Serial.println("");
  Serial.print("Mit WLAN verbinden .");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(" .");
  }
  Serial.println("");
  Serial.print("Verbunden mit ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", Ereignis_Index);
  server.onNotFound ( handleNotFound );

  server.begin();       // Starte den Server
  Serial.println("HTTP Server bereit");

  ArduinoOTA.setHostname("ESP8266_WIFIrelais");
  ArduinoOTA.setPassword("langer23");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
}














void loop()
{
  server.handleClient();
  if (val3 == 1) {                              //Wenn Haupschalter "val3" == 1
    if ( i1 <= intervalR1) {
      i1++;
      val1 = 1;
      digitalWrite(R1Pin, active_low ^ val1);
      delay(1);
    }
    else {
      val1 = 0;
      digitalWrite(R1Pin, active_low ^ val1);
    }
  }

  ArduinoOTA.handle();
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    ESP.restart();
  });
}

















void Ereignis_Index()                          // Wird ausgeführt wenn "http://<ip address>/" aufgerufen wurde
{
  if (server.args() > 0)                       // wenn Argumente im GET Anfrage vorhanden sind z.B http://<ip address>/?zustand=r
  {
    if (server.arg("r1") == "0")               // prüfen ob "/?r1=0"
    {
      Serial.println("R1 Aus");
      val1 = 0;                                // Ausschalten
      i1 = intervalR1 + 1;
      digitalWrite(R1Pin, active_low ^ val1);
    }
    if (server.arg("r1") == "1")              // prüfen ob "/?r1=1"
    {
      Serial.println("R1 Ein");
      val1 = 1;                               // Einschalten
      i1 = 0;
      if (val3 == 0) val1 = 0;
      digitalWrite(R1Pin, active_low ^ val1);
    }
    if (server.arg("r1") == "2")              // prüfen ob "/?r1=2"  Toggle Funktion
    {
      val1 = !val1;                           // Umschalten
      if (val1 == 1) i1 = 0;
      else i1 = intervalR1 + 1;
      Serial.println("R1 Umschalten auf " + String(val1));
      if (val3 == 0) val1 = 0;
      digitalWrite(R1Pin, active_low ^ val1);
    }
    if (server.arg("r3") == "2")              // prüfen ob "/?r3=2"  Toggle Funktion
    {
      val3 = !val3;                           // Umschalten
      if (val3 == 0) i1 = 0;
      Serial.println("Hauptschalter Umschalten auf " + String(val3));
    }

    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/plain", String(int(val1)) + String(int(val2)) + String(int(val3))); // Antwort an Internet Browser senden nur eine Zahl ("0"/"1")
  }
  else        // wenn keine Argumente im GET Anfrage
  {
    Serial.println("Sende Index.html");
    server.send(200, "text/html", indexHTML);           //dann Index Webseite senden (sehe Datei "index_html.h")
  }
}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not found");     // Send HTTP status 404 (Not Found)
}
