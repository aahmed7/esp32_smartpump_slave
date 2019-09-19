#include "time.h"

#define SensorPin A0
#define MotorPin 1

float moisture_level = 0;
int i;

struct tm now;
int min_moisture_level;
int pump_duration;
//flags
bool working_whole_year = false;
bool working_whole_day = false;
bool water_empty = false;
bool if_working_time = false;

//working range
int working_days[] = {0, 1, 2, 3, 4, 5, 6};
int working_hours[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};

void setup() {
  Serial.begin(9600);
  pinMode(MotorPin, OUTPUT);
}


void loop() {
  check_if_working_time();
  if (if_working_time & !water_empty) {
    checkMoisture();
    if (moisture_level < min_moisture_level)
      pumpOn();
    else
      pumpOff();
  }
  for (i = 0; i < 60 * 60 ; i++)
    delay(1000);

  Serial.println(moisture_level);
  delay(30);
}

void checkMoisture()
{
  for (int i = 0; i <= 100; i++)
  {
    moisture_level = moisture_level + analogRead(SensorPin);
    delay(1);
  }
  moisture_level = moisture_level / 100.0;
}

void pumpOn()
{
  digitalWrite(MotorPin, HIGH);
  for (i = 0; i < pump_duration * 60; i++)
    delay(1000);
  digitalWrite(MotorPin, LOW);
}

void pumpOff()
{
  digitalWrite(MotorPin, LOW);
}

void check_if_working_time()
{
  if (working_whole_day && working_whole_year)
    if_working_time = true;
  else if (!working_whole_day && working_whole_year)
  {
    for (i = 0; i < 24; i++)
      if (working_hours[i] == now.tm_sec)
      {
        if_working_time = true;
        break;
      }
      else
        if_working_time = false;
  }
  else if (working_whole_day && !working_whole_year)
  {
    for (i = 0; i < 7; i++)
      if (working_days[i] == now.tm_wday)
      {
        if_working_time = true;
        break;
      }
      else
        if_working_time = false;
  }
  else if (!working_whole_day && !working_whole_year)
    for (i = 0; i < 7; i++)
      if (working_days[i] == now.tm_wday)
      {
        for (i = 0; i < 24; i++)
          if (working_hours[i] == now.tm_hour)
          {
            if_working_time = true;
            break;
          }
          else
            if_working_time = false;
      }
      else
        if_working_time = false;



  printf("%d", if_working_time);
}

void check_pump_empty()
{

}
