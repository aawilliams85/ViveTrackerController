// Based on the following sources:
// Peter S Hollander - Vive Tracker HID reports http://www.talariavr.com/blog/vive-tracker-initial-documentation/
// Matzman666 - Arduino Due USB Host example https://github.com/matzman666/USBHost
// SkyprtX - Teensy USB Host example https://forum.pjrc.com/threads/58460-USBHost_t36-Buffer-parameter-handling

#include <Arduino.h>
#include <USBHost_t36.h>
#include "ViveTrackerController.h"

static void println(const char *title, uint32_t val, uint8_t b = DEC) {
	Serial.print(title);
	Serial.println(val, b);	
}

static void println(const char *title) {
	Serial.println(title);
}

static void println(uint8_t b = DEC) {
	Serial.println(b);
}

static void print(const char *title, uint32_t val, uint8_t b = DEC) {
	Serial.print(title);
	Serial.print(val, b);	
}

void ViveTrackerController::Init() {
  Connected = false;
}

void ViveTrackerController::Task(USBHost &host) {
 /* uint32_t devices;
  uint32_t pipes;
  uint32_t transfers;
  uint32_t strs;
  host.countFree(devices, pipes, transfers, strs);
  if (transfers == 4) {
	  Connected = true;
  }
  else
  {
	  Connected = false;
  }*/
}

void ViveTrackerController::GetControllerInfo(USBHIDParser &hidparser) {
  println("*** HID Device %x:%x connected ***\n", hidparser.idVendor(), hidparser.idProduct());
}

bool ViveTrackerController::SetControllerMode(USBHIDParser &hidparser) {
  println("Setting controller mode to accessory");
  return hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthMode, PacketDataMode);
}

bool ViveTrackerController::SetControllerState(USBHIDParser &hidparser) {
  println("Setting controller state");
  return hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthState, PacketDataState);
}

void ViveTrackerController::UpdateState() {
  // Pack button states into single byte
  PacketDataState[3] = (ButtonStateTrigger) | (ButtonStateGrip << 1) | (ButtonStateMenu << 2) | (ButtonStateSystem << 3) | (ButtonStatePadClick << 4) | (ButtonStatePadTouch << 5);

  // Unpack analog states into MSB and LSB
  PacketDataState[4] = 0;
  PacketDataState[5] = 0;
  PacketDataState[6] = 0;
  PacketDataState[7] = 0;
  PacketDataState[8] = 0;
  PacketDataState[9] = 0;
  
  //Last Update Time
  LastUpdateTime = millis();
}