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

// Vibration control
// change pin according to pin connected to motor!
// default: use pin of onboard LED to showcase the feature
#define VIBRATION_MOTOR_PIN LED_BUILTIN
#define VIBRATION_TIME_MS 200 

// relay pins
#define RELAY_PIN_1 53
#define RELAY_PIN_2 49

// Cerea settings
#define MIN_GPS_SPEED 2.5
#define MAX_NR_BOOM_SECTIONS 10

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
#define GRAY 0xDEFB

// UI details
#define BUTTON_COUNT 8
#define BUTTON_X 2
#define BUTTON_Y 45
#define BUTTON_W 150
#define BUTTON_H 80
#define BUTTON_SPACING_X 17
#define BUTTON_SPACING_Y 14
#define BUTTON_TEXTSIZE 3

#define NAV_BUTTON_X 110
#define NAV_BUTTON_Y 420
#define NAV_BUTTON_W 100
#define NAV_BUTTON_H 55
#define NAV_BUTTON_TEXTSIZE 2

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 3
#define BUTTON_RELAY_AUTO 4
#define BUTTON_RELAY_MANUAL 5
#define BUTTON_MARC 6
#define BUTTON_CEREA_AUTO 7

#define SECTION_RECT_SIZE 19
#define SECTION_RECT_X 7
#define SECTION_RECT_SPACING_X 32
#define SECTION_RECT_Y 6

#define SEPERATOR_LINE_START_X 1
#define SEPERATOR_LINE_END_X 329
#define SEPERATOR_LINE_Y 32

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
#define MIN_PRESSURE 5
#define MAX_PRESSURE 1000

// ### Global variables ###

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button control_buttons[BUTTON_COUNT];
Adafruit_GFX_Button navigation_button;

char button_labels[BUTTON_COUNT][8] = {"A", "B", "links", "rechts", "Aktiv", "Streuen", "MARK", "AUTO"};
char nav_button_label[2] = ">";
uint16_t button_colors[BUTTON_COUNT] = {NAVY, NAVY, ORANGE, ORANGE, RED, MAGENTA, BLUE, GREEN};

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

typedef enum boom_state {    
    BOOM_INACTIVE,
    BOOM_ACTIVE,
    BOOM_UNUSED
} boom_state_t;

boom_state_t boom_section_states[MAX_NR_BOOM_SECTIONS] = {BOOM_UNUSED};

// output command string
char cerea_command_out[33] = {0};
// Cerea input command buffer
String cerea_command_in = "";

unsigned long start_time_ms = 0;

void setup(void)
{
    Serial.begin(9600);

    for (uint8_t i = 0; i < MAX_NR_BOOM_SECTIONS; i++) {
        tft.fillRect(SECTION_RECT_X + SECTION_RECT_SPACING_X * i, SECTION_RECT_Y,
                     SECTION_RECT_SIZE, SECTION_RECT_SIZE, GRAY);
    }

    tft.drawLine(SEPERATOR_LINE_START_X, SEPERATOR_LINE_Y,
                 SEPERATOR_LINE_END_X, SEPERATOR_LINE_Y, WHITE);  
    
    navigation_button.initButtonUL(&tft, NAV_BUTTON_X, NAV_BUTTON_Y,
                                         NAV_BUTTON_W, NAV_BUTTON_H, WHITE,
                                         BLACK, WHITE, nav_button_label,
                                         NAV_BUTTON_TEXTSIZE);
    navigation_button.drawButton();

    init_remote_control();

    // init pins
    pinMode(VIBRATION_MOTOR_PIN, OUTPUT); 
    digitalWrite(VIBRATION_MOTOR_PIN, LOW);
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

    // create control buttons
    for (uint8_t row = 0; row < BUTTON_COUNT / 2; row++) {
        for (uint8_t col = 0; col < BUTTON_COUNT / 4; col++) {
            control_buttons[col + row * 2].initButtonUL(&tft, BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X),
                                                BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y),
                                                BUTTON_W, BUTTON_H,
                                                YELLOW, // outline
                                                button_colors[col + row * 2], WHITE, button_labels[col + row * 2],
                                                BUTTON_TEXTSIZE); // text
            control_buttons[col + row * 2].drawButton();            
        }
    }
}

