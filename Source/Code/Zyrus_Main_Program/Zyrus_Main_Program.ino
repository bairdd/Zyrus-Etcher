/*
 * -------ZYRUS ETCHER CODE BY BAIRD BANKOVIC BDB5454@PSU.EDU-------
 _______  ______  __  _______    ____________________  ____________ 
/__  /\ \/ / __ \/ / / / ___/   / ____/_  __/ ____/ / / / ____/ __ \
  / /  \  / /_/ / / / /\__ \   / __/   / / / /   / /_/ / __/ / /_/ /
 / /__ / / _, _/ /_/ /___/ /  / /___  / / / /___/ __  / /___/ _, _/ 
/____//_/_/ |_|\____//____/  /_____/ /_/  \____/_/ /_/_____/_/ |_|  
 */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Fonts/Orbitron_Medium_28.h>
#include <Fonts/Orbitron_Medium_10.h>



//DISPLAY:
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#define LIGHT_BLUE     0x73DD

const int PLOT_WIDTH = 128;    // Width of the plot area
const int PLOT_HEIGHT = 160;   // Height of the plot area
const int PLOT_MARGIN = 10;    // Margin around the plot area
const int PLOT_BOTTOM = tft.height() - PLOT_MARGIN;  // Bottom coordinate of the plot area

int plotData[PLOT_WIDTH];      // Array to store the plot data
int plotIndex = 0;             // Current index in the plot data

//

//BUTTONS:
const int up_button = 28;
const int down_button = 29;
const int enter_button = 30;
//

//RELAYS:
const int jump_start_relay = 23;
const int nozzle_relay = 24;
const int wire_relay = 25;
//

//BUZZER:
const int buzzer = 2;
//

//STEPPER MOTORS:
const int dir_e = 31;
const int step_e = 32;
const int ms1_e = 33;
const int ms2_e = 34;
const int enable_e = 35;
const int dir_z = 14;
const int step_z = 37;
const int ms1_z = 15;
const int ms2_z = 39;
const int enable_z = 40;
const int steps_per_mm_z = 1600;
const int steps_per_mm_e = 110;
const int speed_z = 80; //arbitrary units, larger is slower
const int speed_e = 1000; //arbitrary units, larger is slower
//

//MISSILANIOUS PINS:
const int nozzle_homing = 43;
const int wire_homing = 44;
const int attenuated_voltage_divider = A2;
const int nozzle_homing_analog = A4;
const int wire_homing_analog = A3;
//

//MISSLILANIOUS PROGRAM VAIRABLES:
int main_menu_selection = 1;
bool first_run = true;
bool selection_choosen = false;
bool manual_mode_used = false;
bool nozzle_relay_state = false;
bool jump_start_relay_state = false;
bool wire_relay_state = false;
float wire_submersion_distance = 2;
float nozzle_above_etchant_distance = 4;
float etch_current_gain = 0.0288; //to milliamps
bool redip = true;
//

