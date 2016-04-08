#define LEDPIN 13
#define msgSize 24

float pitch_err;   // Error variables from the raspberry
float yaw_err;
float roll_err;
char container[msgSize]; 
byte Size;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  }

void loop() {
  // Reads the serial monitor for the inputs from raspberry pi
  // If no input yet, wait till something is available
  
  if(Serial.available() > 0){
    serialAvailable();
    Size = Serial.readBytes(container, msgSize -1);
    discardReading();
    parseReading(container);
//    Serial.print(pitch_err); Serial.print(" "); Serial.print(roll_err); Serial.print(" ");
//    Serial.println(yaw_err);
   }
 else{
    serialWaiting();
  } 

  ////////////// Implement further code here ////////////////
  ///                                                     ///
  ///////////////////////////////////////////////////////////
  
  
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

void serialAvailable(){
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
 Serial.println(tmp);
 return tmp.toFloat();
}

void discardReading(){
  // Discard the next set of readings from the serial buffer
  Serial.readStringUntil('@');
  Serial.read();
  delay(100);
  }

