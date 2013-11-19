/*
  Based on TimeAlarms.cpp - Arduino Time alarms for use with Time library   
  Copyright (c) 2010 Michael Margolis. 
                2011 optimized by Justin Shaw wyojustin@gmail.com (TJS)
  
  Renamed to C3Alarms.cpp/.h because backward compatibility was broken.
  TJS Jan 2011.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

 */

extern "C" {
#include <string.h> // for memset
}

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Time.h"
#include "C3Alarms.h"
//**************************************************************
//* Alarm Class Constructor

AlarmClass::AlarmClass()
{
  init(); // TJS:
}

/* public methods */
// TJS:
void AlarmClass::init()
{
  set_alarm(false);
  set_enabled(false);
  set_repeat(0);
  set_alarm(false);
  set_countdown(false);
  value = MAX_TIME_T;
  nextTrigger = 0;
  argument = 0;
  onTickHandler = NULL;  // prevent a callback until this pointer is explicitly set 
}
void AlarmClass::set_allocated(bool val){ // TJS:
  /*  true -> alarm memory canNOT be resued
   * false -> alarm memory can be resued
   */
    Mode.isAllocated = val;
}
void AlarmClass::set_enabled(bool val){   // TJS:
  /*  true -> alarm will trigger when time expires
   * false -> alarm will NOT trigger when time expires
   */
  Mode.isEnabled = val;
}
void AlarmClass::set_repeat(uint8_t val){  // TJS:
  /*  bit field repeat every...
   * 0 -- annnual event
   * 1 -- sunday
   * 2 -- monday
   * 3 -- tuesday
   * 4 -- wednesday
   * 5 -- thursday
   * 6 -- friday
   * 7 -- saturday
   *  if all bits are zero -> oneshot alarm
   *  if all bits are one -> repeat every 5 min
   */
  Mode.repeat = val;
}
void AlarmClass::set_countdown(uint8_t countdown){
  /* Bit field
   * 0 -- 10 second count down
   * 1 -- 60 second
   * 2 -- 5 minute
   * 3 -- 1 hour
   * 4 -- 1 day
   * 5 -- NOT USED
   * 6 -- NOT USED
   * 7 -- NOT USED
   */
  if(1 << 0 && countdown) Mode.countdown_10sec = true;
  if(1 << 1 && countdown) Mode.countdown_min = true;
  if(1 << 2 && countdown) Mode.countdown_5min = true;
  if(1 << 3 && countdown) Mode.countdown_hour = true;
  if(1 << 4 && countdown) Mode.countdown_day = true;
}

void AlarmClass::set_alarm(bool val){    // TJS:
  /*  true -> wall clock alarm
   * false -> count down timer
   */
  Mode.isAlarm = val;
}
// repeat alarm based on days of the week (for instance: could be every day, or just one day per week)
bool AlarmClass::is_daily(){                // TJS:
  // return value < SECS_PER_DAY;
  return 1 < Mode.repeat && Mode.repeat < 255;
}
bool AlarmClass::is_5min_repeat(){        // TJS:
  return Mode.repeat == 255;               
}
bool AlarmClass::is_annual(){             // TJS:
  return Mode.repeat == 1;
}
bool AlarmClass::is_oneshot(){            // TJS:
  return Mode.repeat == 0;
}
/*
  Return true if value represents # seconds past start or periodic interval.
 */
bool AlarmClass::is_periodic(){
  return !get_alarm();
}
bool AlarmClass::is_repeated(){            // TJS:
  return Mode.repeat > 0;
}
bool AlarmClass::is_armed(){              // TJS:
  return ((value != MAX_TIME_T) && 
	  get_allocated() && 
	  get_enabled() && 
	  onTickHandler != NULL);
}
bool AlarmClass::get_allocated(){         // TJS:
  return Mode.isAllocated;
}
bool AlarmClass::get_enabled(){           // TJS:
  return Mode.isEnabled;
}
uint8_t AlarmClass::get_repeat(){         // TJS:
  return Mode.repeat;
}
bool AlarmClass::get_alarm(){             // TJS:
  return Mode.isAlarm;
}
// for daily alarms, get num days till next trigger
bool AlarmClass::get_delta_days(time_t time){
  uint8_t dow = dayOfWeek(time) - dowSunday;
  uint8_t delta_days = 1;
  uint16_t two_weeks = ((Mode.repeat & ~1) >>1) | (((Mode.repeat & ~1) >> 1) << 7);
  //      m-f looks like this then 01111100111110
  // everyday looks like this then 11111111111111
  for(; delta_days < 8; delta_days++){
    if((two_weeks >> (dow + delta_days)) & 1){
      break;
    }
  }
  return delta_days;
}
//**************************************************************
//* Private Methods

