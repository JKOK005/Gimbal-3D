#define LEDPIN 13
#define msgSize 30    // Sample message will be of the form P:0.401;R:2.932;Y:3.653@ or P:-0.40;R:-8.12;Y:-6.59
#define pi  3.142
  
float pitch_err;   // Error variables from the raspberry
float yaw_err;
float roll_err;
char container[msgSize]; 
byte Size;
int increment;

const word pitchMotorPin1 = 2;      // Pitch motor control pins
const word pitchMotorPin2 = 3;
const word pitchMotorPin3 = 4;
const word rollMotorPin1 = 5;
const word rollMotorPin2 = 6;
const word rollMotorPin3 = 7;
const word yawMotorPin1 = 8;
const word yawMotorPin2 = 9;
const word yawMotorPin3 = 10;
const word pwmPitchPin = 11;        // pwm reading Pin
const word pwmRollPin = 12;
const word pwmYawPin = 13;

const word gap = 10;
const float gimbalMaxReading = 919;
const float angleResolution = 0.05;     // Angular resolution across 2*pi

int pwmSin[]= {0,1,2,4,6,8,12,15,19,24,29,34,40,
              46,52,59,66,73,80,88,95,103,111,119,
              127,135,143,151,159,166,174,181,188,
              195,202,208,214,220,225,230,235,239,
              242,246,248,250,252,253,254,255,254,
              253,252,250,248,246,242,239,235,230,
              225,220,214,208,202,195,188,181,174,
              166,159,151,143,135,127,119,111,103,
              95,88,80,73,66,59,52,46,40,34,29,24,
              19,15,12,8,6,4,2,1,0};              // array of PWM duty values - sine function
              
int arraySize = (sizeof(pwmSin)/sizeof(int)) -1;                // Goes from index 0 to arraySize
int pitchStepA = 0; int pitchStepB = (int) (arraySize /3); int pitchStepC = (int) (arraySize*2 /3);   // Stepping sequence for pitch motor
int rollStepA = 0; int rollStepB = (int) (arraySize /3); int rollStepC = (int) (arraySize*2 /3);   // Stepping sequence for pitch motor
int yawStepA = 0; int yawStepB = (int) (arraySize /3); int yawStepC = (int) (arraySize*2 /3);   // Stepping sequence for pitch motor

void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
//  Serial.print(angleResolution);
  }

void loop() {
  // Reads the serial monitor for the inputs from raspberry pi
  // If no input yet, wait till something is available
  
  if(Serial.available() > 0){
    signalAvailable();            // If signal received -> Light up LED
    memset(container, 0, sizeof(container));
    String str = Serial.readStringUntil('@');
    str.toCharArray(container, sizeof(container));
    
    discardReading();
    parseReading(container);
//    
//    Serial.print(pitch_err); Serial.print(" "); Serial.print(roll_err); Serial.print(" ");
//    Serial.println(yaw_err);
    movePitchMotor();         // Motor correction
//    moveRollMotor();
//    moveYawMotor();
   }
 else{
    serialWaiting();
    }   
  }


void parseReading(char* container){
  // Expect sample data from raspberry to be: Pitch=<pitch>;Roll=<roll>;Yaw=<yaw>;@
  // Parses the serial input of tilt angles from the raspberry and assigns the values to pitch / roll / yaw errors
  char* command = strtok(container, ";");
  char result;
  
  while(command != NULL){
    char* separator = strchr(command, ':');  // Splits each reading to individual components 
    if(separator != 0){
      *separator = 0;
      float value = extractFloat(++separator);
      result = *command;

      switch(result){
        case 'P':
          pitch_err = value;
          break;

        case 'R':
          roll_err = value;
          break;

        case 'Y':
          yaw_err = value;
        }
      }
    command = strtok(NULL, ";");
//    Serial.println(command);
  }
}

void signalAvailable(){
  // Turns on when processing serial inputs
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  }

void serialWaiting(){
  // Turns on when processing serial inputs
  digitalWrite(LEDPIN, LOW);
  delay(100);
  }
  
float extractFloat(char* ptr){
  // Gets the float data from the separator
  String tmp = "";

  while(*ptr != NULL){
    tmp += *ptr;        // Float digit terminates at NULL
    ptr++;
    }
// Serial.println(tmp);
 return tmp.toFloat();
}

void discardReading(){
  // Discard the next set of readings from the serial buffer
//  Serial.readBytes(container, msgSize -1);
//  delay(100);
  Serial.read();
  Serial.readStringUntil('@');
  delay(1);
  }

