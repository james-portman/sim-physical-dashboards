// http://www.loopybunny.co.uk/CarPC/k_can.html
// H revision sketch
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------
// ----------------------------------------------------- PLEASE USE SETUP ASSISTANT AVAILABLE IN SUB FOLDER !!!! -------------------------------------------

#include <mcp_can.h>
#include <SPI.h>

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin


#include "FlowSerialRead.h"

#define MESSAGE_HEADER 0x03


String DEVICE_NAME = String("E90 Cluster"); //{"Group":"General","Name":"DEVICE_NAME","Title":"Device name","DefaultValue":"SimHub Dash","Type":"string","Template":"String DEVICE_NAME = String(\"{0}\");"}


//void setSpeed(int mph) {
//}
//
//void setFuelPercent(int percent) {
//
//}
//
//void setMpg(int percent) {
//}
//
//// got this pretty good, 90 degrees as middle/vertical
//void setTemp(int degrees) {
//}
//
//void setRpmFreq(int freq) {
//  // translate this for simhub
//}
//void setRpmPercent(int percent) {
//  
//}
//void setRpm(int rpm) {
//}
//
//bool blinkWarningLeds = false;
//int ledState = LOW;
//unsigned long ledStateStarted = 0;
//int blinkTime = 100;
//void blinkLED(void)
//{
//  if (blinkWarningLeds) {
//    if (millis() > ledStateStarted+blinkTime) {
//      ledState = !ledState;
//      ledStateStarted = millis(); 
//    }
//  } else {
//    ledState = LOW;
//  }
//  digitalWrite(10, ledState);
//}
//



unsigned char wake_up_packet[5] = {0x45, 0x40, 0x21, 0x8F, 0xFE};
unsigned char ignition_state[8] = {0x00, 0x40, 0x7F, 0x50, 0xFF, 0xFF, 0xFF, 0xFF};



void sendRpmPercent(int percent) {
  sendRpm(percent * 70);
}

void sendRpm(long rpm) {
  unsigned char rpm_packet[8] = {0xFE, 0xFE, 0xFF, 0x00, 0x10, 0x10, 0xFE, 0x99};
  // 500 = barely moves
  // 1000 = 190~ = 5.26
  // 2000 = 400~ = 5
  // 4000 = 900~ = 4.444
  // 6000 = 1400 = 4.28
  // 8000 = 1900 = 4.21
  // 20000 = 4900 = 4.08
  // 25000 = 6150 = 4.065
  // 30,000 = off the chart
  rpm = (rpm * 4.05) + 300;
  rpm_packet[4] = rpm & 0xFF;
  rpm_packet[5] = rpm >> 8;
  CAN.sendMsgBuf(0xAA, 0, 8, rpm_packet);
}

void sendLightsState(bool on) {
  unsigned char lights_packet[3] = {0x05, 0x12, 0xF7};
  if (!on) {
    lights_packet[0] = 0;
    lights_packet[1] = 0;
  }
  CAN.sendMsgBuf(0x21A, 0, 3, lights_packet);

}

// doesn't work
void sendHandbrake(bool on) {
  // unsigned char handbrake_packet[2] = {0xFE, 0xFF};
  unsigned char handbrake_packet[2] = {0xFD, 0xFF};
  CAN.sendMsgBuf(0x34F, 0, 2, handbrake_packet);
}

long speedLastSent = 0;
long speedSentCounter = 0;
long lastSpeedFinalValue = 0;
void sendSpeed(long speed) {
  // fix it displaying under!?
  if (speed >= 145) {
    speed += 6;
  } else if (speed >= 90) {
    speed += 5;
  } else if (speed >= 70) {
    speed += 4;
  } else if (speed >= 60) {
    speed += 3;
  } else if (speed >= 30) {
    speed += 2;
  }

  unsigned char speed_packet[8] = {0x13, 0x4D, 0x46, 0x4D, 0x33, 0x4D, 0xD0, 0xFF};

  long newValues = speed;
  newValues = newValues + lastSpeedFinalValue;
  newValues = newValues;
  lastSpeedFinalValue = newValues;

  int newValue1 = newValues & 0xFF;
  int newValue2 = newValues >> 8;
  speed_packet[0] = newValue1;
  speed_packet[1] = newValue2;
  speed_packet[2] = newValue1;
  speed_packet[3] = newValue2;
  speed_packet[4] = newValue1;
  speed_packet[5] = newValue2;

  speed_packet[6] = speedSentCounter & 0xFF;
  speed_packet[7] = (speedSentCounter >> 8) | 0xF0;
  
  CAN.sendMsgBuf(0x1A6, 0, 8, speed_packet);
  speedLastSent = millis();
  speedSentCounter += 200;
}

