#include <Arduino.h>  // needed for PlatformIO
#include <Servo.h>
#include <math.h>
#include <Ethernet.h>
#include <SPI.h>
#include <Ramp.h>

#define SPEED 10  // degree/s

int Speed = 150;

const unsigned long interval = 1000;  // 1 second

// Variable to store the last time the message was printed
unsigned long previousMillis = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED  //MAC Address
};

int pos1 = 90;
int pos2 = 90;
int rlPos = 86;
int udPos = 90;
int SPos;
int EPos;
int duration = 100;

IPAddress ip(192, 168, 1, 177);  //Custom IP address
//IPAddress espCamIP(192, 168, 89, 11); // IP address of the server
EthernetServer server(80);  //Server port

Servo FFlipper;
Servo BFlipper;

Servo LeftMotor;
Servo RightMotor;
Servo rlWrist;
Servo udWrist;
Servo tWrist;
Servo LFinger;
Servo RFinger;

Servo shoulder;
Servo elbow;

// Arm lengths
const float L1 = 202;
const float L2 = 159;

int i = 0;

String stringCommand = "";
String commandTemp = "";

float x_d = 200;
float y_d = 100;
float target_x = 250;
float target_y = 150;
float step_size = 1.0;

rampInt myAngle;  // create a byte ramp object to interpolate servo position
byte previousAngle = 0;

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing");

  pinMode(LED_BUILTIN, OUTPUT);

  LeftMotor.attach(3);
  RightMotor.attach(4);

  FFlipper.attach(A0);
  BFlipper.attach(A1);

  FFlipHome();
  BFlipHome();

  shoulder.attach(A4);
  elbow.attach(A3);
  rlWrist.attach(A8);  //LeftRightWrist
  udWrist.attach(8);   //UpDownWrist
  tWrist.attach(A7);   //TurnWrist
  LFinger.attach(6);   //right
  RFinger.attach(2);   //left
  homeServos();
  EthernetInit();
}

void loop() {
  EthernetClient client = server.available();  // Client Object variable
  if (client) {                                // If client is connected to server
    while (client.connected()) {               // While client is connected
      char command;
      if (client.available()) {                     // Client send a packet
        command = client.read();                    // Receive Command
        stringCommand += command;                   // Append the character to the String
      } else if (stringCommand.endsWith("\n\r")) {  // Assuming commands end with a newline character
        stringCommand = removeUnwantedChars(stringCommand, "\n\r");
        //Serial.println(stringCommand);
        commandTemp = stringCommand;
        stringCommand = "";
      } 
      
      //Motor and Flippers Commands
      else if (commandTemp == "W") {                //Robot moves forward
        Forward();
      } else if (commandTemp == "A") {              //Robot turns left
        Left();
      } else if (commandTemp == "S") {              //Robot moves backward
        Backward();
      } else if (commandTemp == "D") {              //Robot turns right
        Right();
      } 
      
      else if (commandTemp == "Space") {            //All servos goes to home position
        FFlipHome();
        BFlipHome();
        homeServos();
        commandTemp = "";
      } else if (commandTemp == "C") {              //Front flipper goes to home position
        FFlipHome();
        commandTemp = "";
      } else if (commandTemp == "V") {              //Back flipper goes to home position
        BFlipHome();
        commandTemp = "";
      } 
      
      else if (commandTemp == "U") {                //Front flipper rotate upwards
        if (pos1 >= 10) {
          pos1 = pos1 - 1;
          FFlipper.write(pos1);
          String string = "F:" + pos1;
          client.print(string);
          delay(5);
        } else {
          pos1 = 10;
        }
      } else if (commandTemp == "J") {              //Front flipper rotate downwards
        if (pos1 <= 160) {
          pos1 = pos1 + 1;
          FFlipper.write(pos1);
          String string = "F:" + pos1;
          client.print(string);
          delay(5);
        } else {
          pos1 = 160;
        }
      }

      else if (commandTemp == "I") {                //Back flipper rotate upwards
        if (pos2 >= 10) {
          pos2 = pos2 - 1;
          BFlipper.write(pos2);
          String string = "B:" + pos2;
          client.print(string);
          delay(5);
        } else {
          pos2 = 10;
        }
      } else if (commandTemp == "K") {              //Back flipper rotate downwards
        if (pos2 <= 160) {
          pos2 = pos2 + 1;
          BFlipper.write(pos2);
          String string = "B:" + pos2;
          client.print(string);
          delay(5);
        } else {
          pos1 = 160;
        }
      }

      else if (commandTemp == "F") {                //Front flipper goes on a straight position
        FFlipStretch();
        commandTemp = "";
      } else if (commandTemp == "G") {              //Back flipper goes on a straight position
        BFlipStretch();
        commandTemp = "";
      }

      else if (commandTemp == "Z") {                //Front flipper goes on a standing position
        FFlipDog();
        commandTemp = "";
      } else if (commandTemp == "X") {              //Back flipper goes on a standing position
        BFlipDog();
        commandTemp = "";
      } 
      
      else if (commandTemp == "R") {                //Z Position
        FFlipZ();
        commandTemp = "";
      } else if (commandTemp == "T") {              //Z Position
        BFlipZ();
        commandTemp = "";
      }

      //Robot Arm Commands
      else if (commandTemp == "NumPad7") {          //Shoulder Joint goes upwards
        shoulderU();
      } else if (commandTemp == "NumPad1") {        //Shoulder Joint goes downwards
        shoulderD();
      }

      else if (commandTemp == "NumPad8") {          //Elbow Joint goes upwards
        elbowU();
      } else if (commandTemp == "NumPad2") {        //Elbow Joint goes downwards
        elbowD();
      }

      else if (commandTemp == "NumPad9") {          //Wrist Joint goes upwards
        wristU();
      } else if (commandTemp == "NumPad3") {        //Wrist Joint goes downwards
        wristD();
      }

      else if (commandTemp == "NumPad4") {          //Wrist Joint turn left
        wristL();
      } else if (commandTemp == "NumPad6") {        //Wrist Joint turn right
        wristR();
      }

      else if (commandTemp == "NumPad0") {          //All robot servos goes to home position
        homeServos();
      }

      else if (commandTemp == "KeyUp") {
        commandTemp = "";
        MStop();
      }
    }
    client.stop();  // Disconnect communication
  }
}

