#include <esp_now.h>
#include <WiFi.h>
#include <ESP8266WiFi.h>
extern "C" {
    #include <espnow.h>
}




#define WIFI_CHANNEL 1
uint8_t rcvd[5] = {0, 0, 0, 0, 0};
int i = 0;
uint8_t water_empty = 0;
uint8_t masterDeviceMac[] = {0x80, 0x7D, 0x3A, 0xC5, 0x23, 0xE8};
esp_now_peer_info_t master;
const esp_now_peer_info_t *masterNode = &master;

<<<<<<< HEAD
unsigned long start;
double fin = 600000.0;  //ms  (600k = 10 min)
int k = 0;

int pump_speed_percent = 50;  //enter the speed in percentage here
int pump_speed = (pump_speed_percent/100 * 1024);

#define MotorPin 4
#define waterPin 5
=======
#define SensorPin 34
#define MotorPin 1
#define waterPin 2
>>>>>>> parent of 969fdc9... Update smartpump_slave.ino

uint8_t moisture_level;

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
<<<<<<< HEAD
  rcvd[k] = *data;
  k++;
  if (k > 5)
    k = 0;
=======
  rcvd[i] = *data;
  i++;
  if (i > 4)
    i = 0;
>>>>>>> parent of 969fdc9... Update smartpump_slave.ino
  config.working_whole_year = rcvd[0];
  config.working_whole_day = rcvd[1];
  config.min_moisture_level = rcvd[2]; //<=
  config.pump_duration = rcvd[3];
  config.if_working_time = rcvd[4];
  moisture_level = rcvd[5];

  if (config.working_whole_day && config.working_whole_year)
    config.if_working_time = 1;
  for(i=0;i<6;i++)
    masterDeviceMac[i]=mac_addr[i];

}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void sendData() {
  const uint8_t *peer_addr = master.peer_addr;
    Serial.print("Sending: "); Serial.println(water_empty);
    esp_err_t result = esp_now_send(peer_addr, &water_empty, sizeof(water_empty));
    delay(100);
  }
<<<<<<< HEAD
void check_tank_empty() {
=======
}
void check_pump_empty() {
>>>>>>> parent of 969fdc9... Update smartpump_slave.ino
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
  analogWrite(MotorPin, pump_speed);
  for (i = 0; i < config.pump_duration * 60; i++)
  {
    check_pump_empty();
    delay(1000);
  }

  if (water_empty)
  {
    pumpOff();
    return;
  }
  digitalWrite(MotorPin, LOW);
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
<<<<<<< HEAD
  if(config.working_whole_day && config.working_whole_year)
    config.if_working_time = true;
  check_tank_empty();
  if (config.if_working_time && !water_empty) {
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
      Serial.println(moisture_level);
      check_tank_empty();
      sendData();
    }
    delay(1000);
  }

  if (millis() - start > fin)  //store the settings after every 10s
  {
    start = millis();
    Serial.println(moisture_level);
    check_tank_empty();
    sendData();
  }
=======
  //  check_pump_empty();
  //  if (config.if_working_time && !water_empty) {
  //    checkMoisture();
  //    if (moisture_level < config.min_moisture_level)
  //      pumpOn();
  //    else
  //      pumpOff();
  //  }
  //  for (i = 0; i < 60 * 60 ; i++)
  //    delay(1000);
  checkMoisture();

  Serial.println(moisture_level);
  delay(3000);
  sendData();
>>>>>>> parent of 969fdc9... Update smartpump_slave.ino
}
