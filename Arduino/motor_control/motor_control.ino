#define LEDPIN 13
#define msgSize 30    // Sample message will be of the form P:0.401;R:2.932;Y:3.653@ or P:-0.40;R:-8.12;Y:-6.59
#define pi  3.142
  
float pitch_err;   // Error variables from the raspberry
float roll_err;
char container[msgSize]; 
int increment;

//////////////////// Pin declaration ///////////////////////////

const word pitchMotorPin1 = 3;      // Pitch motor control pins
const word pitchMotorPin2 = 5;
const word pitchMotorPin3 = 6;
const word pwmPitchPin    = 7;      // pwm pitch reading Pin
const word rollMotorPin1  = 9;      // Roll motor control pins
const word rollMotorPin2  = 10;
const word rollMotorPin3  = 11;
const word pwmRollPin     = 12;     // pwm roll reading Pin

//////////////////// PID variables /////////////////////////////
unsigned long last_time;
float err_sum;     // PID summation error

////////////////////////////////////////////////////////////////

const word gimbalMaxReading = 919;
const word angleResolution = 0.05;     // Angular resolution across 2*pi
const int motorPinState[]={1,1,1,0,0,0};    // Square wave
              
int arraySize = (sizeof(motorPinState)/sizeof(int)) -1;        // Goes from index 0 to arraySize
int pitchStepA = 0; int pitchStepB = 2; int pitchStepC = 4;    // Stepping sequence for pitch motor
int rollStepA = 0; int rollStepB = 2; int rollStepC = 4;      // Stepping sequence for pitch motor

void setup() {
  Serial.begin(9600);
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

    movePitchMotor();         // Motor correction
//    moveRollMotor();
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
  last_time = 0.0;        // Start timer at 0
  err_sum = 0.0;          // Reset summed error
  float in_err = -pitch_err;
  float last_err = in_err;
  float currentAngleNow = readCurrentAngle(pwmPitchPin);
  float desiredAngle = currentAngleNow - pitch_err;
  float out_err; 
  float t_delay;

  if (desiredAngle < 0){desiredAngle = 2*pi + desiredAngle;}
  else if (desiredAngle >= 2*pi){desiredAngle = desiredAngle - 2*pi;}

  Serial.println("Debug here");
  
  while(abs(in_err) > angleResolution){
    // Apply PID control

    out_err = PID_controller(in_err, last_err, &err_sum, last_time);
    last_time = millis();
//    Serial.print(pitch_err); Serial.print(" ");
    Serial.print(desiredAngle); Serial.print(" ");
    Serial.print(currentAngleNow); Serial.println(" ");
//    Serial.print(out_err); Serial.print(" ");
    
    t_delay = mapfloat(abs(out_err), 0, abs(pitch_err)*0.5, 0.0,99.0);
    t_delay = max(min(t_delay, 99.0), 0.0);
//    Serial.print(t_delay); Serial.print(" ");

    if (out_err >= 0) increment = -1;
    else increment = 1;
//    Serial.print(increment); Serial.println(" ");
    pitchStepA = checkLimits(pitchStepA + increment);   
    pitchStepB = checkLimits(pitchStepB + increment);   
    pitchStepC = checkLimits(pitchStepC + increment);
    
    analogWrite(pitchMotorPin1, 254 * motorPinState[pitchStepA]);    // Move the pitch motor
    analogWrite(pitchMotorPin2, 254 * motorPinState[pitchStepB]);
    analogWrite(pitchMotorPin3, 254 * motorPinState[pitchStepC]);
    delay(100.0 - t_delay);
      
    currentAngleNow = readCurrentAngle(pwmPitchPin);                    // Resample reading
    last_err = in_err;
    if(abs(desiredAngle - currentAngleNow) <= 2*pi - abs(desiredAngle - currentAngleNow)){in_err = desiredAngle - currentAngleNow;} 
    else if(desiredAngle < currentAngleNow){in_err = 2*pi - abs(desiredAngle - currentAngleNow);}
    else in_err = -2*pi + abs(desiredAngle - currentAngleNow);
    }
    currentAngleNow = readCurrentAngle(pwmPitchPin);                    // Resample reading
    Serial.print(desiredAngle); Serial.print(" ");
    Serial.print(increment); Serial.print(" ");
    Serial.println(currentAngleNow);
    Serial.println("");
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

  while(abs(desiredAngle - currentAngleNow) >= angleResolution){
    if(cos(err1) >= cos(err2) && cos(err3) >= cos(err2)) break;  
//    rollStepA = checkLimits(rollStepA + gap*increment);   
//    rollStepB = checkLimits(rollStepB + gap*increment);   
//    rollStepC = checkLimits(rollStepC + gap*increment);
    currentAnglePrev = currentAngleNow;
    
    analogWrite(rollMotorPin1,  254 * motorPinState[rollStepA]);    // Move the roll motor
    analogWrite(rollMotorPin2,  254 * motorPinState[rollStepB]);
    analogWrite(rollMotorPin3,  254 * motorPinState[rollStepC]);
    
    currentAngleNow = readCurrentAngle(pwmRollPin);                    // Resample reading
    err1 = increment*(desiredAngle - currentAnglePrev);
    err2 = increment*(currentAngleNow - currentAnglePrev);
    err3 = increment*(currentAngleNow - desiredAngle);
//      Serial.print(roll_err);Serial.print(" "); Serial.print(desiredAngle);Serial.println(" ");
//      Serial.print(currentAnglePrev);Serial.print(" "); Serial.print(currentAngleNow);Serial.print(" "); Serial.println(currentAngleNow - currentAnglePrev);
//      Serial.println(" ");
    delay(5);
  }
  }
  
float inline readCurrentAngle(int pwmPin){
  // Reads the current angle from the connection 
  // pwmPin1 var connected to PWM pin of the motor
  float val = (float) pulseIn(pwmPin, LOW);
  return (float) mapfloat(val, 0.0, gimbalMaxReading, 0, 2*3.142);
  }

float inline mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  // Map function for handling floating point numbers
 return (float)out_min + (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min);
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

float inline PID_controller(float in_err, float last_err, float *error_sum, unsigned long last_time){
  // Implementation of PID controller with gains
  double Kp, Kd, Ki;
  Kp = 0.5;
  Kd = 1;
  Ki = 1;
  
  unsigned long now = millis();
  float timeChange = (float)(now - last_time);
  /*Compute all the working error variables*/

  float dErr = (in_err - last_err) / timeChange;
  /*Compute PID Output*/

  *error_sum += in_err * timeChange;
  return Kp * in_err + Kd * dErr + *error_sum * Ki;
  }