void AlarmClass::updateNextTrigger()
{
  time_t time = now();
  time_t start_time; // to keep track of when 5min repeat started
  tmElements_t tm;
  if(!is_armed()){
    return; // multiple return statements === ugly
  }
  if(nextTrigger == 0){
    // initialize
    if(is_periodic()){ // value is number of seconds between triggers or from start
                       // may or may not be repeated
      nextTrigger = time + value;
    }
    else if(is_5min_repeat()){ 
      // value hold absolute time of original trigger
      // 5 minute type alarms also repeat every year
      tm.Year = CalendarYrToTm(year());
      tm.Month = month(value);
      tm.Day = day(value);
      tm.Hour = hour(value);
      tm.Minute = minute(value);
      tm.Second = second(value);
      
      nextTrigger = makeTime(tm);
      if(nextTrigger < time){
	nextTrigger += SECS_PER_YEAR(tm.Year);
      }
    }
    else if(is_daily()){ // time of day alarm, find next time of day when this time occurs
      if(value < SECS_PER_DAY){ // value holds time of day
	nextTrigger = previousMidnight(time) + value;
      }
      else{ // value holds abs time (but may be long past)!  
	nextTrigger = previousMidnight(time) + value % SECS_PER_DAY;
      }
      if(nextTrigger <= time){  // find next day of the week
	nextTrigger += (time_t)(SECS_PER_DAY * get_delta_days(time));
      }
    }
    else if(is_annual()){
      tm.Year = CalendarYrToTm(year());
      tm.Month = month(value);
      tm.Day = day(value);
      tm.Hour = hour(value);
      tm.Minute = minute(value);
      tm.Second = second(value);
      
      nextTrigger = makeTime(tm);
      if(nextTrigger < time){
	nextTrigger += SECS_PER_YEAR(tm.Year);
      }
    }
    else{// absolute time single shot!
      // check if fresh (i.e. not stale)
      if(time < value){
	nextTrigger = value;
      }
      else{ // stale one shot delete it (init(), called by Alarm.free(i), accomlishes this)
	init();
      }
    }
  }
  else{ 
    // update
    if(nextTrigger <= time){
      if(is_periodic() && is_repeated()){
	  nextTrigger = time + value;
      }
      else if(is_daily()){ // time of day alarm, find next time of day when this time occurs
	// find next day of the week
	nextTrigger += (time_t)(SECS_PER_DAY * get_delta_days(time));
      }
      else if(is_annual()){
	  tm.Year = CalendarYrToTm(year(nextTrigger)) + 1;
	  tm.Month = month(nextTrigger);
	  tm.Day = day(nextTrigger);
	  tm.Hour = hour(nextTrigger);
	  tm.Minute = minute(nextTrigger);
	  tm.Second = second(nextTrigger);
	  
	  nextTrigger = makeTime(tm);
      }
      else if(is_5min_repeat()){
	tm.Year = CalendarYrToTm(year());
	tm.Month = month(value);
	tm.Day = day(value);
	tm.Hour = hour(value);
	tm.Minute = minute(value);
	tm.Second = second(value);

	start_time = makeTime(tm);
	if(start_time > time){
	  // wait
	}
	else if(start_time <= time && time < start_time + SECS_PER_DAY){ // repeat for 24 hours
	  nextTrigger = time + 5 * SECS_PER_MIN;
	}
	else{ // set up for next year
	  // value holds absolute time of firs trigger
	  tm.Year = CalendarYrToTm(year());
	  tm.Month = month(value);
	  tm.Day = day(value);
	  tm.Hour = hour(value);
	  tm.Minute = minute(value);
	  tm.Second = second(value);

	  start_time = makeTime(tm);
	  
	  nextTrigger = makeTime(tm);
	}
      }
      else{// absolute time
	   // absolute time has passed // should not get here
	init();
      }
    }
  }
}

//**************************************************************
//* Time Alarms Public Methods

TimeAlarmsClass::TimeAlarmsClass()
{
  isServicing = false;
  for(uint8_t id = 0; id < dtNBR_ALARMS; id++)
    Alarm[id].set_allocated(false);       // ensure  all Alarms are avialable for allocation  
  nextTrigger = MAX_TIME_T;               // TJS: next tigger time in seconds past epoch (Feb 7, 2106 w/ 32 bit uint)
}

