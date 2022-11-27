#include "header_files.h"

WebServer server(80);
Ticker coinCounter;
WiFiUDP ntpUDP;
TimerHandle_t wifiReconnectTimer;
LiquidCrystal_I2C lcd(lcdDisplayAddress, lcdColumns, lcdRows);

void launchWeb();

void runEach30Seconds() {
  count30s++;
  next30s = millis() + 30000;
  loopCounterLast = loopCounter;
  loopCounter = 0;
  if (loopCounterLast > loopCounterMax) {
    loopCounterMax = loopCounterLast;
  }
  //  String reply = "";
  //  if (count30s > 0) {
  //    reply += 100 - (100 * loopCounterLast / loopCounterMax);
  //    reply += F("% (LC=");
  //    reply += int(loopCounterLast / 30);
  //    reply += F(")");
  //    Serial.print("Load: "); Serial.println(reply);
  //  }
}

void blink_led(int dly) {
  digitalWrite(ONBOARDLED, ONBOARDLED_ON); //on
  delay(dly);
  digitalWrite(ONBOARDLED, ONBOARDLED_OFF);  //off
}

String get_date_time(boolean show_time) {
  char buf[20];
  buf[0] = '\0';
  if (timeStatus() != timeNotSet) {
    if (show_time == true) {
      sprintf_P(buf, PSTR("%02d/%02d/%02d %02d:%02d:%02d"), year(), month(), day(), hour(), minute(), second());
    } else {
      sprintf_P(buf, PSTR("%02d/%02d/%02d"), year(), month(), day());
    }
  }
  return buf;
}
void check_signal() {
  if (SERIAL_DEBUG) {
    Serial.print(F("Signal: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dB"));
  }
}

uint32_t get_esp_chip_id() {
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}


void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.printf_P(PSTR("[WiFi-event] event: %d\n"), event);
  switch (event) {
    //case WIFI_EVENT_STAMODE_GOT_IP:
    case SYSTEM_EVENT_STA_GOT_IP:
      //Serial.print(F("WiFi connected IP: "));
      //Serial.println(WiFi.localIP());
      onWifiConnect();
      break;
    //case WIFI_EVENT_STAMODE_DISCONNECTED:
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.print(F("WiFi conn lost reason:"));
      Serial.println(info.disconnected.reason);
      onWifiDisconnect();
      //xTimerStart(wifiReconnectTimer, 0);
      break;
    //case WIFI_EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP:   // client coneected to AP
    case SYSTEM_EVENT_AP_STACONNECTED:   // client coneected to AP
      Serial.println(F("Client connected"));
      break;
      //case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:   // client disconnected from AP
  }
}

void connectToWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("Wifi is connected"));
    return;
  }
  if (config_wifi == false) {
    config_wifi = true;
    WiFi.persistent(false);
    //WiFi.disconnect();
    if (SERIAL_DEBUG) {
      Serial.print(F("Connecting to: "));
      Serial.println(WIFI_SSID);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
  }
}

void onWifiConnect() {
  if (WiFi.status() == WL_CONNECTED) {
    if (SERIAL_DEBUG) {
      Serial.println(F(""));
      Serial.println(F("WiFi connected"));
      Serial.print(F("IP address: "));
      Serial.println(WiFi.localIP());
      lastLcdUpdated = millis();
      system_status = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WIFI: CONNECTED");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
    }
    config_wifi = false;
    check_signal();
    digitalWrite(ONBOARDLED, ONBOARDLED_ON);  // LED ON
  }
  launchWeb();
}

void onWifiDisconnect() {
  digitalWrite(ONBOARDLED, ONBOARDLED_OFF);  // LED OFF
  if (SERIAL_DEBUG) {
    Serial.println(F("Disconnected from Wi-Fi."));
  }
  if (system_status > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WIFI NETWORK");
    lcd.setCursor(0, 1);
    lcd.print("DISCONNECTED");
  }
  system_status = 0;
}

