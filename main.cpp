#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Pin Definitions
const int ultrasonicTrig = 7;
const int ultrasonicEcho = 2;
const int pirPin = 4;
const int gasSensorPin = A2;
const int humiditySensorPin = A3; // Added for humidity sensor
const int tempSensorPin = A1;
const int hotMotorPin = 5;
const int ledPin = 10;

// Servo for ventilation
Servo ventServo;

// I2C LCD (Address: 0x27, Columns: 16, Rows: 2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
bool systemActive = true;
bool pirMotionDetected = false;
int ultrasonicMovements = 0;
unsigned long lastMotionTime = 0;
unsigned long ventTimer = 0;
bool ventOpen = false;

// Thresholds
float tempLowerThreshold = 18.0;
float tempUpperThreshold = 24.0;
int gasThreshold = 500;           // Threshold for gas detection
int humidityThreshold = 700;     // Threshold for humidity detection

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize Pins
  pinMode(ultrasonicTrig, OUTPUT);
  pinMode(ultrasonicEcho, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(hotMotorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Initialize Servo
  ventServo.attach(3);
  ventServo.write(0);  // Vent closed initially

  // Initialize I2C LCD
  lcd.init();
  lcd.backlight();
  lcd.print("System: Active");
}

void loop() {
  if (systemActive) {
    monitorTemperature();
    manageVentilation();
    detectMotion();
    checkGasAndHumidity();
  }
  delay(1000);
}

void monitorTemperature() {
  int tempValue = analogRead(tempSensorPin);
  // Convert analog value to temperature in Celsius
  float voltage = tempValue * (5.0 / 1023.0);  // Assuming 5V ADC
  float temperature = (voltage - 0.5) * 100.0; // Example formula (depends on the sensor)

  // Display temperature on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" C ");

  // Display thresholds on LCD
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(tempLowerThreshold, 1);
  lcd.print(" H:");
  lcd.print(tempUpperThreshold, 1);
  
  delay(3000);

  // Print temperature to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  // Control fan speed based on temperature
  if (temperature < tempLowerThreshold) {
    int fanSpeed = map(temperature, tempLowerThreshold - 10, tempLowerThreshold, 255, 0);
    fanSpeed = constrain(fanSpeed, 0, 255); // Ensure the speed is within range
    analogWrite(hotMotorPin, fanSpeed);
    analogWrite(ledPin, fanSpeed / 2);  // Moderate brightness based on fan speed
  } else {
    analogWrite(hotMotorPin, 0); // Turn off fan
    digitalWrite(ledPin, LOW);   // Turn off LED
  }
}

void manageVentilation() {
  // Ventilation control is now handled in checkGasAndHumidity()
  // This function is left for future independent management if needed
}

void detectMotion() {
  // Ultrasonic Sensor
  long duration;
  digitalWrite(ultrasonicTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonicTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTrig, LOW);
  duration = pulseIn(ultrasonicEcho, HIGH);
  int distance = duration * 0.034 / 2;

  if (distance < 50) {  // Object detected
    // PIR Sensor
    pirMotionDetected = digitalRead(pirPin);
    if (pirMotionDetected) {
      lastMotionTime = millis();
    }
    Serial.print("PIR Motion Detected: ");
    Serial.println(pirMotionDetected ? "Yes" : "No");
  }
}

void checkGasAndHumidity() {
  int gasValue = analogRead(gasSensorPin);
  int humidityValue = analogRead(humiditySensorPin);

  Serial.print("Gas Sensor Value: ");
  Serial.println(gasValue);
  Serial.print("Humidity Sensor Value: ");
  Serial.println(humidityValue);

  // Check if either sensor exceeds its threshold
  if (gasValue > gasThreshold || humidityValue > humidityThreshold) {
    ventServo.write(90);  // Open Vent
    analogWrite(ledPin, 64);  // Low brightness to indicate active ventilation
    ventOpen = true;
    lcd.setCursor(0, 1);
    lcd.print("Vent: Open   ");
    // lcd.print("Gas: ", gasValue);
    // lcd.print("Humidaity: ", humidityValue);
  } else {
    ventServo.write(0);  // Close Vent
    digitalWrite(ledPin, LOW);  // Turn off LED
    ventOpen = false;
    lcd.setCursor(0, 1);
    lcd.print("Vent: Closed ");
    delay(1000);
  }
}
