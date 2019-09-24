#include <esp_now.h>
#include <WiFi.h>

#define WIFI_CHANNEL 1
uint8_t rcvd[5] = {0, 0, 0, 0, 0};
uint8_t sent[2] = {0, 0};
int i = 0;
uint8_t water_empty = 0;
uint8_t masterDeviceMac[] = {0x80, 0x7D, 0x3A, 0xC5, 0x23, 0xE8};
esp_now_peer_info_t master;
const esp_now_peer_info_t *masterNode = &master;

unsigned long start;
double fin = 600000.0;  //ms  (600k = 10 min)
int k = 0;


#define SensorPin 34
#define MotorPin 22
#define waterPin 21

uint8_t moisture_level;
float moisture_level_int;

struct Config {
  int min_moisture_level;
  int pump_duration;
  //flags
  bool working_whole_year;
  bool working_whole_day;
  bool tank_empty;
  bool if_working_time;
};
Config config;

void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", WIFI_CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");
  rcvd[k] = *data;
  k++;
  if (k > 4)
    k = 0;
  config.working_whole_year = rcvd[0];
  config.working_whole_day = rcvd[1];
  config.min_moisture_level = rcvd[2]; //<=
  config.pump_duration = rcvd[3];
  config.if_working_time = rcvd[4];

  if (config.working_whole_day && config.working_whole_year)
    config.if_working_time = 1;
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void sendData() {
  sent[0] = moisture_level;
  sent[1] = water_empty;
  const uint8_t *peer_addr = master.peer_addr;
  for (i = 0; i < 2; i++) {
    Serial.print("Sending: "); Serial.println(sent[i]);
    esp_err_t result = esp_now_send(peer_addr, &sent[i], sizeof(sent[i]));
    delay(100);
  }
}
void check_tank_empty() {
  if (digitalRead(waterPin) == LOW)
  {
    water_empty = true;
    sendData();
  }
  if (digitalRead(waterPin) == HIGH)
  {
    delay(600);
    if (digitalRead(waterPin) == LOW)
    {
      water_empty = true;
      sendData();
    }
    else
    {
      water_empty = false;
      sendData();
    }
  }
}
void pumpOff() {
  digitalWrite(MotorPin, LOW);
}
void pumpOn() {
  digitalWrite(MotorPin, HIGH);
  for (i = 0; i < config.pump_duration * 60; i++)
  {
    check_tank_empty();
    delay(1000);
  }

  if (water_empty)
  {
    pumpOff();
    return;
  }
  digitalWrite(MotorPin, LOW);
}
void checkMoisture() {
  for (int i = 0; i <= 50; i++)
  {
    moisture_level_int = moisture_level_int + analogRead(SensorPin);
    delay(1);
  }
  moisture_level_int = moisture_level_int / 50.0;
  moisture_level_int = moisture_level_int / 4096 * 100;
  moisture_level = moisture_level_int;
  if (moisture_level > 100)
    moisture_level = 100;
}


void setup() {
  Serial.begin(9600);
  pinMode(MotorPin, OUTPUT);
  pinMode(waterPin, INPUT);

  //ESPNow Config
  WiFi.mode(WIFI_AP);
  configDeviceAP();
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  InitESPNow();
  memcpy( &master.peer_addr, &masterDeviceMac, 6 );
  master.channel = WIFI_CHANNEL;
  master.encrypt = 0;
  master.ifidx = ESP_IF_WIFI_AP;
  if ( esp_now_add_peer(masterNode) == ESP_OK)
  {
    Serial.println("Added Master Node!");
  }
  else
  {
    Serial.println("Master Node could not be added...");
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  config.min_moisture_level = 0; //<=
  config.pump_duration = 2;
  config.working_whole_year = true;
  config.working_whole_day = true;
  config.if_working_time = true;
}

void loop() {
  if(config.working_whole_day && config.working_whole_year)
    config.if_working_time = true;
  check_tank_empty();
  if (config.if_working_time && !water_empty) {
    checkMoisture();
    if (moisture_level < config.min_moisture_level)
      pumpOn();
    else
      pumpOff();
  }
  for (i = 0; i < 60 * 60 ; i++)
  {
    if (millis() - start > fin)  //store the settings after every 10s
    {
      start = millis();
      checkMoisture();
      Serial.println(moisture_level);
      check_tank_empty();
      sendData();
    }
    delay(1000);
  }

  if (millis() - start > fin)  //store the settings after every 10s
  {
    start = millis();
    checkMoisture();
    Serial.println(moisture_level);
    check_tank_empty();
    sendData();
  }
}