void handle_ntp() {
  //is_ntp_sync_success = false;
  if (WiFi.status() == WL_CONNECTED) {
    /*
      if (SERIAL_DEBUG) {
      Serial.print(F("Sync NTP:"));
      }
    */
    boolean is_ntp_ok = false;
    NTPClient timeClient(ntpUDP, NTP_SERVER_IP, NTP_TIME_SHIFT_IN_MIN * 60);
    timeClient.begin();
    is_ntp_ok = timeClient.update();
    if (is_ntp_ok == true) {
      //Serial.println(timeClient.getFormattedTime());
      time_t rawtime = timeClient.getEpochTime();

      struct tm * ti;
      ti = localtime (&rawtime);
      uint16_t ntp_year = ti->tm_year + 1900;
      uint8_t ntp_month = ti->tm_mon + 1;
      uint8_t ntp_day = ti->tm_mday;
      uint8_t ntp_hours = ti->tm_hour;
      uint8_t ntp_minutes = ti->tm_min;
      uint8_t ntp_seconds = ti->tm_sec;

      setTime(ntp_hours, ntp_minutes, ntp_seconds, ntp_day, ntp_month, ntp_year); // set time

      char t[22];
      sprintf_P(t, PSTR("%04d/%02d/%02d %02d:%02d:%02d"),  ntp_year, ntp_month, ntp_day, ntp_hours, ntp_minutes, ntp_seconds);
      is_ntp_sync_success = true;
      //next_ntp_sync = millis() + (0.1 * 60 * 60 * 1000);  // if success
      next_ntp_sync = millis() + (NTP_SYNC_PERIOD_HR * 60 * 60 * 1000) + random(60000, 600000);  // if success
      system_status = 2;
      if (SERIAL_DEBUG) {
        Serial.println(F("Sync NTP: OK"));
        Serial.print("Current Time: ");
        Serial.println(t);
      }
    } else {
      next_ntp_sync = millis() + random(5000, 20000);  // if last attempt fail try again in random time within 5s to 20sec
      if (SERIAL_DEBUG) {
        Serial.println(F("Sync NTP: Fail"));
      }
    }
  }
}

void heartBeatLed() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(ONBOARDLED, ONBOARDLED_ON);
    return;
  }
  if (millis() - heartBeatPulse > 1000) {
    heartBeatPulse = millis();
    led_toggle = !led_toggle;
    digitalWrite(ONBOARDLED, led_toggle);
  }
}

void handleNextNtpSync() {
  if (millis() >= next_ntp_sync) {
    unsigned long ntp_start = millis();
    handle_ntp();
    //Serial.print(millis() - ntp_start); Serial.println(F("ms"));
    if (is_ntp_sync_success == true) {
      Serial.printf("NTP Sync: Success (%ims)\n", (millis() - ntp_start));
    }
  }
}

void wifiConnectionCheck() {
  if ((millis() - last_wifi_status_test >= WIFI_CHECK_INTERVAL)) {
    last_wifi_status_test = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("Reconnecting to WiFi..."));
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}

void IRAM_ATTR coin_ms_isr() {
  coinPuleTimeout = coinPuleTimeout + 1;
  //Serial.println(digitalRead(COIN_PULSE_PIN));
}

void IRAM_ATTR coin_pulse_isr() {
  coinImpulsCount = coinImpulsCount + 1;
  coinPuleTimeout = 0;
  coinDetected = true;
}

void saveHistory_record(boolean isNewRecord) {
  if (timeStatus() != timeNotSet) {
    if (isNewRecord) {
      if (settings.recordCount >= HISTORY_RECORD_COUNT) {
        Serial.println(F("Maximum history record limit reached!!"));
        return;
      }
      // save as a new record
      history.h_time[settings.recordCount]  = now();
      history.h_1000[settings.recordCount]  = coinUnitCounts[0];
      history.h_500[settings.recordCount]   = coinUnitCounts[1];
      history.h_200[settings.recordCount]   = coinUnitCounts[2];
      history.h_100[settings.recordCount]   = coinUnitCounts[3];
      settings.recordCount = settings.recordCount + 1;
    } else {
      // modify the last record values
      if (settings.recordCount > 0) {
        history.h_time[settings.recordCount - 1]  = history.h_time[settings.recordCount - 1];
        history.h_1000[settings.recordCount - 1]  = coinUnitCounts[0];
        history.h_500[settings.recordCount - 1]   = coinUnitCounts[1];
        history.h_200[settings.recordCount - 1]   = coinUnitCounts[2];
        history.h_100[settings.recordCount - 1]   = coinUnitCounts[3];
        //settings.recordCount = settings.recordCount + 1;
      }
    }
    save_settings();
    save_history();
  }
}

void stop_counting() {
  detachInterrupt(COIN_PULSE_PIN);
  digitalWrite(COIN_POWER_PIN, COIN_POWER_OFF);   // power off the coin machine
  printTotal();
  system_status = 3;
  char display_msg[16];
  sprintf_P(display_msg, PSTR("Total=%d"), coinTotalValue);
  lcdUpdateFirstLine(display_msg);
  //resetCoinCounters();
  coinDetected = false;

  // add to the history report as a new record
  saveHistory_record(true);
}

void isCoinCountTimeout() {
  if (coinDetected) {
    if (millis() - latCoinReceived > COIN_TIMEOUT_MS) {
      //Serial.println("Counter Timeout");
      //stop_counting();
    }
  }
}

void printTotal() {
  latCoinReceived = millis();
  system_status = 5;
  coinTotalValue = coinUnitCounts[0] * coinUnitValues[0] + coinUnitCounts[1] * coinUnitValues[1] + coinUnitCounts[2] * coinUnitValues[2] + coinUnitCounts[3] * coinUnitValues[3];
  if (SERIAL_DEBUG) {
    Serial.printf("[1000x%d + 500x%d + 200x%d + 100x%d] = Total=%d\n",
                  coinUnitCounts[0],
                  coinUnitCounts[1],
                  coinUnitCounts[2],
                  coinUnitCounts[3],
                  coinTotalValue);
  }
}

