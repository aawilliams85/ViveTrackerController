// Library files for Vive Tracker USB control via the Teensy USBHost_t36 library
//
// Based on the following sources:
// Peter S Hollander - Vive Tracker HID reports http://www.talariavr.com/blog/vive-tracker-initial-documentation/
// Matzman666 - Arduino Due USB Host example https://github.com/matzman666/USBHost
// SkyprtX - Teensy USB Host example https://forum.pjrc.com/threads/58460-USBHost_t36-Buffer-parameter-handling

#ifndef __ViveTrackerController_h_
#define __ViveTrackerController_h_

#include <Arduino.h>
#include <USBHost_t36.h>

class ViveTrackerController {
public:
  ViveTrackerController(USBHIDParser &hidparser) : hidparser(hidparser) {}

  // Object State
  bool Connected = false;
  void Task();

  // Raw Actions
  void GetControllerInfo();
  void SetControllerMode();
  void SetControllerState();
  void UpdateState();

  // Basic Coordinated Actions
  enum TouchpadSwipeDirection {UP, DOWN, LEFT, RIGHT};
  void TouchpadSwipe(TouchpadSwipeDirection Direction, uint16_t TotalTime = 250, uint16_t StepTime = 25);
  void TouchpadRelease();
  void TriggerPull(uint16_t TotalTime = 250, uint16_t StepTime = 25);
  void TriggerRelease(uint16_t TotalTime = 250, uint16_t StepTime = 25);

  // Controller State
  bool ButtonStateTrigger = false;
  bool ButtonStateGrip = false;
  bool ButtonStateMenu = false;
  bool ButtonStateSystem = false;
  bool ButtonStatePadClick = false;
  bool ButtonStatePadTouch = false;
  int16_t AnalogStatePadX = 0;
  int16_t AnalogStatePadY = 0;
  uint16_t AnalogStateTrigger = 0;

private:
  USBHIDParser &hidparser;
  bool HIDParserActive = false;
  void Init();
  void CalculateStepSize(uint16_t &TotalTime, uint16_t &StepTime, uint16_t &StepSize, uint16_t &LoopSize, uint16_t &LoopUBound, uint16_t LoopSpan);
  unsigned long LastUpdateTime = 0;

  // USB Identification
  uint16_t ActualVID = 0x0000;
  uint16_t ActualPID = 0x0000;
  static const uint16_t ExpectedVID = 0x28DE;
  static const uint16_t ExpectedPID = 0x2300;

  // Control Transfer Definitions
  static const uint32_t RequestType = 0x21;
  static const uint32_t Request = 0x09;
  static const uint32_t Value = 0x0300;
  static const uint32_t Index = 2;
  static const uint32_t LengthMode = 6;
  static const uint32_t LengthState = 12;
  uint8_t  PacketDataMode[6] = {
    0xB3, // Address B3 is for setting the device mode
	0x03, // Device Mode (1=PC, 2=Phone, 3=Accessory)
	0x03, // Length of remaining registers
	0x00, // Reserved (Charge Enable)
	0x00, // Reserved (OS Type)
	0x00  // Reserved (LPF 0=184Hz, 1=5Hz, 2=10Hz, 3=20Hz)
  };
  uint8_t PacketDataState[12] = {
    0xB4, // Address B4 is for setting the device state
	0x0A, // Length of remaining registers
	0x00, // Data format version for device state
	0x00, // Button state 
          // Bit 0: TRIGGER
		  // Bit 1: BUMPER (GRIP)
		  // Bit 2: MENU
		  // Bit 3: STEAM (SYSTEM)
		  // Bit 4: PAD (TOUCHPAD CLICK)
		  // Bit 5: PAD_FINGERDOWN (TOUCHPAD TOUCH)
		  // Bit 6: RESERVED
		  // Bit 7: RESERVED
	0x00, // Pad X LSB
	0x00, // Pad X MSB
	0x00, // Pad Y LSB
	0x00, // Pad Y MSB
	0x00, // Trigger LSB
	0x00, // Trigger MSB
	0x00, // Reserved (Battery Level LSB)
	0x00  // Reserved (Battery Level MSB)
  };
};

#endif // __ViveTrackerController_h_