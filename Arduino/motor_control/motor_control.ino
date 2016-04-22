#define LEDPIN 13
#define msgSize 30    // Sample message will be of the form P:0.401;R:2.932;Y:3.653@ or P:-0.40;R:-8.12;Y:-6.59
#define pi  3.142
  
float pitch_err;   // Error variables from the raspberry
float roll_err;
char container[msgSize]; 
byte Size;
int increment;
float err_sum;    // Accumulated error for PID

const word pitchMotorPin1 = 3;      // Pitch motor control pins
const word pitchMotorPin2 = 5;
const word pitchMotorPin3 = 6;
const word pwmPitchPin    = 7;      // pwm pitch reading Pin
const word rollMotorPin1  = 9;      // Roll motor control pins
const word rollMotorPin2  = 10;
const word rollMotorPin3  = 11;
const word pwmRollPin     = 12;     // pwm roll reading Pin

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
//
//int pwmSin[] = {225,233,140,30,6,88,203,243,173,56,1,
//                57,174,243,202,87,5,31,141,234,225,121,
//                19,12,106,216,239,156,42,2,73,190,244,
//                187,70,2,44,159,240,213,103,11,21,125,
//                226,233,138,29,6,90,204,243,171,54,1,59,
//                176,244,200,85,5,32,143,235,223,119,18,
//                13,108,217,239,153,40,2,75,192,244,185,
//                68,1,46,161,241,212,101,10,22,127,227,
//                232,135,27,7,93,206,242,169,53,1};  // Sine wave with mean at 122 and amplitude of 123
              
int arraySize = (sizeof(pwmSin)/sizeof(int)) -1;                // Goes from index 0 to arraySize
int pitchStepA = 0; int pitchStepB = (int) (arraySize /3); int pitchStepC = (int) (arraySize*2 /3);   // Stepping sequence for pitch motor
int rollStepA = 0; int rollStepB = (int) (arraySize /3); int rollStepC = (int) (arraySize*2 /3);   // Stepping sequence for pitch motor

void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  }

void loop() {
  // Reads the serial monitor for the inputs from raspberry pi
  // If no input yet, wait till something is available
  // problem is that serial data is being sent twice even though we don't input in anything. Even though the serial buffer is empty, it still prints
  if(Serial.available() > 0){
    String str="";
    signalAvailable();            // If signal received -> Light up LED
    memset(container, 0, sizeof(container));

    char c = Serial.read();
    if (c =='#'){
      c = Serial.read();
      while(c != '&'){
        str += c;
        c = Serial.read();}
    str.toCharArray(container, sizeof(container));
    parseReading(container);
//    Serial.print(pitch_err); Serial.print(" "); Serial.println(roll_err);
//    Serial.println(str);
//    movePitchMotor();         // Motor correction
    moveRollMotor();
     }
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
        }
      }
    command = strtok(NULL, ";");
  }
}

void signalAvailable(){
  // Turns on when processing serial inputs
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  }