int absValue = 0;
long absLastSent = 0;
void sendAbsCounter() {
  unsigned char abs_force_packet[8] = {0x00, 0xE0, 0xB3, 0xFC, 0xF0, 0x00, 0x00, 0x65};
  CAN.sendMsgBuf(0x19E, 0, 8, abs_force_packet);
  
  if (millis() < absLastSent + 200) {
    return;
  }
  unsigned char abs_packet[2] = {0xF4, 0xFF};
  absValue = (absValue + 1);
  if (absValue > 0x0F) { absValue = 0; }
  abs_packet[0] = absValue + 0xF0;
  CAN.sendMsgBuf(0x0C0, 0, 8, abs_packet);
  absLastSent = millis();

}

long cruiseLastSent = 0;
int cruiseCounter = 0;
void sendCruise() {
  if (millis() > cruiseLastSent + 200) {
    unsigned char cruise_packet[8] = {0x27, 0xFE, 0xF1, 0x00, 0xF0, 0x50, 0x00, 0x00};
//    unsigned char cruise_packet[8] = {0x5B, 0x27, 0xF5, 0x00, 0xF8, 0x58, 0x00, 0x00}; // cruise on
    cruise_packet[0] = cruiseCounter;
    CAN.sendMsgBuf(0x193, 0, 8, cruise_packet);
    cruiseLastSent = millis();
    cruiseCounter += 17;
    cruiseCounter = cruiseCounter & 0xFF;
  }
}

// fixed airbag and seatbelt lights
long airBagCounter = 0;
long airBagLastSent = 0;
void sendAirbag() {
  if (millis() < airBagLastSent + 200) {
    return;
  }
  unsigned char airbag_packet[2] = {0xF4, 0xFF};
  airbag_packet[0] = airBagCounter;
  CAN.sendMsgBuf(0x0D7, 0, 2, airbag_packet);
  airBagCounter++;
  if (airBagCounter > 254) {
    airBagCounter = 0;
  }
  airBagLastSent = millis();
}


/* check engine light:
In short this message turns the check engine on
ID = 0x592
Len = 8
B0 = 0x40
B1 = 0x22
B2 = 0
B3 = 0x31
B4 = 0xFF
B5 = 0xFF
B6 = 0xFF
B7 = 0xFF

Change B3 to 30 and it will force the light off. Change B1 to a different Error ID and it'll light other messages.
 */


void sendFuelPercent(float percentage) {

  unsigned char fuel_packet[5] = {0x76, 0x20, 0xBE, 0x20, 0x00};

  // update, looks like 2 sensors - bytes 1+2 and 3+4
  // do litres * 160
  // need to figure out how many litres this dash thinks it has
  int full_litres = 43;
//  float litres = (percentage / 100) * full_litres;
  int output = ((percentage * 160) / 100) * (full_litres);
  fuel_packet[1] = output >> 8;
  fuel_packet[2] = output & 0xFF;
  fuel_packet[3] = output >> 8;
  fuel_packet[4] = output & 0xFF;
  
  CAN.sendMsgBuf(0x349, 0, 5, fuel_packet);

}


int speedMph = 0;
int rpmPercent = 0;
int fuelPercent = 0;

int blinkTime = 200;
long lastBlinkChange = 0;
bool blinkOn = true;

// actually send the current values to the dash
void updateDash() {
  CAN.sendMsgBuf(0x130, 0, 5, wake_up_packet);
  CAN.sendMsgBuf(0x26E, 0, 8, ignition_state);

  sendRpmPercent(rpmPercent);
  sendSpeed(speedMph);
  sendFuelPercent(fuelPercent);

  sendHandbrake(true); // not working
  sendAirbag(); // works
  sendAbsCounter();
  sendCruise(); // doesnt work

  if (rpmPercent > 90) {
    if (rpmPercent > 95) {
      blinkTime = 100;
    } else {
      blinkTime = int((100 - rpmPercent) * 40);
    }
    blinkLights();
  } else {
    sendLightsState(true);
  }

}


