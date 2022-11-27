void launchWeb() {
  if (SERIAL_DEBUG) {
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print(F("Web server on: "));
      Serial.println(WiFi.localIP());
    }
  }

  // Start the server
  server.on("/", handle_root);
  server.on("/start", handle_start);
  server.on("/stop", handle_stop;
  server.on("/reset", handle_reset);
  server.on("/count", handle_count);
  server.on("/system", handle_system);
  server.on("/history", handle_history);
  server.onNotFound(handleNotFound);
  server.begin();

  if (!MDNS.begin(MDNS_HOST_NAME)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    MDNS.addService("http", "tcp", 80);
    Serial.print(F("Please browse: "));
    Serial.printf("%s.local\n", MDNS_HOST_NAME);
  }
  if (SERIAL_DEBUG) {
    Serial.println(F("Server started"));
  }
}

void add_header(String& str) {
  str += F("<!DOCTYPE HTML>\r\n<html><head><meta charset=\"utf-8\"/><title>");
  str += String(MDNS_HOST_NAME);
  str += F("</title>");

  str += F("<style>");
  str += F("* {font-family:Verdana; font-size:10pt;}");
  str += F("h1 {font-size:16pt; color:black;text-align: center;}");
  str += F(".button-link {padding:5px 15px; background-color:#0077dd; color:#fff; border:solid 1px #fff; text-decoration:none}");
  str += F(".button-link:hover {background:#369;}");
  str += F("th {padding:10px; background-color:black; color:#ffffff;}");
  str += F("td {padding:7px;}");
  str += F("table {color:black;margin-left:auto;margin-right:auto;}");
  str += F(".live {text-align:center;}");
  str += F("</style>");
  str += F("</head><body><h1>");
  str += String("Koinan");
  str += F("</h1><br>");
}

void add_footer(String& str) {
  str += F("<p align=\"center\"><a href=\"https://www.yourCompany.com\">yourCompany.com</a></p>");
}

void handle_root() {
  if (server.method() == HTTP_POST) {
    if (server.args() > 0) {
      if (server.arg("action") != "") {
        if (server.arg("action") == "Reset") {
          Serial.println("Counter reset");
          resetCoinCounters();
          system_status = 3;
          lcdUpdateFirstLine("Total=0");
        } else if (server.arg("action") == "Start") {
          Serial.println("Counter start");
          resetCoinCounters();
          system_status = 5;
          delay(100);
          digitalWrite(COIN_POWER_PIN, COIN_POWER_ON);
          delay(250);
          attachInterrupt(COIN_PULSE_PIN, coin_pulse_isr, FALLING);
        } else if (server.arg("action") == "Stop") {
          Serial.println("Counter stop");
          stop_counting();
        } else if (server.arg("action") == "Modify") {
          if (server.arg("new_1000") != "") {
            coinUnitCounts[0] = server.arg("new_1000").toInt();
          }
          if (server.arg("new_500") != "") {
            coinUnitCounts[1] = server.arg("new_500").toInt();
          }
          if (server.arg("new_200") != "") {
            coinUnitCounts[2] = server.arg("new_200").toInt();
          }
          if (server.arg("new_100") != "") {
            coinUnitCounts[3] = server.arg("new_100").toInt();
          }
          printTotal();   // recalculate the total
          system_status = 3;
          char display_msg[16];
          sprintf_P(display_msg, PSTR("Total=%d"), coinTotalValue);
          lcdUpdateFirstLine(display_msg);
          saveHistory_record(false);  // modify the last histry record
        }
      }
    }
  }
  content = "";
  add_header(content);

  content += F("<form name='frmroot' method='post'>");
  content += F("<table class='live'>");
  content += F("<tr><td>Koin 1000:");
  content += coinUnitCounts[0];
  content += F("<tr><td>Koin 500:");
  content += coinUnitCounts[1];
  content += F("<tr><td>Koin 200:");
  content += coinUnitCounts[2];
  content += F("<tr><td>Koin 100:");
  content += coinUnitCounts[3];
  content += F("<tr><td>Total Rp:");
  content += coinTotalValue;

  content += F("<tr><td><input class=\"button-link\" type='submit' name=\"action\" value=\"Reset\" />");
  content += F("<tr><td><input class=\"button-link\" type='submit' name='action' value='Start'>");
  content += F("<tr><td><input class=\"button-link\" type='submit' name='action' value='Stop'>");

  content += F("<tr><td>");
  content += F("<tr><td>Koin 1000:<input name='new_1000' type='number' size='10' value='");
  content += coinUnitCounts[0];
  content += F("'>");
  content += F("<tr><td>Koin 500: <input name='new_500' type='number' size='10' value='");
  content += coinUnitCounts[1];
  content += F("'>");
  content += F("<tr><td>Koin 200: <input name='new_200' type='number' size='10' value='");
  content += coinUnitCounts[2];
  content += F("'>");
  content += F("<tr><td>Koin 100: <input name='new_100' type='number' size='10' value='");
  content += coinUnitCounts[3];
  content += F("'>");
  content += F("<tr><td><input class=\"button-link\" type='submit' name='action' value='Modify'>");

  content += F("</table></form>");
  add_footer(content);
  server.send(200, PSTR("text/html"), content);
}

void handle_start() {
  Serial.println("Counter start");
  resetCoinCounters();
  system_status = 5;
  delay(100);
  digitalWrite(COIN_POWER_PIN, COIN_POWER_ON);
  delay(250);
  attachInterrupt(COIN_PULSE_PIN, coin_pulse_isr, FALLING);

  String response = "{";
  response+= "\"status\": \"success\"";
  response+="}";
 
  server.send(200, "text/json", response);
}

