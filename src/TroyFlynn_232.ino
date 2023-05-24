// =======================================================================================
//                         TroyFlynn's 232 Transition Code
// =======================================================================================
//                          Last Revised Date: 10/3/2019
//   Based HEAVILY on code written by: Maxstang, with MAJOR help from Linor and Croy9000
//
// =======================================================================================
//
//    This code was created to drive the widely avaliable BTS-7960 motor driver for use in
//    Droid transitions from two to three legs and back using brushed encoderless motors. It
//    is written to be used on an Arduino Mega.  
//
//    Most command inputs are from a simple RF receiver paired to a keyfob remote, or an onboard control panel.
//
//    9/15/19   -   Code Revised to work with Martyman's 3D printed 2-3-2 system
//    10/3/19   -   Code Revised to adjust pin assignments based to new wiring diagram
//
// 232 Motor Identification and pin assignment
//
// Shoulder BTS-7960
int RPWMS = 12; //PWM signal for "right" rotation of the shoulder motor (Yellow wire)
int LPWMS = 11; //PWM signal for "left" rotation of the shoulder motor (Orange wire)
int R_ENS = 46; //Enable pin for "right" rotation of the shoulder motor (Purple wire - Pin was 47)
int L_ENS = 48; //Enable pin ofor the "left" rotation of the shoulder motor (Blue wire - Pin was 45)

// Center Leg BTS-7960
int RPWMC = 7; // (Brown wire)
int LPWMC = 6; // (Red wire)
int R_ENC = 50; // (Green wire)
int L_ENC = 52; // (Yellow wire - Pin was 48) 


// Activation button inputs

const int THREELEGButton = 36;      //D Button on remote  (Green wires and button - Pin was 27)
const int TWOLEGButton = 30;        //B Button on remote (Red wires and button - Pin was 23)
const int THREELEGSLOWButton = 34;  //C Button on remote (Yellow wires and button - Pin was 29)


const int Shoulder3Limit = 42; //Shoulder limit switch for the 3 leg position (Out limit - White wire - Pin was 22)
const int Shoulder2Limit = 40; //Shoulder limit swtich for the 2 leg position (In limit - Purple wire - Pin was 24)
const int CenterLeg3Limit = 38; // (Down limit - Blue wire - Pin was 26)
const int CenterLeg2Limit = 32; // (Up limit - Orange wire - Pin was 28)


bool Shoulder3LimitTriggered = false; //Boolean variables to minimize false reporting from the limit switches
bool Shoulder2LimitTriggered = false;
bool CenterLeg3LimitTriggered = false;
bool CenterLeg2LimitTriggered = false;


unsigned long TimeOutValue = 28000; //Motor timeout value. Was 21000,23000.  Acts as a kill switch at the end of the sequence
long TransitionStarted;   //Time count from initiating transition
int state = 0; //Initial Start up state and idle state after timeout 


void setPWMfrequency(int freq) { //I upped the frequency of the PWM signal to quiet the motors down.
  TCCR0B = TCCR0B & 0b11111000 | freq ;
  TCCR1B = TCCR1B & 0b11111000 | freq ;
  TCCR2B = TCCR2B & 0b11111000 | freq ;
  TCCR3B = TCCR3B & 0b11111000 | freq ;
  TCCR4B = TCCR4B & 0b11111000 | freq ;
}

//Motor Identification
//Direction indicators assume
//you are looking at the gear
//side of the motor.
//
//  Shoulder Motor     R=A
//                     L=B
//
//  Center Leg Motor   R=C
//                     L=D



