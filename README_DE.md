# Cerea Fernbedienung

[![MIT Lizenz](https://img.shields.io/github/license/danrauch/arduino-cerea-remote-control.svg)](https://opensource.org/licenses/MIT)
[![Aktuelles Release](https://img.shields.io/github/release/danrauch/arduino-cerea-remote-control/all.svg)](https://github.com/danrauch/arduino-cerea-remote-control/releases/latest)
[![Spende](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/danrauch)

Sprache: [EN](README.md) | [**DE**](README_DE.md)

Mit dieser Arduino-basierten Touchscreen-Fernbedienung kann man **Cerea**-Funktionen, die über die serielle Schnittstelle zur Verfügung stehen, komfortabel steuern. [Hier](https://www.youtube.com/watch?v=ZrCHwqn8U7Y) kann man sie in Aktion sehen!

**Aktuell unterstützte Funktionen**

- Setzen von MARK, A, B, rechts, links, AUTO via Touchscreen (Kommando: @CEREA)
- Zurücklesen des @SDOSE-Kommandos um, abhängig von der ersten Teilbreite und der GPS-Geschwindigkeit ein Relais zu steuern (RELAY_PIN_1/2; Standard: Pins 49/53). Kann über den Touchscreen aktiviert bzw. deaktiviert werden (Aktiv wenn "Aktiv" und "Streu" Tasten gesetzt sind).
- Steuert Relais auch manuell über Touchscreen ("Streu" Taste).
- Zurücklesen von @STATUSAM, falls verfügbar, um GUI synchron zu halten
- Vibrationsfeedback bei Touchscreenbetätigung; Signal kann auf frei wählbaren Digitalpin gelegt werden (VIBRATION_MOTOR_PIN; Standard: eingebaute LED-Pin des Boards, welcher auf dem MEGA Pin 13 entspricht) und die Vibrationsdauer ist einstellbar (VIBRATION_TIME_MS; Standard: 200 ms)

**Nötige Hardware**

- [Arduino MEGA 2560](https://store.arduino.cc/mega-2560-r3)
- 3.5" TFT/Touchscreen Shield (480x320; 16 Bit Farben) unterstützt durch die Adafruit TFTLCD und Touchscreen Bibliotheken (getestet: [Kuman Version mit ILI9486](https://www.amazon.com/Kuman-Arduino-Screen-Tutorials-Mega2560/dp/B075FP83V5/); weitere können ergänzt werden)
- *Bei USB-basierter seriellen Kommunikation:* Kondensator (>22 µF)
- Optional: Vibrationsmotor (z.B. [seeed Groove VM](https://www.seeedstudio.com/category/Grove-c-1003/Grove-Vibration-Motor.html))
- Optional: Relais (z.B. [seeed Groove 2CH Relay](https://www.seeedstudio.com/Grove-2-Channel-SPDT-Relay-p-3118.html))

*Wichtig bei USB-basierter seriellen Kommunikation:* Standardmäßig setzt sich der Arduino bei der Initialisierung der seriellen Kommunikation komplett zurück. Im Falle der Cerea-Kommunikation denkt der Arduino aus irgendeinem Grund er wird neu geflasht und friert dadurch ein. Um diesen Auto-Reset zu Verhindern muss ein Kondensator zwischen dem "Reset" und dem "GND" Pin angeschlossen werden.

Riku vom deutschen [Cerea forum](http://cerea-forum.de/) hat ein ausgezeichnetes Gehäuse für den Arduino konstruiert und die Daten netterweise zur Verfügung gestellt. Die Basisanleitung kann man [hier](https://cerea-forum.de/forum/index.php?thread/440-externe-touchscreen-bedieneinheit-bauanleitung/) finden und die Erweiterung [hier](https://cerea-forum.de/forum/index.php?thread/493-externe-touchscreen-bedieneinheit-automatisierte-schaltung/).

**Abhängigkeiten**

Um diesen Sketch zu verwenden müssen die folgenden Bibliotheken heruntergeladen und in die Arduino IDE importiert werden. Entweder kann man diese direkt herunterladen und mittels ("Sketch"->"Add .ZIP Library") importieren oder man sucht sie im Arduino IDE Library Manager ("Tools"->"Library Manager"). Es wird empfohlen die Bibliotheken aktuell zu halten.

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)  <-- *Wichtig: Version 1.5.3 nutzen*
- [Adafruit TFT-LCD Library](https://github.com/adafruit/TFTLCD-Library)
- [Adafruit Touchscreen Library](https://github.com/adafruit/Adafruit_TouchScreen)
- [MCUFRIEND_kbv Library](https://github.com/prenticedavid/MCUFRIEND_kbv)

## Über Cerea

[Cerea](https://www.cereagps.com/) ist ein GPS-basiertes Autolenksystem für die Landwirtschaft. Es wird von einer spanischen Firma namens SOLUCIONES TECNOLOGICAS CEREA aktiv für Windows entwickelt.

Das System nutzt die serielle Schnittstelle um Kommandosequenzen auszutauschen, welche es ermöglichen dieses mit einem beliebigen Gerät mit serieller Schnittstelle fernzusteuern. Die folgenden gelisteten Kommandos wurden Reverse Engineered, daher keine Garantie!

Generelles:

- "1" repräsentiert an/true
- "0" repräsentiert aus/false
- Jedes Kommando wird mit einem CR-LF (`\r\n`) beendet

### **Cerea > Fernbedienung**

Kommandos scheinen im 0,5 Sekundentakt anzukommen (Bestätigung benötigt).

| Cerea Kommando                               | Funktion                             | Beschreibung                     | Beispiel                |
|------------------------------------------------|-----------------------------------------------|---------------------------------|------------------------|
| **@STATUSAM;*A*;*M*;END**                     | *A*:&nbsp;AUTO&nbsp;[1/0]<br/>*M*:&nbsp;MARK&nbsp;[1/0] | Aktueller Status MARK und AUTO. | `"@STATUSAM;1;1;END\r\n"`<br/>(AUTO und MARK aktiv) |
| **@CEREA;*VGPS*;*-1*;<br/>*S0*;*S1*;...;*S9*;END** | *VGPS*:&nbsp;GPS&nbsp;Geschw. in km/h&nbsp;[float]<br/>*-1*: reserviert<br/>*S0&#8209;9*:&nbsp;Teilbreiten&nbsp;[1/0] | Aktuelle GPS Geschw. und Teilbreitenstatus (von links nach rechts).<br/>Anzahl an übertragenen Teilbreiten entspricht Einstellung in Cerea (max. 10).  | `"@CEREA;5.0;-1;1;END\r\n"`<br/>(GPS Geschw. 5.0 km/h; Teilbreite 1 aktiv) |
| **@HIDRAU;*F*;END**                            | *F*:&nbsp;Flag&nbsp;[1/0]                 | Falls Sektion schon bearbeitet wurde, wird dies hier signalisiert. | `"@HIDRAU;1;END\r\n"`<br/>(Sektion schon bearbeitet) |

### **Fernbedienung > Cerea**

| Cerea Kommando                                  | Funktion                                      | Beschreibung                                | Beispiel                |
|----------------------------------------------- |-----------------------------------------------|------------------------------------------- |------------------------|
| **@SDOSE;*M*;*C*;*U*;*U*;<br/>*AP*;*BP*;*A*;*L*;*R*;*TL*;*TR*;END** | *M*:&nbsp;MARK&nbsp;[1/0]<br/>*C*:&nbsp;CONTOUR&nbsp;[1/0]<br/>*U*: UNBEKANNT&nbsp;[1/0]<br/>*AP*: A point&nbsp;[1/0]<br/>*BP*:&nbsp;B point&nbsp;[1/0]<br/>*A*:&nbsp;AUTO&nbsp;[1/0]<br/>*L*: LINKS&nbsp;[1/0]<br/>*R*: RECHTS&nbsp;[1/0]<br/>*TL*:&nbsp;WENDE LINKS&nbsp;[1/0]<br/>*TR*:&nbsp;WENDE&nbsp;RECHTS&nbsp;[1/0] | Steuert Cerea. | `"@SDOSE;1;0;0;0; \ 0;0;1;0;0;0;0;END\r\n"`<br/>(aktiviere MARK und AUTO) |
| **@MARK** | - | Aktiviert MARK. | `"@MARK\r\n"` (aktiviert MARK) |
| **@AUTO** | - | Aktiviert AUTO. | `"@AUTO\r\n"` (aktiviert MARK) |
| **@BOOMBOX;<br/>*S0*;*S1*;...;*S9*;END** | *S0&#8209;9*: Teilbreiten&nbsp;[1/0] | Setzt Teilbreitenstatus (von links nach rechts).<br/>Der Status von allen 10 Teilbreiten muss übertragen werden. | `"@BOOMBOX;  \ 0;0;0;0;0;0;0;0;0;0;\r\n"`<br/>(deaktiviere alle Teilbreiten) |
| **@PRESION;*P*;END** | *P*: Druck&nbsp;[float] | Setzte Druck in ?<br/>(Bestätigung benötigt).  | `"@PRESION;1.0;END\r\n"`<br/>(setze Druck auf 1.0) |
| **@CAUDAL;*F*;END** | *F*: Durchflussmenge&nbsp;[float] | Setzte Durchflussmenge in L/min<br/>(Bestätigung benötigt).  | `"@CAUDAL;1.0;END\r\n"`<br/>(setze Durchflussmenge auf 1.0) |
| **@APLICADO;*A*;END** | *A*: Angew.&nbsp;Durchfluss&nbsp;[float] | Setzte angew. Durchflussmenge in L/ha<br/>(Bestätigung benötigt).  | `"@APLICADO;1.0;END\r\n"`<br/>(setze angew. Durchflussmenge auf 1.0) |

## Spende

Falls dieses Projekt / die Info hilfreich war, kann man mich [**hier**](https://paypal.me/danrauch) mit einer ["Maß"](https://bar.wikipedia.org/wiki/Bia) Bier unterstützten :)
