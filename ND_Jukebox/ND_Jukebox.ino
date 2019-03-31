//************************************************
// ND Jukebox
//************************************************
//************************************************
// Included Libraries
//************************************************
#include <LiquidCrystal_I2C.h>
#include <MP3Trigger.h>

//************************************************
// LCD Setup
//************************************************
// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Set the LCD I2C address (0x3F or 0x27)

//************************************************
// LCD Display Variables
//************************************************
String clear_text = "                    ";
String restoreLine1 = "";
String restoreLine2 = "";
String restoreLine3 = "";
String restoreLine4 = "";
String currentLine1 = "";
String currentLine2 = "";
String currentLine3 = "";
String currentLine4 = "";

//************************************************
// Push Button Setup
// -Pin2    STOP
// -Pin3    START
//************************************************
int stopButtonPin = 2;
int startButtonPin = 3;

int startState = 0;
int stopState = 0;
boolean extraClicks = false;
long previousMillis = millis();
//------------------------------------------------
// MP3 Trigger Setup
//------------------------------------------------
MP3Trigger MP3Trigger;

int volLevel = 35;

//************************************************
// Juke Box Variables
//************************************************

int currentSong = 1;

String songTitle[36] = {"Walk the Line",
                    "Ring of Fire",
                    "Blue Suede Shoes",
                    "So Lonesome",
                    "Folsom Prison",
                    "Cheatin Heart",
                    "Jolene",
                    "Big River",
                    "Blues Eyes Cryin",
                    "Imagine",
                    "Long Tall Sally",
                    "Pretty Woman",
                    "Peggy Sue",
                    "Everyday",
                    "La Bamba",
                    "Sweet Dreams",
                    "Desperado",
                    "The Twist",
                    "Respect",
                    "People Get Ready",
                    "Dock of the Bay",
                    "Dancing Streets",
                    "My Imagination",
                    "Stay Together",
                    "Papa New Bag",
                    "Stany By Me",
                    "Who Do You Love",
                    "My Generation",
                    "Yesterday",
                    "Mr Tambourine",
                    "Fighting Man",
                    "Paranoid",
                    "Highway to Hell",
                    "Roxanne",
                    "Lola",
                    "Love Rock N Roll"};
                    
int songTrack[36]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36};

long songLength[36]={166,157,136,169,165,163,162,168,140,184,129,180,150,128,124,211,121,156,148,159,162,158,181,198,128,178,150,199,127,150,139,173,208,195,250,175};

void setup() {

  //************************************************
  // Serial Setup
  //************************************************
  Serial.begin(115200);
  // wait for the console to connect so we can see what's happening
  while (!Serial) {
    ; // wait for Console port to connect.
  }

  //************************************************
  // MP3 Setup
  //************************************************
  MP3Trigger.setup(&Serial3);
  Serial3.begin( MP3Trigger::serialRate() );
  while (!Serial3) {
    ; // wait for MP3 port to connect.
  }

  MP3Trigger.setVolume(volLevel);

  //************************************************
  // Joysticks Setup
  //************************************************
  // Left Joystick
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  // Right Joystick
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  //************************************************
  // Buttons Setup
  //************************************************
  pinMode(stopButtonPin, INPUT);
  pinMode(startButtonPin, INPUT);

  //************************************************
  // LCD Setup
  //************************************************
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight

  // Print a message to the LCD.
  display_lcd(0, 0, "Notre Dame", true);
  display_lcd(0, 1, "    **JUKEBOX**", false);
  display_lcd(0, 2, "  Loading...", false);

  loadPlayList();
}

void loop() {

//  readStop = digitalRead(stopButtonPin);
//  readStart = digitalRead(startButtonPin);
//  scrollValue = analogRead(2);
//  volValue = analogRead(7);

  // Process MP3 Trigger Commands

  startState = digitalRead(startButtonPin);
  stopState = digitalRead(stopButtonPin);
  if(startState == HIGH && !extraClicks) {
    MP3Trigger.trigger(currentSong);
    Serial.print(F("\r\nSong playing"));
    startState = 0;
    extraClicks = true;
  }
  
  //if(stopState == HIGH && !extraClicks) {
   // MP3Trigger.stop();
    //Serial.print(F("\r\nSong stop"));
    //stopState = 0;
    //extraClicks = true;
  //}
  if (extraClicks)
   {
      if ((previousMillis + 500) < millis())
      {
          extraClicks = false;
      }
   }
  MP3Trigger.update();
}

void checkButtons() {
  startState = digitalRead(startButtonPin);
  stopState = digitalRead(stopButtonPin);
  if(startState == HIGH && !extraClicks) {
    MP3Trigger.trigger((currentSong + 1) % 36);
    Serial.print(F("\r\nSong playing"));
    startState = 0;
    extraClicks = true;
    previousMillis = millis();
  }
  if(stopState == HIGH && !extraClicks) {
    MP3Trigger.stop();
    Serial.print(F("\r\nSong stop"));
    stopState = 0;
    extraClicks = true;
    previousMillis = millis();
  }
}

//************************************************
// Display LCD Function
//************************************************
void display_lcd(int col, int row, String display_text, boolean clear_lcd)
{
  if (clear_lcd)
  {
    lcd.setCursor(0, 0);
    lcd.print(clear_text);
    lcd.setCursor(0, 1);
    lcd.print(clear_text);
    lcd.setCursor(0, 2);
    lcd.print(clear_text);
    lcd.setCursor(0, 3);
    lcd.print(clear_text);
    currentLine1 = "                    ";
    currentLine2 = "                    ";
    currentLine3 = "                    ";
    currentLine4 = "                    ";
  }

  lcd.setCursor(col, row);
  lcd.print(display_text);

  if (row == 0)
  {
    currentLine1 = display_text;
  }

  if (row == 1)
  {
    currentLine2 = display_text;
  }

  if (row == 2)
  {
    currentLine3 = display_text;
  }

  if (row == 3)
  {
    currentLine4 = display_text;
  }
}


//************************************************
// Load Initial Play List
//************************************************
void loadPlayList()
{
  display_lcd(0, 0, songTitle[0], true);
  display_lcd(0, 1, ">>  " + songTitle[1], false);
  display_lcd(0, 2, songTitle[2], false);

}
