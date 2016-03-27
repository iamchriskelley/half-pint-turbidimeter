#include "U8glib.h"
#include <PinChangeInt.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
//#include <Wire.h>

#define MUTABLE_SAMPLE_TIME true
#define SERIAL_DEBUG true
#define CALIB_DEBUG false
String calib_date = "01/10/2015";

#define BAUD_RATE 9600
#define DELAY__REFERENCE_SWITCH  50
#define DELAY__BETWEEN_ADC_READS 50
#define DELAY__BETWEEN_DUMMY_READS 500
#define DELAY__BETWEEN_TURBIDITY_READS 1000

#define VERY_LOW_LONG_READ_BOUND 25
#define LOW_LONG_READ_BOUND 50
#define HIGH_SHORT_READ_BOUND 500
#define READ_MULTIPLIER 2.0
#define NUMBER__TURBIDITY_READS 8
#define NUMBER__TURBIDITY_READS_VERY_LOW 6

#define TEMP_OFFSET     0.0   //mV at 0C
#define TEMP_RESPONSE   10.0  //mV/C 
#define VREF_INTERNAL   1.085   //default: 1.1
#define VPIN A2
//#define TPIN A3
#define LED1 A3
#define VDIV_RATIO 11

#define CBUF_WIDTH 16

#define LBUTTON 5
//#define MBUTTON 6
//#define RBUTTON 4
#define LED1 6
#define LED2 4

#define LED        7
#define TSL_FREQ   0    //hardware interrupt number, this is pin 2
#define TSL_S0    13
#define TSL_S1    12    
#define TSL_OE    11    // OE, S1 and S0 are pins on the TSL230R chip
#define SCALE      2
#define HIGH_SENSITIVITY  100
#define MED_SENSITIVITY    10   
#define LOW_SENSITIVITY     1     

boolean bchange = false, lchange = false, mchange = false, rchange = false;
boolean b_cbuf0 = false, b_cbuf1 = true, b_cbuf2 = false, b_cbuf3 = false;
volatile unsigned long timer, pulse_count;
int sensitivity, xplace = 0, yplace = 25;

int num_reads = NUMBER__TURBIDITY_READS;
int read_time = DELAY__BETWEEN_TURBIDITY_READS;
float read_divisor = READ_MULTIPLIER;

struct config_t{
  int foo;           
  long machine_id;   
  unsigned long last_calibration_timestamp; 
  float y0, y1, y2,/*ylow, yhigh,*/ a0, b0, c0, d0, a1, b1, c1, d1;//, a2, b2, c2, d2;
}
config;

void lif() {
  bchange = true;
  lchange = true;
}
void mif() {
  bchange = true;
  mchange = true;
}
void rif() {
  bchange = true;
  rchange = true;
}

void add_pulse() {
  pulse_count++;
}

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI, but doesn't require Wire.h include

char cbuf0[CBUF_WIDTH], cbuf1[CBUF_WIDTH], cbuf2[CBUF_WIDTH], cbuf3[CBUF_WIDTH];
String text;
float system_voltage;

void draw_setup_update(int code) {
  u8g.firstPage();
  do draw_setup(code);
  while (u8g.nextPage());
}

void draw_setup(int code){
  if(code == 0){
    //draw large circle  
    u8g.setColorIndex(1);
    u8g.drawFilledEllipse(xplace,yplace,20,20);

    //cut it into a gear
    u8g.setColorIndex(0);
    u8g.drawFilledEllipse(xplace-22,yplace,5,5);
    u8g.drawFilledEllipse(xplace+22,yplace,5,5);
    u8g.drawFilledEllipse(xplace-11,yplace-18,5,5);
    u8g.drawFilledEllipse(xplace-11,yplace+18,5,5);
    u8g.drawFilledEllipse(xplace+11,yplace-18,5,5);
    u8g.drawFilledEllipse(xplace+11,yplace+18,5,5);
    u8g.drawFilledEllipse(xplace,yplace,12,12);

    //add in water drop and footer message
    u8g.setColorIndex(1);
    u8g.drawFilledEllipse(xplace,yplace+4,5,5);
    u8g.drawLine(xplace-6,yplace+4, xplace,yplace-5);
    u8g.drawLine(xplace,yplace-5, xplace+6,yplace+4);
    u8g.drawLine(xplace-4,yplace+4, xplace,yplace-3);
    u8g.drawLine(xplace,yplace-3, xplace+4,yplace+4);
    u8g.drawStr(0, 59, cbuf3);
  }
  else if(code == 1){
    //write (menu text) in footer
    u8g.drawStr(0, 59, cbuf3);
  }
}

void draw_loop_update() {
  u8g.firstPage();
  do draw_loop();
  while (u8g.nextPage());
}

