#include <Arduino.h>
#include <SimpleSDAudio.h>

int flexSensors = 0; // Analog pin to which the flex sensors are conntected
int motorPin = 6;    // Motor / Servo pin

const int numReadings = 10; // Number of readings before setting average
int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int average = 0;            // the average
int averageClock = 0;       // Average clock/interval to calibrate average after x amount of loops
int averageInterval = 500;  // Interval after how many loops the average should be calibrated
int averageCalibrated = 0;  // Value for the calibrated average
boolean calibrated = false; // Value to store calibrated state
float multiplier = 0.35;    //  Multiplier for value boundry (motor trigger, max and min average)
char *audioFiles[] = {"sound_01.wav", "sound_02.wav", "sound_03.wav", "sound_04.wav", "sound_05.wav"};
String selectedFile = "";

void setup()
{
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT); // Set pin for motor
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }

  averageClock = averageInterval; // Set inital average calc, countdown clock
}

void loop()
{
  total = total - readings[readIndex];           // Subtract the last reading:
  readings[readIndex] = analogRead(flexSensors); // Read from the sensor:
  total = total + readings[readIndex];           // Add the reading to the total:
  readIndex += 1;                                // advance to the next position in the array:

  // if we're at the end of the array /wrap around to the beginning:
  if (readIndex >= numReadings)
  {
    readIndex = 0;
  }

  average = total / numReadings; // calculate the average:

  // set calibration value
  if (calibrated == false)
  {
    if (averageClock > 0)
    {
      // If still counting down, keep doing so
      averageClock -= 1; // Subtract from clock (countdown)
    }
    else
    {
      // Else set new average
      averageCalibrated = average;
      calibrated = true;
    }
  }
  else
  {
    averageClock = averageInterval;
    calibrated = false;
  }

  if (average > (averageCalibrated + (averageCalibrated * multiplier)) ||
      average < (averageCalibrated - (averageCalibrated * multiplier)))
  {
    digitalWrite(motorPin, HIGH);
    if (!SdPlay.isPlaying())
    {
      playMusic();
    }
  }
  else
  {
    digitalWrite(motorPin, LOW);
  }

  // Open serial plotter for graph (ctrl + shift + L)
  Serial.print(averageCalibrated + (averageCalibrated * multiplier)); // Blue line
  Serial.print(", ");
  Serial.print(averageCalibrated - (averageCalibrated * multiplier)); // Red line
  Serial.print(", ");
  Serial.print(averageCalibrated); // Green line
  Serial.print(", ");
  Serial.println(average); // Orange line
  delay(1);
}

void playMusic()
{
  int randomNumber = random(0, 5);
  // Serial.print("Play audio :O  ");
  // Serial.println(String(audioFiles[randomNumber]));
  SdPlay.setSDCSPin(10); // sd card cs pin
  SdPlay.init(SSDA_MODE_FULLRATE | SSDA_MODE_MONO | SSDA_MODE_AUTOWORKER);
  SdPlay.setFile(audioFiles[randomNumber]); // music name file
  //  SdPlay.setFile("sound_01.wav"); // music name file
  SdPlay.play(); // play music
}