void handle_stop() {
  Serial.println("Counter stop");
  stop_counting();
  
  String response = "{";
  response+= "\"status\": \"success\"";
  response+="}";
 
  server.send(200, "text/json", response);
}

void handle_reset() {
  Serial.println("Counter reset");
  resetCoinCounters();
  system_status = 3;
  lcdUpdateFirstLine("Total=0");
   
  String response = "{";
  response+= "\"status\": \"success\"";
  response+="}";
 
  server.send(200, "text/json", response);
}

void handle_count() {
  String response = "{";
 
  response+= "\"seratus\": "+server.arg("new_100").toInt();
  response+= ",\"duaratus\": "+server.arg("new_200").toInt();
  response+= ",\"limaratus\": "+server.arg("new_500").toInt();
  response+= ",\"seribu\": "+server.arg("new_1000").toInt();
  response+= ",\"total\": "+coinTotalValue;
  response+="}";
 
  server.send(200, "text/json", response);
}

void handle_system() {
  //IPAddress ip = WiFi.softAPIP();
  IPAddress ip = WiFi.localIP();
  IPAddress gw = WiFi.gatewayIP();
  content = "";
  add_header(content);

  content += F("<form name='frmsystem' method='post'>");
  content += F("<table><th>System Info<th>");
  content += F("<tr><td>Firmware:<td>");
  content += FWV;
  content += F("<tr><td>Core Version:<td>");
  content += ESP.getSdkVersion();
  content += F("<tr><td>Time:<td>");
  content += get_date_time(true);
  content += F("<tr><td>Load:<td>");
  if (count30s > 0) {
    content += 100 - (100 * loopCounterLast / loopCounterMax);
    content += F("% (LC=");
    content += int(loopCounterLast / 30);
    content += F(")");
  }
  content += F("<tr><td>Chip ID:<td>");
  //content += ESP.getChipId();
  content += get_esp_chip_id();
  content += F("<tr><td>Sketch Size/Free:<td>");
  content += ESP.getSketchSize() / 1024;
  content += F(" kB / ");
  content += ESP.getFreeSketchSpace() / 1024;
  content += F(" kB");
  content += F("<tr><td>Flash Used/Free:<td>");
  content += SPIFFS.usedBytes() / 1024;
  content += F(" kB / ");
  content += SPIFFS.totalBytes() / 1024;
  content += F(" kB");

  content += F("<tr><td>Free Mem:<td>");
  content += ESP.getFreeHeap();
  if (WiFi.status() == WL_CONNECTED) {
    content += F("<tr><td>Wifi RSSI:<td>");
    content += WiFi.RSSI();
    content += F(" dB");
  }
  char str[20];
  sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  content += F("<tr><td>IP:<td>");
  content += str;

  sprintf_P(str, PSTR("%u.%u.%u.%u"), gw[0], gw[1], gw[2], gw[3]);
  content += F("<tr><td>GW:<td>");
  content += str;

  content += F("<tr><td>STA MAC:<td>");
  uint8_t mac[] = {0, 0, 0, 0, 0, 0};
  uint8_t* macread = WiFi.macAddress(mac);
  char macaddress[20];
  sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
  content += macaddress;

  content += F("<tr><td>AP MAC:<td>");
  macread = WiFi.softAPmacAddress(mac);
  sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
  content += macaddress;

  content += F("</table></form>");
  add_footer(content);
  server.send(200, PSTR("text/html"), content);
}

void handle_history() {
  if (server.method() == HTTP_POST) {
    if (server.args() > 0) {
      if (server.arg("action") != "") {
        if (server.arg("action") == "Clear") {
          Serial.println("Resetting History Records");
          reset_history();
          settings.recordCount = 0; // reset the counter
          save_settings();
        }
      }
    }
  }
  content = "";
  add_header(content);

  content += F("<form name='frmhistory' method='post'>");
  content += F("<table><th>ID<th>Date Time<th>Summary<th>Total");
  for (byte i = 0; i < settings.recordCount; i++) {
    content += F("<tr><td>");
    content += i;
    content += F("<td>");
    char buf_date[20];
    sprintf_P(buf_date, PSTR("%02d/%02d/%02d %02d:%02d:%02d"),
              year(history.h_time[i]) % 100, month(history.h_time[i]), day(history.h_time[i]),
              hour(history.h_time[i]), minute(history.h_time[i]), second(history.h_time[i]));
    content += buf_date;
    content += F("<td>");
    char buf_value[35];
    sprintf_P(buf_value, PSTR("1000x%d 500x%d 200x%d 100x%d"),
              history.h_1000[i], history.h_500[i], history.h_200[i], history.h_100[i]);
    content += buf_value;
    content += F("<td>");
    content += (history.h_1000[i] * 1000 + history.h_500[i] * 500 + history.h_200[i] * 200 + history.h_100[i] * 100);
  }
  content += F("<tr><td colspan='4'><input class=\"button-link\" type='submit' name='action' value='Refresh'>");
  if (settings.recordCount > 0) {
    content += F("<input onclick=\"return confirm('Are you sure you want to Delete all records?');\" class=\"button-link\" type='submit' name='action' value='Clear'>");
  } else {
    content += F(" Saved records not found<br>");
  }

  content += F("</table></form>");
  add_footer(content);
  server.send(200, PSTR("text/html"), content);
}

void handleNotFound() {
  String message = PSTR("URI: ");
  message += server.uri();
  message += PSTR("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += PSTR("\nArguments: ");
  message += server.args();
  message += PSTR("\n");
  for (uint8_t i = 0; i < server.args(); i++) {
    message += PSTR(" NAME:") + server.argName(i) + PSTR("\n VALUE:") + server.arg(i) + PSTR("\n");
  }
  server.send(404, PSTR("text/plain"), message);
}
