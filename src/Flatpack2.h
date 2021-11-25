#ifndef Flatpack2_h
#define Flatpack2_h

#include <CAN.h>
#include <DebugLog.h>

#define CAN_SPEED           125E3 /* CAN Speed 125 kbps */
#define FLATPACK2_MAX_UNITS 4     /* Maximum number of chargers */

enum FLATPACK2_STATUS
{
  FLATPACK2_STATUS_NORMAL,        // 04 normal voltage reached
  FLATPACK2_STATUS_CURRENT_LIMIT, // 08 current-limiting active
  FLATPACK2_STATUS_WALK_IN,       // 10 Walk in (voltage ramping up)
  FLATPACK2_STATUS_ALARM,         // 0C Alarm
  FLATPACK2_STATUS_UNKNOWN        // unknown response
};

enum FLATPACK2_WALKIN
{
  FLATPACK2_WALKIN_5, // 5 second walk-in
  FLATPACK2_WALKIN_60 // 60 second walk-in
};

struct FLATPACK2_UNIT {
  int              id;             // ID of charger (first charger is 1, second is 2, etc)
  uint8_t          serial[6];      // Serialnumber in binary format
  char             serialStr[13];  // Serialnumber in string format
  unsigned long    lastupdate;     // Last time object was updated (millis())
  unsigned long    lastlogin;      // Last time login packet was send
  FLATPACK2_STATUS status;         // Current status of charger
  int              intake_temp;    // Temperature in centigrade of intake
  int              output_temp;    // Temperature in centigrade of exhaust
  int              input_voltage ; // Input voltage in volts
  int              output_current; // Output current in deciamps (10 deciamps = 1 amp)
  int              output_voltage; // Output voltage in centivolts (100 centivolt = 1 volt)
};

class Flatpack2
{
  public:
    Flatpack2();
    static void Start();
    static void setOutput(int, int, int, FLATPACK2_WALKIN =  FLATPACK2_WALKIN_5);

    static /*inline*/ FLATPACK2_UNIT units[FLATPACK2_MAX_UNITS];
    static /*inline*/ int units_count/* = 0*/ ;
    static /*inline*/ void (*onUpdate)(int)/* = 0*/;
  private:
     static void onReceive(int );
     static void sendLogin(uint8_t *, int );
     static void sendCAN(long , uint8_t * , int, bool = false );
     static void toHex(char *dst, uint8_t *data, int len);
     static void toHexReverse(char *dst, uint8_t *data, int len);
};


#endif