void setup(void) {
  randomSeed(analogRead(A7));
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));
  pinMode(enter_button,INPUT_PULLUP);
  pinMode(up_button,INPUT_PULLUP);
  pinMode(down_button,INPUT_PULLUP);
  pinMode(nozzle_homing,INPUT_PULLUP);
  pinMode(wire_homing,INPUT_PULLUP);
  pinMode(attenuated_voltage_divider,INPUT);
  pinMode(nozzle_homing_analog,INPUT);
  pinMode(wire_homing_analog,INPUT);
  pinMode(jump_start_relay,OUTPUT);
  pinMode(nozzle_relay,OUTPUT);
  pinMode(wire_relay,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(dir_z,OUTPUT);
  pinMode(step_z,OUTPUT);
  pinMode(ms1_z,OUTPUT);
  pinMode(ms2_z,OUTPUT);
  pinMode(enable_z,OUTPUT);
  pinMode(dir_e,OUTPUT);
  pinMode(step_e,OUTPUT);
  pinMode(ms1_e,OUTPUT);
  pinMode(ms2_e,OUTPUT);
  pinMode(enable_e,OUTPUT);
  digitalWrite(ms1_z,HIGH); //use 1/16th microstepping
  digitalWrite(ms2_z,HIGH);
  digitalWrite(ms1_e,HIGH);
  digitalWrite(ms2_e,HIGH);
  digitalWrite(enable_z,LOW); //enable motors
  digitalWrite(enable_e,LOW);
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(3);  // Landscape Mode
  Serial.println(F("Initialized"));
  
  //Zyrus Startup Screen
  tft.fillScreen(ST77XX_BLACK);
  tft.setFont(&Orbitron_Medium_28);
  tft.setCursor(20,40);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("ZYRUS");
  tft.setCursor(10,68);
  tft.print("ETCHER");
  tft.setFont(&Orbitron_Medium_10);
  tft.setTextColor(LIGHT_BLUE);
  tft.setCursor(24,78);
  tft.print("BAIRD BANKOVIC");
  tft.setCursor(40,90);
  tft.print("MK2, 2023");
  tft.drawLine(0,0,160,0,ST77XX_WHITE);
  tft.drawLine(0,3,160,3,ST77XX_WHITE);
  tft.drawLine(0,127,160,127,ST77XX_WHITE);
  tft.drawLine(0,124,160,124,ST77XX_WHITE);
  //Inialize all devices:
  digitalWrite(ms1_z,HIGH); //use 1/16th microstepping
  digitalWrite(ms2_z,HIGH);
  digitalWrite(ms1_e,HIGH);
  digitalWrite(ms2_e,HIGH);
  digitalWrite(enable_z,LOW); //enable motors
  digitalWrite(enable_e,LOW);
  //Startup Tone
  tone(buzzer,523,120);
  delay(120);
  tone(buzzer,659,120);
  delay(120);
  tone(buzzer,1047,120);
  delay(120);
  homez();
/*
  //After Startup, Wait for User:
  digitalWrite(wire_relay,HIGH);
  digitalWrite(nozzle_relay,HIGH);
  bool a=false;
  while(a==false){
    tft.setCursor(10,122);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("REMOVE DISH, HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
    tft.setCursor(10,122);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("REMOVE DISH, HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
  }
  */
  homee();
  tone(buzzer,2000,50);
  tone(buzzer,4000,10);
}

