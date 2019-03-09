/*
 * cerea remote control
 *
 * MIT License
 *
 * Copyright © 2019 Daniel Rauch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// ### Macros ###

// The control pins for the LCD can be assigned to any digital or analog pins,
// but usage of analog pins allows to double up the pins with the touch screen.
#define LCD_CS A3    // Chip Select goes to Analog 3
#define LCD_CD A2    // Command/Data goes to Analog 2
#define LCD_WR A1    // LCD Write goes to Analog 1
#define LCD_RD A0    // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Touchscreen pins
#define YP A2 // must be an analog pin, use "An" notation!
#define XM A3 // must be an analog pin, use "An" notation!
#define YM 8  // can be a digital pin
#define XP 9  // can be a digital pin

// Vibration pin
#define VP 13 // change according to pin connected to motor (default: 13 to activate LED)

// relay pins
#define RELAY_PIN_1 53
#define RELAY_PIN_2 49

// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define NAVY 0x000F

// UI details
#define BUTTON_X 1
#define BUTTON_Y 10
#define BUTTON_W 150
#define BUTTON_H 90
#define BUTTON_SPACING_X 19
#define BUTTON_SPACING_Y 34
#define BUTTON_TEXTSIZE 4

// Touchscreen calibration (values might vary for different touchscreens)
/*
  Touchscreen x/y (USB top):
  95/890   95/150
      ________
     |        |
     |        |
     |        |
     |        |
     |        |
     |________|
  950/885   950/150

  Touchscreen x/y (USB bottom):
  950/150    950/890
      ________
     |        |
     |        |
     |        |
     |        |
     |        |
     |________|
  95/150   95/890

  recalculate to LCD:
  0/0        0/width
      ________
     |        |
     |        |
     |        |
     |        |
     |        |
     |________|
  0/height   width/height

*/

#define TS_MINX 95
#define TS_MAXX 950

#define TS_MINY 150
#define TS_MAXY 890

#define MIN_PRESSURE 10
#define MAX_PRESSURE 1000

// ### Global variables ###

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button buttons[8];
char buttonlabels[8][7] = {"A", "B", "links", "rechts", "On", "Off", "MARK", "AUTO"};
uint16_t buttoncolors[8] = {NAVY, NAVY, ORANGE, ORANGE, RED, MAGENTA, BLUE, GREEN};

// command buffer (TODO: may be easier to read if a struct is used, but more memory is used due to padding)
uint8_t commands[11] = {0};
char command_string[33] = {0};

// CEREA input vars
String cmd;
char next_char;
float gpsspeed;

void setup(void)
{
    Serial.begin(9600);

    init_remote_control();

    // init relay pins
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
}

void init_remote_control()
{
    Serial.println(F("TFT LCD test"));
    tft.reset();

    // additional LCDs can be added here
    uint16_t identifier = tft.readID();
    if (identifier == 0x9486) {
        Serial.println(F("Found 0x9486 LCD driver"));
    } else {
        Serial.println(F("ERROR: LCD driver chip not compatible"));
    }

    tft.begin(identifier);
    Serial.print("TFT size is ");
    Serial.print(tft.width());
    Serial.print("x");
    Serial.println(tft.height());

    tft.setRotation(2);
    tft.fillScreen(BLACK);

    // create buttons
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 2; col++) {
            buttons[col + row * 2].initButtonUL(&tft, BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X),
                                                BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y), BUTTON_W, BUTTON_H,
                                                YELLOW, // outline
                                                buttoncolors[col + row * 2], WHITE, buttonlabels[col + row * 2],
                                                BUTTON_TEXTSIZE); // text
            buttons[col + row * 2].drawButton();
        }
    }
}

