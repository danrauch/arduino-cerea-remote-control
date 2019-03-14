# Cerea remote control

Control some Cerea features comfortably with an Arduino based touchscreen remote control.

Currently supported features are: MARC, A, B, right, left, AUTO

Necessary Hardware:

- Arduino MEGA 2560 with capacitor (>22 µF) between reset and ground
- TFT/touchscreen shield supported by the Adafruit TFTLCD and Touchscreen library
- Optional: vibration motor

**Important:** Per default the Arduino resets completely as soon as a serial connection is opened. For some reason in case of the Cerea output the board thinks it is flashed and thus freezes. To avoid this the auto reset must be disabled via a capacitor (>22 µF) between the reset and ground pin.

## About Cerea

[Cerea](https://www.cereagps.com/) is a Spain-based agricultural GPS auto-steer system for Windows.

It uses serial communication to exchange command sequences, which allow to remote control the system from any device with a serial interface. The following commands were reverse engineered, so no guarantee!

### Cerea -> Remote

**@STATUSAM;X;Y;END** - info about the current state of AUTO (X: 1/0) and MARC (Y: 1/0)

**@CEREA;X;-1;0;0;0;0;0;0;0;0;0;0;END** - current speed (X) and state of up to 10 sections
