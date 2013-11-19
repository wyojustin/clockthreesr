#include "Time.h"
#include "C3Alarms.h"

void setup()
{
  Serial.begin(57600);
  setTime(8,29,0,10,1,11); // set time to 8:29:00am Jan 1 2010
  // create the alarms 
  AlarmId mal, dont_alarm;
  
  Alarm.free(mal); // free before allocated ok?
  dont_alarm = Alarm.alarmRepeat(8,29,12, DontAlarm);  // 08:29:12  every da
  Alarm.free(dont_alarm);
  /*
  mal = Alarm.alarmRepeat(8,29,12, MorningAlarm);  // 08:29:12  every day
  Alarm.alarmRepeat(8,29,17,EveningAlarm);  // 08:29:17 every day 
 
  Alarm.timerRepeat(7, Repeats7);            // timer for every 1 seconds    
  Alarm.timerRepeat(3, Repeats3);            // timer for every 5 seconds    
  Alarm.timerOnce(2, OnceOnly);                 // called once after 2 seconds

//create(      value,  onTickHandler,  IS_ALARM,  COUNTDOWN, IS_REPEAT,   arg, IS_ENABLED=true);  
  // Annual event 10 secs from now
  Alarm.create(now() + 10, fire_alarm, true, 0, REPEAT_ANNUAL, 10);
  
  // Weekday event 25 secs from now
  Alarm.create(now() + 25, fire_alarm, true, 0, REPEAT_WEEKDAYS, 25);

  // ONCE 30 secs from now

  //         create(      value,  onTickHandler,  IS_ALARM,  COUNTDOWN, IS_REPEAT,   arg, IS_ENABLED=true);  
  */

  const int FMIN = 1;
  const int ANNUAL = 2;
  const int DAILY = 3;
  const int ONESHOT = 4;
  int testtype = ONESHOT;
  AlarmID_t id1, id2, id3;
  
  time_t t = now();
  switch(testtype){
  case(FMIN):
    id1 = Alarm.create(t - 2 * 86400 + 5, fire_alarm, true, 0, REPEAT_5MIN, 1);
    id2 = Alarm.create(t + 2 * 86400 + 5, fire_alarm, true, 0, REPEAT_5MIN, 2);
    id3 = Alarm.create(t + 0 * 86400 + 5, fire_alarm, true, 0, REPEAT_5MIN, 3); //5min_repeat tested
    break;
  case(ANNUAL):
    id1 = Alarm.create(t - SECS_PER_YEAR(2010) + 5, fire_alarm, true, 0, REPEAT_ANNUAL, 1);
    id2 = Alarm.create(t + 5, fire_alarm, true, 0, REPEAT_ANNUAL, 2);
    break;
  case(DAILY):
    id1 = Alarm.create(t - SECS_PER_YEAR(2010) + 5, fire_alarm, true, 0, REPEAT_DAILY, 1);
    id2 = Alarm.create(t + SECS_PER_YEAR(2010) + 10, fire_alarm, true, 0, REPEAT_DAILY, 2);
    break;
  case(ONESHOT):
    id1 = Alarm.create(t - SECS_PER_DAY +  5,  DontAlarm, true, 0, NO_REPEAT, 1); // passed
    id2 = Alarm.create(t +           0  + 10, fire_alarm, true, 0, NO_REPEAT, 2);  
    id3 = Alarm.create(t +           0  + 20, fire_alarm, true, 0, NO_REPEAT, 3);  
    Serial.print("id2 next: ");
    Serial.print(Alarm.Alarm[id2].nextTrigger - t);
    Serial.print(", id3 next:");
    Serial.println(Alarm.Alarm[id3].nextTrigger - t);
    break;
  }
  Serial.println(Alarm.Alarm[id1].is_5min_repeat(), DEC);
  Serial.println(Alarm.Alarm[id1].is_daily(), DEC);
  Serial.println(Alarm.Alarm[id1].is_annual(), DEC);
  Serial.println(Alarm.Alarm[id1].is_armed(), DEC);
  
  // ONCE 17 Secs from now
  /*Alarm.create(t + 17, fire_alarm, true, 0, NO_REPEAT, 17);*/
}

void  loop(){  
  Serial.println(Alarm.nextTrigger - now());

  // digitalClockDisplay();
  Alarm.serviceAlarms();
  delay(1000);  // wait one second 
}

// functions to be called when an alarm triggers:
void DontAlarm(uint8_t arg){
  Serial.println("Oops, should not have alarmed!");
}
void fire_alarm(uint8_t arg){
  Serial.print("Alarm fired with arg");
  Serial.println(arg, DEC);
}

void MorningAlarm(uint8_t arg){
  if(hour() != 8 || minute() != 29 || second() != 12){
    Serial.println("Morning alarm false triggered");
  }
  else{
    Serial.println("Morning Alarm: - turn lights off");    
  }
}

void EveningAlarm(uint8_t arg){
  if(hour() != 8 || minute() != 29 || second() != 17){
    Serial.println("Evening alarm false triggered");
  }
  else{
    Serial.println("Evening Alarm: - turn lights on");           
  }
}

void Repeats3(uint8_t arg){
  if(millis() < 3000){
    Serial.println("3 second timer alarm pre triggered");
  }
  else{
    Serial.println("3 second timer");         
  }
}

void Repeats7(uint8_t arg){
  if(millis() < 1000){
    Serial.println("7 second timer alarm pre triggered");
  }
  else{
    Serial.println("7 second timer");         
  }
}

void OnceOnly(uint8_t arg){
  if(millis() < 2000){
    Serial.println("OnceOnly alarm pre triggered");
  }
  else{
    Serial.println("This timer only triggers once"); 
  }
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}

void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

