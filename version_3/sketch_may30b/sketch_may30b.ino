//Written by Ahmet Burkay KIRNIK
//Measurement of Angle with MPU-6050(GY-521)

#include<Wire.h>
#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>

#define NO_PRESS 0
#define SHORT_PRESS 1 
#define LONG_PRESS 2
#define MAX_INTENSITY 255
#define COLOR_1 101
#define COLOR_2 102
#define NO_COLOR 100
const int MPU_addr=0x68; int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

int minVal=265; int maxVal=402;
uint32_t timer; //it's a timer, saved as a big-ass unsigned int.  We use it to save times from the "micros()" command and subtract the present time in microseconds from the time stored in timer to calculate the time for each loop.
double x; double y; double z;
int rollangle, new_rollangle;
int light_intensity = 255;
int light_change = 0;
int cal_int;
int pushButton = 6;
int lightPin = 3;
int Press_State = 0;
double timePressed = 0;
double timeReleased = 0;
bool isOn = false;
int colorState = NO_COLOR;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(16, lightPin, NEO_GRB + NEO_KHZ800);

void setup(){
  Wire.begin(); 
  Wire.beginTransmission(MPU_addr); 
  Wire.write(0x6B); Wire.write(0); 
  Wire.endTransmission(true); 
  Serial.begin(115200);
  timer = micros();
  pinMode(pushButton, INPUT_PULLUP);
  pixels.begin();
  }
void loop(){ 
  int Press_State = ButtonPress();
  if(Press_State != NO_PRESS)
  {
    if(Press_State == SHORT_PRESS && !isOn){
      colorState = COLOR_1;
      for (float ledNumber = 0; ledNumber < 16; ledNumber ++) {
        pixels.setPixelColor(ledNumber, pixels.Color(0,light_intensity,light_intensity));
        pixels.show();
        isOn = true;
//        Serial.print ("Triggered the Lights\n");
      }
    }
    else if (Press_State == SHORT_PRESS && isOn)
    {
        switch (colorState){
        case COLOR_1: 
          colorState = COLOR_2;
          break;
        case COLOR_2: 
          colorState = NO_COLOR;
          break;
        default: colorState = NO_COLOR;
      }
      if (colorState == COLOR_2)
      {
        for (float ledNumber = 0; ledNumber < 16; ledNumber ++) {
        pixels.setPixelColor(ledNumber, pixels.Color(light_intensity,light_intensity,0));
        pixels.show();
//        Serial.print ("Triggered new color the Lights\n");   
        }
      }
      else
      {
        for (float ledNumber = 0; ledNumber < 16; ledNumber ++) {
        pixels.setPixelColor(ledNumber, pixels.Color(0,0,0));
        pixels.show();
        isOn = false;
//        Serial.print ("Triggered off the Lights\n");   
        }
      }
    }
    else if(Press_State == LONG_PRESS && isOn)
    {
      for (int i = 0; i < 10; i++){
        recordRegisters();
        rollangle += y;
      }
      rollangle /= 10;
//      if (rollangle<0) rollangle = rollangle * -1;
      while(digitalRead(pushButton) == LOW){
      for (int i = 0; i < 10; i++){
        recordRegisters();
        new_rollangle += y;
      }
      new_rollangle /= 10;
//      if (new_rollangle<0) new_rollangle = new_rollangle * -1;
      Serial.print("new_rollangle: ");
      Serial.print(new_rollangle);
      Serial.print("\n");
//      if (new_rollangle - rollangle > 6)
      light_change = (int)((new_rollangle) - (rollangle));
      light_intensity = light_intensity - light_change*220/180;
      if (light_intensity > 240 || light_change > 240) light_intensity = 240;
      else if(light_intensity < 20 || light_change < -255) light_intensity = 20; 
      Serial.print("rollangle: ");
      Serial.print(rollangle);
      Serial.print("\n");
      Serial.print("light_change: ");
      Serial.print(light_change);
      Serial.print("\n");
      rollangle = new_rollangle;
      for (float ledNumber = 0; ledNumber < 16; ledNumber ++) {
         if(colorState == COLOR_1)
          pixels.setPixelColor(ledNumber, pixels.Color(0,
                           light_intensity ,
                           light_intensity));
         else 
          pixels.setPixelColor(ledNumber, pixels.Color(light_intensity ,
                           light_intensity,
                           0));
        pixels.show();
        }
     }
  }
}
}
void recordRegisters(){
  Wire.beginTransmission(MPU_addr); 
  Wire.write(0x3B); Wire.endTransmission(false); 
  Wire.requestFrom(MPU_addr,14,true); 
  AcX=Wire.read()<<8|Wire.read(); 
  AcY=Wire.read()<<8|Wire.read(); 
  AcZ=Wire.read()<<8|Wire.read(); 
  int xAng = map(AcX,minVal,maxVal,-90,90); 
//  int yAng = map(AcY,minVal,maxVal,-90,90); 
  int zAng = map(AcZ,minVal,maxVal,-90,90);
//  x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI); 
  y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI); 
//  z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
//  Serial.print("AngleX= "); Serial.println(x);
  Serial.print("AngleY= "); Serial.println(y);
//  Serial.print("AngleZ= "); Serial.println(z); 
  Serial.println("-----------------------------------------"); 
  }
 int ButtonPress(){
  timePressed = millis();
  while(digitalRead(pushButton) == LOW)
  {
     if (millis() - timePressed > 300){
      Serial.println("Buttonpressed returns = LongPress");
      return LONG_PRESS;
     }
  }
  timeReleased = millis();
  if (timeReleased - timePressed > 300)
    return LONG_PRESS;
  else if (timeReleased - timePressed < 10)
    return NO_PRESS;
  else
    return SHORT_PRESS;
  }
