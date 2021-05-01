#ifndef __ViveTrackerController_h_
#define __ViveTrackerController_h_

// Based on the following sources:
// Peter S Hollander - Vive Tracker HID reports http://www.talariavr.com/blog/vive-tracker-initial-documentation/
// Matzman666 - Arduino Due USB Host example https://github.com/matzman666/USBHost
// SkyprtX - Teensy USB Host example https://forum.pjrc.com/threads/58460-USBHost_t36-Buffer-parameter-handling

#include <Arduino.h>
#include <USBHost_t36.h>

class ViveTrackerController {
public:
  bool Connected;
  bool ControllerModeAck;
  bool ControllerModeSent;
  unsigned long LastUpdateTime;
  void Task(USBHost &host);

  void GetControllerInfo(USBHIDParser &hidparser);
  bool SetControllerMode(USBHIDParser &hidparser);
  bool SetControllerState(USBHIDParser &hidparser);
  void UpdateState();

  enum TouchpadSwipeDirection {N, S, E, W, NE, SE, SW, NW};
  void TouchpadSwipe(USBHIDParser &hidparser, TouchpadSwipeDirection dir);
  void TouchpadRelease(USBHIDParser &hidparser);
  
  void TriggerPull(USBHIDParser &hidparser);
  void TriggerRelease(USBHIDParser &hidparser);
  
  bool ButtonStateTrigger;
  bool ButtonStateGrip;
  bool ButtonStateMenu;
  bool ButtonStateSystem;
  bool ButtonStatePadClick;
  bool ButtonStatePadTouch;
  int16_t AnalogStatePadX;
  int16_t AnalogStatePadY;
  uint16_t AnalogStateTrigger;
private:
  void Init();
  
  const static uint32_t RequestType = 0x21;
  const static uint32_t Request = 0x09;
  const static uint32_t Value = 0x0300;
  const static uint32_t Index = 2;
  const static uint32_t LengthMode = 6;
  const static uint32_t LengthState = 12;
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