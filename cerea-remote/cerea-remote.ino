/*
 * Cerea remote control
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
#define VP LED_BUILTIN  // change according to pin connected to motor (default: activate LED)

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
#define BUTTON_COUNT 8
#define BUTTON_X 1
#define BUTTON_Y 10
#define BUTTON_W 150
#define BUTTON_H 90
#define BUTTON_SPACING_X 19
#define BUTTON_SPACING_Y 34
#define BUTTON_TEXTSIZE 4

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 3
#define BUTTON_RELAY_AUTO 4
#define BUTTON_RELAY_MANUAL 5
#define BUTTON_MARC 6
#define BUTTON_CEREA_AUTO 7

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

#define MIN_GPS_SPEED 2.5

// ### Global variables ###

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button buttons[BUTTON_COUNT];
char buttonlabels[BUTTON_COUNT][7] = {"A", "B", "links", "rechts", "Aktiv", "Streue", "MARK", "AUTO"};
uint16_t buttoncolors[BUTTON_COUNT] = {NAVY, NAVY, ORANGE, ORANGE, RED, MAGENTA, BLUE, GREEN};

// commands struct
struct {
    bool marc;
    bool contour;
    bool A;
    bool B;
    bool auto_on;
    bool left;
    bool right;
    bool turn_left;
    bool turn_right;
} cerea_commands;

struct {
    bool automatic;
    bool manual_override;
} relay_control;

bool relays_marc_on = false;

char command_string[33] = {0};

// CEREA input vars
String cmd;
char next_char;

void setup(void)
{
    Serial.begin(9600);
    cmd = "";

    pinMode(VP, OUTPUT);  

    init_remote_control();

    // init relay pins
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
}

void init_remote_control()
{
    memset(&cerea_commands, 0, sizeof(cerea_commands));
    memset(&relay_control, 0, sizeof(relay_control));
    
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
    for (uint8_t row = 0; row < BUTTON_COUNT / 2; row++) {
        for (uint8_t col = 0; col < BUTTON_COUNT / 4; col++) {
            buttons[col + row * 2].initButtonUL(&tft, BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X),
                                                BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y),
                                                BUTTON_W, BUTTON_H,
                                                YELLOW, // outline
                                                buttoncolors[col + row * 2], WHITE, buttonlabels[col + row * 2],
                                                BUTTON_TEXTSIZE); // text
            buttons[col + row * 2].drawButton();
        }
    }
}

void loop(void)
{
    // if read_serial did reach end, eval string
    if (read_serial()) {
        if (cmd.startsWith("@CEREA;")) {
            evaluate_cerea_string();
        }
        cmd = "";
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

    for (uint8_t b = 0; b < BUTTON_COUNT; b++) {
        if (buttons[b].contains(lcd_point.x, lcd_point.y)) {
            buttons[b].press(true); // tell the button it is pressed
        } else {
            buttons[b].press(false); // tell the button it is NOT pressed
        }
    }

    for (uint8_t b = 0; b < BUTTON_COUNT; b++) {
        if (buttons[b].justPressed()) {
            digitalWrite(VP, HIGH); // activate the vibration pin if a button is pressed
            if (b < 4) {
                buttons[b].drawButton(true);
            }

            // possible CEREA commands:
            // marc, contour, New, none, A, B, auto, left, right, turn left, turn right
            // e.g. activate marc:
            // Serial.println ("@SDOSE;1;0;0;0;0;0;0;0;0;0;0;END");

            switch (b) {
                case BUTTON_A: cerea_commands.A = true; break;
                case BUTTON_B: cerea_commands.B = true; break;
                case BUTTON_LEFT: cerea_commands.left = true; break;
                case BUTTON_RIGHT: cerea_commands.right = true; break;
                case BUTTON_RELAY_AUTO:
                    if (!relay_control.automatic) {
                        relay_control.automatic = true;
                        buttons[BUTTON_RELAY_AUTO].drawButton(true);
                    } else {                        
                        relay_control.automatic = false;
                        buttons[BUTTON_RELAY_AUTO].drawButton();
                    }
                    break;
                case BUTTON_RELAY_MANUAL:
                    if (!relay_control.manual_override) {
                        relay_control.manual_override = true;
                        buttons[BUTTON_RELAY_MANUAL].drawButton(true);
                        buttons[BUTTON_MARC].drawButton(true);
                        if (!relay_control.automatic) {
                            control_marc_and_relays(true);
                        }
                    } else {
                        relay_control.manual_override = false;
                        buttons[BUTTON_RELAY_MANUAL].drawButton();
                        buttons[BUTTON_MARC].drawButton();
                        control_marc_and_relays(false);
                    }
                    break;
                case BUTTON_MARC:
                    if (!cerea_commands.marc) {
                        cerea_commands.marc = true;
                        buttons[BUTTON_MARC].drawButton(true);
                    } else {
                        cerea_commands.marc = false;
                        buttons[BUTTON_MARC].drawButton();
                    }
                    break;
                case BUTTON_CEREA_AUTO:
                    if (!cerea_commands.auto_on) {
                        cerea_commands.auto_on = true;
                        cerea_commands.marc = true; // enable marc with auto
                        buttons[BUTTON_MARC].drawButton(true);
                        buttons[BUTTON_CEREA_AUTO].drawButton(true);
                    } else {
                        cerea_commands.auto_on = false;
                        cerea_commands.marc = false; // disable marc with auto
                        buttons[BUTTON_MARC].drawButton();
                        buttons[BUTTON_CEREA_AUTO].drawButton();
                    }
                    break;
                default: break;
            }

            // no command to CEREA is necessary if button relay auto is pressed
            if (b != BUTTON_RELAY_AUTO) {
                // build and send command string (boolean implicitely casted to decimal 0/1)
                sprintf(command_string, "@SDOSE;%d;0;0;0;%d;%d;%d;%d;%d;0;0;END", cerea_commands.marc,
                                                                                  cerea_commands.A, 
                                                                                  cerea_commands.B,
                                                                                  cerea_commands.auto_on,
                                                                                  cerea_commands.left,
                                                                                  cerea_commands.right);
                Serial.println(command_string);
            }

            // reset non-toggle buttons
            cerea_commands.A = false;  
            cerea_commands.B = false;
            cerea_commands.left = false;
            cerea_commands.right = false;
        }
        if (buttons[b].justReleased() && b < 4) {
            buttons[b].drawButton();
        }
    }

    // debounce UI
    delay(10);

    digitalWrite(VP, LOW);
}

// read from serial interface
bool read_serial()
{
    while (Serial.available() > 0) {
        next_char = Serial.read();
        if (next_char >= 32) {
            cmd += next_char;
        }
        // if there is a '\n' the whole input is read
        if (next_char == '\n') {
            return true;
        }
    }

    return false;
}

void evaluate_cerea_string()
{    
    // no automatic if manual override is false
    if (!relay_control.manual_override) {
        return;
    }

    // remove @Cerea; and search for ;
    cmd.remove(0, 7);
    int first_semicolon = cmd.indexOf(';');
    int second_semicolon = cmd.indexOf(';', first_semicolon + 1);

    // read GPS speed
    float gps_speed = cmd.substring(0, first_semicolon).toFloat();

    // remove speed and -1 then search for command end
    cmd.remove(0, second_semicolon + 1);
    int command_end = cmd.indexOf("END");

    // get number of partial fields
    int number_partial_fields = (command_end - 2) / 2;
    // at this point only the first part width is getting evaluated
    if (number_partial_fields < 1) {
        return;
    }

    // extract partial field 1
    String partial_fields = cmd.substring(0, command_end - 3);
    int partial_field_1 = partial_fields.substring(0, 1).toInt();

    // activate partial field if vehicle is moving & auto is active 
    bool enable_relay_marc = gps_speed >= MIN_GPS_SPEED && 
                             relay_control.automatic &&
                             partial_field_1 == 1;
    // do nothing if state requested by automatic already set
    if (enable_relay_marc == relays_marc_on &&
        enable_relay_marc == cerea_commands.marc) {
        return;
    }

    control_marc_and_relays(enable_relay_marc);
}

void control_marc_and_relays(bool enable)
{
    relays_marc_on = enable;
    cerea_commands.marc = enable;
    buttons[BUTTON_MARC].drawButton(enable);
    if (enable) {
        digitalWrite(RELAY_PIN_1, HIGH);
        digitalWrite(RELAY_PIN_2, HIGH);
    } else {
        digitalWrite(RELAY_PIN_1, LOW);
        digitalWrite(RELAY_PIN_2, LOW);
    }
}