void serialWaiting(){
  // Turns off when processing serial inputs
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

void movePitchMotor(){
  // Function to move the motor to correct for pitch errors
  unsigned long last_time = 0.0;
  float currentAngleNow = readCurrentAngle(pwmPitchPin);
  float desiredAngle = currentAngleNow - pitch_err;
  float in_err = desiredAngle - currentAngleNow;
  float last_err = in_err;
  float out_err; 
  if (desiredAngle < 0){desiredAngle = 2*pi + desiredAngle;}
  else if (desiredAngle >= 2*pi){desiredAngle = desiredAngle - 2*pi;}
  
  while(abs(in_err) > angleResolution){
    // Apply PID control

    out_err = PID_controller(in_err, last_err, last_time);
    last_time = millis();
      
    increment = (int) mapfloat(out_err, -pi/4, pi/4, -15.0,15.0);

    pitchStepA = checkLimits(pitchStepA - increment);   
    pitchStepB = checkLimits(pitchStepB - increment);   
    pitchStepC = checkLimits(pitchStepC - increment);
    
    analogWrite(pitchMotorPin1, pwmSin[pitchStepA]);    // Move the pitch motor
    analogWrite(pitchMotorPin2, pwmSin[pitchStepB]);
    analogWrite(pitchMotorPin3, pwmSin[pitchStepC]);
    delay(10);
      
    currentAngleNow = readCurrentAngle(pwmPitchPin);                    // Resample reading
    last_err = in_err;
    
    if(abs(desiredAngle - currentAngleNow) <= 2*pi - abs(desiredAngle - currentAngleNow)){in_err = desiredAngle - currentAngleNow;} 
    else if(desiredAngle < currentAngleNow){in_err = 2*pi - abs(desiredAngle - currentAngleNow);}
    else in_err = -2*pi + abs(desiredAngle - currentAngleNow);
    }
//    Serial.print(desiredAngle); Serial.print(" ");
//    Serial.println(currentAngleNow);
//    Serial.println("");
  }


void moveRollMotor(){
  // Function to move the motor to correct for roll errors
  unsigned long last_time = 0.0;
  float currentAngleNow = readCurrentAngle(pwmRollPin);
  float desiredAngle = currentAngleNow - roll_err;
  float in_err = desiredAngle - currentAngleNow;
  float last_err = in_err;
  float out_err; 
  err_sum = 0.0;
  
  if (desiredAngle < 0){desiredAngle = 2*pi + desiredAngle;}
  else if (desiredAngle >= 2*pi){desiredAngle = desiredAngle - 2*pi;}
  
  while(abs(in_err) > angleResolution){
    // Apply PID control

    out_err = PID_controller(in_err, last_err, last_time);
    last_time = millis();
    
    increment = (int) mapfloat(out_err, -pi/4, pi/4, -15.0,15.0);
//    Serial.print(roll_err); Serial.print(" ");
    
    rollStepA = checkLimits(rollStepA - increment);   
    rollStepB = checkLimits(rollStepB - increment);   
    rollStepC = checkLimits(rollStepC - increment);
    
    analogWrite(rollMotorPin1, pwmSin[rollStepA]);    // Move the roll motor
    analogWrite(rollMotorPin2, pwmSin[rollStepB]);
    analogWrite(rollMotorPin3, pwmSin[rollStepC]);
//    Serial.println(increment);
    delay(10);
      
    currentAngleNow = readCurrentAngle(pwmRollPin);                    // Resample reading
    last_err = in_err;

    if(abs(desiredAngle - currentAngleNow) <= 2*pi - abs(desiredAngle - currentAngleNow)){in_err = desiredAngle - currentAngleNow;} 
    else if(desiredAngle < currentAngleNow){in_err = 2*pi - abs(desiredAngle - currentAngleNow);}
    else in_err = -2*pi + abs(desiredAngle - currentAngleNow);
    }
//    Serial.print(desiredAngle); Serial.print(" ");
//    Serial.print(increment); Serial.print(" ");
//    Serial.println(currentAngleNow);
//    Serial.println("");
  }
  
  
float inline readCurrentAngle(int pwmPin){
  // Reads the current angle from the connection 
  // pwmPin1 var connected to PWM pin of the motor
  float val = (float) pulseIn(pwmPin, LOW);
  return (float) mapfloat(val, 0.0, gimbalMaxReading, 0, 2*3.142);
  }

float inline mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  // Map function for handling floating point numbers
 float val = (float)out_min + (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min);
 return max(min(val, out_max), out_min);
}

int inline checkLimits(int currentStep){
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

float inline PID_controller(float in_err, float last_err, unsigned long last_time){
  // Implementation of PID controller with gains
  float Kp, Kd, Ki;
  Kp = 1;
  Kd = 0.1;
  Ki = 0;
  
  unsigned long now = millis();
  float timeChange = (float)(now - last_time);
  /*Compute all the working error variables*/

  float dErr = (in_err - last_err) / timeChange;

  err_sum += timeChange * in_err;
//  Serial.print(in_err); Serial.print(" "); Serial.print(dErr); Serial.print(" "); Serial.println(err_sum);
  /*Compute PID Output*/
  return Kp * in_err + Kd * dErr + Ki * err_sum;
  }
