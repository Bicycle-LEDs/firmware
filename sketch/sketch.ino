//      <-- Libraries -->
// Filesystem
#include <FS.h>

// LEDs
#include <Adafruit_NeoPixel.h>



//                      <--- Settings --->

// LEDs
const int numStrips = 3;                           // Number of led strips
const int numOfLEDs[] = { 7, 7, 7 };   // Number of leds on every strip
const int LEDpin[] = { 0, 2, 14 };  // Pin for every led strip
Adafruit_NeoPixel LEDstrip[] = {
  Adafruit_NeoPixel(numOfLEDs[0], LEDpin[0], NEO_GRB + NEO_KHZ800),  // Create strip objects
  Adafruit_NeoPixel(numOfLEDs[1], LEDpin[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(numOfLEDs[2], LEDpin[2], NEO_GRB + NEO_KHZ800),
};                               // Delete lines if using less or add lines but with other numOfLEDs and LEDPin if using more led strips
const int numberOfPatterns = 1;  // Number of available patterns
int setPatternNum = 0;           // Startup pattern number

// Button
const int buttonPin = 1;  // Button Pin


//                      <--- LEDs engine --->

unsigned long previousLEDMillis;  // Auxiliary var to hold time
int animationProgress = 0;        // Animation progress

void LEDsInit()  // Set LEDs and initialize them
{
  if (!Settings_LEDs) return;  // If turned off return nothing

  for (int i = 0; i < numStrips; i++) {  // For every strip
    LEDstrip[i].begin();                 // Begin

    // Pattern of startup screen
    LEDstrip[i].clear();  // Clear
    for (int k = 0; k < numOfLEDs[i]; k++) {
      LEDstrip[i].setPixelColor(k, 60, 0, 0);  // Set all strips color to red
      LEDstrip[i].show();                      // Update strip
      delay(100);
    }
  }
}

void updateLEDPattern(int patternNum = 0, bool prev = false, bool next = false, bool onlyUpdate = false) {

  if (!Settings_LEDs) return;  // If turned off return nothing

  if (!onlyUpdate) {                  // If pattern change
    if (prev) setPatternNum--;        // Next pattern
    else if (next) setPatternNum++;   // Prev pattern
    else setPatternNum = patternNum;  // Set pattern number
    previousLEDMillis = 0;
    animationProgress = 0;
  }

  const unsigned long currentMillis = millis();


  if (updateStatus == 2) {  // Updater pattern

    for (int i = 0; i < numStrips; i++) {  // For every strip
      if (i % 2 == 0) {                    // For half of strips
        for (int k = 0; k < numOfLEDs[i]; k++) {
          LEDstrip[i].setPixelColor(k, 60, 60, 0);  // Set all pixels to yellow with timeouts
          LEDstrip[i].show();
          delay(100);
        }
      } else {
        for (int k = 0; k < numOfLEDs[i]; k++) {                   // For another half
          LEDstrip[i].setPixelColor(numOfLEDs[i] - k, 60, 60, 0);  // Set all pixels to yellow with timeouts, but in other direction
          LEDstrip[i].show();
          delay(100);
        }
      }
    }
  }

   else if (setPatternNum == 0) {  // OFF
    for (int i = 0; i < numStrips; i++) {
      LEDstrip[i].fill(LEDstrip[i].Color(0, 0, 0));
      LEDstrip[i].show();
    }
  }

  else if (setPatternNum == 1) {  // RED
    for (int i = 0; i < numStrips; i++) {
      LEDstrip[i].fill(LEDstrip[i].Color(60, 0, 0));
      LEDstrip[i].show();
    }
  }

  else if (setPatternNum == 2) {  // GREEN
    for (int i = 0; i < numStrips; i++) {
      LEDstrip[i].fill(LEDstrip[i].Color(0, 60, 0));
      LEDstrip[i].show();
    }
  }

  else if (setPatternNum == 3) {  // BLUE
    for (int i = 0; i < numStrips; i++) {
      LEDstrip[i].fill(LEDstrip[i].Color(0, 0, 60));
      LEDstrip[i].show();
    }
  }

  else if (setPatternNum == 4) {  // 1st pattern

    if (!onlyUpdate || (animationProgress == 0 && currentMillis - previousLEDMillis >= 1000)) {  // If animation starting or 1sec passed
      for (int i = 0; i < numStrips; i++) {
        if (i % 2 == 0) LEDstrip[i].fill(LEDstrip[i].Color(0, 60, 0));  // Set half strips color to green
        else LEDstrip[i].fill(LEDstrip[i].Color(0, 0, 60));             // Set half strips color to blue
        LEDstrip[i].show();
      }
      previousLEDMillis = currentMillis;
      animationProgress = 1;  // 2nd step
    }

    else if (animationProgress == 1 && currentMillis - previousLEDMillis >= 1000) {  // If next stage and 1sec passed
      for (int i = 0; i < numStrips; i++) {
        if (i % 2 == 0) LEDstrip[i].fill(LEDstrip[i].Color(0, 0, 60));  // Set half strips color to blue
        else LEDstrip[i].fill(LEDstrip[i].Color(0, 60, 0));             // Set half strips color to green
        LEDstrip[i].show();
      }
      previousLEDMillis = currentMillis;
      animationProgress = 0;  // 1st step again
    }
  }

  else {                                                          // Too low/high pattern num
    if (setPatternNum < 0) setPatternNum = numberOfPatterns + 3;  // Set to max pattern number
    else setPatternNum = 0;
    updateLEDPattern(0, 0, 0, true);
  }
}



//                      <--- WiFi connector --->

void wiFiInit(bool forceAP = false) {
  if (!Settings_ActivateWiFi) return;  // If turned off return nothing

  if (!forceAP) {
    File file = SPIFFS.open("/network_config.txt", "r");  // Open wifi config file
    ssidFromFile = file.readStringUntil('\n');            // Read network info
    passwordFromFile = file.readStringUntil('\n');
    file.close();

    ssidFromFile.trim();  // Delete spaces at the beginning and end
    passwordFromFile.trim();

    if (strcmp(ssidFromFile.c_str(), "")) {  // Check if SSID provided

      WiFi.begin(ssidFromFile.c_str(), passwordFromFile.c_str());  // If yes, try to connect
      Serial.print(ssidFromFile.c_str());
      Serial.println("'...");
      return;
    }
  }

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);       // AP mode
  WiFi.softAP("LED Wall");  // Create network
}

void saveWifiCfg(const char* s, const char* p)  //Save network info into file
{
  if (!WiFi_UpdateCredentialsFile) return;  // If turned off return nothing

  SPIFFS.remove("/network_config.txt");  // Recreate config file
  File file = SPIFFS.open("/network_config.txt", "w");
  file.println(s);  // Save info into file
  file.println(p);

  file.close();
}



//                      <--- Server handler -->

// AsyncWebServer server(80); // Server
// // AsyncEventSource events("/events");

// void configureWebServer() // Create server
// {
//   server.begin(); // Start server
//   // server.addHandler(&events); // Add server events handler

//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){  // Root request

//     request->send_P(200, "text/html", "XD"); // Send root page

//   });

//   Serial.print("[STATUS] Server ready!");
// }



//                      <--- Firmware updater --->

// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF( 
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);

