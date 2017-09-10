/* Project: MSc project, Human-Computer Interaction programme, University College London
   Required HW: Arduino Mega 2560 */

/* -------------------------- GLOBAL VARIABLES & LIBRARIES -----------------------------------*/
#include <avr/pgmspace.h> // PROGMEM library—allows storing data in flash memory instead of SRAM

/* Device states and buttons */
int newDeviceState;
int currentDeviceState;
int previousDeviceState;
boolean prevStateFlag = false; // tells the system that a new device state is actually its previous (when a user triggers a confirmation window but decides to go back)

const int numberOfButtons = 17;
const int numberOfDeviceStates = 49;

int buttonReadings[numberOfButtons];
int prevButtonReadings[numberOfButtons];

/* Experimental conditions related variables */
int experimentNo = 1;
boolean experiment2Flag = false; // after first 3 topics, all buttons are enabled (i.e. switching to Experiment 3 setup)



/* Enabling additional Story Unit buttons in semi-linear and non-linear experiments */
int topicMenuScreensExperiment2[] = {21, 22, 23, 28, 29, 34, 35, 41}; // screens that need to show all topic options for Experiment 3
int topicMenuScreensExperiment3[] = {2, 7, 8, 13, 14, 21, 22, 23, 28, 29, 34, 35, 41}; // screens that need to show all topic options for Experiment 3

/* Variables for flagging visited topics in Experiments 2 and 3 */
boolean flagTopicVisited[6] = {false, false, false, false, false, false};
boolean flagAllTopicsVisited = false;

/* Automatic restart */
unsigned long timeFinishScreenSet;
unsigned long currentTime;

/* A variable holding incoming serial data from Processing */
char processingData;


/* Video */
float prevVol; // previous volume reading
boolean togglePlay = true;

/* -------------------------- MAP: 16X2 LCD & LEDs -----------------------------------*/
#include <Wire.h>
#include "rgb_lcd.h"
rgb_lcd lcd;

#include <FastLED.h>
#define DATA_PIN 10   // Arduino PIN
#define NUM_LEDS 15   // number of LEDs
#define COLOR_ORDER GRB
#define LED_TYPE WS2812B
#define FRAMES_PER_SECOND  120
struct CRGB leds[NUM_LEDS];

/* Hard coded IoT data from the bat sensors */
int batCallsPerDay[10];
int dates[10] = {26, 25, 24, 23, 22, 21, 20, 19, 18, 17}; // these are dates in August, i.e. 26 Aug, 25 Aug,...

/* sensor data from 26 Aug to 17 Aug 2017 */
int sensors[15][10] = {
  {11, 14, 17, 0, 15, 6, 0, 0, 0, 0},                               // sensor 1
  {38, 38, 31, 0, 18, 5, 0, 0, 0, 0},                               // sensor 2
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                                   // sensor 3
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                                   // sensor 4
  {19, 3, 265, 21, 0, 10, 10, 0, 20, 25},                           // sensor 5
  {30, 25, 10, 19, 2, 15, 0, 0, 0, 1},                              // sensor 6
  {2972, 8162, 13095, 14154, 5715, 7912, 2286, 13490, 4337, 6678},  // sensor 7
  {11, 64, 8, 21, 25, 19, 3, 2, 0, 2},                              // sensor 8
  {55, 48, 33, 13, 13, 40, 11, 18, 8, 1},                           // sensor 9
  {60, 22, 36, 0, 0, 14, 0, 3, 4, 1},                               // sensor 10
  {61, 59, 73, 37, 77, 37, 0, 71, 0, 1},                            // sensor 11
  {97, 86, 134, 39, 5, 103, 11, 32, 45, 170},                       // sensor 12
  {2, 0, 3, 1, 5, 4, 4, 2, 0, 0},                                   // sensor 13
  {82, 31, 98, 600, 74, 38, 0, 146, 122, 437},                      // sensor 14
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}                                    // sensor 15
};

/* Slider */
boolean flagInitialSliderScreen = true; // showing "try using slider" message when using the interactive map
int prevDate;
int initialSliderReading;

/* -------------------------- BUTTON CLASS --------------------------*/
class Button {

    int buttonPin;
    int LEDpin;

