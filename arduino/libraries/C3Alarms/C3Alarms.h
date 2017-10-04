// Updated 7, 2011 optimized by Justin Shaw wyojustin@gmail.com (TJS)

#ifndef TimeAlarms_h
#define TimeAlarms_h

#include <inttypes.h>

#include "Time.h"
#include "rtcBOB.h"

#define dtNBR_ALARMS 30 // may need more alarms! // 100 is too many

#define IS_ONESHOT  0
#define NO_REPEAT   0 // same as IS_ONESHOT
#define TIMER_REPEAT 1
#define    REPEAT_ANNUAL (1 << 0)
#define    REPEAT_SUNDAY (1 << 1)
#define    REPEAT_MONDAY (1 << 2)
#define   REPEAT_TUESDAY (1 << 3)
#define REPEAT_WEDNESDAY (1 << 4)
#define  REPEAT_THURSDAY (1 << 5)
#define    REPEAT_FRIDAY (1 << 6)
#define  REPEAT_SATURDAY (1 << 7) 
#define REPEAT_WEEKDAYS 0b01111100
#define REPEAT_WEEKENDS 0b10000010
#define    REPEAT_DAILY 0b11111110
#define     REPEAT_5MIN 0b11111111
#define NO_COUNTDOWN 0
#define COUNTDOWN_10SEC (1 << 0)
#define COUNTDOWN_1MIN (1 << 1)
#define COUNTDOWN_5MIN (1 << 2)
#define COUNTDOWN_1HOUR (1 << 3)
#define COUNTDOWN_1DAY (1 << 4)
#define IS_ALARM    true
#define IS_TIMER    false 


typedef enum { dtMillisecond, dtSecond, dtMinute, dtHour, dtDay } dtUnits_t;

typedef struct  {
  uint8_t isAllocated            :1;  // the alarm is avialable for allocation if false
  uint8_t isEnabled              :1;  // the timer is only actioned if isEnabled is true 
  uint8_t isAlarm                :1;  // time of day alarm if true, period timer if false
  uint8_t countdown_10sec        :1;  // count down from t - ten seconds
  uint8_t countdown_min          :1;  // count down from t - sixty seconds
  uint8_t countdown_5min         :1;  // count down from t - five minutes
  uint8_t countdown_hour         :1;  // count down from t - one hour
  uint8_t countdown_day          :1;  // count down from t - one day
  uint8_t repeat;                  // bit field sunday= bit0--> annual, bit1 --> monday, ...  bit7 --> sat
                                   // If all bits are one, repeat every 5 minutes 
 }
    AlarmMode_t   ;

typedef uint8_t AlarmID_t;
typedef AlarmID_t AlarmId;  // Arduino friendly name
#define dtINVALID_ALARM_ID 255
#define dtUNALLOCATED_ALARM_ID 254


class AlarmClass;  // forward reference
typedef void (*OnTick_t)(uint8_t argument);  // alarm callback function typedef 

// class defining an alarm instance, only used by dtAlarmsClass
class AlarmClass
{  
private:
  AlarmMode_t Mode;

  // TJS: horid bit parsing function, edit at your own risk!
  bool get_delta_days(time_t time); // TJS:
public:
  AlarmClass();
  void init(); // TJS.
  void set_allocated(bool val); // TJS:
  void set_enabled(bool val);   // TJS:
  void set_countdown(uint8_t val); // TJS:
  void set_repeat(uint8_t val); // TJS:
  void set_alarm(bool val);     // TJS:
  bool is_5min_repeat();        // TJS:
  bool is_daily();              // TJS:
  bool is_annual();             // TJS:
  bool is_armed();              // TJS:
  bool is_oneshot();            // TJS:
  bool is_periodic();           // TJS:
  bool is_repeated();           // TJS:
  bool get_allocated();         // TJS:
  bool get_enabled();           // TJS:
  uint8_t get_repeat();         // TJS:
  bool get_alarm();             // TJS:
  uint8_t argument;             // TJS:
  OnTick_t onTickHandler;  
  void updateNextTrigger();
  /* TJS Interpretation of value:
   * value holds time to next trigger:
   * if value is larger than a week, it is an absolute time in seconds past epoch
   * if it is less than a day, it is repeat time in seconds past midnight
   * if it is less than a week, it is repeat time in seconds past midnight
   * otherwise is it not valid.
   */
  time_t value;
  time_t nextTrigger; //TJS: holds next trigger time in absolute time (seconds from epoch).
};

