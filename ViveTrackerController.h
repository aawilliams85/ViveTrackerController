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
  // Object State
  bool Connected = false;
  bool ControllerModeAck = false;
  bool ControllerModeSent = false;
  unsigned long LastUpdateTime = 0;
  void Task(USBHost &host);

  // Raw Actions
  void GetControllerInfo(USBHIDParser &hidparser);
  bool SetControllerMode(USBHIDParser &hidparser);
  bool SetControllerState(USBHIDParser &hidparser);
  void UpdateState();

  // Basic Coordinated Actions
  enum TouchpadSwipeDirection {N, S, E, W, NE, SE, SW, NW};
  void TouchpadSwipe(USBHIDParser &hidparser, TouchpadSwipeDirection dir);
  void TouchpadRelease(USBHIDParser &hidparser);
  void TriggerPull(USBHIDParser &hidparser);
  void TriggerRelease(USBHIDParser &hidparser);

  // Memory   
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
  void Init();

  // 
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
	0x00  // Reserved (LPF? 0=184Hz, 1=5Hz, 2=10Hz, 3=20Hz)
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