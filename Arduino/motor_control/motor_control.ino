float pitch_err;   // Error variables from the raspberry
float yaw_err;
float roll_err;
char result;
String container = ""; 
const char check_first = '$';
const char check_last = '@';
char tmp[20];
bool flag = 0;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  }

void loop() {
  // Reads the serial monitor for the inputs from raspberry pi
  // If no input yet, wait till something is available
//  result = ' '; int i = 0;
  
  if(Serial.available() > 0){
////    result = Serial.read();
////    container[i]= result;
////    Serial.println(container);
////    i++;
////    delay(100);
    container = Serial.readStringUntil('@');
    container.trim("\r\n");
//    container += result;
    flag = 1;
   } // Polls for serial availability

  if(flag){
    digitalWrite(13,HIGH);
    Serial.println(container);
    delay(1000);
    digitalWrite(13,LOW);
    }
//  
//  Serial.println(container);  
//  result = Serial.read();
//  Serial.println(result);
//  delay(500);
//  delay(1000);
//  readSerialInput();  
//  dtostrf(pitch_err, 4, 4, tmp);
//  delay(5000);
//  
//  Serial.print(tmp);

  ////////////// Implement further code here ////////////////
  ///                                                     ///
  ///////////////////////////////////////////////////////////
  
  
  }


void readSerialInput(){
  // Expect sample data from raspberry to be: $ Pitch=<pitch> ; Roll=<roll> ; Yaw=<yaw> ; @
  // Parses the serial input of tilt angles from the raspberry and assigns the values to pitch / roll / yaw errors
  String smplWord = "";
  String smplValue = "";
  do{result = Serial.read();}
  while(result != check_first);

  for(int i=0; i <3; i++){
      moveToChar(' ');                  // Moves to angle reading
      smplWord = readTillChar('=');         // Reads the type of tilt
      smplValue = readTillChar(';');        // Reads the value of the tilt

      switch(smplWord[0]){
        case 'P':
          pitch_err = smplValue.toFloat();
          break;

        case 'R':
          roll_err = smplValue.toFloat();
          break;

        case 'Y':
          yaw_err = smplValue.toFloat();
        }   
    }

  moveToChar('@');                      // Moves to end of message
  return;
  }


void moveToChar(char stopper){
  // Moves result pointer to specified stopper
    while(result != stopper){result = Serial.read();}
  }


String readTillChar(char stopper){
  // Moves result pointer to specified stopper while returning everything scanned in the process as a string
  // White spaces are ignored since we want to read a continuous string of message
  String returnString = "";

  while(result != stopper){
    result = Serial.read();
    if(result == ' ') continue;         // Ignores white spaces
    returnString += result;
    }
  return returnString; 
  }
