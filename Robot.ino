#include <Servo.h>//Loads commands to create Servo objects which generate PWM signals
#include <NewPing.h>
#include "U8glib.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>


#define TRIGGER_PIN  9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define THRESHOLD 20

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo leftDrive;  // create servo object to control a servo
Servo rightDrive; //another servo object for the left side
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
U8GLIB_SSD1306_128X64 u8g(0x3C|U8G_I2C_OPT_DEV_0);
char DISTANCE_STR[20];
char HEADING_STR[20];

void setup()
{
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  leftDrive.attach(12);  // attaches the servo on pin 9 to the servo object
  rightDrive.attach(13);  // attaches the servo on pin 9 to the servo object
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 
  Serial.begin(115200);
}

int reverse(int input)
{
  return 180 - input;
}

void turn(int angle) {
  leftDrive.write(100);
  rightDrive.write(100);

  delay(200);
}

void stop()
{
  leftDrive.write(90);
  rightDrive.write(90);  
}

void goStraight(int dist)
{
  int rightspeed = reverse(95);
  int leftspeed = 102;
  leftDrive.write(leftspeed);
  rightDrive.write(rightspeed);

  delay(dist);
}

float getHeading()
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);

  float heading = atan2(event.magnetic.y, event.magnetic.x);
  float declinationAngle = 0.0523599;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  return headingDegrees;
}

void loop()
{
  float heading = getHeading();
  
  int meassure = sonar.ping_cm();
  if(meassure > 0)
  {
    u8g.firstPage();
    do {  
      String distance_msg = "Distance: " + String(meassure) + " cm";
      distance_msg.toCharArray(DISTANCE_STR, distance_msg.length()+1);
      u8g.drawStr( 0, 15, DISTANCE_STR);

      String heading_msg = "Heading: " + String(heading);
      heading_msg.toCharArray(HEADING_STR, heading_msg.length()+1);
      u8g.drawStr( 0, 30, HEADING_STR);
    } while( u8g.nextPage() );
  }    
  
  if(meassure < THRESHOLD && meassure > 0)
  {
    turn(45);
  }
  else
  {
    goStraight(200);
  }
}

