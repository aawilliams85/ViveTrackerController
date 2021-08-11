// Example project for Vive Tracker USB control via the Teensy USBHost_t36 library
//
// Based on the following sources:
// Peter S Hollander - Vive Tracker HID reports http://www.talariavr.com/blog/vive-tracker-initial-documentation/
// Matzman666 - Arduino Due USB Host example https://github.com/matzman666/USBHost
// SkyprtX - Teensy USB Host example https://forum.pjrc.com/threads/58460-USBHost_t36-Buffer-parameter-handling

#include <USBHost_t36.h>
#include <ViveTrackerController.h>

// Variables
USBHost usb1;
USBHIDParser hid1(usb1);
ViveTrackerController tracker1(hid1);

void setup()
{
  //Configure USB host
  Serial.println("\n\nUSB HID Vive Tracker Control");
  usb1.begin();
}


void loop()
{
  // USB Host Tasks
  usb1.Task();
  tracker1.Task();

  if (tracker1.Connected) {
    // Simple example -- while the tracker is connected and in accessory mode, cycle through some I/O conditions.
    // See the ViveTrackerController library header for all available I/O definitions.
    tracker1.TriggerPull();
    delay(250);
    tracker1.TriggerRelease();
    delay(250);
    tracker1.ButtonStatePadTouch = true;
    tracker1.SetControllerState();
    delay(250);
    tracker1.ButtonStatePadTouch = false;
    tracker1.SetControllerState();
    delay(250);
  }
  delay(50);
}