void resetCoinCounters() {
  coinUnitCounts[0] = 0;
  coinUnitCounts[1] = 0;
  coinUnitCounts[2] = 0;
  coinUnitCounts[3] = 0;
  coinTotalValue    = 0;
}

void countCoins() {
  if (coinPuleTimeout >= COIN_PULSE_MS*5) {
    /*
    Serial.print(coinPuleTimeout); Serial.print(":"); Serial.print(COIN_PULSE_MS);
    Serial.print(":");
    Serial.println(coinImpulsCount);
    */
    //Serial.println(millis());
    switch (coinImpulsCount) {
      case 1:
        coinUnitCounts[0] = coinUnitCounts[0] + 1;
        lcdUpdateFirstLine("1000 Detected");
        printTotal();
        break;
      case 2:
        coinUnitCounts[1] = coinUnitCounts[1] + 1;
        lcdUpdateFirstLine("500 Detected");
        printTotal();
        break;
      case 3:
        coinUnitCounts[2] = coinUnitCounts[2] + 1;
        lcdUpdateFirstLine("200 Detected");
        printTotal();
        break;
      case 4:
        coinUnitCounts[3] = coinUnitCounts[3] + 1;
        lcdUpdateFirstLine("100 Detected");
        printTotal();
        break;
      default:
        // default is optional
        break;
    }
    coinImpulsCount = 0;
    coinPuleTimeout = 0;
    //isCoinCountTimeout(); // check for coin counter timeout
  }
}

void setup() {
  pinMode(ONBOARDLED, OUTPUT);
  pinMode(COIN_POWER_PIN, OUTPUT);
  pinMode(COIN_PULSE_PIN, INPUT_PULLUP);
  digitalWrite(COIN_POWER_PIN, COIN_POWER_OFF);
  pinMode(COIN_PULSE_OUT_PIN, OUTPUT);
  digitalWrite(COIN_PULSE_OUT_PIN, LOW);


  lcd.begin();       // initialize LCD
  //lcd.begin(21,22);       // initialize LCD
  lcd.backlight();  // turn on LCD backlight

  //if (SERIAL_DEBUG) {
  Serial.begin(115200);
  Serial.println();
  Serial.print(F("Firmware: ")); Serial.println(FWV);
  //}

  if (SPIFFS.begin(true)) {
    if (SERIAL_DEBUG) {
      Serial.println(F("SPIFFS Mount successful"));
    }
  } else {
    if (SERIAL_DEBUG) {
      Serial.println(F("SPIFFS Mount failed. Formatting"));
      set_factory();
    }
  }

  if (fileSystemCheck(0)) {
    LoadFromFile("/settings.txt", (byte*)&settings, sizeof(settings_t));
    LoadFromFile("/history.txt", (byte*)&history, sizeof(history_t));
    /*
      if (SERIAL_DEBUG) {
      Serial.println(F("-------"));
      Serial.print(F("config file size:"));
      Serial.println(sizeof(settings_t));
      Serial.print(F("history file size:"));
      Serial.println(sizeof(history_t));
      Serial.println(F("-------"));
      print_settings(settings, history);
      }
    */
  }

  if (settings.isConfigured != CONFIG_VERSION) {
    lcd.setCursor(0, 0);
    lcd.print("RESET TO FACTORY");
    fileSystemCheck(true);  // format the SPIFFS
    set_factory();          // this will reset to factory if the config version is different
    lcd.clear();
    lcd.print("DONE!");
    delay(500);
  }

  WiFi.disconnect(true);  // delete old config
  WiFi.onEvent(WiFiEvent);
  lcdUpdateBothLines("CONNECTING WIFI", WIFI_SSID);
  connectToWifi();
  digitalWrite(COIN_POWER_PIN, COIN_POWER_ON);
  delay(50);    // wait till the coin machine get settled
  coinCounter.attach_ms(1, coin_ms_isr);
  //attachInterrupt(COIN_PULSE_PIN, coin_pulse_isr, FALLING);
  //resetCoinCounters();
}

void loop() {
  loopCounter++;
  handleNextNtpSync();  // Sync NTP
  if (millis() > next30s) {
    runEach30Seconds();
  }
  serial_command_listner();
  simulate_puleses();
  if (millis() - lastLcdUpdated > 2000) {
    if (system_status == 2) {
      //lcdUpdateFirstLine("INSERT A COIN");
      lcdUpdateFirstLine("SYSTEM READY");
      system_status = 4;
    } else if (system_status == 5) {
      lcdUpdateFirstLine("INSERT A COIN");
      system_status = 4;
    }
  }
  countCoins();
  heartBeatLed();
  wifiConnectionCheck();
  server.handleClient();
  //delay(1);
}
