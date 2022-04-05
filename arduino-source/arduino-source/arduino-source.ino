#include <Adafruit_NeoPixel.h>

int backlightOn = false;

#define DELAY_TIME 50
#define WRITE_FREQUENCY 1000
int write_delay_counter = 0;
float db_total = 0.0;
float max_db = 0.0;
long loop_counter = 0;

// LED Panel
#define LED_PIN 12
#define LED_COUNT 256
int upper_db_limit = 90;
int lower_db_limit = 45;
int defaultBrightness = 10;
int brightness = defaultBrightness;
int brightnessJumpAmount = 5;
#define ROWS 32
#define COLS 8
Adafruit_NeoPixel strip((ROWS * COLS), LED_PIN, NEO_GRB + NEO_KHZ800);


#define SoundSensorPin A1  //this pin read the analog voltage from the sound level meter
#define VREF 5.0  //voltage on AREF pin,default:operating voltage
float calibration = 1.00;

#include <Keypad.h>;
// Keypad
const byte KEY_ROWS = 4; //four rows
const byte KEY_COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[KEY_ROWS][KEY_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[KEY_ROWS] = {23,25,27,29}; //connect to the row pinouts of the keypad
byte colPins[KEY_COLS] = {31,33,35,37}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, KEY_ROWS, KEY_COLS); 

#include <EEPROM.h>
//EEPROM
#define LOWER_DB_LIMIT_ADDR 69
#define UPPER_DB_LIMIT_ADDR 70
#define CALIBRATION_ADDR 71
#define BRIGHTNESS_ADDR 72

class Colour {
  public: 
    Colour(byte inRed, byte inGreen, byte inBlue);
    Colour();
    void setColour(byte inRed, byte inGreen, byte inBlue);
    byte getRed(void);
    byte getGreen(void);
    byte getBlue(void);
  
  private :
    byte red;
    byte green;
    byte blue;
};

Colour::Colour(byte inRed, byte inGreen, byte inBlue) { 
  setColour(inRed, inGreen, inBlue); 
}

Colour::Colour(void) { setColour(0,0,0); }

void Colour::setColour(byte inRed, byte inGreen, byte inBlue) {
  red = inRed;
  green = inGreen;
  blue = inBlue;
}

byte Colour::getRed(void) { return red; }
byte Colour::getGreen(void) { return green; }
byte Colour::getBlue(void) { return blue; }

Colour current_min_colour(0,255,0);
Colour current_max_colour(255,0,0);

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  read_from_eeprom();
  strip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show(); // Turn OFF all pixels ASAP
  strip.setBrightness(brightness);

  lcd.init();  //initialize the lcd
  swapBacklight();
  Serial.println("Test!");

  // do some fun stuff with the light bar and the lcd during bootup
  startupMessages();
  // rainbowCycle(1, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
  int board[ROWS][COLS];
  get_board(board);

  float voltageValue,dbValue;
  voltageValue = analogRead(SoundSensorPin) / 1024.0 * VREF;
  dbValue = (voltageValue * calibration) * 50.0;  //convert voltage to decibel value
  
  display_sound(board, dbValue);
  write_delay_counter += 1;
  smartDelay(DELAY_TIME, dbValue);
}


void rainbowCycle(uint8_t wait, int cycles) {
  
  for(int c = 0; c <= cycles; c++) {
    uint16_t i, j;
 
    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, wheel(((i * 256 / strip.numPixels()) + j) & 255));
        
      }
      strip.show();
      delay(wait);
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte wheelPosition) {
  if(wheelPosition < 85) {
   return strip.Color(wheelPosition * 3, 255 - wheelPosition * 3, 0);
  } else if(wheelPosition < 170) {
   wheelPosition -= 85;
   return strip.Color(255 - wheelPosition * 3, 0, wheelPosition * 3);
  } else {
   wheelPosition -= 170;
   return strip.Color(0, wheelPosition * 3, 255 - wheelPosition * 3);
  }
}