void movePitchMotor(){
  // Function to move the motor to correct for pitch errors
  float currentAnglePrev, err1, err2, err3;
  float currentAngleNow = readCurrentAngle(pwmPitchPin);
  float desiredAngle = currentAngleNow - pitch_err;
  err1 = -1; err2 = 0; err3 = -1;

  if (desiredAngle < 0){desiredAngle = 2*pi + desiredAngle;}
  else if (desiredAngle >= 2*pi){desiredAngle = desiredAngle - 2*pi;}
  if(pitch_err > 0){increment = -1;}            // Move CW 
  else{increment = 1;}
 
//  while((abs(currentAngleNow - desiredAngle) >= angleResolution)){
    while(abs(desiredAngle - currentAngleNow) >= angleResolution){
      if(cos(err1) >= cos(err2) && cos(err3) >= cos(err2)) break;  
      pitchStepA = checkLimits(pitchStepA + gap*increment);   
      pitchStepB = checkLimits(pitchStepB + gap*increment);   
      pitchStepC = checkLimits(pitchStepC + gap*increment);
      currentAnglePrev = currentAngleNow;
      
      analogWrite(pitchMotorPin1, pwmSin[pitchStepA]);    // Move the pitch motor
      analogWrite(pitchMotorPin2, pwmSin[pitchStepB]);
      analogWrite(pitchMotorPin3, pwmSin[pitchStepC]);
      
      currentAngleNow = readCurrentAngle(pwmPitchPin);                    // Resample reading
      err1 = increment*(desiredAngle - currentAnglePrev);
      err2 = increment*(currentAngleNow - currentAnglePrev);
      err3 = increment*(currentAngleNow - desiredAngle);
//      Serial.print(yaw_err);Serial.print(" "); Serial.print(currentAngleNow - desiredAngle);Serial.println(" ");
//      Serial.print(currentAnglePrev);Serial.print(" "); Serial.print(currentAngleNow);Serial.print(" "); Serial.println(currentAngleNow - currentAnglePrev);
//      Serial.println(" ");
      delay(10);
    }
  }

void moveRollMotor(){
  // Function to move the mmotor to correct for roll errors
  float currentAnglePrev, err1, err2, err3;
  float currentAngleNow = readCurrentAngle(pwmRollPin);
  float desiredAngle = currentAngleNow - roll_err;
  err1 = -1; err2 = 0; err3 = -1;

  if (desiredAngle < 0){desiredAngle = 2*pi + desiredAngle;}
  else if (desiredAngle >= 2*pi){desiredAngle = desiredAngle - 2*pi;}
  if(roll_err > 0){increment = -1;}            // Move CW 
  else{increment = 1;}
 
//  while((abs(currentAngleNow - desiredAngle) >= angleResolution)){
    while(abs(desiredAngle - currentAngleNow) >= angleResolution){
      if(cos(err1) >= cos(err2) && cos(err3) >= cos(err2)) break;  
      rollStepA = checkLimits(rollStepA + gap*increment);   
      rollStepB = checkLimits(rollStepB + gap*increment);   
      rollStepC = checkLimits(rollStepC + gap*increment);
      currentAnglePrev = currentAngleNow;
      
      analogWrite(rollMotorPin1, pwmSin[rollStepA]);    // Move the roll motor
      analogWrite(rollMotorPin2, pwmSin[rollStepB]);
      analogWrite(rollMotorPin3, pwmSin[rollStepC]);
      
      currentAngleNow = readCurrentAngle(pwmRollPin);                    // Resample reading
      err1 = increment*(desiredAngle - currentAnglePrev);
      err2 = increment*(currentAngleNow - currentAnglePrev);
      err3 = increment*(currentAngleNow - desiredAngle);
      Serial.print(roll_err);Serial.print(" "); Serial.print(desiredAngle);Serial.println(" ");
      Serial.print(currentAnglePrev);Serial.print(" "); Serial.print(currentAngleNow);Serial.print(" "); Serial.println(currentAngleNow - currentAnglePrev);
      Serial.println(" ");
      delay(10);
    }
  }
  
void moveYawMotor(){
  // Function to move the mmotor to correct for pitch, roll, yaw errors
  float currentAngle = readCurrentAngle(pwmYawPin);
  float deltaAngle = yaw_err - currentAngle;
  Serial.println(deltaAngle);
  while(abs(deltaAngle) > angleResolution){
    if(deltaAngle > 0){increment = 1;}    // Move CW 
    else{increment = -1;}
      
    yawStepA = checkLimits(yawStepA + gap*increment);   
    yawStepB = checkLimits(yawStepB + gap*increment);   
    yawStepC = checkLimits(yawStepC + gap*increment);
  
    analogWrite(yawMotorPin1, pwmSin[yawStepA]);    // Move the pitch motor
    analogWrite(yawMotorPin2, pwmSin[yawStepB]);
    analogWrite(yawMotorPin3, pwmSin[yawStepC]);
    
    currentAngle = readCurrentAngle(pwmYawPin);                    // Resample reading
    deltaAngle = yaw_err - currentAngle;
    }
  }    

float readCurrentAngle(int pwmPin){
  // Reads the current angle from the connection 
  // pwmPin1 var connected to PWM pin of the motor
  float val = (float) pulseIn(pwmPin, LOW);
  return (float) mapfloat(val, 0.0, gimbalMaxReading, 0, 2*3.142);
  }

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  // Map function for handling floating point numbers
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

int checkLimits(int currentStep){
  // Function checks whether the current step goes beyond the limit and pegs it to the limit
  if(currentStep > arraySize){
    return currentStep - arraySize -1;
    }
  else if(currentStep < 0){
    return arraySize + currentStep +1;
    }
  else{
    return currentStep;
    }
  }
