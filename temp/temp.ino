#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define GHOTA_HOST "api.github.com"
#define GHOTA_SSL_PORT 443
#define GHOTA_TIMEOUT 1500
#define GHOTA_MAX_RELEASES 3

#define GHOTA_USER "yknivag"
#define GHOTA_REPO ""
#define CURRENT_TAG ""

struct assetDetails_t{
  long id;
  char* name;
  char* url;
  char* updated_at;
  char* browser_download_url;
};

struct releaseDetails_t{
  long id;
  char* tag_name;
  char* name;
  char* url;
  bool prerelease;
  char* published_at;
  int asset_count;
  char* assets_url;
  assetDetails_t assetDetails[];
};

releaseDetails_t releases[GHOTA_MAX_RELEASES];

void setup() {
  // put your setup code here, to run once:
  releases[0].id = 10134966;
  releases[0].tag_name = "1.0";
  releases[0].name = "Initial Release";
  releases[0].url = "https://api.github.com/repos/yknivag/Arduino-Dodow-Clone/releases/10134966";
  releases[0].prerelease = false;
  releases[0].published_at = "2018-03-17T12:47:26Z";
  releases[0].asset_count = 2;
  releases[0].assetDetails[0].id = 6820510;
  releases[0].assetDetails[0].name = "DowDowATTiny85.ino.tiny8.hex";
  releases[0].assetDetails[0].url = "https://api.github.com/repos/yknivag/Arduino-Dodow-Clone/releases/assets/6820510";
  releases[0].assetDetails[0].updated_at = "2018-04-12T11:03:53Z";
  releases[0].assetDetails[0].browser_download_url = "https://github.com/yknivag/Arduino-Dodow-Clone/releases/download/1.0/DowDowATTiny85.ino.tiny8.hex";
  releases[0].assetDetails[1].id = 6820511;
  releases[0].assetDetails[1].name = "1DowDowATTiny85.ino.tiny8.hex";
  releases[0].assetDetails[1].url = "https://api.github.com/repos/yknivag/Arduino-Dodow-Clone/releases/assets/6820511";
  releases[0].assetDetails[1].updated_at = "2018-04-12T11:03:53Z";
  releases[0].assetDetails[1].browser_download_url = "https://github.com/yknivag/Arduino-Dodow-Clone/releases/download/1.0/1DowDowATTiny85.ino.tiny8.hex";
}

void loop() {
  // put your main code here, to run repeatedly:

}