void loop() {
  tft.drawLine(0,9,35,9,ST77XX_WHITE);
  tft.drawLine(125,9,160,9,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(48,14);
  tft.print("Main Menu");
  //selection loop
  if((Up()==true) || (Down()==true) || (Enter()==true) || (first_run == true)){
    first_run = false;
    Serial.println("In Main Menu Update Loop");
    if(Up()==true){
      while(Up()==true){} //debounce
      tone(buzzer,4000,1);
      if(main_menu_selection == 1){
        main_menu_selection = 5;
      }
      main_menu_selection = main_menu_selection-1;
      Serial.print("Main Menu: ");
      Serial.println(main_menu_selection);
    }
    if(Down()==true){
      while(Down()==true){} //debounce
      tone(buzzer,4000,1);
      if(main_menu_selection == 4){
        main_menu_selection = 0;
      }
      main_menu_selection = main_menu_selection+1;
      Serial.print("Main Menu: ");
      Serial.println(main_menu_selection);
    }
    if(Enter()==true){
      while(Enter()==true){} //debounce
      tone(buzzer,4000,1);
      selection_choosen = true;
    }
    switch(main_menu_selection){
      Serial.println("Update Switch");
    case 1:
      //Etch
      tft.fillRect(0,17,160,16,ST77XX_WHITE);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(3,30);
      tft.print("ETCH");
      //Manual Move
      tft.fillRect(0,33,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,46);
      tft.print("MANUAL MOVE");
      //Shutdown
      tft.fillRect(0,65,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,78);
      tft.print("SHUTDOWN");
      //Information
      tft.fillRect(0,49,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,62);
      tft.print("INFORMATION");
      if(selection_choosen == true){
        etch();
      }
      break;
    case 2:
      //Etch
      tft.fillRect(0,17,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,30);
      tft.print("ETCH");
      //Manual Move
      tft.fillRect(0,33,160,16,ST77XX_WHITE);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(3,46);
      tft.print("MANUAL MOVE");
      //shutdown
      tft.fillRect(0,65,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,78);
      tft.print("SHUTDOWN");
      //Inormatin
      tft.fillRect(0,49,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,62);
      tft.print("INFORMATION");
      if(selection_choosen == true){
        manual_move();
      }
      break;
    case 3:
      //Etch
      tft.fillRect(0,17,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,30);
      tft.print("ETCH");
      //Manual Move
      tft.fillRect(0,33,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,46);
      tft.print("MANUAL MOVE");
      //Information
      tft.fillRect(0,49,160,16,ST77XX_WHITE);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(3,62);
      tft.print("INFORMATION");
      //Shut_down
      tft.fillRect(0,65,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,78);
      tft.print("SHUTDOWN");
      if(selection_choosen == true){
        information();
      }
      break;
    case 4:
      //Etch
      tft.fillRect(0,17,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,30);
      tft.print("ETCH");
      //Manual Move
      tft.fillRect(0,33,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,46);
      tft.print("MANUAL MOVE");
      //Information
      tft.fillRect(0,49,160,16,ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(3,62);
      tft.print("INFORMATION");
      //Shut_down
      tft.fillRect(0,65,160,16,ST77XX_WHITE);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(3,78);
      tft.print("SHUTDOWN");
      if(selection_choosen == true){
        shut_down();
      }
      break;
  }
  }
}

bool Enter(){
  if(digitalRead(enter_button)==HIGH){
    return false;
  }
  if(digitalRead(enter_button)==LOW){
    return true;
  }
}

bool Up(){
  if(digitalRead(up_button)==HIGH){
    return false;
  }
  if(digitalRead(up_button)==LOW){
    return true;
  }
}

bool Down(){
  if(digitalRead(down_button)==HIGH){
    return false;
  }
  if(digitalRead(down_button)==LOW){
    return true;
  }
}

void zmove(float distance,char *rate){
  int zmove_pulse_delay;
  if(distance > 0){ //move up
    digitalWrite(dir_z,HIGH);
  }
  if(distance < 0){ //move down
    digitalWrite(dir_z,LOW);
  }
  long steps = floor(abs(distance)*steps_per_mm_z);
  if(rate == "slow"){
    zmove_pulse_delay = 800;
  }
  if(rate == "normal"){
    zmove_pulse_delay = 300;
  }
  if(rate == "plad"){
    zmove_pulse_delay = 50;
  }
  
  for(long i = 0;i<steps;i++){
    digitalWrite(step_z,HIGH);
    delayMicroseconds(zmove_pulse_delay);
    digitalWrite(step_z,LOW);
    delayMicroseconds(zmove_pulse_delay);
  }
}

void emove(float distance){
  if(distance > 0){
    digitalWrite(dir_e,LOW);
  }
  if(distance < 0){
    digitalWrite(dir_e,HIGH);
  }
  long steps = floor(abs(distance)*steps_per_mm_e);
  for(int i = 0;i<=steps;i++){
    digitalWrite(step_e,HIGH);
    delayMicroseconds(speed_e);
    digitalWrite(step_e,LOW);
    delayMicroseconds(speed_e);
  }
}

void clear_screen(){
  for(int i=0;i<160;i++){
    tft.fillRect(i,0,2,128,ST77XX_BLACK);
  }
}

void etch(){
  clear_screen();
  tft.drawLine(0,9,35,9,ST77XX_WHITE);
  tft.drawLine(125,9,160,9,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(42,14);
  tft.print("WIRE ETCH");
  if(manual_mode_used == true){ //rehome
    bool a=false;
    while(a==false){
      tft.setCursor(10,122);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("REMOVE DISH, HOME");
      for(unsigned int i=0;i<60000;i++){
        if(Enter()==true){
          a=true;
        }
      }
      tft.setCursor(10,122);
      tft.setTextColor(ST77XX_BLACK);
      tft.print("REMOVE DISH, HOME");
      for(unsigned int i=0;i<60000;i++){
        if(Enter()==true){
          a=true;
        }
      }
    }
    tone(buzzer,4000,1);
    emove(-2);
    homez();
    zmove(1,"normal");
    homee();
    emove(-1);
    tone(buzzer,2000,50);
    tone(buzzer,4000,10);
    //zmove(39,"plad");
    zmove(60,"plad");
  }
  emove(wire_submersion_distance+nozzle_above_etchant_distance);
    
  clear_screen();
  tft.drawLine(0,9,35,9,ST77XX_WHITE);
  tft.drawLine(125,9,160,9,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(42,14);
  tft.print("WIRE ETCH");
  //1.Make wire flush with nozzle//////////////////////////
  tft.fillCircle(5,26,3,ST77XX_YELLOW);
  tft.setCursor(10,30);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("STREIGHTEN WIRE");
  tft.setCursor(10,40);
  tft.print("WITH TWEEZERS");
  tft.fillCircle(5,48,3,ST77XX_YELLOW);
  tft.setCursor(10,52);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("INSERT DISH");
  tft.fillCircle(5,70,3,ST77XX_YELLOW);
  tft.setCursor(10,74);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Submersion Dist: ");
  tft.print(wire_submersion_distance,2);
  tft.fillCircle(5,92,3,ST77XX_YELLOW);
  tft.setCursor(10,96);
  tft.print("Redip Truth: ");
  tft.print(redip);
  bool a;
  a=false;
  while(a==false){
    tft.setCursor(32,122);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("PRESS HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
    tft.setCursor(32,122);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("PRESS HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
  }
  tone(buzzer,4000,1);
  zmove(-48,"plad");
  homez_against_e();
  zmove(-1*wire_submersion_distance,"slow");
  if(redip == true){
    zmove(-1,"slow");
    zmove(1,"slow"); //wets the upper portion of the wire to aleviate surface tension
  }
  //start etch
  digitalWrite(jump_start_relay,HIGH);
  jump_start_relay_state = true;
  delay(2000);
  digitalWrite(jump_start_relay,LOW);
  jump_start_relay_state = false;
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /*
  clear_screen();
  int analog_reading = 1023;
  unsigned long etch_start_time = millis();
  float current_etch_time = 0;
  float etch_current = 0;
  int number_of_refreshes = 5; //has to be less than 400
  unsigned int array_length = number_of_refreshes*160;
  int dataArray[array_length] = {};
  //inialize array with all 126 so that it is on the bottom of the screen
  for(int i = 0; i<array_length;i++){
    dataArray[i]=126;
  }
  unsigned int number_of_points = 0;
  int x=0;
  int y;
  int indexing = 1; //fill screen first
  while((x<=160)&&(Enter() == false) && (analog_reading>2)){ //if true, we have not filled the screen yet, continue plotting
      tft.fillRect(0,0,160,12,ST7735_BLACK);
      analog_reading = analogRead(attenuated_voltage_divider);
      Serial.println(analog_reading);
      int y = 126-map(analog_reading, 0, 1023,0,128);
      Serial.print("y: ");
      Serial.println(y);
      dataArray[number_of_points] = y;
      number_of_points++;
      x++;
      tft.drawPixel(x, y, ST7735_WHITE);
      //update text:
      current_etch_time = (millis()-etch_start_time)*0.0000167;
      etch_current = analog_reading*etch_current_gain;
      tft.setCursor(0,12);
      tft.print(current_etch_time);
      tft.print(" min");
      tft.setCursor(80,12);
      tft.print(etch_current,0);
      tft.print(" mA");
      delay(1000);
    }
  x=0;
  indexing++;
  Serial.print("Indexing Changed: ");
  Serial.println(indexing);
  while((Enter() == false) && (analog_reading>2)){
    tft.fillScreen(ST7735_BLACK);  // Clear the screen
    float scale = number_of_points/106;
    int next_pixel;
    for(int j=0; j<106; j++){  //draw previous data after screen refresh
      x++;
      next_pixel = floor(x*scale);
      y = dataArray[next_pixel];
      tft.drawPixel(x, y, ST7735_WHITE);
      y=0;
    }
    Serial.println("Done Drawing History");
    while((x<=160)  && (analog_reading>2)){ //if true, we have not filled the screen yet, continue plotting
      tft.fillRect(0,0,160,12,ST7735_BLACK);
      analog_reading = analogRead(attenuated_voltage_divider);
      Serial.println(analog_reading);
      int y = 126-map(analog_reading, 0, 1023,0,128);
      dataArray[number_of_points] = y;
      number_of_points++;
      x++;
      tft.drawPixel(x, y, ST7735_WHITE);
      //update text:
      current_etch_time = (millis()-etch_start_time)*0.0000167;
      etch_current = analog_reading*etch_current_gain;
      tft.setCursor(0,12);
      tft.print(current_etch_time);
      tft.print(" min");
      tft.setCursor(80,12);
      tft.print(etch_current,0);
      tft.print(" mA");
      y=0;
      delay(1000);
    }
    x=0;
    indexing++;
    Serial.print("Indexing Changed: ");
    Serial.println(indexing);
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  */
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Simple Linear plotting:
  clear_screen();
  int analog_reading = 1023;
  unsigned long etch_start_time = millis();
  float current_etch_time = 0;
  float etch_current;
  int y1;
  int y2;
  int x1;
  int x2;
  //inital reading
  analog_reading = analogRead(attenuated_voltage_divider);
  etch_current = analog_reading*etch_current_gain;
  y1 = 126-map(analog_reading, 0, 1023,0,128);
  x1 = 0;
  while((etch_current > 0) && (Enter() == false)){
    //Clear previous text:
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(0,12);
    tft.print(current_etch_time);
    tft.print(" min");
    tft.setCursor(80,12);
    tft.print(etch_current,0);
    tft.print(" mA");
    analog_reading = analogRead(attenuated_voltage_divider);
    y2 = 126-map(analog_reading, 0, 1023,0,128);
    x2 = x1+1;    
    //draw line
    tft.drawLine(x1, y1, x2, y2, ST7735_WHITE);
    //update text:
    current_etch_time = (millis()-etch_start_time)*0.0000167;
    etch_current = analog_reading*etch_current_gain;
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0,12);
    tft.print(current_etch_time);
    tft.print(" min");
    tft.setCursor(80,12);
    tft.print(etch_current,0);
    tft.print(" mA");
    //reset points for new line to be drawn:
    y1 = y2;
    x1 = x2;
    delay(7500);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  tone(buzzer,2000,1000);
  while(Enter()==true){}//debounce
  //prepair for main loop again
  main_menu_selection = 1;
  first_run = true;
  selection_choosen = false;
  //etching finished
  zmove(40,"normal");
  emove(10);
  a = false;
  while(a==false){
    for(unsigned long i=0;i<600000;i++){
      if(Enter()==true){
        tone(buzzer,4000,10);
        a=true;
      }
    }
    tone(buzzer,4000,100);
    delay(200);
    tone(buzzer,2000,100);
  }
  tone(buzzer,4000,10);
  if(Enter()==true){
    while(Enter()==true){}//debouncing
  }
  clear_screen();


  
  //prepair for main loop again
  main_menu_selection = 4;
  first_run = true;
  selection_choosen = false;
  clear_screen();

}

void manual_move(){
  manual_mode_used = true;
  int axis_mode = 1;
  int count = 0;
  bool first_run_manual_move = true;
  clear_screen();
  tft.drawLine(0,5,25,5,ST77XX_WHITE);
  tft.drawLine(135,5,160,5,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(30,10);
  tft.print("MANUAL MOVE");
  tft.setCursor(15,128);
  tft.print("LONG PRESS HOME");
  
  while(count < 1500){
    //update display:
    //first, clear rectangle screen section to avoid confusion
    tft.drawRect(0,16,160,100,ST77XX_BLACK);
    tft.fillRect(0,16,160,100,ST77XX_BLACK);
    if(axis_mode==1){
      tft.fillRect(0,16,80,50,ST77XX_YELLOW);
      tft.drawRect(80,16,80,50,ST77XX_WHITE);
      tft.drawRect(0,66,80,50,ST77XX_WHITE);
      tft.drawRect(80,66,80,50,ST77XX_WHITE);
      tft.setFont(&Orbitron_Medium_28);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(25,49);
      tft.print("Z");
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(105,49);
      tft.print("E");
      if(nozzle_relay_state == true){
        tft.setTextColor(ST77XX_MAGENTA);
      }else{
        tft.setTextColor(ST77XX_WHITE); 
      }
      tft.setFont(&Orbitron_Medium_10);
      tft.setCursor(12,89);
      tft.print("NOZZLE");
      tft.setCursor(16,99);
      tft.print("RELAY");
      if(jump_start_relay_state == true){
        tft.setTextColor(ST77XX_MAGENTA);
      }else{
        tft.setTextColor(ST77XX_WHITE); 
      }
      tft.setFont(&Orbitron_Medium_10);
      tft.setCursor(100,84);
      tft.print("JUMP");
      tft.setCursor(98,94);
      tft.print("START");
      tft.setCursor(98,104);
      tft.print("RELAY");
    }
    if(axis_mode==2){
      tft.drawRect(0,16,80,50,ST77XX_WHITE);
      tft.fillRect(80,16,80,50,ST77XX_YELLOW);
      tft.drawRect(0,66,80,50,ST77XX_WHITE);
      tft.drawRect(80,66,80,50,ST77XX_WHITE);
      tft.setFont(&Orbitron_Medium_28);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(25,49);
      tft.print("Z");
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(105,49);
      tft.print("E");
      if(nozzle_relay_state == true){
        tft.setTextColor(ST77XX_MAGENTA);
      }else{
        tft.setTextColor(ST77XX_WHITE); 
      }
      tft.setFont(&Orbitron_Medium_10);
      tft.setCursor(12,89);
      tft.print("NOZZLE");
      tft.setCursor(16,99);
      tft.print("RELAY");
      if(jump_start_relay_state == true){
        tft.setTextColor(ST77XX_MAGENTA);
      }else{
        tft.setTextColor(ST77XX_WHITE); 
      }
      tft.setFont(&Orbitron_Medium_10);
      tft.setCursor(100,84);
      tft.print("JUMP");
      tft.setCursor(98,94);
      tft.print("START");
      tft.setCursor(98,104);
      tft.print("RELAY");
    }
    if(axis_mode==3){
      tft.drawRect(0,16,80,50,ST77XX_WHITE);
      tft.drawRect(80,16,80,50,ST77XX_WHITE);
      tft.fillRect(0,66,80,50,ST77XX_YELLOW);
      tft.fillRect(80,66,80,50,ST77XX_YELLOW);
      tft.setFont(&Orbitron_Medium_28);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(25,49);
      tft.print("Z");
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(105,49);
      tft.print("E");
      if(nozzle_relay_state == true){
        tft.setTextColor(ST77XX_MAGENTA);
      }else{
        tft.setTextColor(ST77XX_BLACK); 
      }
      tft.setFont(&Orbitron_Medium_10);
      tft.setCursor(12,89);
      tft.print("NOZZLE");
      tft.setCursor(16,99);
      tft.print("RELAY");
      if(jump_start_relay_state == true){
        tft.setTextColor(ST77XX_MAGENTA);
      }else{
        tft.setTextColor(ST77XX_BLACK); 
      }
      tft.setFont(&Orbitron_Medium_10);
      tft.setCursor(100,84);
      tft.print("JUMP");
      tft.setCursor(98,94);
      tft.print("START");
      tft.setCursor(98,104);
      tft.print("RELAY");
      tft.drawLine(80,66,80,116,ST77XX_BLACK);
      tft.drawLine(79,66,79,116,ST77XX_BLACK);
    }
  switch(axis_mode){
    case 1: //z
      while(Enter()==false){
        while(Up()==true){
            digitalWrite(dir_z,HIGH);
            digitalWrite(step_z,HIGH);
            delayMicroseconds(speed_z);
            digitalWrite(step_z,LOW);
            delayMicroseconds(speed_z);
          }
          while(Down()==true){
            digitalWrite(dir_z,LOW);
            digitalWrite(step_z,HIGH);
            delayMicroseconds(speed_z);
            digitalWrite(step_z,LOW);
            delayMicroseconds(speed_z);
          }
      }
      break;
    case 2: //e
      while(Enter()==false){
        while(Up()==true){
            digitalWrite(dir_e,HIGH);
            digitalWrite(step_e,HIGH);
            delayMicroseconds(speed_e);
            digitalWrite(step_e,LOW);
            delayMicroseconds(speed_e);
          }
          while(Down()==true){
            digitalWrite(dir_e,LOW);
            digitalWrite(step_e,HIGH);
            delayMicroseconds(speed_e);
            digitalWrite(step_e,LOW);
            delayMicroseconds(speed_e);
          }
      }
      break;
    case 3: //relays
      while(Enter()==false){
        if(Up()==true){ //change state of jumpstart relay
          while(Up()==true){}//debounce
          if(jump_start_relay_state == false){
            digitalWrite(jump_start_relay,HIGH);
            jump_start_relay_state = true;
          }else{
            digitalWrite(jump_start_relay,LOW);
            jump_start_relay_state = false;
          }
          Serial.print("Jump Start Relay State: ");
          Serial.println(jump_start_relay_state);
          if(nozzle_relay_state == true){
            tft.setTextColor(ST77XX_MAGENTA);
          }else{
            tft.setTextColor(ST77XX_BLACK); 
          }
          tft.setFont(&Orbitron_Medium_10);
          tft.setCursor(12,89);
          tft.print("NOZZLE");
          tft.setCursor(16,99);
          tft.print("RELAY");
          if(jump_start_relay_state == true){
            tft.setTextColor(ST77XX_MAGENTA);
          }else{
            tft.setTextColor(ST77XX_BLACK); 
          }
          tft.setFont(&Orbitron_Medium_10);
          tft.setCursor(100,84);
          tft.print("JUMP");
          tft.setCursor(98,94);
          tft.print("START");
          tft.setCursor(98,104);
          tft.print("RELAY");  
          delay(200);
        }
        if(Down()==true){ //change state of nozzle relay
          while(Down()==true){}//debounce
          if(nozzle_relay_state == false){
            digitalWrite(nozzle_relay,HIGH);
            nozzle_relay_state = true;
          }else{
            digitalWrite(nozzle_relay,LOW);
            nozzle_relay_state = false;
          }
          Serial.print("Nozzle Relay State: ");
          Serial.println(nozzle_relay_state);
          if(nozzle_relay_state == true){
            tft.setTextColor(ST77XX_MAGENTA);
          }else{
            tft.setTextColor(ST77XX_BLACK); 
          }
          tft.setFont(&Orbitron_Medium_10);
          tft.setCursor(12,89);
          tft.print("NOZZLE");
          tft.setCursor(16,99);
          tft.print("RELAY");
          if(jump_start_relay_state == true){
            tft.setTextColor(ST77XX_MAGENTA);
          }else{
            tft.setTextColor(ST77XX_BLACK); 
          }
          tft.setFont(&Orbitron_Medium_10);
          tft.setCursor(100,84);
          tft.print("JUMP");
          tft.setCursor(98,94);
          tft.print("START");
          tft.setCursor(98,104);
          tft.print("RELAY");
          delay(200);
        }
      }
      break;
  }
  count = 0;
  while((Enter()==true) && (count < 1500)){
    count++;
    delay(1);
  }
  if(Enter()==true){
    tone(buzzer,4000,20);
    while(Enter()==true){}
  } //debounce
  tone(buzzer,4000,1);
  if(count<1500){ //change modes
    Serial.print("Axis Mode Change, Inital Mode: ");
    Serial.println(axis_mode);
    if(axis_mode == 3){
      axis_mode = 1;
    }else{
      axis_mode++;
    }
    Serial.print("New Mode: ");
    Serial.println(axis_mode);
  }
  }



  
  //prepair for main loop again
  main_menu_selection = 4;
  first_run = true;
  selection_choosen = false;
  manual_mode_used = true;
  clear_screen();
}

void shut_down(){
  clear_screen();
  tft.drawLine(0,9,35,9,ST77XX_WHITE);
  tft.drawLine(125,9,160,9,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(42,14);
  tft.print("SHUTDOWN");
  tft.fillCircle(5,26,3,ST77XX_YELLOW);
  tft.setCursor(10,30);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("MAKE WIRE FLUSH ");
  tft.setCursor(10,40);
  tft.print("WITH NOZZLE");
  tft.fillCircle(5,48,3,ST77XX_YELLOW);
  tft.setCursor(10,52);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("CLEAN ETCHING DISH,");
  tft.setCursor(10,62);
  tft.print("INSERT INTO BASE");
  bool a=false;
  while(a==false){
    tft.setCursor(32,122);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("PRESS HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
    tft.setCursor(32,122);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("PRESS HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
  }
  tone(buzzer,4000,1);
  emove(-1);
  tft.setCursor(50,122);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("HOMING...");
  //Start Screen now cleared, now home the nozzle
  homez();
  zmove(-68,"plad");
  tone(buzzer,4000,5);
  clear_screen();
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(20,12);
  tft.print("DO NOT STORE IN");
  tft.setCursor(25,22);
  tft.print("IN FUME HOOD!!!");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10,38);
  tft.print("MOVE NOZZLE TILL");
  tft.setCursor(28,48);
  tft.print("DISH IS FIRM");
  while(true){
      while(Up()==true){
            digitalWrite(dir_z,HIGH);
            digitalWrite(step_z,HIGH);
            delayMicroseconds(800);
            digitalWrite(step_z,LOW);
            delayMicroseconds(800);
      }
      while(Down()==true){
            digitalWrite(dir_z,LOW);
            digitalWrite(step_z,HIGH);
            delayMicroseconds(800);
            digitalWrite(step_z,LOW);
            delayMicroseconds(800);
      }
    tone(buzzer,4000,1);
    tft.setCursor(25,79);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("REMOVE POWER");
    int count = 0;
    while((Up()==false) && (Down()==false) && (count<800)){
      delay(1);
      count++;
    }
    tft.setCursor(25,79);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("REMOVE POWER");
    count = 0;
    while((Up()==false) && (Down()==false) && (count<800)){
      delay(1);
      count++;
    }
  }
}

void information(){
  clear_screen();
  tft.drawLine(0,5,15,5,ST77XX_WHITE);
  tft.drawLine(145,5,160,5,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(20,10);
  tft.print("PARAMETER EDIT");
  tft.setCursor(15,128);
  tft.print("LONG PRESS HOME");
  tft.setCursor(0,60);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Wire Submersion (mm):");
  tft.setCursor(0,70);
  tft.print(wire_submersion_distance,2);
  tft.setCursor(0,80);
  tft.print("Redip Truth: ");
  tft.setCursor(0,90);
  tft.print(redip);
  int mode = 0;
  int parameter_mode = 0;
  int count = 0;
  while(count < 1500){
    if(mode == 0){
        if(Up()==true){
          tone(buzzer,4000,1);
          tft.setTextColor(ST77XX_BLACK);
          tft.setCursor(0,70);
          tft.print(wire_submersion_distance,2);
          wire_submersion_distance = wire_submersion_distance + 0.05;
          tft.setCursor(0,60);
          tft.setTextColor(ST77XX_WHITE);
          tft.print("Wire Submersion (mm):");
          tft.setCursor(0,70);
          tft.print(wire_submersion_distance,2);
          if(Up()==true){
            while(Up()==true){}
            }
        }
        if(Down()==true){
          tone(buzzer,4000,1);
          tft.setTextColor(ST77XX_BLACK);
          tft.setCursor(0,70);
          tft.print(wire_submersion_distance,2);
          wire_submersion_distance = wire_submersion_distance - 0.05;
          tft.setCursor(0,60);
          tft.setTextColor(ST77XX_WHITE);
          tft.print("Wire Submersion (mm):");
          tft.setCursor(0,70);
          tft.print(wire_submersion_distance,2);
          if(Down()==true){
            while(Down()==true){}
          }
        }
    }
    if(mode == 1){
        if(Up()==true){
          tone(buzzer,4000,1);
          tft.setTextColor(ST77XX_BLACK);
          tft.setCursor(0,90);
          tft.print(redip);
          redip = true;
          tft.setCursor(0,90);
          tft.setTextColor(ST77XX_WHITE);
          tft.print(redip);
          if(Up()==true){
            while(Up()==true){}
            }
        }
        if(Down()==true){
          tone(buzzer,4000,1);
          tft.setTextColor(ST77XX_BLACK);
          tft.setCursor(0,90);
          tft.print(redip);
          redip = false;
          tft.setTextColor(ST77XX_WHITE);
          tft.setCursor(0,90);
          tft.print(redip);
          if(Down()==true){
            while(Down()==true){}
          }
        }
    }
    
  count = 0;
  if(Enter()==true){
  while((Enter()==true) && (count < 1500)){
    count++;
    delay(1);
  }
  if(Enter()==true){
    tone(buzzer,4000,20);
    while(Enter()==true){}
  } //debounce
  tone(buzzer,4000,1);
  if(count<1500){ //change modes
    if(mode == 1){
      mode = 0;
      Serial.print("Mode now: ");
      Serial.println(mode);
    }else{
      mode = 1;
      Serial.print("Mode now: ");
      Serial.println(mode);
    }
  }
  }
  }

  
  tone(buzzer,4000,1);
  while(Enter()==true){}//debounce
  //prepair for main loop again
  main_menu_selection = 1;
  first_run = true;
  selection_choosen = false;
  clear_screen();
}

void homez(){
  tft.setCursor(50,122);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("HOMING...");
  //Start Screen now cleared, now home the nozzle
  zmove(77,"plad");
  tone(buzzer,4000,100);
  digitalWrite(nozzle_relay,LOW);
  digitalWrite(wire_relay,LOW);
  nozzle_relay_state = false;
  clear_screen();
}

void homee(){
  clear_screen();
  tft.drawLine(0,9,35,9,ST77XX_WHITE);
  tft.drawLine(125,9,160,9,ST77XX_WHITE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&Orbitron_Medium_10);
  tft.setCursor(42,14);
  tft.print("WIRE HOME");
  tft.setCursor(15,28);
  tft.print("MAKE WIRE FLUSH");
  tft.setCursor(28,38);
  tft.print("WITH NOZZLE");
  bool a;
  a=false;
  while(a==false){
    tft.setCursor(32,122);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("PRESS HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
    tft.setCursor(32,122);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("PRESS HOME");
    for(unsigned int i=0;i<60000;i++){
      if(Enter()==true){
        a=true;
      }
    }
    tone(buzzer,4000,1);
  }
  tone(buzzer,4000,1);
  tone(buzzer,2000,10);
  digitalWrite(nozzle_relay,LOW);
  digitalWrite(wire_relay,LOW);
  nozzle_relay_state = false;
  clear_screen();
}

void homez_against_e(){ //home the z axis, but do so by looking at the signal on the wire. This is used to home the wire on the etchant
  tft.setCursor(50,122);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("HOMING...");
  //Start Screen now cleared, now home the nozzle
  digitalWrite(nozzle_relay,HIGH);
  digitalWrite(wire_relay,HIGH);
  nozzle_relay_state = true;
  float count = 5115;
  digitalWrite(dir_z,LOW);
  while(digitalRead(wire_homing) == HIGH){
    digitalWrite(step_z,HIGH);
    delayMicroseconds(800);
    digitalWrite(step_z,LOW);
    delayMicroseconds(800);
  }
  tone(buzzer,2000,10);
  digitalWrite(nozzle_relay,LOW);
  digitalWrite(wire_relay,LOW);
  nozzle_relay_state = false;
  clear_screen();
}