void startupMessages() {
  lcd.setCursor(0, 0);
  lcd.print(pad_with_spaces("Terrain Generation"));
  lcd.setCursor(0, 1);
  lcd.print(pad_with_spaces("25% Hills"));
  lcd.setCursor(0, 2);
  lcd.print(pad_with_spaces("10% Water"));
  lcd.setCursor(0, 3);
  lcd.print(pad_with_spaces("31% Percent Trees"));
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print(pad_with_spaces("Reticulating Splines"));
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print(pad_with_spaces(""));
  lcd.setCursor(0, 1);
  lcd.print(pad_with_spaces(""));
  lcd.setCursor(0, 2);
  lcd.print(pad_with_spaces(""));
  lcd.setCursor(0, 3);
  lcd.print(pad_with_spaces(""));
  }

static void swapBacklight() {

  if (backlightOn) {
    lcd.noBacklight();  //open the backlight 
    backlightOn = false;
   }
   else {
    lcd.backlight();
    backlightOn = true;
   }  
}

static void smartDelay(unsigned long ms, float dbValue) {  
  unsigned long start = millis();
  do 
  {
    char customKey = customKeypad.getKey();
    if (customKey){
      lcd.setCursor(0, 2);
      switch(customKey){
        
        // reduce brightness
        // increase brightness
   
        case '1': 
          Serial.println("CALIBRATE UP"); 
          calibration += 0.01;
          EEPROM.write(CALIBRATION_ADDR, calibration * 100);
          lcd.print(pad_with_spaces((String) "       ADJ: " + calibration));
          break;     
        case '4': 
          Serial.println("CALIBRATE DOWN");
          calibration -= 0.01;
          EEPROM.write(CALIBRATION_ADDR, calibration * 100);
          lcd.print(pad_with_spaces((String) "       ADJ: " + calibration));
          break;
        case '3': 
          Serial.println("MAX DB UP");
          upper_db_limit += 1;
          EEPROM.write(UPPER_DB_LIMIT_ADDR, upper_db_limit);
          lcd.print(pad_with_spaces((String) "    MAX DB: " + upper_db_limit));
          break;
        case '6': 
          Serial.println("MAX DB DOWN");  
          upper_db_limit -= 1;
          EEPROM.write(UPPER_DB_LIMIT_ADDR, upper_db_limit);
          lcd.print(pad_with_spaces((String) "    MAX DB: " + upper_db_limit));
          break;
        case '2': 
          Serial.println("MIN DB UP");
          lower_db_limit += 1;
          EEPROM.write(LOWER_DB_LIMIT_ADDR, lower_db_limit);
          lcd.print(pad_with_spaces((String) "    MIN DB: " + lower_db_limit));
          break;
        case '5': 
          Serial.println("MIN DB DOWN");  
          lower_db_limit -= 1;
          EEPROM.write(LOWER_DB_LIMIT_ADDR, lower_db_limit);
          lcd.print(pad_with_spaces((String) "    MIN DB: " + lower_db_limit));
          break;
        case 'A':
          current_min_colour.setColour(255,255,255);
          current_max_colour.setColour(255,0,0);
          break;
        case 'B':
          Serial.println("Green -> Red");  
          current_min_colour.setColour(0,255,0);
          current_max_colour.setColour(255,0,0);
          break;
        case 'C':
          Serial.println("Blue -> White");  
          current_min_colour.setColour(0,0,255);
          current_max_colour.setColour(255,255,255);
          break;
        case 'D':
          Serial.println("Blue -> Red");  
          current_min_colour.setColour(0,0,255);
          current_max_colour.setColour(255,0,0);
          break;
        case '*':
          swapBacklight();
          break;
        case '7': 
          Serial.println("BRIGHTNESS UP");
          brightness = changeValue("up", brightness, brightnessJumpAmount);
          EEPROM.write(BRIGHTNESS_ADDR, brightness);
          strip.setBrightness(brightness);
          lcd.print(pad_with_spaces((String) "BRIGHTNESS: " + brightness));
          break;
        case '8': 
          Serial.println("SETTING BRIGHTNESS TO DEFAULT");
          brightness = defaultBrightness;
          EEPROM.write(BRIGHTNESS_ADDR, brightness);
          strip.setBrightness(brightness);
          lcd.print(pad_with_spaces((String) "BRIGHTNESS: " + brightness));
          break;
        case '9': 
          Serial.println("BRIGHTNESS DOWN");
          brightness = changeValue("down", brightness, brightnessJumpAmount);
          EEPROM.write(BRIGHTNESS_ADDR, brightness);
          strip.setBrightness(brightness);
          lcd.print(pad_with_spaces((String) "BRIGHTNESS: " + brightness));
          break;
        case '0': 
          Serial.println("RAINBOW");
          rainbowCycle(1, 1);
          break;
        case '#': 
          Serial.println("FIRE");
          meteorRain(0xff,0xff,0xff,10, 64, true, 0);
          break;
        default: 
          Serial.println(customKey);
      }
    }
    
    if (dbValue > max_db) {
      max_db = dbValue;
    }
    
    db_total += dbValue;
    loop_counter += 1;
    if (write_delay_counter * ms >= WRITE_FREQUENCY) {

      float average_db = db_total / loop_counter;
      String avg_db = (String) average_db;

      lcd.setCursor( 0, 0);            // go to the top left corner
      lcd.print(pad_with_spaces("Average db: " + avg_db)); // write this string on the top row
      lcd.setCursor( 0, 1);            // go to the top left corner
      lcd.print(pad_with_spaces("    Max db: " + (String) max_db)); // write this string on the top row

      write_delay_counter = 0;
      db_total = 0;
      loop_counter = 0;
      
    }
  } while (millis() - start < ms);
}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
  
  for(int i = 0; i < LED_COUNT+LED_COUNT; i++) {
    
    
    // fade brightness all LEDs one step
    for(int j=0; j<LED_COUNT; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <LED_COUNT) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      } 
    }
   
    strip.show();
    delay(SpeedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
    
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
    
    strip.setPixelColor(ledNo, r,g,b);
 #endif
}

// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < LED_COUNT; i++ ) {
    setPixel(i, red, green, blue); 
  }
  strip.show();
}



void setPixel(int Pixel, byte red, byte green, byte blue) {
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
}


int changeValue(String dir, int value, int jump) {

  if (dir == "up") {
    if (value < 255) {
      if (value + jump >= 255) {
        value = 255;
      }
      else {
        value = value + jump;
      }  
   }
  }
  else {
    if (value > 0) {
      if (value - jump <= 0) {
        value = 0;
      }
      else {
        value = value - jump;
      }  
   }
  }
  
  return value;
}

String pad_with_spaces(String output_to_pad) {
  String empty_screen = "                    ";
  int whitespace_size = empty_screen.length() - output_to_pad.length();
  return output_to_pad + empty_screen.substring(0, whitespace_size);
}

void recursion(int min_pos, int max_pos, Colour colours[33]) {
  if (((max_pos - min_pos) % 2) == 0) {
        int mid = ((max_pos - min_pos) / 2) + min_pos;
        colours[mid] = blend(colours[min_pos], colours[max_pos]);
        recursion(mid, max_pos, colours);
        recursion(min_pos, mid, colours);
  }
}
   
Colour blend(Colour min_color, Colour max_color){
    int r_combined = min_color.getRed() + max_color.getRed();
    int g_combined = min_color.getGreen() + max_color.getGreen();
    int b_combined = min_color.getBlue() + max_color.getBlue();

    int x[3] = {r_combined, g_combined, b_combined};

    long my_max = 0;
    for (int i = 0; i < 3; i++) {
      if (x[i] > my_max) {
        my_max = x[i];
      }
    }

    int blended_red = (long)r_combined * (long)255 / (long)my_max;
    int blended_green = (long)g_combined * (long)255 / (long)my_max;
    int blended_blue = (long)b_combined * (long)255 / (long)my_max;

    return Colour(blended_red, blended_green, blended_blue);
}