AlarmID_t TimeAlarmsClass::alarmOnce(time_t value, OnTick_t onTickHandler){   // trigger once at the given time of day
  return create( value, onTickHandler, IS_ALARM, NO_COUNTDOWN, IS_ONESHOT );
}

AlarmID_t TimeAlarmsClass::alarmOnce(const int H,  const int M,  const int S,OnTick_t onTickHandler){   // as above with HMS arguments
  return create( AlarmHMS(H,M,S), onTickHandler, IS_ALARM, NO_COUNTDOWN, IS_ONESHOT );
}
   
AlarmID_t TimeAlarmsClass::alarmOnce(const timeDayOfWeek_t DOW, const int H,  const int M,  const int S, OnTick_t onTickHandler){  // as above, with day of week 
   unsigned long dayOffset =  ( 7 + DOW - dayOfWeek(now())) %7;
   return create( (dayOffset * SECS_PER_DAY) + AlarmHMS(H,M,S), onTickHandler, IS_ALARM, NO_COUNTDOWN, IS_ONESHOT );   
}
   
AlarmID_t TimeAlarmsClass::alarmRepeat(time_t value, OnTick_t onTickHandler){ // trigger daily at the given time
     return create( value, onTickHandler, IS_ALARM, NO_COUNTDOWN, REPEAT_DAILY );
}

AlarmID_t TimeAlarmsClass::alarmRepeat(const int H,  const int M,  const int S, OnTick_t onTickHandler){ // as above with HMS arguments
     return create( AlarmHMS(H,M,S), onTickHandler, IS_ALARM, NO_COUNTDOWN, REPEAT_DAILY);
}

AlarmID_t TimeAlarmsClass::alarmRepeat(const timeDayOfWeek_t DOW, const int H,  const int M,  const int S, OnTick_t onTickHandler){  // as above, with day of week 
   unsigned long dayOffset =  ( 7 + DOW - dayOfWeek(now())) %7;
   return create( (dayOffset * SECS_PER_DAY) + AlarmHMS(H,M,S), onTickHandler, IS_ALARM, NO_COUNTDOWN, REPEAT_DAILY);      
}

AlarmID_t TimeAlarmsClass::timerOnce(time_t value, OnTick_t onTickHandler){   // trigger once after the given number of seconds 
     return create(value, onTickHandler, IS_TIMER, NO_COUNTDOWN, IS_ONESHOT );
}

AlarmID_t TimeAlarmsClass::timerOnce(const int H,  const int M,  const int S, OnTick_t onTickHandler){   // As above with HMS arguments
  return create( AlarmHMS(H,M,S), onTickHandler, IS_TIMER, NO_COUNTDOWN, IS_ONESHOT );
}
  
AlarmID_t TimeAlarmsClass::timerRepeat(time_t value, OnTick_t onTickHandler){ // trigger after the given number of seconds continuously
     return create( value, onTickHandler, IS_TIMER, NO_COUNTDOWN, TIMER_REPEAT);
}

AlarmID_t TimeAlarmsClass::timerRepeat(const int H,  const int M,  const int S, OnTick_t onTickHandler){ // trigger after the given number of seconds continuously
     return create( AlarmHMS(H,M,S), onTickHandler, IS_TIMER, NO_COUNTDOWN, TIMER_REPEAT);
}

void TimeAlarmsClass::enable(AlarmID_t ID)
{
  if(ID < dtNBR_ALARMS && Alarm[ID].get_allocated()){
    Alarm[ID].updateNextTrigger(); // trigger is updated whenever  this is called, even if already enabled

    // only enable if value is non zero and a tick handler has been set
    if ((Alarm[ID].value != 0) && (Alarm[ID].onTickHandler != 0)){
      Alarm[ID].set_enabled(true);
    }
    else{
      Alarm[ID].set_enabled(false);
    }
    findNextTrigger(); //TJS: since Alarm[ID].nextTrigger may have been updated we have to check of this.nextTrigger needs to be updated

  }
}

/*
 * TJS: update next trigger for all alarms and find minimum.
 */
