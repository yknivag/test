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
#define GHOTA_FINGERPRINT "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19"
#define GHOTA_CONTENT_TYPE "application/octet-stream"

#define GHOTA_USER "yknivag"
#define GHOTA_REPO "test"
#define GHOTA_CURRENT_TAG "0.1.0"
#define GHOTA_BIN_FILE "temp.ino.d1_mini.bin"

#define GHOTA_ACCEPT_PRERELEASE 0

const char* GHOTA_LastError = "";
const char* GHOTA_UpgradeURL = "";

bool GHOTACheckUpgrade() {
  GHOTA_LastError = "";
  GHOTA_UpgradeURL = "";
  WiFiClientSecure client;
  if (!client.connect(GHOTA_HOST, GHOTA_PORT)) {
    GHOTA_LastError = "Connection failed";
    return false;
  }

#ifdef GHOTA_FINGERPRINT
  if (!client.verify(GHOTA_FINGERPRINT, GHOTA_HOST)) {
    GHOTA_LastError = "Certificate doesn't match";
    return false;
  }
#endif

  String url = "/repos/";
  url += GHOTA_USER;
  url += "/";
  url += GHOTA_REPO;
  url += "/releases/latest";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + GHOTA_HOST + "\r\n" +
               "User-Agent: GitHubOTAUpdateLibraryESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String response = client.readStringUntil('\n');
    if (response == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String response = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(response);
  Serial.println("==========");
  Serial.println("closing connection");
  //client->stop();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(response);

  if (root.success()) {
    if (root.containsKey("tag_name")) {
      const char* current_tag = GHOTA_CURRENT_TAG;
      const char* release_tag = root["tag_name"];
      const char* release_name = root["name"];
      const char* release_prerelease = root["prerelease"];
      Serial.print("Current Tag: ");
      Serial.println(current_tag);
      Serial.print("Release Tag: ");
      Serial.println(release_tag);
      if (strcmp(release_tag, GHOTA_CURRENT_TAG) != 0) {
        Serial.println("Interesting Release Found");
        Serial.println("Release Details");
        Serial.println("===============");
        Serial.print("Name and Tag: ");
        Serial.print(release_name);
        Serial.print("[");
        Serial.print(release_tag);
        Serial.println("]");
        Serial.print("Pre-release: ");
        Serial.println(release_prerelease);

        #ifdef GHOTA_ACCEPT_PRERELEASE
          if (GHOTA_ACCEPT_PRERELEASE == 0 && strcmp(release_prerelease, "true") == 0) {
            GHOTA_LastError = "Latest release is a pre-release and GHOTA_ACCEPT_PRERELEASE is not set to 1.";
            return false;
          }
        #else
          if (strcmp(release_prerelease, "true") == 0) {
            GHOTA_LastError = "Latest release is a pre-release and GHOTA_ACCEPT_PRERELEASE is not defined and set to 1.";
            return false;
          }
        #endif

        JsonArray& assets = root["assets"];
        for (auto& asset : assets) {
          const char* asset_type = asset["content_type"];
          const char* asset_name = asset["name"];
          const char* asset_url = asset["browser_download_url"];

          if (strcmp(asset_type, GHOTA_CONTENT_TYPE) == 0 && strcmp(asset_name, GHOTA_BIN_FILE) == 0) {
            GHOTA_UpgradeURL = asset_url;

            Serial.println("Valid Asset Found");
            Serial.println("Asset Details");
            Serial.println("=============");
            Serial.print("Type: ");
            Serial.println(asset_type);
            Serial.print("Name: ");
            Serial.println(asset_name);
            Serial.print("URL: ");
            Serial.println(asset_url);

            return true;
          }
          else {
            GHOTA_LastError = "No valid binary found for latest release.";
            return false;
          }
        }
      }
      else {
        GHOTA_LastError = "Already running latest release.";
        return false;
      }
    }
    else {
      GHOTA_LastError = "JSON didn't match expected structure.";
      return false;
    }
  }
  else {
    GHOTA_LastError = "Response does not appear to be JSON.";
    return false;
  }
}


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

  bool isUpgradeable = GHOTACheckUpgrade();

  if (isUpgradeable) {
    Serial.print("Upgrade URL: ");
    Serial.println(GHOTA_UpgradeURL);
  }
  else {
    Serial.println("No upgrade available.");
    if (strcmp(GHOTA_LastError, "")) {
      Serial.print("GHOTA Response: ");
      Serial.println(GHOTA_LastError);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1800);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(200);
}
