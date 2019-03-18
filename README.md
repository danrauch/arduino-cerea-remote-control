# Cerea remote control

[![MIT license](https://img.shields.io/github/license/danrauch/arduino-cerea-remote-control.svg)](https://opensource.org/licenses/MIT)
[![Latest Release](https://img.shields.io/github/release/danrauch/arduino-cerea-remote-control/all.svg)](https://github.com/danrauch/arduino-cerea-remote-control/releases/latest)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/danrauch)

Control and evaluate serial-exposed **Cerea** features comfortably with an Arduino based touchscreen remote control. To see it in action have a look [here](https://www.youtube.com/watch?v=ZrCHwqn8U7Y) (German)!

**Currently supported features**

- Setting MARC, A, B, right, left, AUTO via touchscreen (command: @CEREA)
- Reading @SDOSE command to control a relay (RELAY_PIN_1/2; defaul: via pins 49/53) depending on first boom box section and GPS speed. Can be enabled/disabled via touchscreen (enabled when "Aktiv" and "Streu" are active).
- Control relays also manually via touchscreen ("Streu" button).
- Reading @STATUSAM if availabe to sync UI
- Vibrate on touch; output on adjustable pin (VIBRATION_MOTOR_PIN; default: built-in LED pin of board, which is pin 13 on the MEGA) and with adjustable vibration time (VIBRATION_TIME_MS, default: 200 ms)

**Necessary Hardware**

- [Arduino MEGA 2560](https://store.arduino.cc/mega-2560-r3)
- 3.5" TFT/touchscreen shield (480x320; 16 bit colors) supported by the Adafruit TFTLCD and Touchscreen library (tested: [Kuman version with ILI9486](https://www.amazon.com/Kuman-Arduino-Screen-Tutorials-Mega2560/dp/B075FP83V5/); more can be added)
- *If USB serial connection is used:* capacitor (>22 µF)
- Optional: vibration motor (e.g. [seeed Groove VM](https://www.seeedstudio.com/category/Grove-c-1003/Grove-Vibration-Motor.html))
- Optional: relay(s)  (e.g. [seeed Groove 2CH Relay](https://www.seeedstudio.com/Grove-2-Channel-SPDT-Relay-p-3118.html))

*Important if a USB serial connection is used:* Per default the Arduino resets completely as soon as a serial connection is opened. For some reason in case of the Cerea output the board thinks it is flashed and thus freezes. To avoid this the auto reset must be disabled via a capacitor (>22 µF) between the reset and ground pin.

Riku from the German [Cerea forum](cerea-forum.de) built an excellent housing for the Arduino and was nice enough to provide all necessary resources (German). Have look at the basics [here](https://cerea-forum.de/forum/index.php?thread/440-externe-touchscreen-bedieneinheit-bauanleitung/) and the extension [here](https://cerea-forum.de/forum/index.php?thread/493-externe-touchscreen-bedieneinheit-automatisierte-schaltung/).

**Dependencies**

To use this sketch the following libraries must be downloaded and imported to the Arduino IDE. Either download them directly with the provided links and import them ("Sketch"->"Add .ZIP Library") or search them in the Arduino IDE Library Manager ("Tools"->"Library Manager"). It is recommended to keep the libraries up to date!

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit TFT-LCD Library](https://github.com/adafruit/TFTLCD-Library)
- [Adafruit Touchscreen Library](https://github.com/adafruit/Adafruit_TouchScreen)
- [MCUFRIEND_kbv Library](https://github.com/prenticedavid/MCUFRIEND_kbv)

## About Cerea

[Cerea](https://www.cereagps.com/) is an agricultural GPS auto-steer system for Windows, developed by a Spain-based company called SOLUCIONES TECNOLOGICAS CEREA.

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
| **@CAUDAL;*F*;END** | *F*: flow&nbsp;[float] | Set flow in L/min<br/>(confirmation needed).  | `"@CAUDAL;1.0;END\r\n"`<br/>(set flow to 1.0) |
| **@APLICADO;*A*;END** | *A*: applied&nbsp;flow&nbsp;[float] | Set applied flow in L/ha<br/>(confirmation needed).  | `"@APLICADO;1.0;END\r\n"`<br/>(set applied flow to 1.0) |

## Donation

If this project / information helps you, you can fund a ["Maß"](https://bar.wikipedia.org/wiki/Bia) beer for me [**here**](https://paypal.me/danrauch) :)