void firmwareUpdate()  // Updater
{

  if (!Settings_Updater || !Settings_ActivateWiFi) updateStatus = -1;  // If turned off return nothing
  else updateStatus = 0;

  if (!Settings_Updater || !Settings_ActivateWiFi) return;  // If turned off return nothing

  if (WiFi.status() != WL_CONNECTED) {  // No wifi
    updateStatus = -1;
    return;
  }

  WiFiClientSecure client;  // Create secure wifi client
  client.setTrustAnchors(&cert);

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // Set time via NTP, as required for x.509 validation
  time_t now = time(nullptr);

  if (!client.connect(host, httpsPort)) {  // Connect to github
    updateStatus = -1;
    return;
  }

  if (Updater_Check_Version) {
    HTTPClient http;  // Connect to release API
    http.begin(client, updaterVersionCtrlUrl);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
      updateStatus = -1;
      return;
    }

    String new_version = http.getString();  //Download version tag
    new_version.trim();
    http.end();

    if (!strcmp(new_version.c_str(), firmwareVer)) {  // Check if version is the same
      updateStatus = 1;
      return;
    } else if (!new_version.c_str() || new_version.c_str() == "") {
      updateStatus = 0;
      return;
    }
  }

  updateStatus = 2;                                                             // Update status
  updateLEDPattern();                                                           // Init updater LED pattern

  // //downloadHelper(updaterFilesystemUrl + "/mainPage/index.html", "/update/mainPage/index.html");

  // http.begin(client, updaterFilesystemUrl + "/html/main.html");  // Try to download index.html
  // httpCode = http.GET();
  // if (httpCode != HTTP_CODE_OK) {
  //   Serial.println("[WARNING] Cannot download new file /html/main.html, aborting update");
  //   return;
  // }
  // SPIFFS.remove("/update/html/main.html");
  // File file = SPIFFS.open("/update/html/main.html", "w"); // Write the response to the downloaded file
  // int contentLength = http.getSize();
  // int bufSize = 512;
  // uint8_t buf[bufSize];
  // int received = 0;
  // while (received < contentLength) {
  //   int len = client.readBytes(buf, bufSize);
  //   file.write(buf, len);
  //   received += len;
  // }
  // Serial.println("[INFO] Downloaded /html/main.html");
  // file.close(); http.end(); // Close the file and the connection

  // http.begin(client, updaterFilesystemUrl + "/css/style.css"); // Try to download style.css
  // httpCode = http.GET();
  // if (httpCode != HTTP_CODE_OK) {
  //   Serial.println("[WARNING] Cannot download new file /css/style.css, aborting update");
  //   return;
  // }
  // SPIFFS.remove("/update/css/style.css");
  // file = SPIFFS.open("/update/css/style.css", "w"); // Write the response to the downloaded file
  // contentLength = http.getSize();
  // bufSize = 512;
  // buf[bufSize];
  // received = 0;
  // while (received < contentLength) {
  //   int len = client.readBytes(buf, bufSize);
  //   file.write(buf, len);
  //   received += len;
  // }
  // Serial.println("[INFO] Downloaded /css/style.css");
  // file.close(); http.end(); // Close the file and the connection

  // http.begin(client, updaterFilesystemUrl + "/scripts/script.js"); // Try to download script.js
  // httpCode = http.GET();
  // if (httpCode != HTTP_CODE_OK) {
  //   Serial.println("[WARNING] Cannot download new file /scripts/script.js, aborting update");
  //   return;
  // }
  // SPIFFS.remove("/update/scripts/script.js");
  // file = SPIFFS.open("/update/scripts/script.js", "w"); // Write the response to the downloaded file
  // contentLength = http.getSize();
  // bufSize = 512;
  // buf[bufSize];
  // received = 0;
  // while (received < contentLength) {
  //   int len = client.readBytes(buf, bufSize);
  //   file.write(buf, len);
  //   received += len;
  // }
  // Serial.println("[INFO] Downloaded /scripts/script.js");
  // file.close(); http.end(); // Close the file and the connection

  // SPIFFS.remove("/files-update-info.txt");
  // File updateInfo = SPIFFS.open("/files-update-info.txt", "w"); // Save update info into file
  // updateInfo.print(new_version.c_str());

  ESPhttpUpdate.setLedPin(LED_BUILTIN);
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, updaterFirmwareUrl);  // Update firmware
  if (ret) {
    ESP.restart();
  }
}

