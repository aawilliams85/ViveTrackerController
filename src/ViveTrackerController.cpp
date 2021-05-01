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
  HIDParserActive = false;
}

void ViveTrackerController::Task() {
  if (hidparser != HIDParserActive) {
    if (HIDParserActive) {
	  Serial.println("Disconnected");
      Connected = false;
      HIDParserActive = false;
	}
	else {
		Serial.println("Connected: ");
		Serial.println(hidparser.idVendor());
		Serial.println(hidparser.idProduct());

		HIDParserActive = true;
		GetControllerInfo();
		delay(2000);
		Serial.println("Send mode command");
		SetControllerMode();
		delay(5000);
		Connected = true;
		Serial.println("Fin");
	}
  }

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

void ViveTrackerController::GetControllerInfo() {
}

bool ViveTrackerController::SetControllerMode() {
  return hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthMode, PacketDataMode);
}

bool ViveTrackerController::SetControllerState() {
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

void ViveTrackerController::TouchpadSwipe(TouchpadSwipeDirection Direction, uint16_t TotalTime, uint16_t StepTime) {
  // Validate input limits and calculate step size
  uint16_t StepSize = 0;
  uint16_t LoopSize = 0;
  uint16_t LoopUBound = 0;
  CalculateStepSize(TotalTime, StepTime, StepSize, LoopSize, LoopUBound, 65535);

  switch (Direction) {
    case UP:
	  // Set initial position
      ButtonStatePadTouch = true;
	  AnalogStatePadX = 0;
	  AnalogStatePadY = -32767;
	  SetControllerState();

	  // Step through path to final position
      for (int i = 0; i <= LoopUBound; i++) {
    	delay(StepTime);
    	AnalogStatePadY = AnalogStatePadY + StepSize;
    	SetControllerState();
      }
	  
	  // Ensure exact final position is reached
      AnalogStatePadX = 0;
      AnalogStatePadY = 32767;
      SetControllerState();
	  break;
    case DOWN:
	  // Set initial position
      ButtonStatePadTouch = true;
	  AnalogStatePadX = 0;
	  AnalogStatePadY = 32767;
	  SetControllerState();

	  // Step through path to final position
      for (int i = 0; i <= LoopUBound; i++) {
    	delay(StepTime);
    	AnalogStatePadY = AnalogStatePadY - StepSize;
    	SetControllerState();
      }
	  
	  // Ensure exact final position is reached
      AnalogStatePadX = 0;
      AnalogStatePadY = -32767;
      SetControllerState();
	  break;
	case LEFT:
	  // Set initial position
      ButtonStatePadTouch = true;
	  AnalogStatePadX = 32767;
	  AnalogStatePadY = 0;
	  SetControllerState();

	  // Step through path to final position
      for (int i = 0; i <= LoopUBound; i++) {
    	delay(StepTime);
    	AnalogStatePadX = AnalogStatePadX - StepSize;
    	SetControllerState();
      }
	  
	  // Ensure exact final position is reached
      AnalogStatePadX = -32767;
      AnalogStatePadY = 0;
      SetControllerState();
	  break;
	case RIGHT:
	  // Set initial position
      ButtonStatePadTouch = true;
	  AnalogStatePadX = -32767;
	  AnalogStatePadY = 0;
	  SetControllerState();

	  // Step through path to final position
      for (int i = 0; i <= LoopUBound; i++) {
    	delay(StepTime);
    	AnalogStatePadX = AnalogStatePadX + StepSize;
    	SetControllerState();
      }
	  
	  // Ensure exact final position is reached
      AnalogStatePadX = 32767;
      AnalogStatePadY = 0;
      SetControllerState();
	  break;
	default:
	  // TODO
	  break;
  }
}

void ViveTrackerController::TouchpadRelease() {
  // Resets button and analog signals for touchpad
  ButtonStatePadClick = false;
  ButtonStatePadTouch = false;
  AnalogStatePadX = 0;
  AnalogStatePadY = 0;
  SetControllerState();
}

void ViveTrackerController::TriggerPull(uint16_t TotalTime, uint16_t StepTime) {
  // Validate input limits and calculate step size
  uint16_t StepSize = 0;
  uint16_t LoopSize = 0;
  uint16_t LoopUBound = 0;
  CalculateStepSize(TotalTime, StepTime, StepSize, LoopSize, LoopUBound, 65535);

  // Set initial position
  ButtonStateTrigger = false;
  AnalogStateTrigger = 0;
  SetControllerState();

  // Step through path to final position
  for (int i = 0; i <= LoopUBound; i++) {
	delay(StepTime);
	AnalogStateTrigger = AnalogStateTrigger + StepSize;
    if (i >= LoopUBound) {
	  ButtonStateTrigger = true;
	}
	SetControllerState();
  }
  
  // Ensure exact final position is reached
  ButtonStateTrigger = true;
  AnalogStateTrigger = 65535;
  SetControllerState();
}

void ViveTrackerController::TriggerRelease(uint16_t TotalTime, uint16_t StepTime) {  
  // Validate input limits and calculate step size
  uint16_t StepSize = 0;
  uint16_t LoopSize = 0;
  uint16_t LoopUBound = 0;
  CalculateStepSize(TotalTime, StepTime, StepSize, LoopSize, LoopUBound, 65535);

  // Set initial position
  ButtonStateTrigger = true;
  AnalogStateTrigger = 65535;
  SetControllerState();

  // Step through path to final position
  for (int i = 0; i <= LoopUBound; i++) {
	delay(StepTime);
	AnalogStateTrigger = AnalogStateTrigger - StepSize;
    if (i >= 1) {
	  ButtonStateTrigger = false;
	}
	SetControllerState();
  }

  // Ensure exact final position is reached
  ButtonStateTrigger = false;
  AnalogStateTrigger = 0;
  SetControllerState();
}

void ViveTrackerController::CalculateStepSize(uint16_t &TotalTime, uint16_t &StepTime, uint16_t &StepSize, uint16_t &LoopSize, uint16_t &LoopUBound, uint16_t LoopSpan) {
  // Impose a standard set of limits.  All units are in milliseconds.
  if (TotalTime < 25) {
    TotalTime = 25;
  }
  if (TotalTime > 5000) {
    TotalTime = 5000;
  }
  if (StepTime < 25) {
    StepTime = 25;
  }
  if (StepTime > 5000) {
    StepTime = 5000;
  }
  if (StepTime > TotalTime) {
    StepTime = TotalTime;
  }

  // Calculate the number of steps and size of each step
  LoopSize = (TotalTime/StepTime);
  if (LoopSize <= 1) {
    LoopUBound = 1;
	StepSize = LoopSpan;
  }
  else {
    LoopUBound = LoopSize - 1;
    StepSize = LoopSpan / LoopUBound;
  }
  return;
}