void draw_loop() {// graphic commands to redraw the complete screen should be placed here
  //Estimate: letters are 12px tall and 10px wide
  //Keep a line offset of >= 15px to account for this
  if (b_cbuf0) u8g.drawStr( 0, 12, cbuf0);
  if (b_cbuf1) u8g.drawStr( 0, 27, cbuf1);
  if (b_cbuf2) u8g.drawStr( 0, 42, cbuf2);
  if (b_cbuf3) u8g.drawStr( 0, 59, cbuf3);
}

void setup(void) {
  Serial.begin(BAUD_RATE);
  if(!MUTABLE_SAMPLE_TIME){read_divisor = 1.0;}
  analogReference(INTERNAL);
  delay(DELAY__REFERENCE_SWITCH);

  pinMode(LED, OUTPUT);
  pinMode(TSL_FREQ, INPUT); // light sensor
  pinMode(TSL_S0, OUTPUT);  // light sensor
  pinMode(TSL_S1, OUTPUT);  // light sensor
  pinMode(TSL_OE, OUTPUT);  // light source
  digitalWrite(TSL_OE, LOW); 
  digitalWrite(TSL_S0, HIGH);
  digitalWrite(TSL_S1, HIGH);

  pinMode(LBUTTON, INPUT_PULLUP);
  pinMode(MBUTTON, INPUT_PULLUP);
  pinMode(RBUTTON, INPUT_PULLUP);
  PCintPort::attachInterrupt(LBUTTON, lif, RISING);
  PCintPort::attachInterrupt(MBUTTON, mif, RISING);//enableInterrupt(LBUTTON, rif, RISING);
  PCintPort::attachInterrupt(RBUTTON, rif, RISING);

  u8g.setRot180(); // flip screen, if required
  u8g.setFont(u8g_font_8x13B);      //see list at https://code.google.com/p/u8glib/wiki/fontsize
  if      ( u8g.getMode() == U8G_MODE_R3G3B2 )    {u8g.setColorIndex(255);}// white
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )  {u8g.setColorIndex(3);}  // max intensity
  else if ( u8g.getMode() == U8G_MODE_BW )        {u8g.setColorIndex(1);}  // pixel on
  else if ( u8g.getMode() == U8G_MODE_HICOLOR )   {u8g.setHiColorByRGB(255, 255, 255);}

  strcpy(cbuf3, "    Welcome!");
  xplace = 64;
  yplace = 25;
  draw_setup_update(0);
  delay(3000);
  strcpy(cbuf3, "CAL   READ  HELP");
  draw_setup_update(1);
  delay(100);

  if(CALIB_DEBUG){
    config.foo = 255; //EEPROMAnything seems to need the struct to start with a integer in [0,255]
    config.machine_id = 11111111; //example
    config.last_calibration_timestamp = 1442871587;  //example
  //PLEASE NOTE -- the coefficients below are merely reasonable examples. 
  //Your device WILL require a calibration to determine the proper coefficient values.
    config.y0 = 0;
    config.a0 = -0.0039024;
    config.b0 = 0.274395;
    config.c0 = -5.593;
    config.d0 = 35.99;
    config.a1 = 0.000000638;
    config.b1 = -0.0004296;
    config.c1 = 0.76017;
    config.d1 = -12.374;
    config.y1 = 25;
    config.y2 = 450;
    EEPROM_writeAnything(0, config);
  }
  else{ EEPROM_readAnything(0, config); }
   
  b_cbuf0 = true;
  b_cbuf1 = true;
  b_cbuf2 = true;
  b_cbuf3 = true;
  if(SERIAL_DEBUG){
    Serial.println(F("Left button for calibration, middle button to take a reading, right button for help menu."));
    Serial.println("\r\n");
  }

  sensitivity = HIGH_SENSITIVITY;
  setSensitivity(sensitivity); // set sensor sensitivity
}

void loop(void) {
  if (bchange) {
    if(lchange){
      String cd = "done " + calib_date + ".";
      strcpy(cbuf0, "Calibration was");
      cd.toCharArray(cbuf1, CBUF_WIDTH);
      strcpy(cbuf2, "");
      if(SERIAL_DEBUG){
        Serial.println(F("Calibration and stuff."));
        Serial.println("\r\n");
      }
    }
    else if(mchange){
      strcpy(cbuf0, "Reading.....");
      strcpy(cbuf1, "");
      strcpy(cbuf2, "");
      if(SERIAL_DEBUG){
        Serial.println(F("Reading....."));
        Serial.println("\r\n");
      }
      draw_loop_update();
      get_sensor_output();
    }
    else if(rchange){
      strcpy(cbuf0, "");
      strcpy(cbuf1, "");
      strcpy(cbuf2, "www.awqua.org");
      get_helper_data();
    }
    draw_loop_update();
    bchange = false;
    lchange = false;
    mchange = false;
    rchange = false;
    if(SERIAL_DEBUG){
      Serial.println(F("Left button for calibration, middle button to take a reading, right button for help menu."));
      Serial.println("\r\n");
    }
  }
  delay(200);
}


