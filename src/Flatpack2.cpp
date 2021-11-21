#include <Flatpack2.h>

Flatpack2::Flatpack2()
{
  // Constructor
}
void Flatpack2::Start()
{

  CAN.setClockFrequency(CAN_CLOCK_FREQUENCY);
  if (!CAN.begin(CAN_SPEED)) {
    LOG_ERROR("Starting CAN failed!");
    while (1);
  }
  CAN.onReceive(this->onReceive);

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
void Flatpack2::sendCAN(uint32_t packet_id, uint8_t * packet, int len)
{
  char str[256];
  char hexstr[32];
  
  /*uint8_t * ptr;
  ptr = (uint8_t*)&packet_id;
  for(int i=0;i<4;i++)
  {
    snprintf(hexstr+i*2,3,"%02X",ptr[i]);
  }*/
  Flatpack2::toHexReverse(hexstr,(uint8_t*)&packet_id,4);
  snprintf(str, 256, "Sending CAN packet (s): ", hexstr ); 
  for(int i=0;i<len;i++) 
  {
    snprintf(hexstr,3,"%02X",packet[i]);
    strcat(str, hexstr);
  }
  LOG_INFO(str);
  CAN.beginExtendedPacket(packet_id);
  for(int i=0;i<len;i++) CAN.write(packet[i]);
  CAN.endPacket();
  
}
void Flatpack2::sendCAN2(uint32_t packet_id, uint8_t * packet, int len)
{
  char str[256];
  char hexstr[32];

  Flatpack2::toHexReverse(hexstr,(uint8_t*)&packet_id,4);
  snprintf(str, 256, "Sending CAN packet (%s): ", hexstr);
  for(int i=0;i<len;i++) 
  {
    sprintf(str,"%s%02X",str,packet[i]);
  }
  LOG_INFO(str);
  
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


void Flatpack2::setOutput(int current, int voltage, int overvoltage)
{

  uint32_t packet_id = 0x05FF4000;
  packet_id |= 0x4; // 5 second walk-in
  //packet_id != 0x5; // 60 second walk-in
  uint8_t data[8];
  data[0] /* current           */ = current & 0x00ff;
  data[1] /* current           */ = (current & 0xff00) >> 8;
  data[2] /* output voltage 1  */ = voltage & 0x00ff;
  data[3] /* output voltage 1  */ = (voltage & 0xff00) >> 8;
  data[4] /* output voltage 2  */ = voltage & 0x00ff;
  data[5] /* output voltage 2  */ = (voltage & 0xff00) >> 8;
  data[6] /* overvoltage       */ = overvoltage & 0x00ff;
  data[7] /* overvoltage       */ = (overvoltage & 0xff00) >> 8;
  Flatpack2::sendCAN(packet_id, data, 8);
}

bool first_status = true;

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
        for(int i=0;i<6;i++)
        {
          serialNumber[i] = packet[i + 1];
          serialNumberStr[i*2 + 0] =   ((packet[i + 1] & 0xf0) >> 4) + 48;
          serialNumberStr[i*2 + 1] =  (packet[i + 1] & 0x0f) + 48;
        }
        serialNumberStr[12] = 0;

        char str[256];
        snprintf(str, 256 ,"Received CAN BUS Introduction packet from %s",serialNumberStr);
        LOG_INFO(str);

        // Send a login packet
        // See if we already know this power supply
        // Do not login every time
        Flatpack2::sendLogin(serialNumber,1);
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


      snprintf(str,256,"Received status from power supply %i, state=%i. intake_temp=%i, output_temp=%i, input_voltage=%i, output_current=%i, output_voltage=%i", 
               id, state, intake_temp, output_temp, input_voltage, output_current, output_voltage);
      LOG_INFO(str);

      //if (first_status)
      //{
       // first_status = false;
        //Flatpack2::setOutput(100, 5700, 5950);
      Flatpack2::setOutput(10, 5200, 5950);
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