// void downloadHelper(const String url, const String location)
// {

// }

void postUpdaterHelper() {
  if (!Updater_FS) return;  // If turned off return nothing

  //   File file = SPIFFS.open("/files_update_info.txt", "r"); // Open file

  //   String prevVersion = file.readStringUntil('\n'); // Read ssid
  //   prevVersion.trim();
  //   if (strcmp(prevVersion.c_str(), firmwareVer)) {
  //     SPIFFS.remove("/html/main.html");

  //   }

}



//                      <--- Setup and loop --->

void setup() {
  Serial.begin(74880);  // Begin serial
  Serial.println("[STATUS] Start!");

  if (!SPIFFS.begin()) {  // Begin filesystem
    ESP.restart();
  }

  postUpdaterHelper();  // Check if files need update

  saveWifiCfg(ssid, password);  // Save network config
  wiFiInit();                   // Connect to wifi

  LEDsInit();  // Init led strips

  pinMode(LED_BUILTIN, OUTPUT);  // Set pin modes
  pinMode(buttonPin, INPUT_PULLUP);

  delay(1000);

  if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP) wiFiInit(true);  // Check if connected, if not AP mode
}


unsigned long previousMillis = 0;  // will store last time firmware updater was called
int previousButtonState = 0;
void loop() {
  const unsigned long currentMillis = millis();

  updateLEDPattern(0, 0, 0, true);  // Update animations on every loop

  if ((currentMillis - previousMillis) >= updateInterval || !updateStatus) {  // If interval time passed
    previousMillis = currentMillis;
    firmwareUpdate();  // Call updater function
  }

  if (previousButtonState != digitalRead(buttonPin)) {
    previousButtonState = digitalRead(buttonPin);
    if (previousButtonState == LOW) updateLEDPattern(0, 0, 1);  // Change animation to next
  }
}