// class containing the collection of alarms
class TimeAlarmsClass
{
 private:
  uint8_t isServicing;
  void findNextTrigger(); // TJS: Find and set the time of the next alarm to be triggered.
  
 public:
  time_t nextTrigger; // TJS:  Next trigger time accross all Alarm[]
  // TJS: made public
  AlarmID_t create( time_t value, 
		    OnTick_t onTickHandler, 
		    uint8_t isAlarm, 
		    uint8_t countdown, 
		    uint8_t repeat, 
		    uint8_t argument=0,
		    uint8_t isEnabled=true);   
  AlarmClass Alarm[dtNBR_ALARMS];
  void serviceAlarms();// TJS: made public.  This is the method I wish to use.
  TimeAlarmsClass();
  // functions to create alarms and timers
  AlarmID_t alarmRepeat(time_t value, OnTick_t onTickHandler);                    // trigger daily at given time of day
  AlarmID_t alarmRepeat(const int H,  const int M,  const int S, OnTick_t onTickHandler); // as above, with hms arguments
  AlarmID_t alarmRepeat(const timeDayOfWeek_t DOW, const int H,  const int M,  const int S, OnTick_t onTickHandler); // as above, with day of week 
 
  AlarmID_t alarmOnce(time_t value, OnTick_t onTickHandler);                     // trigger once at given time of day
  AlarmID_t alarmOnce( const int H,  const int M,  const int S, OnTick_t onTickHandler);  // as above, with hms arguments
  AlarmID_t alarmOnce(const timeDayOfWeek_t DOW, const int H,  const int M,  const int S, OnTick_t onTickHandler); // as above, with day of week 
  
  AlarmID_t timerOnce(time_t value, OnTick_t onTickHandler);   // trigger once after the given number of seconds 
  AlarmID_t timerOnce(const int H,  const int M,  const int S, OnTick_t onTickHandler);   // As above with HMS arguments
  
  AlarmID_t timerRepeat(time_t value, OnTick_t onTickHandler); // trigger after the given number of seconds continuously
  AlarmID_t timerRepeat(const int H,  const int M,  const int S, OnTick_t onTickHandler);   // As above with HMS arguments
  
  void delay(unsigned long ms);
   
  // utility methods
  uint8_t getDigitsNow( dtUnits_t Units);         // returns the current digit value for the given time unit
  void waitForDigits( uint8_t Digits, dtUnits_t Units);  
  void waitForRollover(dtUnits_t Units);
 
  // low level methods
  void enable(AlarmID_t ID);
  void free(AlarmID_t ID);                   // TJS:
  void disable(AlarmID_t ID);
  void write(AlarmID_t ID, time_t value);    // write the value (and enable) the alarm with the given ID
  time_t read(AlarmID_t ID);                 // return the value for the given timer 
};

extern TimeAlarmsClass Alarm;  // make an instance for the user

/*==============================================================================
 * MACROS
 *============================================================================*/

/* public */
#define waitUntilThisSecond(_val_) waitForDigits( _val_, dtSecond)
#define waitUntilThisMinute(_val_) waitForDigits( _val_, dtMinute)
#define waitUntilThisHour(_val_)   waitForDigits( _val_, dtHour)
#define waitUntilThisDay(_val_)    waitForDigits( _val_, dtDay)
#define waitMinuteRollover() waitForRollover(dtSecond)
#define waitHourRollover()   waitForRollover(dtMinute)
#define waitDayRollover()    waitForRollover(dtHour)

#define AlarmHMS(_hr_, _min_, _sec_) (_hr_ * SECS_PER_HOUR + _min_ * SECS_PER_MIN + _sec_)

#endif /* TimeAlarms_h */

