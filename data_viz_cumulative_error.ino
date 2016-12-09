#include <NewPing.h>
#include <Servo.h>
#include <Array.h>

#define TRIGGER_PIN   12 // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN      11 // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
Servo myservo;  // create servo object to control a servo

unsigned int cm; 
unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.

boolean isRunning = false;

const int buttonPin = 2;
const int ledPin =  13;

// the current state of the output pin
int ledState = LOW;         
// the current reading from the input pin
int buttonState;             
// the previous reading from the input pin
int lastButtonState = HIGH;   

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

class Sweeper
{
  Servo servo;              // the servo
  int id;
  int pos;              // current servo position 
  int increment;        // increment to move for each interval
  int  updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  String sweepString = "";
  
  NewPing *sonar;
  int currentDistance;

  // =============
  // these two vars are pure debug variels to control what gets sent over serial
  boolean sendJSON = true;
  boolean storeDataJSON = true;
  boolean printStringTitle = false;
  // =============
   
public: 
  Sweeper(int ide, int interval, NewPing &sonar, int position)
  {
    updateInterval = interval;
    // can be 1,2,3,4 etc...
    increment = 2;
    id = constrain(ide, 0, 13);
    pos = position;
  }
  
  void Attach(int pin)
  {
    if(servo.attached() == 0){
      servo.attach(pin); 
    }
  }
  
  void Detach()
  {
    servo.detach();
  }

  int isAttached()
  {
    return servo.attached();
  }

  void SetDistance(int d)
  {
    currentDistance = d;

    if(storeDataJSON == true){
      StoreData(currentDistance);
    }
  }

  void SendBatchData() {
    // helping debug the serial buffer issue
    if(sendJSON == true){
        if(sweepString.endsWith("/")){
          int char_index = sweepString.lastIndexOf("/");
          sweepString.remove(char_index);
        }

        Serial.println("");
        if(printStringTitle == true){
          Serial.print("sweepString: ");
        }
        Serial.println(sweepString);
        sweepString = "";
    }
  }

  void StoreData(int currentDistance)
  {
    String tmp = (String)id;
    tmp.concat(":");
    tmp.concat((String)pos);
    tmp.concat(":");
    tmp.concat((String)currentDistance);
    
    sweepString.concat(tmp);
    sweepString.concat("/");
  }
  
  void Update()
  {

    if (pos == -1) {
      pos = 0;
      servo.write(pos);
    }

    if((millis() - lastUpdate) > updateInterval)  // time to update
    {
      lastUpdate = millis();
      pos += increment;
      servo.write(pos);
      if ((pos >= 180) || (pos <= 0)) // end of sweep
      {
        // send data through serial here
        SendBatchData();
        // reverse direction
        increment = -increment;
      }
    }
  }
};

// Init the sweeper
Sweeper sweeper(0, 20, sonar, 0);

void setup() {
  // Open serial monitor at 115200 baud to see ping results.
  Serial.begin(115200); 
  pingTimer = millis(); // Start now.

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  sweeper.Attach(9);

}

void loop() {
  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
  }

  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        isRunning = !isRunning;
      }
    }
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;

//  if (buttonState == HIGH) {
//    // turn LED on:
//    digitalWrite(ledPin, HIGH);
//  } else {
//    // turn LED off:
//    digitalWrite(ledPin, LOW);
//  }

  if(isRunning == true){
    // update
    if(sweeper.isAttached() == true){
      sweeper.Update();
    } else {
      sweeper.Attach(9);
      sweeper.Update();  
    }
  } else {
    // check to make make sure the server is attached before detaching
    if(sweeper.isAttached() == true){
      sweeper.Detach();  
    }
  }
}

// Timer2 interrupt calls this function every 24uS where you can check the ping status.
void echoCheck() { 
  // This is how you check to see if the ping was received.
  if (sonar.check_timer()) { 
   
    // Ping returned, uS result in ping_result, convert to cm with US_ROUNDTRIP_CM.
    // Serial.print("Ping: ");
    // Serial.print(sonar.ping_result / US_ROUNDTRIP_CM); 
    // Serial.println("cm");

    cm = sonar.ping_result / US_ROUNDTRIP_CM;
    sweeper.SetDistance(cm);
  }
}