  public:
    Button(int _buttonPin, int _LEDpin) {
      buttonPin = _buttonPin;
      LEDpin = _LEDpin;
      pinMode(buttonPin, INPUT_PULLUP);
      pinMode(LEDpin, OUTPUT);
    }

    /* Read a button value */
    int read() {
      return digitalRead(buttonPin);
    }

    /* Enable (i.e. illuminate) a button */
    void enable(boolean buttonEnabled) {
      if (buttonEnabled == true) {
        digitalWrite(LEDpin, HIGH);
      }
      else {
        digitalWrite(LEDpin, LOW);
      }
    }
    /* Check if a button is enabled */
    boolean isEnabled() {
      if (digitalRead(LEDpin) == HIGH) {
        return true;
      }
      else {
        return false;
      }
    }
};

/* -------------------------- SETUP BUTTONS -------------------------- */
Button buttons[numberOfButtons] = {
  Button(12, 13),     // buttons[0] = START
  Button(22, 23),     // buttons[1] = TOPIC 1
  Button(24, 25),     // buttons[2] = TOPIC 2
  Button(26, 27),     // buttons[3] = TOPIC 3
  Button(28, 29),     // buttons[4] = TOPIC 4
  Button(30, 31),     // buttons[5] = TOPIC 5
  Button(32, 33),     // buttons[6] = TOPIC 6
  Button(34, 35),     // buttons[7] = FINISH
  Button(36, 37),     // buttons[8] = A
  Button(38, 39),     // buttons[9] = B
  Button(40, 41),     // buttons[10] = C
  Button(42, 43),     // buttons[11] = D
  Button(44, 45),     // buttons[12] = E
  Button(46, 47),     // buttons[13] = PLAY/PAUSE
  Button(48, 49),     // buttons[14] = STOP
  Button(50, 51),     // buttons[15] = PREV
  Button(52, 53),     // buttons[16] = NEXT
};

/* Assign labels to the buttons */
const char *buttonLabels[] = {"START", "TOPIC 1", "TOPIC 2", "TOPIC 3", "TOPIC 4", "TOPIC 5", "TOPIC 6", "FINISH", "A", "B", "C", "D", "E", "PLAY/PAUSE", "STOP", "PREV", "NEXT"};

/* -------------------------- DEVICE STATE CLASS -------------------------- */
class DeviceState {

    int buttonsEnabled[numberOfButtons];
    int buttonsTarget[numberOfButtons];
    int screenNo;


  public:
    DeviceState(int _screenNo, int _buttonsEnabled[numberOfButtons], int _buttonsTarget[numberOfButtons]) {
      screenNo = _screenNo;

      /* Assign values to a given deviceState instance */
      for (int i = 0; i < numberOfButtons; i++) {
        buttonsEnabled[i] = pgm_read_word(&_buttonsEnabled[i]);
        buttonsTarget[i] = pgm_read_word(&_buttonsTarget[i]);
      }
    }

    /* Illuminate enabled buttons for a given deviceState and print a screen number for Processing */
    void makeActive() {

      /* Send screen number to Processing (only do it once-when a new screen number is available) */
      if (prevStateFlag == false) {
        Serial.print("screen");
        Serial.print(screenNo);
        Serial.print('\n');
      }
      else {
        prevStateFlag = false;
      }

      /* Turn all LEDs OFF */
      for (int i = 0; i < numberOfButtons; i++) {
        buttons[i].enable(false);
      }

      /* Enable buttons for the current device state */
      for (int i = 0; i < numberOfButtons; i++) {
        if (i > 0 && buttonsEnabled[i] == 0) { // because "buttonsEnabled" arrays are 17 items long (maximum number of buttons) but not all buttons are enabled at the same time, zeros at the end are just fillers and should be skipped
          break;
        }
        else {
          buttons[buttonsEnabled[i]].enable(true);
        }
      }

      /* Enable extra story unit buttons needed in the mixed and user-driven experimental conditions */
      if (experimentNo == 2 || experiment2Flag == true) {
        if (newDeviceState > 20 && newDeviceState < 42) { // only relevant for screens in this range
          for (int i = 0; i < (sizeof(topicMenuScreensExperiment2) / sizeof(int)); i++) {
            if (newDeviceState == topicMenuScreensExperiment2[i]) {
              for (int j = 3; j < 6; j++) {
                if (flagTopicVisited[j] == true) {
                  buttons[j + 1].enable(false);
                }
                else {
                  buttons[j + 1].enable(true);
                }
              }
              break;
            }
          }
        }
      }

      else if (experimentNo == 3) {
        for (int i = 0; i < (sizeof(topicMenuScreensExperiment3) / sizeof(int)); i++) {
          if (newDeviceState == topicMenuScreensExperiment3[i]) {
            for (int j = 0; j < 6; j++) {
              if (flagTopicVisited[j] == true) {
                buttons[j + 1].enable(false);
              }
              else {
                buttons[j + 1].enable(true);
              }
            }
            break;
          }
        }

      }

      /* Enable Play button */
      if (newDeviceState == 7 || newDeviceState == 23 || newDeviceState == 35) {
        buttons[13].enable(true);
      }
    }

