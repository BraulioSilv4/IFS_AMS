#include <Arduino.h>
#include <Servo.h>

struct 
{
  double leftSensorDistance;
  double rightSensorDistance;
} sensorData;

const int TRIG_PIN1 = 5;
const int TRIG_PIN2 = 3;
const int ECHO_PIN1 = 4;
const int ECHO_PIN2 = 2;
const int ledPinRight = 12;
const int ledPinLeft = 13;
const int ledPinFront = 11;
const int STOPLED = 10;
const int BAUD_RATE = 9600;
double currVelocity = 0.0;
double velocityBuffer[10];

void startUltrasonicSensor(int trigPin, int echoPin);
long readUltrasonicSensor(int trigPin, int echoPin);

/*
* All these functions from here will need to be implemented in ROS2 system.
* For now they will be all implemented in arduino.
*/ 
int pathPlanning(double leftSensorDistance, double rightSensorDistance, Servo motor);
double calculateVelocity(double velocityBuffer[]);
void send2ROS2();
char * receiveFromROS2();

Servo shaft;
Servo motor;
int velocity = 1;
int pos = 0;

void setup() {
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(ECHO_PIN2, INPUT);

  pinMode(ledPinRight, OUTPUT);
  pinMode(ledPinLeft, OUTPUT);
  pinMode(ledPinFront, OUTPUT);
  pinMode(STOPLED, OUTPUT);

  shaft.attach(9);
  motor.attach(6);
  delay(1);

  motor.write(85);
  delay(5000);
  Serial.begin(9600);
}

void loop() {
  sensorData.leftSensorDistance = readUltrasonicSensor(TRIG_PIN1, ECHO_PIN1);
  sensorData.rightSensorDistance = readUltrasonicSensor(TRIG_PIN2, ECHO_PIN2);
  
  if (sensorData.leftSensorDistance < 0 || sensorData.rightSensorDistance < 0) {
    Serial.println("Error reading sensor");
  } else {
    // Send over serial to ros2 system.
    Serial.print("Left sensor distance: ");
    Serial.print(sensorData.leftSensorDistance);
    Serial.print(" cm, Right sensor distance: ");
    Serial.print(sensorData.rightSensorDistance);
    Serial.println(" cm");

    switch (
      pathPlanning(
        sensorData.leftSensorDistance,
        sensorData.rightSensorDistance,
        shaft
      )
    )
    {
    case 1:
      // Turn right
      shaft.write(170);
      Serial.println("Turning right");
      delay(50);
      break;
    
    case 2:
      // Turn left
      shaft.write(60);
      Serial.println("Turning left");
      delay(50);
      break;

    default:
      // Go straight
      
      motor.write(95);
      shaft.write(110);
      delay(50);
      break;
    }
  }
  
  // Receive data from ros2 system.

  // This delay needs to be tested 
  delay(100);
}


void startUltrasonicSensor(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}

long readUltrasonicSensor(int trigPin, int echoPin) {
  startUltrasonicSensor(trigPin, echoPin);
  long duration = pulseIn(echoPin, HIGH);
  return (duration*.0343)/2;
}

/*
* All these functions from here will need to be implemented in ROS2 system.
* For now they will be all implemented in arduino.
*/ 
int pathPlanning(double leftSensorDistance, double rightSensorDistance, Servo motor) {
  if(leftSensorDistance < 30 && rightSensorDistance < 30) {
    digitalWrite(ledPinLeft, HIGH);
    digitalWrite(ledPinRight, HIGH);
    digitalWrite(STOPLED, HIGH);
    return 0;
  } else if(leftSensorDistance < 30) {
    digitalWrite(ledPinLeft, HIGH);
    digitalWrite(ledPinRight, LOW);
    digitalWrite(STOPLED, LOW);
    return 1;
  } else if(rightSensorDistance < 30) {
    digitalWrite(ledPinLeft, LOW);
    digitalWrite(ledPinRight, HIGH);
    digitalWrite(STOPLED, LOW);
    return 2;
  } else {
    digitalWrite(ledPinLeft, LOW);
    digitalWrite(ledPinRight, LOW);
    digitalWrite(STOPLED, LOW);
    return 3;
  }
  
}

double calculateVelocity(double velocityBuffer[]) {
  // This function will use the velocityBuffer to calculate the velocity of the car.
  return 0;
}

void send2ROS2() {
  // This function will send data to the ROS2 system.
}

char * receiveFromROS2() {
  // This function will receive data from the ROS2 system.
  return "Hello";
}