// Library files for Vive Tracker USB control via the Teensy USBHost_t36 library
//
// Based on the following sources:
// Peter S Hollander - Vive Tracker HID reports http://www.talariavr.com/blog/vive-tracker-initial-documentation/
// Matzman666 - Arduino Due USB Host example https://github.com/matzman666/USBHost
// SkyprtX - Teensy USB Host example https://forum.pjrc.com/threads/58460-USBHost_t36-Buffer-parameter-handling

#include <Arduino.h>
#include <USBHost_t36.h>
#include "ViveTrackerController.h"

void ViveTrackerController::Init() {
  Connected = false;
  ControllerModeAck = false;
  ControllerModeSent = false;
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
}

bool ViveTrackerController::SetControllerMode(USBHIDParser &hidparser) {
  return hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthMode, PacketDataMode);
}

bool ViveTrackerController::SetControllerState(USBHIDParser &hidparser) {
  UpdateState();
  return hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthState, PacketDataState);
}

void ViveTrackerController::UpdateState() {
  // Pack button states into single byte
  PacketDataState[3] = (ButtonStateTrigger) | (ButtonStateGrip << 1) | (ButtonStateMenu << 2) | (ButtonStateSystem << 3) | (ButtonStatePadClick << 4) | (ButtonStatePadTouch << 5);

  // Unpack analog states into MSB and LSB
  PacketDataState[4] = lowByte(AnalogStatePadX);
  PacketDataState[5] = highByte(AnalogStatePadX);
  PacketDataState[6] = lowByte(AnalogStatePadY);
  PacketDataState[7] = highByte(AnalogStatePadY);
  PacketDataState[8] = lowByte(AnalogStateTrigger);
  PacketDataState[9] = highByte(AnalogStateTrigger);
    
  //  Last Update Time
  LastUpdateTime = millis();
}

void ViveTrackerController::TouchpadSwipe(USBHIDParser &hidparser, TouchpadSwipeDirection dir) {
  // TODO -- Add all standard directions, parameterize step size and delay, add option for randomness?

  switch (dir) {
    case N:
	  // TODO
	  break;
    case S:
	  // Set initial position
      ButtonStatePadTouch = true;
	  AnalogStatePadX = 0;
	  AnalogStatePadY = 32767;
	  SetControllerState(hidparser);

	  // Step through path to final position
      for (int i = 0; i <= 9; i++) {
		delay(25);
        AnalogStatePadY = AnalogStatePadY - 6553;
		SetControllerState(hidparser);
	  }
	  break;
	default:
	  // TODO
	  break;
  }
}

void ViveTrackerController::TouchpadRelease(USBHIDParser &hidparser) {
  ButtonStatePadClick = false;
  ButtonStatePadTouch = false;
  AnalogStatePadX = 0;
  AnalogStatePadY = 0;
  SetControllerState(hidparser);
}

void ViveTrackerController::TriggerPull(USBHIDParser &hidparser) {
  // TODO -- Parameterize step size and delay, add option for randomness?

  //Set initial position
  ButtonStateTrigger = false;
  AnalogStateTrigger = 0;
  SetControllerState(hidparser);

  // Step through path to final position
  for (int i = 0; i <= 9; i++) {
	delay(25);
	AnalogStateTrigger = AnalogStateTrigger + 6553;
    if (i >= 9) {
	  ButtonStateTrigger = true;
	}
	SetControllerState(hidparser);
  }  
}

void ViveTrackerController::TriggerRelease(USBHIDParser &hidparser) {
  ButtonStateTrigger = false;
  AnalogStateTrigger = 0;
  SetControllerState(hidparser);
}