    /* Returns newDeviceState when a button is clicked */
    int returnButtonTarget(int buttonClicked) {

      /* Special conditions for the extra enabled buttons in mixed and user-driven experimental conditions */
      if ((experimentNo == 2 || experimentNo == 3) && buttonClicked > 0 && buttonClicked < 7) {
        if (buttonClicked == 1) {
          return 3;
        }
        else if (buttonClicked == 2) {
          return 9;
        }
        else if (buttonClicked == 3) {
          return 15;
        }
        else if (buttonClicked == 4) {
          return 24;
        }
        else if (buttonClicked == 5) {
          return 30;
        }
        else if (buttonClicked == 6) {
          return 36;
        }
      }

      /* Skip the Finish confirmation screen if all topics and pages have been already visited */
      else if (flagAllTopicsVisited == true && buttonClicked == 7 && (currentDeviceState == 8 || currentDeviceState == 14 || currentDeviceState == 23 || currentDeviceState == 29 || currentDeviceState == 35 || currentDeviceState == 41)) {
        return 42;
      }
      else {
        for (int i = 0; i < numberOfButtons; i++) {
          if (buttonsEnabled[i] == buttonClicked) {
            return buttonsTarget[i];
            break;
          }
        }
      }
    }
};

/* -------------------------- DEFINITION OF DEVICE STATES -------------------------- */