void get_helper_data() {
  float t = readTemperature();
  text = F("temp: ");
  text += t;
  text += F("C");
  text.toCharArray(cbuf0, CBUF_WIDTH);
  //text = "btn cnt:";  //text += lic;  //text += ",";  //text += mic;
  //text += ",";  //text += ric;  //text.toCharArray(cbuf2, CBUF_WIDTH);
  text = "";
}

/******************************TURBIDITY SENSOR***********************************/
String get_sensor_output() {
  strcpy(cbuf0, "Turbidity:");
  strcpy(cbuf2, "");
  String s = "";
  take_readings(4, true, false, DELAY__BETWEEN_TURBIDITY_READS, 1.0);
  float f = take_readings(1, true, false, DELAY__BETWEEN_TURBIDITY_READS, 1.0);
  if(MUTABLE_SAMPLE_TIME){
    //int est_time = 8;
    //String et = "wait: ";
    if(f < VERY_LOW_LONG_READ_BOUND){ //5+6*4=29s
      read_time = DELAY__BETWEEN_TURBIDITY_READS * READ_MULTIPLIER * 2;
      read_divisor = READ_MULTIPLIER * 2;
      num_reads = NUMBER__TURBIDITY_READS_VERY_LOW;
      Serial.println("very low light -- increasing read time extra much...");
      //et += DELAY__BETWEEN_TURBIDITY_READS * READ_MULTIPLIER * 2 * NUMBER__TURBIDITY_READS_VERY_LOW / 1000;
      //et += "s";
      //et.toCharArray(cbuf1, CBUF_WIDTH); 
    }else if(f < LOW_LONG_READ_BOUND){ //5+8*2=21s 
      read_time = DELAY__BETWEEN_TURBIDITY_READS * READ_MULTIPLIER;
      read_divisor = READ_MULTIPLIER;
      num_reads = NUMBER__TURBIDITY_READS;
      Serial.println("low light -- increasing read time...");
    }else if(f > HIGH_SHORT_READ_BOUND){ //5+8/2=9s
      read_time = DELAY__BETWEEN_TURBIDITY_READS / READ_MULTIPLIER;
      read_divisor = 1/READ_MULTIPLIER;
      num_reads = NUMBER__TURBIDITY_READS;
      Serial.println("high light -- decreasing read time...");
    }else{ //5+8*1=13s
      read_time = DELAY__BETWEEN_TURBIDITY_READS;
      read_divisor = 1.0;
      num_reads = NUMBER__TURBIDITY_READS;
      Serial.println("medium light -- normal read time...");
    }
    //et.toCharArray(cbuf1, CBUF_WIDTH);    
    //et = "";
  }else{
    read_time = DELAY__BETWEEN_TURBIDITY_READS;
    read_divisor = 1.0;
  }
  //int num_reads = NUMBER__TURBIDITY_READS;
  s += take_readings(num_reads, false, false, (int)read_time, read_divisor);
  s += " NTU";
  s.toCharArray(cbuf1, CBUF_WIDTH);
  if(SERIAL_DEBUG){
    Serial.println(s);
    Serial.println("\r\n");
  }
  return F("done");
}

