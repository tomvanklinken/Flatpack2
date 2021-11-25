#include <Flatpack2.h>


int Flatpack2::units_count = 0 ;
void (*Flatpack2::onUpdate)(int) = NULL;
FLATPACK2_UNIT Flatpack2::units[FLATPACK2_MAX_UNITS];


Flatpack2::Flatpack2()
{
  // Not used as we are using only static methods
}
void Flatpack2::Start()
{
  // Start CAN communication
  if (!CAN.begin(CAN_SPEED)) {
    LOG_ERROR("Starting CAN failed!");
    while (1);
  }
  CAN.onReceive(Flatpack2::onReceive);
}
void Flatpack2::toHex(char *dst, uint8_t *data, int len)
{
  for(int i=0;i<len;i++)
  {
    sprintf(dst + i * 2, "%02X", data[i]);
  }
}
void Flatpack2::toHexReverse(char *dst, uint8_t *data, int len)
{
  for(int i=0;i<len;i++)
  {
    sprintf(dst + i * 2, "%02X", data[len-i-1]);
  }
}
void Flatpack2::sendCAN(long packet_id, uint8_t * packet, int len, bool dry_run)
{
  char hexstr[16];
  char packetstr[64];
  


  // Convert packet_id to hex for debug purposes
  Flatpack2::toHexReverse(hexstr,(uint8_t*)&packet_id,sizeof(long));
  Flatpack2::toHex(packetstr,packet,len);
  LOG_INFO("Sending CAN packet (",hexstr,") (len=",len,"): ",packetstr);

  // Show packet to be send
  /*snprintf(str, 64, "Sending CAN packet (%s): ", hexstr ); 
  for(int i=0;i<len;i++) 
  {
    snprintf(hexstr,3,"%02X",packet[i]);
    strcat(str, hexstr);
  }
  LOG_INFO(str);*/
  
  if (! dry_run)
  {
    // Send actual packet
    CAN.beginExtendedPacket(packet_id);
    CAN.write(packet, len);
    CAN.endPacket();
  }
}


void Flatpack2::sendLogin(uint8_t * serialNumber, int id)
{
  uint8_t packet[8];
  for(int i=0;i<6;i++) packet[i]=serialNumber[i];
  packet[6] = 0x00;
  packet[7] = 0x00;

  uint32_t packet_id = 0x05004800 | (id * 4);
  Flatpack2::sendCAN(packet_id, packet, 8);
}

void Flatpack2::setOutput(int current, int voltage, int overvoltage, FLATPACK2_WALKIN walkin)
{

  uint32_t packet_id = 0x05FF4000;
  switch(walkin)
  {
    case FLATPACK2_WALKIN_5:
      packet_id |= 0x4; // 5 second walk-in
      break;
    case FLATPACK2_WALKIN_60:
     packet_id != 0x5; // 60 second walk-in
     break;
  }
  
  uint8_t data[8];
  data[0] /* current           */ =  current     & 0x00ff;
  data[1] /* current           */ = (current     & 0xff00) >> 8;
  data[2] /* output voltage 1  */ =  voltage     & 0x00ff;
  data[3] /* output voltage 1  */ = (voltage     & 0xff00) >> 8;
  data[4] /* output voltage 2  */ =  voltage     & 0x00ff;
  data[5] /* output voltage 2  */ = (voltage     & 0xff00) >> 8;
  data[6] /* overvoltage       */ =  overvoltage & 0x00ff;
  data[7] /* overvoltage       */ = (overvoltage & 0xff00) >> 8;
  Flatpack2::sendCAN(packet_id, data, 8);
}

