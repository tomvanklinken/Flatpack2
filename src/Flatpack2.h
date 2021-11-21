#ifndef Flatpack2_h
#define Flatpack2_h

#include <CAN.h>
#include <DebugLog.h>

#define CAN_CLOCK_FREQUENCY   8E6 /* 8Mhz clock frequency for Arduino UNO */
#define CAN_SPEED           125E3 /* CAN Speed 125 kbps */


      // 04 normal voltage reached
      // 08 current-limiting active
      // 10 Walk in (voltage ramping up)
      // 0C Alarm
      
enum FLATPACK2_STATUS
{
  FLATPACK2_STATUS_NORMAL,
  FLATPACK2_STATUS_CURRENT_LIMIT,
  FLATPACK2_STATUS_WALK_IN,
  FLATPACK2_STATUS_ALARM
};
struct FLATPACK2_UNIT {
  int id;
  uint8_t serial[6];
  char serialStr[13];
  FLATPACK2_STATUS status;
};

class Flatpack2
{
  public:
    Flatpack2();
    void Start();
  private:
    static void onReceive(int );
    static void sendLogin(uint8_t *, int );
    static void sendCAN(uint32_t , uint8_t * , int );
    static void sendCAN2(uint32_t , uint8_t * , int );
    static void setOutput(int, int, int );
    static void Flatpack2::toHex(char *dst, uint8_t *data, int len);
    static void Flatpack2::toHexReverse(char *dst, uint8_t *data, int len);
};

#endif