void loop(void)
{
    // empty string and read from serial port
    cmd = "";
    read_serial();
    if (cmd.startsWith("@CEREA;")) {    // Wenn Cerea entdeckt wird mit ausführung starten
        cerea();
    }

    TSPoint lcd_point;
    lcd_point.x = 0;
    lcd_point.y = 0;
    lcd_point.z = 0;

    TSPoint touch_point = ts.getPoint();

    // XM and YP are shared with LCD and automatically set to INPUT by ts.getPoint();
    // change back to OUTPUT for the LCD to work
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (touch_point.z > MIN_PRESSURE && touch_point.z < MAX_PRESSURE) {
        lcd_point.y = map(touch_point.x, TS_MINX, TS_MAXX, tft.height(), 0);
        lcd_point.x = map(touch_point.y, TS_MINY, TS_MAXY, 0, tft.width());
    }

    for (uint8_t b = 0; b < 8; b++) {
        if (buttons[b].contains(lcd_point.x, lcd_point.y)) {
            buttons[b].press(true); // tell the button it is pressed
        } else {
            buttons[b].press(false); // tell the button it is NOT pressed
        }
    }

    for (uint8_t b = 0; b < 8; b++) {
        if (buttons[b].justPressed()) {
            digitalWrite(VP, HIGH); // activate the vibration pin if a button is pressed
            if (b < 4) {
                buttons[b].drawButton(true);
            }

            // possible CEREA commands:
            // marc, contur, A, B, auto, left, right, turn left, turn right
            // e.g. activate marc:
            // Serial.println ("@SDOSE;1;0;0;0;0;0;0;0;0;0;0;END");

            switch (b) {
                case 0: commands[3] = 1; break; // A
                case 1: commands[4] = 1; break; // B
                case 2: commands[6] = 1; break; // left
                case 3: commands[7] = 1; break; // right
                case 4:
                    if (commands[2] == 0) { // relay automatic active
                        
                        commands[2] = 1;
                        buttons[b].drawButton(true);
                    } else {
                        
                        commands[2] = 0;
                        buttons[b].drawButton();
                    }
                    break;
                case 5:
                    if (commands[1] == 0) { // manual relay control                        
                        commands[1] = 1;
                        buttons[b].drawButton(true);                        
                        commands[0] = 1;  // enable marc with relay
                        buttons[6].drawButton(true);
                        digitalWrite(RELAY_PIN_1, HIGH);
                        digitalWrite(RELAY_PIN_2, HIGH);
                    } else {                        
                        commands[1] = 0;
                        buttons[b].drawButton();
                        commands[0] = 0;  // disable marc with relay
                        buttons[6].drawButton();
                        digitalWrite(RELAY_PIN_1, LOW);
                        digitalWrite(RELAY_PIN_2, LOW);
                    }
                    break;
                case 6:
                    if (commands[0] == 0) { // marc
                        commands[0] = 1;
                        buttons[b].drawButton(true);
                    } else {
                        commands[0] = 0;
                        buttons[b].drawButton();
                    }
                    break;
                case 7:
                    if (commands[5] == 0) { // AUTO
                        commands[5] = 1;
                        commands[0] = 1; // enable marc with auto
                        buttons[6].drawButton(true);
                        buttons[b].drawButton(true);
                    } else {
                        commands[5] = 0;
                        commands[0] = 0; // disable marc with auto
                        buttons[b].drawButton();
                        buttons[6].drawButton();
                    }
                    break;
                default: break;
            }

            // build and send command string
            sprintf(command_string, "@SDOSE;%d;0;0;0;%d;%d;%d;%d;%d;0;0;END", commands[0], commands[3], commands[4],
                                                                              commands[5], commands[6], commands[7]);
            Serial.println(command_string);

            // reset non-toggle buttons
            commands[3] = 0;
            commands[4] = 0;
            commands[6] = 0;
            commands[7] = 0;
        }
        if (buttons[b].justReleased()) {
            if (b < 4) {
                buttons[b].drawButton();
            }
        }
    }

    delay(40); // UI debouncing

    digitalWrite(VP, LOW);
}

// read from serial interface until EOL
void read_serial()
{
    do {
        if (Serial.available() > 0) {
            next_char = Serial.read();
            if (next_char >= 32) {
                cmd += next_char;
            }
        }
    } while (next_char != 10);
}

void cerea()
{

    // ### String mit Teilbreiten extrahieren ###

    // @Cerea; entferenen
    cmd.remove(0, 7);

    //GPS Geschwindigkeit auslesen
    gpsspeed = cmd.toFloat();

    // Nach erstem ; suchen
    int first_semicolon = cmd.indexOf(';');

    // Nach zweiten ; suchen
    int second_semicolon = cmd.indexOf(';', first_semicolon + 1);

    // Geschwindigkeit und -1 entfernen
    cmd.remove(0, second_semicolon + 1);

    // Suche nach "END" (signalisiert Kommandoende)
    int command_end = cmd.indexOf('END');

    // Anzahl teilbreiten ermitteln
    int anzahl_teilbreiten = (command_end - 2) / 2;

    // Teilbreiten in einen neuen String
    String teilbreite = cmd.substring(0, command_end - 3);

    // ### Servos anhand von extrahierten String steuern ###

    //Geschwindigkeitsabfrage

    if (gpsspeed >= 2.5)
    {
        // Schleife über alle gefundenen Teilbreiten
        for (int i = 0; i < anzahl_teilbreiten * 2; i = i + 2)
        {
            if (teilbreite.substring(i, i + 1) == "1")
            {
                // Teilbreite einschalten
                motor[i / 2].write(WINKEL_TEILBREITE_EIN);
            }
            if (teilbreite.substring(i, i + 1) == "0")
            {
                // Teilbreite ausschalten
                motor[i / 2].write(WINKEL_TEILBREITE_AUS);
            }
        }
    }
    else
    {
        for (int i = 0; i < anzahl_teilbreiten * 2; i = i + 2)
        {
            // Teilbreite ausschalten
            motor[i / 2].write(WINKEL_TEILBREITE_AUS);
        }
    }

    /*
  // Nur nötig falls mehr als 5 Teilbreiten vorhanden (dann auch mehr Servos nötig)
  for (i = anzahl_teilbreiten + 1 ; i < 8; i++) { // restlichen teilbreiten aus
    digitalWrite(i + 1, HIGH);
  }
  */
}