void display_sound(int board[ROWS][COLS], float dbValue) {
  strip.clear();
 
  int height = get_height(dbValue);

  int min_pos = 0;
  int max_pos = 32;

  Colour colours[33];
  colours[0] = current_min_colour;
  colours[32] = current_max_colour;

  recursion(min_pos, max_pos, colours);
  
  int cap_size = set_cap(board, height, colours);

  for (int i = 0; i < height - cap_size; i++) {
    Colour colour = colours[i];
    for (int x = 0; x <= 7; x++) {
      strip.setPixelColor(board[i][x], strip.Color(colour.getRed(),colour.getGreen(),colour.getBlue()));
    }
  }
  strip.show(); 
}

int set_cap(int board[ROWS][COLS], int height, Colour colours[33]) {

  int cap_size = 2;
  
  if (height >= 10 and height < 20) {
    cap_size = 3;
  }
  else if (height >= 20 and height <= ROWS) {
    cap_size = 4;
  }

  if ((height - cap_size) < 1) {
    cap_size = 0;
  }

  int indent = 1;
  for (int i = height - cap_size; i < height; i++) {
    Colour colour = colours[i];
    for (int x = 0 + indent; x <= 7 - indent; x++) {
      strip.setPixelColor(board[i][x], strip.Color(colour.getRed(),colour.getGreen(),colour.getBlue()));
    }
    
    if (indent > 2) {
      indent = 3;
    }
    else {
      indent += 1;
    }
  }
  return cap_size;
}

int get_height(float dbValue) {
  
  if (dbValue < lower_db_limit) {
    dbValue = lower_db_limit;
  }  
  else if (dbValue > upper_db_limit) {
    dbValue = upper_db_limit;
  }
  
  int height = round((dbValue - (float) lower_db_limit)*( (float) ROWS /( (float) upper_db_limit - (float) lower_db_limit)));
  return height;
}