void Flatpack2::onReceive(int packetSize)
{
  char output_msg[256]; 
  char tmp[32];
  
  LOG_INFO("Flatpack2::onReceive()");

  //strcpy(output_msg, "Received ");


  if (CAN.packetExtended()) {
    //strcat(output_msg, "extended ");
  }

  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
  //  strcat(output_msg, "RTR ");
  }

  //strcat(output_msg, "packet with id 0x");
  //sprintf(tmp,"%08X",  CAN.packetId());
  //strcat(output_msg, tmp);
  

  if (CAN.packetRtr()) {
    //strcat(output_msg, " and requested length ");
    //sprintf(tmp,"%i", CAN.packetDlc());
    //strcat(output_msg, tmp);
  } else {
    //strcat(output_msg, " and length ");
    //sprintf(tmp,"%i", packetSize);
    //strcat(output_msg, tmp);
    
    uint32_t packet_id = CAN.packetId();

    // CAN Bus introduction
    if ( (packet_id & 0xFFFF0000) == 0x05000000 && packetSize == 8)
    {
      uint8_t packet[8];
      for(int i=0;i<8;i++)
        packet[i]=CAN.read();
      //for(int i=0;i<8;i++) {Serial.print(packet[i],HEX);Serial.print(" ");}Serial.println();
            
      if (packet[0] == 0x1B) // Ignore packet[7] this sometimes seems to count up
      {
        uint8_t serialNumber[6];
        char serialNumberStr[13];
        // Convert serialnumber to string
        for(int i=0;i<6;i++)
        {
          serialNumber[i] = packet[i + 1];
          serialNumberStr[i*2 + 0] =   ((packet[i + 1] & 0xf0) >> 4) + 48;
          serialNumberStr[i*2 + 1] =  (packet[i + 1] & 0x0f) + 48;
        }
        serialNumberStr[12] = '\0';


        LOG_INFO("Received CAN BUS Introduction packet from",serialNumberStr);

        // Search for charger
        int i,id = -1;
        for(i = 0; i< Flatpack2::units_count; i++)
        {
          if (memcmp(Flatpack2::units[i].serial,serialNumber,6) == 0)
          {
            id = Flatpack2::units[i].id;
            break;
          }
        }
        if (id == -1 && Flatpack2::units_count < FLATPACK2_MAX_UNITS)
        {
          i = Flatpack2::units_count;
          id = i + 1; // Array starts from 0, units starts from 1
          Flatpack2::units_count++; // Increment counter
          Flatpack2::units[i].id = id;
          memcpy(Flatpack2::units[i].serial, serialNumber, 6);
          strcpy(Flatpack2::units[i].serialStr, serialNumberStr);
          Flatpack2::units[i].lastupdate = 0; // Never updated
          Flatpack2::units[i].lastlogin = 0;  // Never logged in
        }

        //LOG_INFO("Charger ",serialNumberStr, " has id ", id);

        if (id >= 0)
        {
          unsigned long t = millis();
          if (Flatpack2::units[i].lastlogin == 0 || (t-Flatpack2::units[i].lastlogin) > 10000)
          {
            Flatpack2::sendLogin(serialNumber,id);
            Flatpack2::units[i].lastlogin = millis();
          } else {
            LOG_INFO("Skipping login packet");
          }
        } else {
          LOG_ERROR("Too many chargers ", Flatpack2::units_count);
        }
  /*     
       
        
        {
               LOG_INFO("NIER HIER");
         
          {
            id = Flatpack2::units[j].id;
            snprintf(str, 256, "Found charger %s with id %i", serialNumberStr, id);
            LOG_INFO(str);
          }
        }
 if (id == -1)
        {
            snprintf(str, 256, "Charger %s not found", serialNumberStr);
            LOG_INFO(str);
        }*/
        
      }
  
    } else if ( (packet_id & 0xFF00FF00) == 0x05004000 && packetSize == 8) {
      //
      uint8_t id = (packet_id & 0x00FF0000) >> 16;
      uint8_t state = (packet_id & 0x000000FF) ;
      // 04 normal voltage reached
      // 08 current-limiting active
      // 10 Walk in (voltage ramping up)
      // 0C Alarm
      char str[256];

      uint8_t data[8];
      for(int i=0;i<8;i++) data[i] = CAN.read();
      int intake_temp    = data[0];
      int output_temp    = data[7];
      int input_voltage  = data[6] << 8 | data[5];
      int output_current = data[2] << 8 | data[1];
      int output_voltage = data[4] << 8 | data[3];


      // Search for charger
      int i;
      bool found = false;
      for(i = 0; i< Flatpack2::units_count; i++)
      {
        if (Flatpack2::units[i].id == id)
        {
          found = true;
          Flatpack2::units[i].lastupdate     = millis();
          switch(state)
          {
            case 0x04:
              Flatpack2::units[i].status     =  FLATPACK2_STATUS_NORMAL;
              break;
            case 0x08:
              Flatpack2::units[i].status     =  FLATPACK2_STATUS_CURRENT_LIMIT;
              break;
            case 0x10:
              Flatpack2::units[i].status     =  FLATPACK2_STATUS_WALK_IN;
              break;
            case 0x0C:
              Flatpack2::units[i].status     =  FLATPACK2_STATUS_ALARM;
              break;
            default:
              Flatpack2::units[i].status     =  FLATPACK2_STATUS_UNKNOWN;
              break;
          }
          Flatpack2::units[i].intake_temp    = intake_temp;
          Flatpack2::units[i].output_temp    = output_temp;
          Flatpack2::units[i].input_voltage  = input_voltage;
          Flatpack2::units[i].output_current = output_current;
          Flatpack2::units[i].output_voltage = output_voltage;
          // Call callback if available
          if (Flatpack2::onUpdate != NULL){
            (*(Flatpack2::onUpdate))(i);
          }
        }
      }
      if (!found)
      {
        LOG_ERROR("Received status from charger ",id,", but charger not found");
      }
      //if (first_status)
      //{
       // first_status = false;
        //Flatpack2::setOutput(100, 5700, 5950);
      //Flatpack2::setOutput(10, 5200, 5950);
      //  Flatpack2::setOutput(10, 4800, 5950);
      //}
    } else {

      LOG_INFO(output_msg);
      /*
      Serial.print("Data: ");
      // only print packet data for non-RTR packets
      while (CAN.available()) {
        Serial.print((char)CAN.read());
      } 
      Serial.println();*/
    }
  }

}