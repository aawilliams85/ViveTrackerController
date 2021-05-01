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
  ActualPID = 0x0000;
  ActualVID = 0x0000;
  Connected = false;
  HIDParserActive = false;
}

void ViveTrackerController::Task() {
  if (hidparser != HIDParserActive) {
    if (HIDParserActive) {
      Serial.println("USB Disconnected");
      ActualPID = 0x0000;
      ActualVID = 0x0000;
      Connected = false;
      HIDParserActive = false;
    }
    else {
      // Get controller information and wait for device
      // to be ready.  I don't know why this delay is
      // required yet -- is there something in the HIDParser
      // that can tell what's going on?  Or is this on the
      // SteamVR side?  If the delay in this block is omitted
      // the device doesn't respond to the mode change command.
      Serial.println("USB Connected:");
      HIDParserActive = true;
      GetControllerInfo();
      Serial.println(ActualVID, HEX);
      Serial.println(ActualPID, HEX);
      if ((ActualVID == ExpectedVID) and (ActualPID == ExpectedPID)) {
        Serial.println("Device matched expected VID and PID");
        delay(2000);
      }
      else {
        Serial.println("Device didn't match expected VID and PID.  Halting");
        return;
      }

      // Change controller to accessory mode so that it can be
      // commanded via additional control transfers.  I don't know
      // why this delay is required yet -- whether that's how long
      // the control transfer takes to execute, or what.  If the delay
      // in this block is omitted the device will miss any subsequent
      // commands until the mode change is complete.
      Serial.println("Changing device to accessory mode");
      SetControllerMode();
      delay(5000);
      Connected = true;
      Serial.println("Device ready for use");
    }
  }
}

void ViveTrackerController::GetControllerInfo() {
  // Capture the USB device information
  ActualVID = hidparser.idVendor();
  ActualPID = hidparser.idProduct();
}

void ViveTrackerController::SetControllerMode() {
  // This command follows the Vive Tracker developer guidelines to
  // change the device into accessory mode.
  //
  // Supposedly -- I should chekc that VID/PID is 28DE/2300 before executing
  // this, and that the interface name should change to "Controller" if
  // successful.  I don't know how to check the latter yet.
  hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthMode, PacketDataMode);
  return;
}

void ViveTrackerController::SetControllerState() {
  // This command follows the Vive Tracker developer guidelines to
  // change the device input state (buttons, trackpad, trigger).
  UpdateState();
  hidparser.sendControlPacket(RequestType, Request, Value, Index, LengthState, PacketDataState);
  return;
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
  // This function emulates a trackpad swipe by "touching"
  // the touchpad and then stepping the X and Y position
  // through multiple positions.  The time parameters are
  // adjustable but the default 250ms swipe time (25ms per step)
  // works well for the scenarios I've tested so far.

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
  // This function emulates a trigger pull by stepping it
  // through multiple positions and setting the button input
  // at the end of travel.  The time parameters are adjustable
  // but the default 250ms swipe time (25ms per step)
  // works well for the scenarios I've tested so far.

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
  // This function emulates a trigger release by stepping it
  // through multiple positions and clearing the button input
  // after travel begins.  The time parameters are adjustable
  // but the default 250ms swipe time (25ms per step)
  // works well for the scenarios I've tested so far.

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
  // This function does some housecleaning to make sure
  // that some reasonable limits are followed for input
  // ranges, and returns altered inputs if required as
  // well as the calculated outputs.  According to the
  // Vive Tracker developer guidelines 10ms is the lower
  // limit for command intervals; I arbitrarily chose 25ms
  // based on it being reasonably fast and the math being simple
  // for 250ms events.  Technically there is no upper limit
  // but I arbitrarily chose 5000ms, which is pretty slow for
  // either action currently using this.
  
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