void Backward() {
  LeftMotor.write(0);
  RightMotor.write(180);
}

void Forward() {
  LeftMotor.write(180);
  RightMotor.write(0);
}

void Right() {
  LeftMotor.write(180);
  RightMotor.write(180);
}

void Left() {
  RightMotor.write(0);
  LeftMotor.write(0);
}

void MStop() {
  LeftMotor.write(90);
  RightMotor.write(90);
}

String removeUnwantedChars(String input, String charsToRemove) {
  String result = "";
  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (charsToRemove.indexOf(c) == -1) {  // If the character is not in the charsToRemove list
      result += c;                         // Append it to the result
    }
  }
  return result;
}

void homeServos() {
  shoulder.write(15);  //1            Shoulder
  elbow.write(162);    //2           Elbow
  rlWrist.write(83);   //3หันซ้ายขวา    LeftRightWrist
  udWrist.write(89);   //4หมุนมือขึ้นลง  UpDownWrist
  tWrist.write(90);    //5หมุนมือ       TurnWrist
  LFinger.write(90);   //6มือ     right
  RFinger.write(1);    //7มือ       left

  SPos = shoulder.read();
  EPos = elbow.read();
}

void shoulderU() {
  if (SPos <= 125) {
    SPos = SPos + 1;
    shoulder.write(SPos);
    delay(15);
  } else {
    SPos = 125;
  }
}

void shoulderD() {
  if (SPos >= 15) {
    SPos = SPos - 1;
    shoulder.write(SPos);
    delay(15);
  } else {
    SPos = 15;
  }
}

void elbowU() {
  if (EPos >= 95) {
    EPos = EPos - 1;
    elbow.write(EPos);
    delay(15);
  } else {
    EPos = 95;
  }
}

void elbowD() {
  if (EPos <= 162) {
    EPos = EPos + 1;
    elbow.write(EPos);
    delay(15);
  } else {
    EPos = 162;
  }
}

void wristR() {
  if (rlPos >= 50) {
    rlPos = rlPos - 1;
    rlWrist.write(rlPos);
    delay(20);
  } else {
    rlPos = 50;
  }
}

void wristL() {
  if (rlPos <= 136) {
    rlPos = rlPos + 1;
    rlWrist.write(rlPos);
    delay(20);
  } else {
    rlPos = 136;
  }
}

void wristU() {
  if (rlPos >= 55) {
    udPos = udPos - 1;
    udWrist.write(udPos);
    delay(20);
  } else {
    udPos = 55;
  }
}

void wristD() {
  if (rlPos <= 144) {
    udPos = udPos + 1;
    udWrist.write(udPos);
    delay(20);
  } else {
    udPos = 144;
  }
}

void FFlipHome() {
  FFlipper.write(10);
  pos1 = FFlipper.read();
}

void BFlipHome() {
  BFlipper.write(10);
  pos2 = BFlipper.read();
}

void FFlipDog() {
  FFlipper.write(150);
  pos1 = FFlipper.read();
}

void BFlipDog() {
  BFlipper.write(150);
  pos2 = BFlipper.read();
}

void FFlipStretch() {
  FFlipper.write(85);
  pos1 = FFlipper.read();
}

void BFlipStretch() {
  BFlipper.write(85);
  pos2 = BFlipper.read();
}

void FFlipZ() {
  FFlipper.write(45);
  pos1 = FFlipper.read();
  BFlipper.write(115);
  pos2 = BFlipper.read();
}

void BFlipZ() {
  FFlipper.write(115);
  pos1 = FFlipper.read();
  BFlipper.write(45);
  pos2 = BFlipper.read();
}
