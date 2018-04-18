#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

const char* fingerprint = "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19";

#define GHOTA_HOST "api.github.com"
#define GHOTA_PORT 443
#define GHOTA_TIMEOUT 1500
#define GHOTA_MAX_RELEASES 3

#define GHOTA_USER "yknivag"
#define GHOTA_REPO "test"
#define CURRENT_TAG "0.0.0"

//struct assetDetails_t{
//  long id;
//  char* name;
//  char* url;
//  char* updated_at;
//  char* browser_download_url;
//};
//
//struct releaseDetails_t{
//  long id;
//  char* tag_name;
//  char* name;
//  char* url;
//  bool prerelease;
//  char* published_at;
//  int asset_count;
//  char* assets_url;
//  assetDetails_t assetDetails[];
//};
//
//releaseDetails_t releases[GHOTA_MAX_RELEASES];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
//  releases[0].id = 10134966;
//  releases[0].tag_name = "1.0";
//  releases[0].name = "Initial Release";
//  releases[0].url = "https://api.github.com/repos/yknivag/Arduino-Dodow-Clone/releases/10134966";
//  releases[0].prerelease = false;
//  releases[0].published_at = "2018-03-17T12:47:26Z";
//  releases[0].asset_count = 2;
//  releases[0].assetDetails[0].id = 6820510;
//  releases[0].assetDetails[0].name = "DowDowATTiny85.ino.tiny8.hex";
//  releases[0].assetDetails[0].url = "https://api.github.com/repos/yknivag/Arduino-Dodow-Clone/releases/assets/6820510";
//  releases[0].assetDetails[0].updated_at = "2018-04-12T11:03:53Z";
//  releases[0].assetDetails[0].browser_download_url = "https://github.com/yknivag/Arduino-Dodow-Clone/releases/download/1.0/DowDowATTiny85.ino.tiny8.hex";
//  releases[0].assetDetails[1].id = 6820511;
//  releases[0].assetDetails[1].name = "1DowDowATTiny85.ino.tiny8.hex";
//  releases[0].assetDetails[1].url = "https://api.github.com/repos/yknivag/Arduino-Dodow-Clone/releases/assets/6820511";
//  releases[0].assetDetails[1].updated_at = "2018-04-12T11:03:53Z";
//  releases[0].assetDetails[1].browser_download_url = "https://github.com/yknivag/Arduino-Dodow-Clone/releases/download/1.0/1DowDowATTiny85.ino.tiny8.hex";
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println("saved network.");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(GHOTA_HOST);
  if (!client.connect(GHOTA_HOST, GHOTA_PORT)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, GHOTA_HOST)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = "/repos/esp8266/Arduino/commits/master/status";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + GHOTA_HOST + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);  
}
