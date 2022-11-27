// ######## DO NOT MODIFY ANY PARAMETER ##############

#define HISTORY_RECORD_COUNT 50
#define CONFIG_VERSION  1

typedef struct {
  byte isConfigured;
  byte recordCount;
} settings_t __attribute__ ((packed));

const settings_t settings_default{
  CONFIG_VERSION,
  0
};


typedef struct {
  time_t h_time[HISTORY_RECORD_COUNT];
  byte  h_1000[HISTORY_RECORD_COUNT];
  byte  h_500[HISTORY_RECORD_COUNT];
  byte  h_200[HISTORY_RECORD_COUNT];
  byte  h_100[HISTORY_RECORD_COUNT];
} history_t __attribute__ ((packed));


void print_settings(settings_t s1, history_t h1) {
  Serial.printf_P(PSTR("Is Configured: %i \n"
                       "Record Count: %i \n"),
                  s1.isConfigured,
                  s1.recordCount
                 );
  for (byte i = 0; i < s1.recordCount; i++) {
   Serial.printf_P(PSTR("%d %02d/%02d/%02d %02d:%02d:%02d 1000x%d 500x%d 200x%d 100x%d %d\n"),
                    i, year(h1.h_time[i])%100, month(h1.h_time[i]), day(h1.h_time[i]),
                    hour(h1.h_time[i]), minute(h1.h_time[i]), second(h1.h_time[i]),
                    h1.h_1000[i], h1.h_500[i], h1.h_200[i], h1.h_100[i],
                    (h1.h_1000[i] * 1000 + h1.h_500[i] * 500 + h1.h_200[i] * 200 + h1.h_100[i] * 100)
                   );
  }
}

settings_t settings;
history_t  history;

boolean fileSystemCheck(int force_format) {
  if (force_format == 1) {
    Serial.println(F("Formatting SPIFFS: "));
    if (SPIFFS.format()) {
      // clear the memory blockes
      File f = SPIFFS.open("/settings.txt", "w");
      if (f) {
        for (int x = 0; x < sizeof(settings_t); x++)
          f.write(0);
        f.close();
      }
      f = SPIFFS.open("/history.txt", "w");
      if (f) {
        for (int x = 0; x < sizeof(history); x++)
          f.write(0);
        f.close();
      }
      Serial.println(F("Success"));
      return true;
    } else {
      Serial.println(F("Failed"));
      return false;
    }
  }
  if (SPIFFS.exists("/settings.txt")) {
    Serial.println(F("CFG file found"));
    return true;
  } else {
    Serial.println(F("CFG file not found"));
    return false;
  }
}

void SaveToFile(char* fname, byte* memAddress, int datasize) {
  File confFile = SPIFFS.open(fname, FILE_WRITE);
  confFile.write((byte *)memAddress, datasize);
  confFile.close();
}

void LoadFromFile(char* fname, byte* memAddress, int datasize) {
  File confFile = SPIFFS.open(fname, FILE_READ);
  confFile.read((byte *)memAddress, datasize);
  confFile.close();
}

void save_settings() {
  SaveToFile((char*)"/settings.txt", (byte*)&settings, sizeof(settings_t));
}

void save_history() {
  SaveToFile((char*)"/history.txt", (byte*)&history, sizeof(history_t));
}

void reset_history(){
  for (byte i = 0; i < HISTORY_RECORD_COUNT; i++) {
    history.h_time[i] = 0;
    history.h_1000[i] = 0;
    history.h_500[i] = 0;
    history.h_200[i] = 0;
    history.h_100[i] = 0;
  }
  SaveToFile("/history.txt", (byte*)&history, sizeof(history_t));
  LoadFromFile("/history.txt", (byte*)&history, sizeof(history_t));
}

void set_factory() {
  SaveToFile("/settings.txt", (byte*)&settings_default, sizeof(settings_t));
  LoadFromFile("/settings.txt", (byte*)&settings, sizeof(settings_t));
  reset_history();
  if (SERIAL_DEBUG) {
    Serial.println(F("Factory reset: Done"));
    print_settings(settings, history);
  }
}
