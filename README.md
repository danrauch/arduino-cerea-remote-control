# Cerea remote control

Control some Cerea features comfortably with an Arduino based touchscreen remote control.

Currently supported features are: MARC, A, B, right, left, AUTO

Necessary Hardware:

- Arduino MEGA 2560
- TFT/touchscreen shield supported by the Adafruit TFTLCD and Touchscreen library
- Optional: vibration motor

## About Cerea

[Cerea](https://www.cereagps.com/) is a Spain-based agricultural GPS auto-steer system for Windows.

It uses serial communication to exchange command sequences, which allow to remote control the system from any device with a serial interface. The following commands were reverse engineered, so no guarantee!

### Cerea -> Remote

**@STATUSAM;X;Y;END** - info about the current state of AUTO (X: 1/0) and MARC (Y: 1/0)

**@CEREA;X;-1;0;0;0;0;0;0;0;0;0;0;END** - current speed (X) and state of up to 10 sections