void loop(void)
{
    // if read_serial did reach end, eval string
    if (read_serial()) {
        if (cerea_command_in.startsWith("@CEREA;")) {
            evaluate_cerea_string();
        } else if (cerea_command_in.startsWith("@STATUSAM;")) {
            evaluate_status_string();
        }
        cerea_command_in = "";
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
        if (control_buttons[b].contains(lcd_point.x, lcd_point.y)) {
            control_buttons[b].press(true); // tell the button it is pressed
        } else {
            control_buttons[b].press(false); // tell the button it is NOT pressed
        }
    }

    for (uint8_t b = 0; b < BUTTON_COUNT; b++) {
        if (control_buttons[b].justPressed()) {
            // activate the vibration pin if a button is pressed
            digitalWrite(VIBRATION_MOTOR_PIN, HIGH);
            start_time_ms = millis();
            if (b < 4) {
                control_buttons[b].drawButton(true);
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
                    relay_control.automatic = !relay_control.automatic;
                    control_buttons[BUTTON_RELAY_AUTO].drawButton(relay_control.automatic);
                    break;
                case BUTTON_RELAY_MANUAL:
                    relay_control.manual_override = !relay_control.manual_override;
                    control_buttons[BUTTON_RELAY_MANUAL].drawButton(relay_control.manual_override);
                    cerea_commands.marc = relay_control.manual_override;
                    control_buttons[BUTTON_MARC].drawButton(relay_control.manual_override);
                    control_relays(relay_control.manual_override);
                    break;
                case BUTTON_MARC:
                    cerea_commands.marc = !cerea_commands.marc;
                    control_buttons[BUTTON_MARC].drawButton(cerea_commands.marc);
                    break;
                case BUTTON_CEREA_AUTO:
                    cerea_commands.auto_on = !cerea_commands.auto_on;
                    control_buttons[BUTTON_CEREA_AUTO].drawButton(cerea_commands.auto_on);
                    break;
                default: break;
            }

            // no command to CEREA is necessary if button relay auto is pressed
            if (b != BUTTON_RELAY_AUTO) {
                // build and send command string (boolean implicitely casted to decimal 0/1)
                sprintf(cerea_command_out, "@SDOSE;%d;0;0;0;%d;%d;%d;%d;%d;0;0;END", cerea_commands.marc,
                                                                                     cerea_commands.A, 
                                                                                     cerea_commands.B,
                                                                                     cerea_commands.auto_on,
                                                                                     cerea_commands.left,
                                                                                     cerea_commands.right);
                Serial.println(cerea_command_out);
            }

            // reset non-toggle control_buttons
            cerea_commands.A = false;  
            cerea_commands.B = false;
            cerea_commands.left = false;
            cerea_commands.right = false;
        }
        if (control_buttons[b].justReleased() && b < 4) {
            control_buttons[b].drawButton();
        }
    }

    // debounce UI
    delay(10);

    if ((millis() - start_time_ms) > VIBRATION_TIME_MS) {
        digitalWrite(VIBRATION_MOTOR_PIN, LOW);
    }
}

// read from serial interface
bool read_serial()
{
    while (Serial.available() > 0) {
        char next_char = Serial.read();
        if (next_char >= 32) {
            cerea_command_in += next_char;
        }
        // if there is a '\n' the whole input is read
        if (next_char == '\n') {
            return true;
        }
    }

    return false;
}

void evaluate_status_string ()
{
    // remove @STATUSAM;
    cerea_command_in.remove(0, 10);

    // search for index of command end
    int command_end = cerea_command_in.indexOf("END");
    if (command_end < 4) {
        return;
    }

    bool automatic = cerea_command_in.substring(0, 1).toInt();
    bool marc = cerea_command_in.substring(2, 3).toInt();

    cerea_commands.auto_on = automatic;
    control_buttons[BUTTON_CEREA_AUTO].drawButton(automatic);

    cerea_commands.marc = marc;
    control_buttons[BUTTON_MARC].drawButton(marc);
}

void evaluate_cerea_string()
{
    // remove @Cerea; and search for ;
    cerea_command_in.remove(0, 7);
    int first_semicolon = cerea_command_in.indexOf(';');
    int second_semicolon = cerea_command_in.indexOf(';', first_semicolon + 1);

    // read GPS speed in km/h
    float gps_speed = cerea_command_in.substring(0, first_semicolon).toFloat();

    // remove speed and -1 then search for command end
    cerea_command_in.remove(0, second_semicolon + 1);

    // search for index of command end
    int command_end = cerea_command_in.indexOf("END");

    // get number of boom box sections (divide because of ;)
    int number_boom_sections = command_end / 2;

    // abort if no section is detected
    if (number_boom_sections < 1) {
        return;
    }

    // extract boom sections and save states
    String boom_sections = cerea_command_in.substring(0, command_end);

    for (int i = 0; i < MAX_NR_BOOM_SECTIONS; i++) {
        uint16_t color = GRAY;
        boom_state_t boom_section_state = BOOM_UNUSED;
        if (i < number_boom_sections) {
            // result is either 0 or 1 => BOOM_INACTIVE or BOOM_ACTIVE
            boom_section_state = static_cast<boom_state_t>(boom_sections.substring(i*2, i*2 + 1).toInt());
            color = boom_section_states[i] ? GREEN : RED;
        }
        // only update state and UI if a state changes
        if (boom_section_states[i] != boom_section_state) {
            boom_section_states[i] = boom_section_state;
            tft.fillRect(SECTION_RECT_X + SECTION_RECT_SPACING_X * i, SECTION_RECT_Y,
                         SECTION_RECT_SIZE, SECTION_RECT_SIZE, color);
        }        
    }
    
    // no automatic if manual override is false
    if (!relay_control.manual_override || 
        !relay_control.automatic) {
        return;
    }

    // control boom section 1 if vehicle is moving & auto is active 
    bool enable_relays = gps_speed >= MIN_GPS_SPEED &&
                         boom_section_states[0] == BOOM_ACTIVE;

    control_relays(enable_relays);
}

void control_relays(bool enable)
{    
    if (enable) {
        digitalWrite(RELAY_PIN_1, HIGH);
        digitalWrite(RELAY_PIN_2, HIGH);
    } else {
        digitalWrite(RELAY_PIN_1, LOW);
        digitalWrite(RELAY_PIN_2, LOW);
    }
}