/* A declaration of which buttons are enabled for each content screen in the base (linear) experimental condition */
const PROGMEM int buttonsEnabledState0[numberOfButtons] = {0};
const PROGMEM int buttonsEnabledState1[numberOfButtons] = {0, 8, 9, 10, 11, 12};
const PROGMEM int buttonsEnabledState2[numberOfButtons] = {0, 1, 7};
const PROGMEM int buttonsEnabledState3[numberOfButtons] = {0, 7, 8, 9, 10};
const PROGMEM int buttonsEnabledState4[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState5[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState6[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState7[numberOfButtons] = {0, 2, 7, 16};
const PROGMEM int buttonsEnabledState8[numberOfButtons] = {0, 2, 7, 15};
const PROGMEM int buttonsEnabledState9[numberOfButtons] = {0, 7, 8, 9, 10};
const PROGMEM int buttonsEnabledState10[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState11[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState12[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState13[numberOfButtons] = {0, 3, 7, 16};
const PROGMEM int buttonsEnabledState14[numberOfButtons] = {0, 3, 7, 15};
const PROGMEM int buttonsEnabledState15[numberOfButtons] = {0, 7, 8, 9, 10, 11, 12};
const PROGMEM int buttonsEnabledState16[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState17[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState18[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState19[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState20[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState21[numberOfButtons] = {0, 4, 7, 16};
const PROGMEM int buttonsEnabledState22[numberOfButtons] = {0, 4, 7, 15, 16};
const PROGMEM int buttonsEnabledState23[numberOfButtons] = {0, 4, 7, 15};
const PROGMEM int buttonsEnabledState24[numberOfButtons] = {0, 7, 8, 9, 10};
const PROGMEM int buttonsEnabledState25[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState26[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState27[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState28[numberOfButtons] = {0, 5, 7, 16};
const PROGMEM int buttonsEnabledState29[numberOfButtons] = {0, 5, 7, 15};
const PROGMEM int buttonsEnabledState30[numberOfButtons] = {0, 7, 8, 9, 10};
const PROGMEM int buttonsEnabledState31[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState32[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState33[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState34[numberOfButtons] = {0, 6, 7, 16};
const PROGMEM int buttonsEnabledState35[numberOfButtons] = {0, 6, 7, 15};
const PROGMEM int buttonsEnabledState36[numberOfButtons] = {0, 7, 8, 9, 10, 11};
const PROGMEM int buttonsEnabledState37[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState38[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState39[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState40[numberOfButtons] = {0, 7, 16};
const PROGMEM int buttonsEnabledState41[numberOfButtons] = {0, 7};
const PROGMEM int buttonsEnabledState42[numberOfButtons] = {0, 8, 9, 10};
const PROGMEM int buttonsEnabledState43[numberOfButtons] = {0, 8, 9, 10, 11};
const PROGMEM int buttonsEnabledState44[numberOfButtons] = {0, 8, 9, 10, 11, 12};
const PROGMEM int buttonsEnabledState45[numberOfButtons] = {0, 8, 9, 10, 11};
const PROGMEM int buttonsEnabledState46[numberOfButtons] = {0};
const PROGMEM int buttonsEnabledState47[numberOfButtons] = {0, 15};
const PROGMEM int buttonsEnabledState48[numberOfButtons] = {7, 15};

/* A declaration of button targets (i.e. when a button is pressed what device state should be activated). This declares the base (linear) experimental condition. */
const PROGMEM int buttonsTargetsState0[numberOfButtons] = {1};
const PROGMEM int buttonsTargetsState1[numberOfButtons] = {47, 2, 2, 2, 2, 2};
const PROGMEM int buttonsTargetsState2[numberOfButtons] = {47, 3, 48};
const PROGMEM int buttonsTargetsState3[numberOfButtons] = {47, 48, 4, 5, 6};
const PROGMEM int buttonsTargetsState4[numberOfButtons] = {47, 48, 7};
const PROGMEM int buttonsTargetsState5[numberOfButtons] = {47, 48, 7};
const PROGMEM int buttonsTargetsState6[numberOfButtons] = {47, 48, 7};
const PROGMEM int buttonsTargetsState7[numberOfButtons] = {47, 9, 48, 8};
const PROGMEM int buttonsTargetsState8[numberOfButtons] = {47, 9, 48, 7};
const PROGMEM int buttonsTargetsState9[numberOfButtons] = {47, 48, 10, 11, 12};
const PROGMEM int buttonsTargetsState10[numberOfButtons] = {47, 48, 13};
const PROGMEM int buttonsTargetsState11[numberOfButtons] = {47, 48, 13};
const PROGMEM int buttonsTargetsState12[numberOfButtons] = {47, 48, 13};
const PROGMEM int buttonsTargetsState13[numberOfButtons] = {47, 15, 48, 14};
const PROGMEM int buttonsTargetsState14[numberOfButtons] = {47, 15, 48, 13};
const PROGMEM int buttonsTargetsState15[numberOfButtons] = {47, 48, 16, 17, 18, 19, 20};
const PROGMEM int buttonsTargetsState16[numberOfButtons] = {47, 48, 21};
const PROGMEM int buttonsTargetsState17[numberOfButtons] = {47, 48, 21};
const PROGMEM int buttonsTargetsState18[numberOfButtons] = {47, 48, 21};
const PROGMEM int buttonsTargetsState19[numberOfButtons] = {47, 48, 21};
const PROGMEM int buttonsTargetsState20[numberOfButtons] = {47, 48, 21};
const PROGMEM int buttonsTargetsState21[numberOfButtons] = {47, 24, 48, 22};
const PROGMEM int buttonsTargetsState22[numberOfButtons] = {47, 24, 48, 21, 23};
const PROGMEM int buttonsTargetsState23[numberOfButtons] = {47, 24, 48, 22};
const PROGMEM int buttonsTargetsState24[numberOfButtons] = {47, 48, 25, 26, 27};
const PROGMEM int buttonsTargetsState25[numberOfButtons] = {47, 48, 28};
const PROGMEM int buttonsTargetsState26[numberOfButtons] = {47, 48, 28};
const PROGMEM int buttonsTargetsState27[numberOfButtons] = {47, 48, 28};
const PROGMEM int buttonsTargetsState28[numberOfButtons] = {47, 30, 48, 29};
const PROGMEM int buttonsTargetsState29[numberOfButtons] = {47, 30, 48, 28};
const PROGMEM int buttonsTargetsState30[numberOfButtons] = {47, 48, 31, 32, 33};
const PROGMEM int buttonsTargetsState31[numberOfButtons] = {47, 48, 34};
const PROGMEM int buttonsTargetsState32[numberOfButtons] = {47, 48, 34};
const PROGMEM int buttonsTargetsState33[numberOfButtons] = {47, 48, 34};
const PROGMEM int buttonsTargetsState34[numberOfButtons] = {47, 36, 48, 35};
const PROGMEM int buttonsTargetsState35[numberOfButtons] = {47, 36, 48, 34};
const PROGMEM int buttonsTargetsState36[numberOfButtons] = {47, 48, 37, 38, 39, 40};
const PROGMEM int buttonsTargetsState37[numberOfButtons] = {47, 48, 41};
const PROGMEM int buttonsTargetsState38[numberOfButtons] = {47, 48, 41};
const PROGMEM int buttonsTargetsState39[numberOfButtons] = {47, 48, 41};
const PROGMEM int buttonsTargetsState40[numberOfButtons] = {47, 48, 41};
const PROGMEM int buttonsTargetsState41[numberOfButtons] = {47, 48};
const PROGMEM int buttonsTargetsState42[numberOfButtons] = {47, 43, 43, 43};
const PROGMEM int buttonsTargetsState43[numberOfButtons] = {47, 44, 44, 44, 44};
const PROGMEM int buttonsTargetsState44[numberOfButtons] = {47, 45, 45, 45, 45, 45};
const PROGMEM int buttonsTargetsState45[numberOfButtons] = {47, 46, 46, 46, 46};
const PROGMEM int buttonsTargetsState46[numberOfButtons] = {0};

/* A declaration of the two confirmation screens */
const PROGMEM int buttonsTargetsState47[numberOfButtons] = {0, -1}; // -1 signifies that the user does not want to leave the current screen
const PROGMEM int buttonsTargetsState48[numberOfButtons] = {42, -1};

/*  Device state setup */
DeviceState deviceStates[numberOfDeviceStates] = {
  DeviceState(0, buttonsEnabledState0, buttonsTargetsState0),
  DeviceState(1, buttonsEnabledState1, buttonsTargetsState1),
  DeviceState(2, buttonsEnabledState2, buttonsTargetsState2),
  DeviceState(3, buttonsEnabledState3, buttonsTargetsState3),
  DeviceState(4, buttonsEnabledState4, buttonsTargetsState4),
  DeviceState(5, buttonsEnabledState5, buttonsTargetsState5),
  DeviceState(6, buttonsEnabledState6, buttonsTargetsState6),
  DeviceState(7, buttonsEnabledState7, buttonsTargetsState7),
  DeviceState(8, buttonsEnabledState8, buttonsTargetsState8),
  DeviceState(9, buttonsEnabledState9, buttonsTargetsState9),
  DeviceState(10, buttonsEnabledState10, buttonsTargetsState10),
  DeviceState(11, buttonsEnabledState11, buttonsTargetsState11),
  DeviceState(12, buttonsEnabledState12, buttonsTargetsState12),
  DeviceState(13, buttonsEnabledState13, buttonsTargetsState13),
  DeviceState(14, buttonsEnabledState14, buttonsTargetsState14),
  DeviceState(15, buttonsEnabledState15, buttonsTargetsState15),
  DeviceState(16, buttonsEnabledState16, buttonsTargetsState16),
  DeviceState(17, buttonsEnabledState17, buttonsTargetsState17),
  DeviceState(18, buttonsEnabledState18, buttonsTargetsState18),
  DeviceState(19, buttonsEnabledState19, buttonsTargetsState19),
  DeviceState(20, buttonsEnabledState20, buttonsTargetsState20),
  DeviceState(21, buttonsEnabledState21, buttonsTargetsState21),
  DeviceState(22, buttonsEnabledState22, buttonsTargetsState22),
  DeviceState(23, buttonsEnabledState23, buttonsTargetsState23),
  DeviceState(24, buttonsEnabledState24, buttonsTargetsState24),
  DeviceState(25, buttonsEnabledState25, buttonsTargetsState25),
  DeviceState(26, buttonsEnabledState26, buttonsTargetsState26),
  DeviceState(27, buttonsEnabledState27, buttonsTargetsState27),
  DeviceState(28, buttonsEnabledState28, buttonsTargetsState28),
  DeviceState(29, buttonsEnabledState29, buttonsTargetsState29),
  DeviceState(30, buttonsEnabledState30, buttonsTargetsState30),
  DeviceState(31, buttonsEnabledState31, buttonsTargetsState31),
  DeviceState(32, buttonsEnabledState32, buttonsTargetsState32),
  DeviceState(33, buttonsEnabledState33, buttonsTargetsState33),
  DeviceState(34, buttonsEnabledState34, buttonsTargetsState34),
  DeviceState(35, buttonsEnabledState35, buttonsTargetsState35),
  DeviceState(36, buttonsEnabledState36, buttonsTargetsState36),
  DeviceState(37, buttonsEnabledState37, buttonsTargetsState37),
  DeviceState(38, buttonsEnabledState38, buttonsTargetsState38),
  DeviceState(39, buttonsEnabledState39, buttonsTargetsState39),
  DeviceState(40, buttonsEnabledState40, buttonsTargetsState40),
  DeviceState(41, buttonsEnabledState41, buttonsTargetsState41),
  DeviceState(42, buttonsEnabledState42, buttonsTargetsState42),
  DeviceState(43, buttonsEnabledState43, buttonsTargetsState43),
  DeviceState(44, buttonsEnabledState44, buttonsTargetsState44),
  DeviceState(45, buttonsEnabledState45, buttonsTargetsState45),
  DeviceState(46, buttonsEnabledState46, buttonsTargetsState46),
  DeviceState(47, buttonsEnabledState47, buttonsTargetsState47),
  DeviceState(48, buttonsEnabledState48, buttonsTargetsState48)
};


/* -------------------------- ARDUINO SETUP -------------------------- */
void setup() {
  /* Initialize serial communications at a 115200 baud rate */
  Serial.begin(115200);

  /* Shake hands with Processing */
  establishContact();

  /* Initialise the Bat device */
  deviceStates[0].makeActive();
  Serial.print("Experiment No:");
  Serial.print(experimentNo);
  Serial.print('\n');

  /* Count the cummulative number of bat calls from all sensors for each day */
  for (int i = 0; i < 10; i++) {

    int sum = 0;

    for (int j = 0; j < 15; j++) {
      int val = sensors[j][i];
      sum = sum + val;
      if (j == 14) {
        batCallsPerDay[i] = sum;
        //Serial.println(sum);
      }
    }
  }

  /* Initialise the 16x2 LCD display */
  lcd.begin(16, 2);
  lcd.noDisplay();

  /* Initialise LEDs */
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(150);
  FastLED.clear();
  FastLED.show();

  /* Read initial volume pot value */
  setVolume();
}

/* -------------------------- ARDUINO LOOP ---------------------- */
void loop() {

  /* Do following if the device state has been changed, i.e. the user clicked an enabled button */
  if (newDeviceState != currentDeviceState) {

    /* Reset the following variables if the new device state goes to the initial screen */
    if (newDeviceState == 0) {
      experimentNo++;

      /* Reset the slider */
      initialSliderReading = analogRead(A0);
      flagInitialSliderScreen = true; // show "try using slider" message if on the screen 13 for the first time


      /* Reset all 'visited' flags */
      for (int i = 0; i < 6; i++) {
        flagTopicVisited[i] = false;
      }

      flagAllTopicsVisited = false;

      if (experimentNo == 3) {
        experiment2Flag = false; // reset the Flag for experiment 2, once it's finished;
      }
      else if (experimentNo == 4) {
        experimentNo = 1;
      }

      /* Send the current Experiment number to Processing */
      Serial.print("Experiment No:");
      Serial.print(experimentNo);
      Serial.print('\n');
    }
    else if (newDeviceState == 21 && experimentNo == 2) { // once the user reached half of the Experiment 2 (which was author-driven, linear), switch to the setup of Experiment 3 (user-driven)
      experiment2Flag = true;
    }
    /* Flag that Topic 1 was visited */
    else if (newDeviceState == 3 && flagTopicVisited[0] == false) {
      flagTopicVisited[0] = true;
    }
    /* Flag that Topic 2 was visited */
    else if (newDeviceState == 9 && flagTopicVisited[1] == false) {
      flagTopicVisited[1] = true;
    }
    /* Flag that Topic 3 was visited */
    else if (newDeviceState == 15 && flagTopicVisited[2] == false) {
      flagTopicVisited[2] = true;
    }
    /* Flag that Topic 4 was visited */
    else if (newDeviceState == 24 && flagTopicVisited[3] == false) {
      flagTopicVisited[3] = true;
    }
    /* Flag that Topic 5 was visited */
    else if (newDeviceState == 30 && flagTopicVisited[4] == false) {
      flagTopicVisited[4] = true;
    }
    /* Flag that Topic 6 was visited */
    else if (newDeviceState == 36 && flagTopicVisited[5] == false) {
      flagTopicVisited[5] = true;
    }
    else if (newDeviceState == 46) { // when the user reaches the final screen, inform Processing to sync the automatic device restart
      timeFinishScreenSet = millis();
      Serial.print("time Finish screen set");
      Serial.print(timeFinishScreenSet);
      Serial.print('\n');
    }

    /* Only send All topics visited message to Processing once per experiment */
    if (flagTopicVisited[0] == true && flagTopicVisited[1] == true && flagTopicVisited[2] == true && flagTopicVisited[3] == true && flagTopicVisited[4] == true && flagTopicVisited[5] == true && flagAllTopicsVisited == false) {
      Serial.println("allTopicsVisited");
      flagAllTopicsVisited = true;
    }

    /* Make the new device state active, i.e. change the screen and enable and illuminate relevant buttons */
    deviceStates[newDeviceState].makeActive();
    
    previousDeviceState = currentDeviceState;
    currentDeviceState = newDeviceState;
  }

  /* Only read the volume pot values if the screens with videos are currently activated */
  if (currentDeviceState == 7 || currentDeviceState == 23 || currentDeviceState == 35) {
    setVolume();
  }

  /* Automatically reset the device after 30 seconds */
  currentTime = millis();
  if (currentDeviceState == 46 && (currentTime - timeFinishScreenSet) > 30000) {
    newDeviceState = 0;

    timeFinishScreenSet = 0; //reset
    Serial.print("screen");
    Serial.print(0);
    Serial.print('\n');
  }

  /* Keep checking button readings */
  checkButtonReadings();

  /* Detect if video has finished - From Processing - and if so, toggle the Play value (so that the Play button is not set to "Pause") and switch off the Stop button LED */
  if (Serial.available())
  { 
    processingData = Serial.read(); // read Serial to get data from Processing and store it
    if (processingData == '0') {
      buttons[14].enable(false);
      togglePlay = true;
    }
  }

  /* LCD + LEDs: only enabled when on the related screen  */
  if (currentDeviceState == 13) {
    if (flagInitialSliderScreen == true) {

      lcd.display();
      lcd.clear();
      lcd.setRGB(155, 255, 155);
      lcd.setCursor(4, 0);
      lcd.print("Try using");
      lcd.setCursor(3, 1);
      lcd.print("the slider!");
      flagInitialSliderScreen = false;
    }

    int sliderReading = analogRead(A0);

    /* Change LCD values and LED colours based on the slider position (and debounce it) */
    if (sliderReading >= (initialSliderReading + 30) || sliderReading <= (initialSliderReading - 30) || (sliderReading == 0 && initialSliderReading > 10) || (sliderReading == 1023 && initialSliderReading < 1013) ) {
      int date = map(sliderReading, 0, 1023, 0, 9);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(String("Date:") + String(dates[date]) + String(" Aug 2017"));

      lcd.setCursor(0, 1);
      lcd.print(String("Bat calls:") + String(batCallsPerDay[date]));

      for (int i = 0; i < NUM_LEDS; i++) {
        int sensorVal = sensors[i][date]; // bat activity for a given sensor on a given day

        if (sensorVal == 0) {
          leds[i].setRGB(50, 50, 50); // weak white
          FastLED.show();
        }
        else if (sensorVal > 0 && sensorVal <= 10) {
          leds[i].setRGB(0, 0, 255); // blue
          FastLED.show();
        }
        else if (sensorVal > 10 && sensorVal <= 50) {
          leds[i].setRGB(0, 255, 0); // green
          FastLED.show();
        }
        else if (sensorVal > 50 && sensorVal <= 100) {
          leds[i].setRGB(255, 255, 0); // yellow
          FastLED.show();
        }
        else if (sensorVal > 100 && sensorVal <= 500) {
          leds[i].setRGB(255, 80, 0); // orange
          FastLED.show();
        }
        else { // for values 500+
          leds[i].setRGB(255, 0, 200); // pink
          FastLED.show();
        }
      }

      initialSliderReading = sliderReading;
      if (date != prevDate) {
        Serial.print("Slider date: ");
        Serial.print(date);
        Serial.print('\n');
        prevDate = date;
      }
    }
  }
  else {
    FastLED.clear();
    FastLED.show();
    lcd.setRGB(0, 0, 0);
    lcd.noDisplay();
    flagInitialSliderScreen = true;
  }

  delay(10);

}

/* -------------------------- FUNCTIONS -------------------------- */

/* Check button readings */
void checkButtonReadings() {
  for (int i = 0; i < numberOfButtons; i++) {
    buttonReadings[i] = buttons[i].read();

    if (buttonReadings[i] != prevButtonReadings[i] && buttonReadings[i] == 0 && buttons[i].isEnabled() == true) { // button is pushed when it delivers "0" because of the PULLUP resistor on the input

      /* Send information about a button press to Processing */
      Serial.print("EnabledButtonClicked: ");
      Serial.print(buttonLabels[i]);
      Serial.print('\n');

      if (i == 13) {                // if the button "Play/Pause" is pressed - send a message to Processing
        if (togglePlay == true) {
          Serial.println("play");
          buttons[14].enable(true); // enable the "Stop" button
          togglePlay = false;
        }
        else {
          Serial.println("pause");  // if the button "Play/Pause" is pressed - send a message to Processing
          togglePlay = true;
        }
      }
      else if (i == 14) {         // if the button "Stop" is pressed - send a message to Processing
        Serial.println("stop");
        buttons[14].enable(false);
        togglePlay = true;
      }
      else {
        
        /* If the pressed button was not related to videos, activate the new device state */

        if (currentDeviceState == 7 || currentDeviceState == 23 || currentDeviceState == 35) { // Stop a video playback in case the user would change screens while a video is playing
          togglePlay = true;
          Serial.println("stop-flag"); // send a flag to Processing to stop a video playback
        }

        /* Return the device state number that the button pressed leads to */
        newDeviceState = deviceStates[currentDeviceState].returnButtonTarget(i);

        /* This is for the case in which the user initiates a confirmation screen but decides to cancel it and go back to the experience */
        if (newDeviceState == -1) {
          prevStateFlag = true;
          Serial.print("screen");
          Serial.print(-1);
          Serial.print('\n');

          newDeviceState = previousDeviceState;

        }
      }
    }
    else if (buttonReadings[i] != prevButtonReadings[i] && buttonReadings[i] == 0 && buttons[i].isEnabled() == false) { // used for logging interactions (user clicks a button that's not enabled)
      Serial.print("DisabledButtonClicked: ");
      Serial.print(buttonLabels[i]);
      Serial.print('\n');
    }
    prevButtonReadings[i] = buttonReadings[i];
  }
}

/* Return a decimal number as a volume value (needed for Processing) */
float mapVolume(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/* Set volume */
void setVolume() {
  int volumeReading = analogRead(A1);

  float volumeVal = mapVolume(volumeReading, 0, 1023, 0, 1);

  if (volumeVal > (prevVol + 0.05) || volumeVal < (prevVol - 0.05)) { // to prevent keep sending messages to Processing (pot readings flicker)
    Serial.print("vol");
    Serial.print(volumeVal);
    Serial.print('\n');

    prevVol = volumeVal;
  }

}

/* Shake hands with Processing */
void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("A");   // send a capital A
    delay(300);
  }
}