void get_board(int board[ROWS][COLS])
{
    board[31][7] = 248;
    board[31][6] = 249;
    board[31][5] = 250;
    board[31][4] = 251;
    board[31][3] = 252;
    board[31][2] = 253;
    board[31][1] = 254;
    board[31][0] = 255;

    board[30][7] = 247;
    board[30][6] = 246;
    board[30][5] = 245;
    board[30][4] = 244;
    board[30][3] = 243;
    board[30][2] = 242;
    board[30][1] = 241;
    board[30][0] = 240;

    board[29][7] = 232;
    board[29][6] = 233;
    board[29][5] = 234;
    board[29][4] = 235;
    board[29][3] = 236;
    board[29][2] = 237;
    board[29][1] = 238;
    board[29][0] = 239;

    board[28][7] = 231;
    board[28][6] = 230;
    board[28][5] = 229;
    board[28][4] = 228;
    board[28][3] = 227;
    board[28][2] = 226;
    board[28][1] = 225;
    board[28][0] = 224;

    board[27][7] = 216;
    board[27][6] = 217;
    board[27][5] = 218;
    board[27][4] = 219;
    board[27][3] = 220;
    board[27][2] = 221;
    board[27][1] = 222;
    board[27][0] = 223;

    board[26][7] = 215;
    board[26][6] = 214;
    board[26][5] = 213;
    board[26][4] = 212;
    board[26][3] = 211;
    board[26][2] = 210;
    board[26][1] = 209;
    board[26][0] = 208;

    board[25][7] = 200;
    board[25][6] = 201;
    board[25][5] = 202;
    board[25][4] = 203;
    board[25][3] = 204;
    board[25][2] = 205;
    board[25][1] = 206;
    board[25][0] = 207;

    board[24][7] = 199;
    board[24][6] = 198;
    board[24][5] = 197;
    board[24][4] = 196;
    board[24][3] = 195;
    board[24][2] = 194;
    board[24][1] = 193;
    board[24][0] = 192;

    board[23][7] = 184;
    board[23][6] = 185;
    board[23][5] = 186;
    board[23][4] = 187;
    board[23][3] = 188;
    board[23][2] = 189;
    board[23][1] = 190;
    board[23][0] = 191;

    board[22][7] = 183;
    board[22][6] = 182;
    board[22][5] = 181;
    board[22][4] = 180;
    board[22][3] = 179;
    board[22][2] = 178;
    board[22][1] = 177;
    board[22][0] = 176;

    board[21][7] = 168;
    board[21][6] = 169;
    board[21][5] = 170;
    board[21][4] = 171;
    board[21][3] = 172;
    board[21][2] = 173;
    board[21][1] = 174;
    board[21][0] = 175;

    board[20][7] = 167;
    board[20][6] = 166;
    board[20][5] = 165;
    board[20][4] = 164;
    board[20][3] = 163;
    board[20][2] = 162;
    board[20][1] = 161;
    board[20][0] = 160;

    board[19][7] = 152;
    board[19][6] = 153;
    board[19][5] = 154;
    board[19][4] = 155;
    board[19][3] = 156;
    board[19][2] = 157;
    board[19][1] = 158;
    board[19][0] = 159;

    board[18][7] = 151;
    board[18][6] = 150;
    board[18][5] = 149;
    board[18][4] = 148;
    board[18][3] = 147;
    board[18][2] = 146;
    board[18][1] = 145;
    board[18][0] = 144;

    board[17][7] = 136;
    board[17][6] = 137;
    board[17][5] = 138;
    board[17][4] = 139;
    board[17][3] = 140;
    board[17][2] = 141;
    board[17][1] = 142;
    board[17][0] = 143;

    board[16][7] = 135;
    board[16][6] = 134;
    board[16][5] = 133;
    board[16][4] = 132;
    board[16][3] = 131;
    board[16][2] = 130;
    board[16][1] = 129;
    board[16][0] = 128;

    board[15][7] = 120;
    board[15][6] = 121;
    board[15][5] = 122;
    board[15][4] = 123;
    board[15][3] = 124;
    board[15][2] = 125;
    board[15][1] = 126;
    board[15][0] = 127;

    board[14][7] = 119;
    board[14][6] = 118;
    board[14][5] = 117;
    board[14][4] = 116;
    board[14][3] = 115;
    board[14][2] = 114;
    board[14][1] = 113;
    board[14][0] = 112;

    board[13][7] = 104;
    board[13][6] = 105;
    board[13][5] = 106;
    board[13][4] = 107;
    board[13][3] = 108;
    board[13][2] = 109;
    board[13][1] = 110;
    board[13][0] = 111;

    board[12][7] = 103;
    board[12][6] = 102;
    board[12][5] = 101;
    board[12][4] = 100;
    board[12][3] = 99;
    board[12][2] = 98;
    board[12][1] = 97;
    board[12][0] = 96;

    board[11][7] = 88;
    board[11][6] = 89;
    board[11][5] = 90;
    board[11][4] = 91;
    board[11][3] = 92;
    board[11][2] = 93;
    board[11][1] = 94;
    board[11][0] = 95;

    board[10][7] = 87;
    board[10][6] = 86;
    board[10][5] = 85;
    board[10][4] = 84;
    board[10][3] = 83;
    board[10][2] = 82;
    board[10][1] = 81;
    board[10][0] = 80;

    board[9][7] = 72;
    board[9][6] = 73;
    board[9][5] = 74;
    board[9][4] = 75;
    board[9][3] = 76;
    board[9][2] = 77;
    board[9][1] = 78;
    board[9][0] = 79;

    board[8][7] = 71;
    board[8][6] = 70;
    board[8][5] = 69;
    board[8][4] = 68;
    board[8][3] = 67;
    board[8][2] = 66;
    board[8][1] = 65;
    board[8][0] = 64;

    board[7][7] = 56;
    board[7][6] = 57;
    board[7][5] = 58;
    board[7][4] = 59;
    board[7][3] = 60;
    board[7][2] = 61;
    board[7][1] = 62;
    board[7][0] = 63;

    board[6][7] = 55;
    board[6][6] = 54;
    board[6][5] = 53;
    board[6][4] = 52;
    board[6][3] = 51;
    board[6][2] = 50;
    board[6][1] = 49;
    board[6][0] = 48;

    board[5][7] = 40;
    board[5][6] = 41;
    board[5][5] = 42;
    board[5][4] = 43;
    board[5][3] = 44;
    board[5][2] = 45;
    board[5][1] = 46;
    board[5][0] = 47;

    board[4][7] = 39;
    board[4][6] = 38;
    board[4][5] = 37;
    board[4][4] = 36;
    board[4][3] = 35;
    board[4][2] = 34;
    board[4][1] = 33;
    board[4][0] = 32;

    board[3][7] = 24;
    board[3][6] = 25;
    board[3][5] = 26;
    board[3][4] = 27;
    board[3][3] = 28;
    board[3][2] = 29;
    board[3][1] = 30;
    board[3][0] = 31;

    board[2][7] = 23;
    board[2][6] = 22;
    board[2][5] = 21;
    board[2][4] = 20;
    board[2][3] = 19;
    board[2][2] = 18;
    board[2][1] = 17;
    board[2][0] = 16;

    board[1][7] = 8;
    board[1][6] = 9;
    board[1][5] = 10;
    board[1][4] = 11;
    board[1][3] = 12;
    board[1][2] = 13;
    board[1][1] = 14;
    board[1][0] = 15;

    board[0][7] = 7;
    board[0][6] = 6;
    board[0][5] = 5;
    board[0][4] = 4;
    board[0][3] = 3;
    board[0][2] = 2;
    board[0][1] = 1;
    board[0][0] = 0;
}