void MotorActiveStatus(char Side, boolean s) { //This is just shorthand to make the code simpler to write
  boolean state = s;
  if (Side == 'A') {
    digitalWrite(R_ENS, s);
  }
  if (Side == 'B') {
    digitalWrite(L_ENS, s);
  }
  if (Side == 'C') {
    digitalWrite(R_ENC, s);
  }
  if (Side == 'D') {
    digitalWrite(L_ENC, s);
  }
  
}
void setMotor(char side, byte pwm) {
  if (side == 'A') {
    analogWrite(RPWMS, pwm);
  }
  if (side == 'B') {
    analogWrite(LPWMS, pwm);
  }
  if (side == 'C') {
    analogWrite(RPWMC, pwm);
  }
  if (side == 'D') {
    analogWrite(LPWMC, pwm);
  }


}
void setup() {
  setPWMfrequency(0x02);// timers at 7.81KHz once again to quiet motors

  pinMode(THREELEGButton, INPUT);
  pinMode(TWOLEGButton, INPUT);
  pinMode(THREELEGSLOWButton, INPUT);


  pinMode(RPWMS, OUTPUT); //Shoulder
  pinMode(LPWMS, OUTPUT);
  pinMode(L_ENS, OUTPUT);
  pinMode(R_ENS, OUTPUT);

  pinMode(RPWMC, OUTPUT); //Center leg
  pinMode(LPWMC, OUTPUT);
  pinMode(L_ENC, OUTPUT);
  pinMode(R_ENC, OUTPUT);

  delay(21000); //Just a brief pause for boot of the control ADK in case I switch them on at the same time. 

  MotorActiveStatus('A', true);
  MotorActiveStatus('B', true);
  MotorActiveStatus('C', true);
  MotorActiveStatus('D', true);

  Serial.begin(9600);

  
}
  void loop() {
    if (state == 0) {
      if (digitalRead(THREELEGButton) == HIGH) {
        Serial.println("Begin two leg to three leg fast transition. Deploy center leg, rotate shoulder.");
        state = 6;
        TransitionStarted = millis();
        setMotor('C', 170); // Center leg deploy
        setMotor('A', 180); // Shoulder rotate to 3 leg position
        Serial.print("state - ");Serial.println(state);
      }

      if (digitalRead(THREELEGSLOWButton) == HIGH) {
        Serial.println("Begin two leg to three leg slow transition. Deploy center leg.");
        TransitionStarted = millis();
        setMotor('C', 150); //Center leg deploy
        state = 1;
        Serial.print("state - ");Serial.println(state);
      }

      if (digitalRead(TWOLEGButton) == HIGH) {
        Serial.println("Begin three leg to two leg transition. Rotate shoulder.");
        TransitionStarted = millis();
        setMotor('B', 250); //Shoulder rotate to 2 leg position, was 210
        state = 2;
        Serial.print("state - ");Serial.println(state);
      }

      
              
    }

    if (state == 1) {

      if (CenterLeg3LimitTriggered == false && digitalRead(CenterLeg3Limit) == HIGH) {
        CenterLeg3LimitTriggered = true;
        Serial.println("Center leg motor locked in 3 leg postion. Rotate shoulder.");
        setMotor('C', 0);   //Stop center leg motor
        delay(2000);        //Delay to let things settle
        setMotor('A', 140); //Rotate shoulder to 3 leg position
        state = 5;
      }
    }

    if (state == 2) {

      if (Shoulder2LimitTriggered == false && digitalRead(Shoulder2Limit) == HIGH) {
        Shoulder2LimitTriggered = true;
        setMotor('B', 0); //Stop shoulder motor
        Serial.println("Shoulder locked in two leg position.");
        state = 3; //Make sure shoulder is stopped before proceeding.
      }

    }

    if (state == 3) {
      delay(4000);
      Serial.println("Begin center leg retraction.");
      setMotor('D', 250); //Retract center leg.
      state = 4;

    }

    if (state == 4) {
      if (CenterLeg2LimitTriggered == false && digitalRead(CenterLeg2Limit) == HIGH) {
        CenterLeg2LimitTriggered = true;
        Serial.println("Center leg retracted. Transition complete.");
        setMotor('D', 0); //Stop Center leg
      }
    }

    if (state == 5) {
      if (Shoulder3LimitTriggered == false && digitalRead(Shoulder3Limit) == HIGH) {
        Shoulder3LimitTriggered = true;
        Serial.println("Shoulder motor locked in 3 leg postion.");
        setMotor('A', 0); //Stop shoulder motor
      }
     
      if (Shoulder3LimitTriggered == true) {
        Serial.println("Transition complete.");
        Serial.print("Shoulder2LimitTriggered - ");Serial.println(Shoulder2LimitTriggered);
        Serial.print("Shoulder3LimitTriggered - ");Serial.println(Shoulder3LimitTriggered);
      }
    }

    if (state == 6) {
      if (CenterLeg3LimitTriggered == false && digitalRead(CenterLeg3Limit) == HIGH) {
        CenterLeg3LimitTriggered = true;
        setMotor('C', 0); //Stop center leg motor
        Serial.println("Center leg motor locked in 3 leg postion.");
      }

      if (Shoulder3LimitTriggered == false && digitalRead(Shoulder3Limit) == HIGH) {
        Shoulder3LimitTriggered = true;
        setMotor('A', 0); //Stop shoulder motor
        Serial.println("Shoulder motor locked in 3 leg postion.");

      }

      if (CenterLeg3LimitTriggered == true && Shoulder3LimitTriggered == true) {
        state = 8;
        Serial.print("state - ");Serial.println(state);
      }

    }

    if (state == 8) {
       Serial.println("Transition complete.");
       //state = 0;
       Serial.print("Shoulder2LimitTriggered - ");Serial.println(Shoulder2LimitTriggered);
       Serial.print("Shoulder3LimitTriggered - ");Serial.println(Shoulder3LimitTriggered);
       

     

    }
    


    if (((millis() - TransitionStarted) > TimeOutValue) && (state != 0)) {
      Serial.println("Motor Timeout"); //Timeout the motors if any are still trying to turn AND set state back to 0.
      setMotor('A', 0);
      setMotor('B', 0);
      setMotor('C', 0);
      setMotor('D', 0);
      Shoulder3LimitTriggered = false;  //Set trigger boolean values back to false
      Shoulder2LimitTriggered = false;
      CenterLeg3LimitTriggered = false;
      CenterLeg2LimitTriggered = false;
      Serial.print("Shoulder2LimitTriggered - ");Serial.println(Shoulder2LimitTriggered);
      Serial.print("Shoulder3LimitTriggered - ");Serial.println(Shoulder3LimitTriggered);
      
      state = 0;
    }
  }