void blinkLights() {
  if (millis() > lastBlinkChange + blinkTime) {
    blinkOn = !blinkOn;
    lastBlinkChange = millis();
  }
  sendLightsState(blinkOn);
}

void setup()
{

  // ledStateStarted = millis();

	FlowSerialBegin(500000);

  
  while (CAN_OK != CAN.begin(CAN_100KBPS)) {            // init can bus : baudrate = 500k
    SERIAL.println("CAN BUS Shield init fail");
    SERIAL.println(" Init CAN BUS Shield again");
    delay(100);
  }
  SERIAL.println("CAN BUS Shield init ok!");

  rpmPercent = 100;
  speedMph = 155;
  fuelPercent = 50;

  long before = millis();
  while (millis() < before + 2000) {
    updateDash();
  }

  speedMph = 0;
  rpmPercent = 0;

}


void sendButtonState() {
	bool sendButtons = false;
	FlowSerialFlush();
}

void SetBaudrate() {
	int br = FlowSerialTimedRead();

	delay(200);

	if (br == 1) FlowSerialBegin(300);
	if (br == 2) FlowSerialBegin(1200);
	if (br == 3) FlowSerialBegin(2400);
	if (br == 4) FlowSerialBegin(4800);
	if (br == 5) FlowSerialBegin(9600);
	if (br == 6) FlowSerialBegin(14400);
	if (br == 7) FlowSerialBegin(19200);
	if (br == 8) FlowSerialBegin(28800);
	if (br == 9) FlowSerialBegin(38400);
	if (br == 10) FlowSerialBegin(57600);
	if (br == 11) FlowSerialBegin(115200);
	if (br == 12) FlowSerialBegin(230400);
	if (br == 13) FlowSerialBegin(250000);
	if (br == 14) FlowSerialBegin(1000000);
	if (br == 15) FlowSerialBegin(2000000);
	if (br == 16) FlowSerialBegin(200000);
}

uint8_t header = 0;
char opt;

bool DEBUG = false;