void read_from_eeprom() {

  Serial.println("Searching memory for matrix lower limit value.");
  if (EEPROM.read(LOWER_DB_LIMIT_ADDR) <= 255) {
    Serial.print("Value found! Using value from memory: ");
    lower_db_limit = EEPROM.read(LOWER_DB_LIMIT_ADDR);
    Serial.println(lower_db_limit);
  }
  else {
    Serial.print("No value found in memory, using default value: ");
    Serial.println(lower_db_limit);
  }

  Serial.println("Searching memory for matrix upper limit value.");
  if (EEPROM.read(UPPER_DB_LIMIT_ADDR) <= 255) {
    Serial.print("Value found! Using value from memory: ");
    upper_db_limit = EEPROM.read(UPPER_DB_LIMIT_ADDR);
    Serial.println(upper_db_limit);
  }
  else {
    Serial.print("No value found in memory, using default value: ");
    Serial.println(upper_db_limit);
  }
  
  Serial.println("Searching memory for sound level calibration value.");
  if (EEPROM.read(CALIBRATION_ADDR) <= 255 and EEPROM.read(CALIBRATION_ADDR) >= 50) {
    Serial.print("Value found! Using value from memory: ");
    calibration = (float) EEPROM.read(CALIBRATION_ADDR) / 100;
    Serial.println(calibration);
  }
  else {
    Serial.print("No value found in memory, using default value: ");
    Serial.println(calibration);
  }

  Serial.println("Searching memory for brightness level value.");
  if (EEPROM.read(BRIGHTNESS_ADDR) <= 255 and EEPROM.read(BRIGHTNESS_ADDR) >= 0) {
    Serial.print("Value found! Using value from memory: ");
    brightness = EEPROM.read(BRIGHTNESS_ADDR);
    Serial.println(brightness);
  }
  else {
    Serial.print("No value found in memory, using default value: ");
    brightness = defaultBrightness;
    Serial.println(brightness);
  }


  Serial.println("Memory read complete, have a nice day.");
}
