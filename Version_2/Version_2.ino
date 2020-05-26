
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

double accelX,accelY,accelZ,temperature,gyroX,gyroY,gyroZ,gyro_x_cal,gyro_y_cal,gyro_z_cal; //These will be the raw data from the MPU6050.
uint32_t timer; //it's a timer, saved as a big-ass unsigned int.  We use it to save times from the "micros()" command and subtract the present time in microseconds from the time stored in timer to calculate the time for each loop.
double roll, pitch ,yaw; //These are the angles in the complementary filter
float rollangle,pitchangle, new_rollangle;
int light_intensity = 255;
int cal_int;
int pushButton = 6;
int lightPin = 3;
int Press_State = 0;
double timePressed = 0;
double timeReleased = 0;
bool isOn = false;
int colorState = NO_COLOR;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(16, lightPin, NEO_GRB + NEO_KHZ800);

void setup() {
  // Set up MPU 6050:
  Serial.begin(115200);
  Wire.begin();

  setupMPU();   
  Serial.println("Calibrating Gyroscope......");
  for(cal_int=1;cal_int<=2000;cal_int++)
  {
   recordRegisters();
   gyro_x_cal += gyroX;
   gyro_y_cal  += gyroY ;
   gyro_z_cal += gyroZ;
  }
  Serial.println("Calibration Done..!!!");
  gyro_x_cal /= 2000;
  gyro_y_cal /= 2000;
  gyro_z_cal /= 2000;
 
  //start a timer
  timer = micros();
  pinMode(pushButton, INPUT_PULLUP);
  pixels.begin();
}

void loop() {
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
      recordRegisters();
      rollangle=atan2(accelY,accelZ)*180/PI; // FORMULA FOUND ON INTERNET
//      if (rollangle<0) rollangle = rollangle * -1;
      while(digitalRead(pushButton) == LOW){
      recordRegisters();
      new_rollangle=atan2(accelY,accelZ)*180/PI; // FORMULA FOUND ON INTERNET
//      if (new_rollangle<0) new_rollangle = new_rollangle * -1;
      Serial.print("Original angle change\n");
      Serial.print(light_intensity);
      Serial.print("\n");
      if (new_rollangle - rollangle > 10)
        light_intensity = (int)(new_rollangle - rollangle);
      if (light_intensity > 255) light_intensity = 255;
      else if(light_intensity < 0) light_intensity = 0; 
      Serial.print(light_intensity);
      Serial.print("\n");
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
//      delay(30);
      }
      
    }

  }
//Now begins the main loop.
  //Collect raw data from the sensor.
  recordRegisters();
  gyroX = gyroX / 65.5;
  gyroY = gyroY / 65.5;
  gyroZ = gyroZ / 65.5;

  accelX = accelX / 4096.0;
  accelY = accelY / 4096.0;
  accelZ= accelZ / 4096.0;
 
  double dt = (double)(micros() - timer) / 1000000; //This line does three things: 1) stops the timer, 2)converts the timer's output to seconds from microseconds, 3)casts the value as a double saved to "dt".
  timer = micros(); //start the timer again so that we can calculate the next dt.

  //the next two lines calculate the orientation of the accelerometer relative to the earth and convert the output of atan2 from radians to degrees
  //We will use this data to correct any cumulative errors in the orientation that the gyroscope develops.
//  rollangle=atan2(accelY,accelZ)*180/PI; // FORMULA FOUND ON INTERNET
  pitchangle=atan2(accelX,sqrt(accelY*accelY+accelZ*accelZ))*180/PI; //FORMULA FOUND ON INTERNET

 
 

  //THE COMPLEMENTARY FILTER
  //This filter calculates the angle based MOSTLY on integrating the angular velocity to an angular displacement.
  //dt, recall, is the time between gathering data from the MPU6050.  We'll pretend that the
  //angular velocity has remained constant over the time dt, and multiply angular velocity by
  //time to get displacement.
  //The filter then adds a small correcting factor from the accelerometer ("roll" or "pitch"), so the gyroscope knows which way is down.
  roll = 0.99 * (roll+ gyroX * dt) + 0.01 * rollangle; // Calculate the angle using a Complimentary filter
  pitch = 0.99 * (pitch + gyroY * dt) + 0.01 * pitchangle;
  yaw=gyroZ;
 
//  Serial.print("The button status: ");
//  Serial.print(digitalRead(pushButton));
//  Serial.print("\n");
//  delay(10);
  
//
//  Serial.print("roll  ");
//  Serial.print(roll);
//  Serial.print("   pitch  ");
//  Serial.print(pitch);
//  Serial.print("   yaw    ");
//  Serial.println(yaw);

}

void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x08); //Setting the gyro to full scale +/- 500deg./s
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0x10); //Setting the accel to +/- 8g
  Wire.endTransmission();
 
  Wire.beginTransmission(0b1101000);                                      //Start communication with the address found during search
  Wire.write(0x1A);                                                          //We want to write to the CONFIG register (1A hex)
  Wire.write(0x03);                                                          //Set the register bits as 00000011 (Set Digital Low Pass Filter to ~43Hz)
  Wire.endTransmission();                                                    //End the transmission with the gyro   
}
void recordRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,14); //Request Accel Registers (3B - 40)
  while(Wire.available() < 14);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  temperature=Wire.read()<<8|Wire.read();
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ

  if(cal_int == 2000)
  {
    gyroX -= gyro_x_cal;
    gyroY -= gyro_y_cal;
    gyroZ -= gyro_z_cal;
   
  }
}
int ButtonPress(){
  timePressed = millis();
  while(digitalRead(pushButton) == LOW)
  {
     if (millis() - timePressed > 500){
      Serial.println("Buttonpressed returns = LongPress");
      return LONG_PRESS;
     }
  }
  timeReleased = millis();
  if (timeReleased - timePressed > 500)
  
    return LONG_PRESS;
  else if (timeReleased - timePressed < 10)
    return NO_PRESS;
  else
    return SHORT_PRESS;
  }
