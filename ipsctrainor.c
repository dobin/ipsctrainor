/*
*/

#define DEBUG true

#define LED_WLAN 13

#define BUTTON1 4 // 5 
#define BUTTON2 10
#define PIEZO 12


#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial esp8266(11, 12); // RX, TX

int ID;

enum t_state {
  StateIdle,
  StateStart,
  StateRunning,
  StateStop,
};

enum t_state State;

unsigned long TimeStart;

unsigned long TimeMeasureStart;
unsigned long TimeMeasureEnd;
unsigned long TimeMeasureLast;

unsigned long getTime() {
  return millis();
}

boolean btn1_pressed = false;
boolean btn2_pressed = false;

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  lcd.clear();
  lcd.begin(16, 2);
  lcd.print("IPSC Trainer ");

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  debug("Yay");
  State = StateIdle;

  tone(PIEZO, 400, 500);
}

void loop() {
  //lcd.clear();
  char out[32];
  unsigned long time = getTime();

  // Loop
  switch (State) {
    case StateStart:
      if (time > TimeStart) {
        debug("Start -> Running");
        tone(PIEZO, 400, 150);
        State = StateRunning;
        TimeMeasureStart = getTime();
      }
      break;
  }

  // Output
  lcd.setCursor(0, 1);
  switch (State) {
    case StateIdle:
      snprintf(out, 32 - 1, "Idle          ");
      lcd.print(out);
      break;

    case StateStart:
      snprintf(out, 32 - 1, "Start           ");
      lcd.print(out);
      break;

    case StateRunning:
      snprintf(out, 32 - 1, "Run : %5i", time - TimeMeasureStart);
      lcd.print(out);
      break;

    case StateStop:
      snprintf(out, 32 - 1, "Stop: %5i", TimeMeasureLast);
      lcd.print(out);
      break;
  }

  // Input
  int press;
  press = digitalRead(BUTTON1);
  if (press == LOW) {
    if (! btn1_pressed) {
      changeState(BUTTON1);
      btn1_pressed = true;
    }
  } else {
    if (btn1_pressed) {
      btn1_pressed = false;
    }
  }
  press = digitalRead(BUTTON2);
  if (press == LOW) {
    if (! btn2_pressed) {
      changeState(BUTTON2);
      btn2_pressed = true;
    }
  } else {
    if (btn2_pressed) {
      btn2_pressed = false;
    }
  }
}



void changeState(int button) {
  char out[32];
  sprintf(out, "changeState btn: %i\n", button);
  debug(out);
  unsigned long currentTime = getTime();

  //debug("TONE");
  //tone(PIEZO, 400, 500);

  // Button 2 = on weapon
  if (button == BUTTON2) {
    switch (State) {
      case StateRunning:
        debug("Running -> Stop");
        State = StateStop;
        TimeMeasureEnd = currentTime;

        unsigned long time;
        time = TimeMeasureEnd - TimeMeasureStart;
        TimeMeasureLast = time;
        sprintf(out, "Time: %i\n", time);
        debug(out);
        break;
    }

  }

  // Button 1 = on display
  if (button == BUTTON1) {
    switch (State) {
      case StateIdle:
        debug("Idle -> Start");
        State = StateStart;
        TimeStart = currentTime + 3000 + random(0, 2000);

        break;

      case StateStop:
        // Start from the beginning
        State = StateIdle;
        break;
    }
  }

}

//-----------------------------------------Config ESP8266------------------------------------

boolean espConfig()
{
  boolean success = true;
  char *ready = "ready";
  esp8266.setTimeout(5000);
  success &= sendCom("AT+RST", ready);
  esp8266.setTimeout(1000);

  return success;
}


boolean sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  else
  {
    debug("ESP SEND ERROR: " + command);
    return false;
  }
}

String sendCom(String command)
{
  esp8266.println(command);
  return esp8266.readString();
}



//-------------------------------------------------Debug Functions------------------------------------------------------
void serialDebug() {
  while (true)
  {
    if (esp8266.available())
      Serial.write(esp8266.read());
    if (Serial.available())
      esp8266.write(Serial.read());
  }
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
