#define FWV             "V.1.2"
#define SERIAL_DEBUG    true
#define ONBOARDLED      2
#define ONBOARDLED_ON   HIGH
#define ONBOARDLED_OFF  LOW

#define COIN_PULSE_PIN  15                // coin counter pulse input pin
#define COIN_POWER_PIN  16
#define COIN_PULSE_MS   30                // 30, 50, 100
#define COIN_TIMEOUT_MS 5000              // coin counting timeout in ms   
#define COIN_POWER_ON   LOW               // coin machine power control relay on
#define COIN_POWER_OFF  HIGH              // coin machine power control relay off

// testing purpose (to simulate the coin machine pulses)
#define COIN_PULSE_OUT_PIN  17
#define PHW                 30            //PULSE_HIGH_WIDTH
#define PLW                 90            //PULSE_LOW_WIDTH

#define WIFI_SSID           "SSID"      // WiFi SSID
#define WIFI_PWD            "PWD"     // WiFi PWD
#define MDNS_HOST_NAME      "coin_counter"    // host name for mDNS

//NTP settings
#define NTP_SERVER_IP           "europe.pool.ntp.org"   // NTP server IP
#define NTP_TIME_SHIFT_IN_MIN   420       // 420 min (7H *60)
#define NTP_SYNC_PERIOD_HR      1         // Sync NTP frequency in Hours

// Coin unit value list
int coinUnitValues[]            = { 1000, 500, 200, 100 }; // set the coin unit values

// LCD Display settings
#define lcdColumns              16          // set the LCD number of columns and rows
#define lcdRows                 2
#define lcdDisplayAddress       0x27


//############## DO NOT CHANGE ANYTHING BELOW ######################
#define WIFI_CHECK_INTERVAL           5000
boolean config_wifi                   = false;
boolean is_ntp_sync_success           = false;
boolean led_toggle                    = false;

byte system_status                    = 0;

//General functions
unsigned long next_ntp_sync           = 0;
unsigned long heartBeatPulse          = 0;
unsigned long last_wifi_status_test   = 0;

//CPU load calculation
unsigned long next30s                 = 0;
unsigned long count30s                = 0;
unsigned long loopCounter             = 0;
unsigned long loopCounterLast         = 0;
unsigned long loopCounterMax          = 1;

//Coin counting
volatile int coinImpulsCount          = 0;
volatile int coinPuleTimeout          = 0;
volatile boolean coinDetected         = false;
int coinUnitCounts[]                  = {0, 0, 0, 0};
int coinTotalValue                    = 0;
unsigned long latCoinReceived         = 0;
unsigned long lastLcdUpdated          = 0;

String content;