int loops = 0;
void loop() {




//  blinkLED();

	// Wait for data
	if (FlowSerialAvailable() > 0) {
		// Reader header
		header = FlowSerialRead();

		if (header == MESSAGE_HEADER)
		{
			// Read command
			opt = FlowSerialTimedRead();

			// Hello command
			if (opt == '1') {
				FlowSerialTrigger = FlowSerialTimedRead();
				delay(10);
				FlowSerialPrint('h');
				FlowSerialFlush();
			}

			// Set baudrate
			if (opt == '8') {
				SetBaudrate();
			}

			// Simple buttons count
			if (opt == 'J') {
				FlowSerialWrite((byte)(0));
				FlowSerialFlush();
			}

			//  Module count command
			if (opt == '2') {
				FlowSerialWrite((byte)(0));
				FlowSerialFlush();
			}

			//  SIMPLE Module count command
			if (opt == 'B') {
				FlowSerialWrite((byte)(0));
				FlowSerialFlush();
			}

			// ACQ Packet
			if (opt == 'A') {
				FlowSerialWrite(0x03);
				FlowSerialFlush();
			}

			// Buttons state
			if (opt == 'C') {
				sendButtonState();
				FlowSerialFlush();
			}

			// Device Name
			if (opt == 'N') {
				FlowSerialPrint(DEVICE_NAME);
				FlowSerialPrint("\n");
				FlowSerialFlush();
			}

			// Tachometer
			if (opt == 'T') {
				int freq = FlowSerialReadStringUntil('\n').toInt();
        // setRpmFreq(freq);
			}

			// Features command
			if (opt == '0') {
				delay(10);

				// Gear
				FlowSerialPrint("G");

				// Name
				FlowSerialPrint("N");

				// Additional buttons
				FlowSerialPrint("J");

				// Custom Protocol Support
				FlowSerialPrint("P");

				FlowSerialPrint("T");

				FlowSerialPrint("\n");
				FlowSerialFlush();
			}

			//  RGBLED count command
			if (opt == '4') {
				FlowSerialWrite((byte)(0));
				FlowSerialFlush();
			}

			// GEAR
			if (opt == 'G') {
				char gear = FlowSerialTimedRead();
				//
				// Do what you want with current gear here ;)
				//
			}

			// Custom protocol for end users (See Wiki for more infos : https://github.com/zegreatclan/AssettoCorsaTools/wiki/Custom-Arduino-hardware-support)
			if (opt == 'P') {

				// -------------------------------------------------------
				// EXAMPLE 3 - Driving a E36 speed gauge connected to pin D3
				// Protocol formula must be set in simhub to round([DataCorePlugin.GameData.NewData.SpeedKmh],0)
				// format([DataCorePlugin.GameData.NewData.SpeedKmh],'0')
				// -------------------------------------------------------
				CustomProtocolExample3_E36Speedo();
			}
		}
	}

  updateDash();

  // clear errors?
  //  unsigned char clear_packet[8] = {0x12, 0x3, 0x14, 0xFF, 0xFF, 0xFF, 0x0, 0x0};
  //  CAN.sendMsgBuf(0x6F1, 0, 5, clear_packet);
  
  // hazards, fires clicker/speaker
  //unsigned char indicator_packet[2] = {0xB1, 0xF2};
  //CAN.sendMsgBuf(0x1F6, 0, 2, indicator_packet);
  
  // wheel speeds
  unsigned char wheel_speed_packet[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  CAN.sendMsgBuf(0x0CE, 0, 8, wheel_speed_packet);

  // TODO: send some things less often
  // millis % 1000 ?


  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive()) {         // check if data coming
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

    unsigned long canId = CAN.getCanId();

    if (DEBUG) {
      // dont print some
      if (canId == 0x1B4) { // kombi outputting speed/handbrake
      } else if (canId == 0x328) { // kombi outputting time (realtime?)?
      } else if (canId == 0x205) { // kombi outputting Acoustic demand combination
      } else if (canId == 0x2Ca) { // kombi outputting Outdoor temperature
      } else if (canId == 0x367) { // kombi outputting Control Display Demand-oriented Service
      } else if (canId == 0x330) { // kombi outputting mileage/range
      } else if (canId == 0x202) { // dimming
      } else if (canId == 0x2BA) { // kombi output stopwatch?
      } else if (canId == 0x35E) { // Data on board computer (travel data) [5]  0x60  Kombi
      } else if (canId == 0x2C0) { // LCD lighting [7]  0x60  Kombi
      } else if (canId == 0x2F8) { // Time / Date [12]  0x60  Kombi
      } else if (canId == 0x366) { // Combi / External Display [3]  0x60  Kombi
      } else if (canId == 0x35C) { // Status of board computer [5]  0x60  Kombi
      } else if (canId == 0x364) { // Data board computer (arrival) [2] 0x60  Kombi
      } else if (canId == 0x362) { // [4] Data on board computer (average values) [4] 0x60  Kombi
      } else if (canId == 0x360) { // [2]  Data board computer (start of journey) [2]  0x60  Kombi
      } else {
  
        SERIAL.println("-----------------------------");
        SERIAL.print("Get data from ID: 0x");
        SERIAL.println(canId, HEX);
  
        if (canId == 0x338) {
          SERIAL.println("Control display Checkcontrol message");
        } else if (canId == 0x4E0) {
          SERIAL.println("Network management  0x60  Kombi");
        } else if (canId == 0x5E0) {
          SERIAL.println("services  0x60  Kombi");
        } else if (canId == 0x336) {
          SERIAL.println("Display of checkcontrol message (role) [3]  0x60  Kombi");
        } else if (canId == 0x394) {
          SERIAL.println("[5] RDA Request / Data Storage [5]  0x60  Kombi");
        } else if (canId == 0x2F7) {
          SERIAL.println("0x2F7 Einheiten [10]  Units [10]  0x62  M_ASK/CCC_GW");
          SERIAL.println("0x2F7 Einheiten [10]  Units [10]  0x63  CCC_MM");
        } else {
          SERIAL.println("UNKNOWN (yet) MESSAGE *******************************************************");
        }
    
        for (int i = 0; i < len; i++) { // print the data
          SERIAL.print(buf[i], HEX);
          SERIAL.print("\t");
        }
        SERIAL.println();
      }
    }
  }


  //    SERIAL.println(".");
  delay(100);                       // send data per 100ms
  loops++;
}


void CustomProtocolExample3_E36Speedo() {
  fuelPercent = FlowSerialReadStringUntil(';').toInt();
	speedMph = FlowSerialReadStringUntil(';').toInt();
  rpmPercent = FlowSerialReadStringUntil(';').toInt();
  int temp = FlowSerialReadStringUntil(';').toInt();
  int mpg = FlowSerialReadStringUntil('\n').toInt();
}
