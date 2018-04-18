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
#define GHOTA_CURRENT_TAG "0.0.0"
#define GHOTA_BIN_FILE "temp.ino.d1_mini.bin"

#define GHOTA_ACCEPT_PRERELEASE 0



#define DEBUG_ESP_HTTP_UPDATE true
#define DEBUG_ESP_PORT Serial

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

typedef struct urlDetails_t {
  String proto;
  String host;
  String path;
  String url;
};

struct urlDetails_t urlDetails(String url) {
  String proto = "";
  if (url.startsWith("http://")) {
    proto = "http://";
    url.replace("http://", "");
  }
  else {
    proto = "https://";
    url.replace("https://", "");
  }
  int firstSlash = url.indexOf('/');
  String host = url.substring(0, firstSlash);
  String path = url.substring(firstSlash);

  urlDetails_t urlDetail;

  urlDetail.proto = proto;
  urlDetail.host = host;
  urlDetail.path = path;
  urlDetail.url = proto + host + path;

  return urlDetail;
}

String getFinalURL(String url) {
  struct urlDetails_t splitURL = urlDetails(url);
  String proto = splitURL.proto;
  String host = splitURL.host;
  String path = splitURL.path;
  int port = 80;
  if (proto.startsWith("https")) {
    port = 443;
  }
  bool isFinalURL = false;
  //
  //  WiFiClientSecure client;
  //
  //  while (!isFinalURL) {
  //    Serial.print("Connecting to ");
  //    Serial.println(host);
  //    if (!client.connect(host, port)) {
  //      Serial.println("connection failed");
  //      return;
  //    }
  //
  //    //    if (client.verify(GHOTA_FINGERPRINT, GHOTA_HOST)) {
  //    //      Serial.println("certificate matches");
  //    //    } else {
  //    //      Serial.println("certificate doesn't match");
  //    //      return;
  //    //    }
  //
  //    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
  //                 "Host: " + host + "\r\n" +
  //                 "User-Agent: GitHubOTAUpdateLibraryESP8266\r\n" +
  //                 "Connection: close\r\n\r\n");
  //
  //    Serial.println("request sent");
  //
  //    while (client.connected()) {
  //      String response = client.readStringUntil('\n');
  //      if (response.startsWith("Location: ")) {
  //        String location = response;
  //        location.replace("Location: ", "");
  //
  //        Serial.println();
  //        Serial.print("Location: ");
  //        Serial.println(location);
  //        Serial.println();
  //
  //        if (location.startsWith("http://") || location.startsWith("https://")) {
  //          //absolute URL - separate host from path
  //          urlDetails_t url = urlDetails(location);
  //          proto = url.proto;
  //          host = url.host;
  //          path = url.path;
  //          if (strcmp(proto, "https://") == 0) {
  //            port = 443;
  //          }
  //          else {
  //            port = 80;
  //          }
  //        }
  //        else {
  //          path = location;
  //        }
  //      }
  //      else {
  //        isFinalURL = true;
  //      }
  //      if (response == "\r") {
  //        Serial.println("headers received");
  //        break;
  //      }
  //    }
  //  }

  String finalURL = proto + host + path;

  return finalURL;
}


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

  //bool isUpgradeable = GHOTACheckUpgrade();
  bool isUpgradeable = true;

  if (isUpgradeable) {
    Serial.print("Upgrade URL: ");
    Serial.println(GHOTA_UpgradeURL);

    String GHOTA_FinalURL = getFinalURL(GHOTA_UpgradeURL);
    Serial.print("Final Upgrade URL: ");
    Serial.println(GHOTA_FinalURL);


    //    WiFiClientSecure client;
    //    Serial.print("connecting to ");
    //    Serial.println("github.com");
    //    if (!client.connect("github.com", GHOTA_PORT)) {
    //      Serial.println("connection failed");
    //      return;
    //    }
    //
    //    //    if (client.verify(GHOTA_FINGERPRINT, GHOTA_HOST)) {
    //    //      Serial.println("certificate matches");
    //    //    } else {
    //    //      Serial.println("certificate doesn't match");
    //    //      return;
    //    //    }
    //
    //    client.print(String("GET ") + "/yknivag/test/releases/download/0.1.0/temp.ino.d1_mini.bin" + " HTTP/1.1\r\n" +
    //                 "Host: " + "github.com" + "\r\n" +
    //                 "User-Agent: GitHubOTAUpdateLibraryESP8266\r\n" +
    //                 "Connection: close\r\n\r\n");
    //
    //    Serial.println("request sent");
    //    String new_host = "github.com";
    //    String new_path = "/yknivag/test/releases/download/0.1.0/temp.ino.d1_mini.bin";
    //    while (client.connected()) {
    //
    //      String response = client.readStringUntil('\n');
    //      if (response.startsWith("Location: ")) {
    //        String location = response;
    //        location.replace("Location: ", "");
    //
    //        Serial.println();
    //        Serial.print("Location: ");
    //        Serial.println(location);
    //        Serial.println();
    //        if (location.startsWith("http://") || location.startsWith("https://")) {
    //          //absolute URL - separate host from path
    //          urlDetails_t url = urlDetails(location);
    //          new_host = url.host;
    //          new_path = url.path;
    //          //        if (location.startsWith("http://")) {
    //          //          location.replace("http://", "");
    //          //        }
    //          //        else {
    //          //          location.replace("https://", "");
    //          //        }
    //          //        int firstSlash = location.indexOf('/');
    //          //        new_host = location.substring(0, firstSlash);
    //          //        new_path = location.substring(firstSlash);
    //        }
    //        else {
    //          new_path = location;
    //        }
    //      }
    //      if (response == "\r") {
    //        Serial.println("headers received");
    //
    //        break;
    //      }
    //    }
    //    Serial.println("closing connection");
    //    Serial.println();
    //    Serial.print("New Host: ");
    //    Serial.println(new_host);
    //    Serial.print("New Path: ");
    //    Serial.println(new_path);
    //    Serial.println();
    //
    //    //    Serial.print("Starting OTA from: ");
    //    //    Serial.println(GHOTA_UpgradeURL);

    //t_httpUpdate_return ret = ESPhttpUpdate.update(client, GHOTA_HOST, GHOTA_UpgradeURL);
    //t_httpUpdate_return ret = ESPhttpUpdate.update("github.com", 443, "https://github.com/yknivag/test/releases/download/0.1.0/temp.ino.d1_mini.bin");

    //      switch(ret) {
    //        case HTTP_UPDATE_FAILED:
    //          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    //          break;
    //
    //        case HTTP_UPDATE_NO_UPDATES:
    //          Serial.println("HTTP_UPDATE_NO_UPDATES");
    //          break;
    //
    //        case HTTP_UPDATE_OK:
    //          Serial.println("HTTP_UPDATE_OK");
    //          break;
    //      }

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
  //put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1800);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(200);
}