void TimeAlarmsClass::findNextTrigger(){
  nextTrigger = MAX_TIME_T; // (Feb 7, 2106 w/ 32 bit uint)

  for(int i = 0; i < dtNBR_ALARMS; i++){
    if(Alarm[i].get_enabled() && (Alarm[i].onTickHandler != NULL)){
      if(Alarm[i].nextTrigger <= nextTrigger){
	// TJS: Found the new next trigger!
	nextTrigger = Alarm[i].nextTrigger;
      }
    }
  }
}
void TimeAlarmsClass::free(AlarmID_t ID)
{
  if(ID < dtNBR_ALARMS){
    Alarm[ID].init();
  }
}
void TimeAlarmsClass::disable(AlarmID_t ID)
{
  time_t new_next_trigger = MAX_TIME_T;
  if(ID < dtNBR_ALARMS && Alarm[ID].get_allocated()){
    Alarm[ID].set_enabled(false);
    if(Alarm[ID].nextTrigger == nextTrigger){
      //TJS: find new nextTrigger!
      findNextTrigger();
    }
  }
}

// write the given value to the given alarm
void TimeAlarmsClass::write(AlarmID_t ID, time_t value)
{
  if(ID < dtNBR_ALARMS && Alarm[ID].get_allocated()){
    Alarm[ID].value = value;
    enable(ID); // TJS: Enable will check for nextTrigger
  }
}

// return the value for the given alarm
time_t TimeAlarmsClass::read(AlarmID_t ID)
{
  if(ID < dtNBR_ALARMS && Alarm[ID].get_allocated())
    return Alarm[ID].value;
  else 	
    return 0l;  
}
 
// following functions are not Alarm ID specific.
void TimeAlarmsClass::delay(unsigned long ms)
{
  unsigned long start = millis();
  while( millis() - start  <= ms)
    serviceAlarms();
}
		
void TimeAlarmsClass::waitForDigits( uint8_t Digits, dtUnits_t Units)
{
  while(Digits != getDigitsNow(Units) )
  {
    serviceAlarms();
  }
}

void TimeAlarmsClass::waitForRollover( dtUnits_t Units)
{
  while(getDigitsNow(Units) == 0  ) // if its just rolled over than wait for another rollover	                            
    serviceAlarms();
  waitForDigits(0, Units);
}

uint8_t TimeAlarmsClass::getDigitsNow( dtUnits_t Units)
{
  time_t time = now();
  if(Units == dtSecond) return numberOfSeconds(time);
  if(Units == dtMinute) return numberOfMinutes(time); 
  if(Units == dtHour) return numberOfHours(time);
  if(Units == dtDay) return dayOfWeek(time);
  return 255;  // This should never happen 
}

//***********************************************************
//* Private Methods

void TimeAlarmsClass::serviceAlarms()
{
  if(! isServicing)
  {
    isServicing = true;
    /* TJS: Speed up servicing (now() called multiple times to save memory)    
     * Just check against next trigger
     */
    time_t time = now();
    if(time >= nextTrigger){ 
      // now alarm ALL alarms that need to be triggered
      for(uint8_t i = 0; i < dtNBR_ALARMS; i++){
	if(Alarm[i].is_armed() && 
	   (0 < Alarm[i].nextTrigger &&  Alarm[i].nextTrigger <= time)){
	  Alarm[i].onTickHandler(Alarm[i].argument);
	  if(Alarm[i].is_oneshot()){
	    free(i);  // free the ID if mode is OneShot		
	  }
	  else{
	    Alarm[i].updateNextTrigger();
	  }
	}
      }
      //TJS: Find next trigger!
      findNextTrigger();
    }
    isServicing = false;
  }
}

// returns true if has been registerd ok
//                         create(       value,  onTickHandler,  IS_ALARM,  COUNTDOWN, IS_REPEAT,   arg, IS_ENABLED);
AlarmID_t TimeAlarmsClass::create(time_t value, 
				  OnTick_t onTickHandler,
				  boolean isAlarm, 
				  uint8_t countdown, 
				  uint8_t repeat, 
				  uint8_t argument,
				  boolean isEnabled ){
  for(uint8_t id = 0; id < dtNBR_ALARMS; id++)
  {
    if(Alarm[id].get_allocated() == false)
    {
      // here if there is an Alarm id is available
      Alarm[id].set_allocated(true);
      Alarm[id].onTickHandler = onTickHandler;
      Alarm[id].set_alarm(isAlarm);
      Alarm[id].set_countdown(countdown);
      Alarm[id].set_repeat(repeat);
      Alarm[id].value = value;
      isEnabled ?  enable(id) : disable(id);   
      Alarm[id].updateNextTrigger();
      Alarm[id].argument = argument;
      findNextTrigger(); // TJS: find the next alarm that needs to be triggered
      return id;
    }  
  }
  return dtINVALID_ALARM_ID; // no IDs available // TJS: multiple return statements! ** hurumph! **
}

// make one instance for the user to use
TimeAlarmsClass Alarm = TimeAlarmsClass() ;

