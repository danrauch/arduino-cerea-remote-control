# Cerea remote control

[![MIT license](https://img.shields.io/github/license/danrauch/arduino-cerea-remote-control.svg)](https://opensource.org/licenses/MIT)
[![Latest Release](https://img.shields.io/github/release/danrauch/arduino-cerea-remote-control/all.svg)](https://github.com/danrauch/arduino-cerea-remote-control/releases/latest)

Control Cerea features comfortably with an Arduino based touchscreen remote control.

Currently supported features are: MARC, A, B, right, left, AUTO

Necessary Hardware:

- Arduino MEGA 2560
- TFT/touchscreen shield supported by the Adafruit TFTLCD and Touchscreen library (tested ID: `0x9486`; more can be added)
- *If USB serial connection is used:* capacitor (>22 µF)
- *Optional:* vibration motor

**Important if a USB serial connection is used:** Per default the Arduino resets completely as soon as a serial connection is opened. For some reason in case of the Cerea output the board thinks it is flashed and thus freezes. To avoid this the auto reset must be disabled via a capacitor (>22 µF) between the reset and ground pin.

## About Cerea

[Cerea](https://www.cereagps.com/) is a Spain-based agricultural GPS auto-steer system for Windows.

It uses serial communication to exchange command sequences, which allow to remote control the system from any device with a serial interface. The following commands were reverse engineered, so no guarantee!

General information:

- "1" represents on/true
- "0" represents off/false
- every command is followed by a CR-LF (`\r\n`).

### **Cerea > Remote**

Commands seem to arrive in 0.5 second cycles (confirmation needed).

| Cerea command                                  | Controls                                      | Description                     | Example                |
|------------------------------------------------|-----------------------------------------------|---------------------------------|------------------------|
| **@STATUSAM;*A*;*M*;END**                     | *A*:&nbsp;AUTO&nbsp;[1/0]<br/>*M*:&nbsp;MARC&nbsp;[1/0] | Current state of MARC and AUTO. | `"@STATUSAM;1;1;END\r\n"`<br/>(AUTO and MARC active) |
| **@CEREA;*VGPS*;*-1*;<br/>*S0*;*S1*;...;*S9*;END** | *VGPS*:&nbsp;GPS&nbsp;speed in km/h&nbsp;[float]<br/>*-1*: reserved<br/>*S0&#8209;9*:&nbsp;boom sections&nbsp;[1/0] | Current GPS speed and boom box section states (left to right).<br/>Number of transmitted sections matches Cerea settings (max. 10).  | `"@CEREA;5.0;-1;1;END\r\n"`<br/>(GPS speed 5.0 km/h; section 1 active) |
| **@HIDRAU;*F*;END**                            | *F*:&nbsp;Flag&nbsp;[1/0]                 | If section was already taken care of info is sent. | `"@HIDRAU;1;END\r\n"`<br/>(Section already taken care of) |

### **Remote > Cerea**

| Cerea command                                  | Controls                                      | Description                                | Example                |
|----------------------------------------------- |-----------------------------------------------|------------------------------------------- |------------------------|
| **@SDOSE;*M*;*C*;*U*;*U*;<br/>*AP*;*BP*;*A*;*L*;*R*;*TL*;*TR*;END** | *M*:&nbsp;MARC&nbsp;[1/0]<br/>*C*:&nbsp;CONTOUR&nbsp;[1/0]<br/>*U*: UNKNOWN&nbsp;[1/0]<br/>*AP*: A point&nbsp;[1/0]<br/>*BP*:&nbsp;B point&nbsp;[1/0]<br/>*A*:&nbsp;AUTO&nbsp;[1/0]<br/>*L*: LEFT&nbsp;[1/0]<br/>*R*: RIGHT&nbsp;[1/0]<br/>*TL*:&nbsp;TURN LEFT&nbsp;[1/0]<br/>*TR*:&nbsp;TURN&nbsp;RIGHT&nbsp;[1/0] | Controls Cerea. | `"@SDOSE;1;0;0;0; \ 0;0;1;0;0;0;0;END\r\n"`<br/>(activate MARC and AUTO) |
| **@MARC** | - | Activates MARC. | `"@MARC\r\n"` (activate MARC) |
| **@AUTO** | - | Activates AUTO. | `"@AUTO\r\n"` (activate MARC) |
| **@BOOMBOX;<br/>*S0*;*S1*;...;*S9*;END** | *S0&#8209;9*: boom sections&nbsp;[1/0] | Set boom box section states (left to right).<br/>The state of all 10 sections must be transmitted. | `"@BOOMBOX;  \ 0;0;0;0;0;0;0;0;0;0;\r\n"`<br/>(deactivate all sections) |
| **@PRESION;*P*;END** | *P*: pressure&nbsp;[float] | Set pressure in ?<br/>(confirmation needed).  | `"@PRESION;1.0;END\r\n"`<br/>(set pressure to 1.0) |
| **@CAUDAL;*C*;END** | *F*: flow&nbsp;[float] | Set flow in L/min<br/>(confirmation needed).  | `"@CAUDAL;1.0;END\r\n"`<br/>(set flow to 1.0) |
| **@APLICADO;*A*;END** | *A*: applied&nbsp;flow&nbsp;[float] | Set applied flow in L/ha<br/>(confirmation needed).  | `"@APLICADO;1.0;END\r\n"`<br/>(set applied flow to 1.0) |