float take_readings(int rdgs, boolean throwaway, boolean dark_counts, int read_time, float read_divisor) {
  Serial.print("read time: ");
  Serial.println(read_time);
  attachInterrupt(TSL_FREQ, add_pulse, RISING);
  delay(5);
  long rd = 0, high = 0, low = 1000000, sum = 0;
  long avg = 0.0;
  for (int i = 0; i < rdgs; ++i) {
    rd = 0.0;
    if (dark_counts) {
      digitalWrite(LED, LOW);
      delay(2);
      timer = millis();
      pulse_count = 0;
      while (timer + read_time > millis()) {;}
      rd -= pulse_count / SCALE;
    }
    digitalWrite(LED, HIGH);
    delay(2);
    timer = millis();
    pulse_count = 0;
    while (timer + read_time > millis()) {;}
    rd += pulse_count / SCALE;
    if (rd > high) {
      high = rd;
    }
    if (rd < low) {
      low = rd;
    }
    sum += rd;
    if (SERIAL_DEBUG && !throwaway) {
      Serial.print(F("reading: "));
      Serial.println(rd);
      Serial.println("\r\n");
    }
  }
  digitalWrite(LED, LOW);
  avg = 0.0;
  int b = 0;
  if (rdgs > 2) {
    b = 2;
    sum -= high + low;
    avg = sum / (rdgs - 2) / read_divisor;
  } else {
    avg = sum / rdgs / read_divisor;
  }
  detachInterrupt(TSL_FREQ);
  delay(5);
  if (SERIAL_DEBUG && !throwaway) {
    Serial.print(F("average: "));
    Serial.println(avg);
    Serial.println();
    Serial.println("\r\n");
    delay(100);
  }
  float d_f = divisionFactor_TSL230R();
  if(SERIAL_DEBUG){
    Serial.print(F("sensor division factor:"));
    Serial.println(d_f);
    Serial.print(F("light count average: "));
    Serial.println(float(sum) / float(rdgs - b));
    Serial.print(F("adjusted average: "));
    Serial.println("\r\n");
  }
  float raw_value = float(sum) / float(rdgs - b) / d_f  / read_divisor;
  float ntu_value = -1;
  if(CALIB_DEBUG){
    String t = "Raw: ";
    t += raw_value;
    t.toCharArray(cbuf0, CBUF_WIDTH);
  }
  if(SERIAL_DEBUG){
    Serial.println(raw_value);
    Serial.println("\r\n");
  }
  if(sensitivity == HIGH_SENSITIVITY){
    if(raw_value > config.y2)       {Serial.println("This is above calibration upper bound. Results should be confirmed independently if possible...");}//use this to raise an alarm -- for example, if the value is beyond the range the device is calibrated to measure
    if(raw_value > config.y1)       { ntu_value = raw_value * (raw_value * raw_value * config.a1 + raw_value * config.b1 + config.c1) + config.d1; }
    else                            { ntu_value = raw_value * (raw_value * raw_value * config.a0 + raw_value * config.b0 + config.c0) + config.d0; }
    if(ntu_value < 0.00){ntu_value = 0.00;}
    if(ntu_value > 9999){ntu_value = 9999;}
    return ntu_value;
  }
  else{return 9999;}
  
}

void setSensitivity(int sens){    //set sensor sensitivity
  if(sens == LOW_SENSITIVITY){  
    digitalWrite(TSL_S0, LOW);
    digitalWrite(TSL_S1, HIGH);
    sensitivity = LOW_SENSITIVITY;
  }
  else if(sens == MED_SENSITIVITY){
    digitalWrite(TSL_S0, HIGH);
    digitalWrite(TSL_S1, LOW);
    sensitivity = MED_SENSITIVITY;
  }
  else if(sens == HIGH_SENSITIVITY){
    digitalWrite(TSL_S0, HIGH);
    digitalWrite(TSL_S1, HIGH);
    sensitivity = HIGH_SENSITIVITY;
  }
  return;
}

/**************************TEMPERATURE AND VOLTAGE****************************/
float divisionFactor_TSL230R(){
  float m = .0052;   //slope of sensor's linear response curve
  float vmin = 3.0;  //min operating v of sensor
  float vmax = 5.5;  //max operating v of sensor
  float v100 = 4.9;  //voltage | normalized response of TSL230r = 1.0
  float v = getVoltageLevel();
  //Serial.print(F("Sensor normalization factor: "));
  //Serial.println(1000 * (4.9 - v) * m);
  if(v < vmin || v > vmax){ return -1; }
  else{ return 1.0 - (4.9 - v) * m; } 
}

float getVoltageLevel() {
  delay(100);
  //analogReference(INTERNAL);
  //delay(DELAY__REFERENCE_SWITCH);
  float vpin = 0.0;
  int reads = 8;
  for(int i = 0; i < reads; i++){
    vpin += analogRead(VPIN);
    if(i < reads - 1){delay(DELAY__BETWEEN_ADC_READS);}
  }
  vpin /= reads;
  if(SERIAL_DEBUG){
    Serial.print(F("raw voltage read: "));
    Serial.println(vpin);
    Serial.println("\r\n");
  }
  system_voltage = (vpin) / 1023.0 * VREF_INTERNAL * VDIV_RATIO;
  text = F("voltage: ");
  text += system_voltage;
  text += F("V");
  text.toCharArray(cbuf1, CBUF_WIDTH);
  text = "";
  if(SERIAL_DEBUG){
    Serial.print(F("voltage in mV: "));
    Serial.println(system_voltage * 1000);
    Serial.println("\r\n");
  }
  return system_voltage;
}

float readTemperature() { //Assumes using an LM35: 10mV/C responsivity
  delay(100); //delay after button press
  //analogReference(INTERNAL);
  //delay(DELAY__REFERENCE_SWITCH);
  analogRead(TPIN);
  delay(10);
  float t0 = analogRead(TPIN);
  //float v = getVoltageLevel();
  float t = TEMP_OFFSET + t0 * VREF_INTERNAL * 1000 / 1024.0 / TEMP_RESPONSE; 
  if(SERIAL_DEBUG){
    Serial.print("raw temperature read: ");
    Serial.println(t0);
    Serial.print(F("temperature: "));
    Serial.print(t);
    Serial.println(F("C"));
    Serial.println("\r\n");
  }
  return float(t);
}
