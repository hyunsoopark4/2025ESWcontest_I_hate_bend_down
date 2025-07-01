//서보모터
#include <Servo.h>
#include <NeoSWSerial.h>
#include "bt_serial.h"

Servo myservo;  // create servo object to control a servo

#define SERVO_PIN 10    // D10
#define SERVO_ON 125    // servo motor on time : 180deg / 0.250 sec
#define LEFT_DEGREE 135
#define FRONT_DEGREE 90
#define RIGHT_DEGREE 45
#define CYCLE_DEGREE 5 // the degree cycled per the assigned tick
//DC모터
#define R_IA  6    // Right Motor IA (must be PWM output)
#define R_IB  11
#define L_IA  3    // Left Motor IA (must be PWM output)
#define L_IB  5

#define OPT_SPEED 200   // DC Motor speed 200 recommended (Min.150 ~ Max.255)
#define TURN_DURATION 150   // turn in TURN_DURATION time
#define TURNBACK_DURATION 425 //
#define STRAIGHT_TICKRATE 30
enum each_func {L_MOTOR, R_MOTOR, FORWARD, BACK, LEFT_TURN, R_TURN};

//초음파센서
#define TRIG_PIN  8
#define ECHO_PIN  9

#define OBS 30          // Distance of obstacles [Cm]

//BT
#define COUNTOF_COMMAND 2
#define COUNTOF_KEY 5
#define BUFFERSIZE 64
#define AUTO_MODE 0
#define MANUAL_MODE 1
#define MANUAL_TICKRATE

#define LOOP_LATENCY 50

char clientRead;
char command[COUNTOF_COMMAND][BUFFERSIZE] = {"auto","manual"};
char sendBuffer[BUFFERSIZE];
char key[COUNTOF_KEY] = "sflrb";
int currentMode=MANUAL_MODE;

NeoSWSerial mySerial(12, 13); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);         // **  9600 Baud rate should be. **
  mySerial.begin(9600);

  //serial verify
  Serial.println("Serial initiated");
  mySerial.println("BT Serial initiated");

  //서보모터
  myservo.detach();  //detach the servo
  servo_front();

  //DC모터
  car_stop();

  //초음파
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  clientRead = -1; //init client message

  if (mySerial.available()) {//only when recieved
    clientRead = mySerial.read();//read from client

    Serial.println(clientRead);

    switch(clientRead){
      case 's':car_stop(); break;
      case 'f':forward(OPT_SPEED); break;
      case 'l':left_turn(OPT_SPEED); break;
      case 'r':r_turn(OPT_SPEED); break;
      case 'b':back(OPT_SPEED); break;
    }

    if(clientRead=='/'||clientRead=='!'){
      call_command();
    }
    return;
  }
  else{
    if(currentMode==AUTO_MODE){
      autopilot();
    }
    return;
  }
}

void autopilot()
{
  car_stop();
  Serial.println("autopilot initiated");

  servo_front(); //look left
  if(!is_obstacle()) //no obstacle left?
  {
    servo_front(); //init servo
    //Dont turn
    Serial.println("heading front");
    go_straight(); //and go straight
    
    return;
  }

  servo_left(); //look left
  if(!is_obstacle()) //no obstacle left?
  {
    servo_front(); //init servo
    turn_left(TURN_DURATION); //then turn left
    Serial.println("heading left");
    return;
  }

  servo_front(); //init servo

  servo_right(); //look left
  if(!is_obstacle()) //no obstacle left?
  {
    servo_front(); //init servo
    turn_right(TURN_DURATION); //then turn left
    Serial.println("heading right");
    return;
  }
  
  //obstacle everywhere
  Serial.println("turning back");
  servo_front(); //init servo
  turn_back(); //then turn back
  //don't let go
  return;

}


void call_command()
{
  int n=0;
  int i=0;
  char buffer[BUFFERSIZE];
  memset(buffer, 0, sizeof(buffer));  // buffer 배열 초기화

  delay(LOOP_LATENCY);

  while (mySerial.available() && n < BUFFERSIZE - 1) {
    char ch = mySerial.read();

    // 명령 구분 문자 도달 시 종료 (예: 개행문자)
    if (ch == '\n' || ch == '\r') break;

    buffer[n++] = ch;
  }
  buffer[n] = '\0';  // null 종료

  while(i<COUNTOF_COMMAND)
  {
    if(strcmp(buffer,command[i])==0){
      break;
    }
    i++;
  }

  switch(i){
    case 0:switchmode(AUTO_MODE);break;
    case 1:switchmode(MANUAL_MODE);break;
    case 2:
    snprintf(sendBuffer,sizeof(sendBuffer),"Unknown Command: %s\n",buffer);
    //mySerial.write(sendBuffer);
    printf_chunked(mySerial,"Unknown Command: %s\n",buffer);
    Serial.write(sendBuffer);
    break;
  }

  return;
}

void switchmode(int modifiedMode)
{
  currentMode=modifiedMode;
  //mySerial.write("The mode has been set to "); mySerial.write(command[currentMode]); mySerial.write(".\n");
  //Serial.write("The mode has been set to "); Serial.write(command[currentMode]); Serial.write(".\n");
  snprintf(sendBuffer, sizeof(sendBuffer), "The mode has been set to %s.\n", command[currentMode]);
  printf_chunked(mySerial,"The mode has been set to %s.\n", command[currentMode]);
  //mySerial.write(sendBuffer);  // BT 출력 안정화
  Serial.write(sendBuffer);    // USB Serial에도 동일하게 출력
}


//서보모터
void servo_front()
{
  myservo.attach(SERVO_PIN);  // attaches the servo
  myservo.write(FRONT_DEGREE);          // front position
  delay(SERVO_ON);
  myservo.detach();  //detach the servo
}
void servo_left()
{
  myservo.attach(SERVO_PIN);  // attaches the servo
  myservo.write(LEFT_DEGREE);          // left position
  delay(SERVO_ON);
  myservo.detach();  //detach the servo
}
void servo_right()
{
  myservo.attach(SERVO_PIN);  // attaches the servo
  myservo.write(RIGHT_DEGREE);          // right position
  delay(SERVO_ON);      // double because shift from 180 degree to 0 degree
  myservo.detach();  //detach the servo
}


//DC 모터
void go_straight() //코드 예쁜거 나발이고 걍 했음
{
  int darr[3][2]={{90,180},{180,0},{0,90}};
  int loop=0;
  int deg=darr[loop][0];
  int degf=darr[loop][1];

  myservo.attach(SERVO_PIN);
  forward(OPT_SPEED);

  while(1){
    myservo.write(deg);
    delay(STRAIGHT_TICKRATE);

    if(!(deg%15)){  //only call is_obstacle func EVERY 15x degree, in order to save resources. 
      if(is_obstacle()){
        return 1;
      }
    }

    if(loop==0){
      deg+=CYCLE_DEGREE;
      if(deg>degf){
        loop++;
        deg=darr[loop][0];
        degf=darr[loop][1];
      }
      continue;
    }
    if(loop==1){
      deg-=CYCLE_DEGREE;
      if(deg<degf){
        loop++;
        deg=darr[loop][0];
        degf=darr[loop][1];
      }
      continue;
    }
    if(loop==2){
      deg+=CYCLE_DEGREE;
      if(deg>degf){
        loop=0;
        deg=darr[loop][0];
        degf=darr[loop][1];
      }
      continue;
    }
  }
}



void turn_left(int duration)
{
  left_turn(OPT_SPEED);
  delay(duration);
  car_stop();
  return;
}

void turn_right(int duration)
{
  r_turn(OPT_SPEED);
  delay(duration);
  car_stop();
  return;
}

void turn_back()
{
  r_turn(OPT_SPEED);
  delay(TURNBACK_DURATION);
  car_stop();
  return;
}

void car_stop()
{
  analogWrite(R_IA,0);
  analogWrite(R_IB,0);
  analogWrite(L_IA,0);
  analogWrite(L_IB,0);
  
}

void r_motor_on(int speed)
{
  analogWrite(R_IA,speed);    // right motor (+ direction)
  analogWrite(R_IB,0);
  analogWrite(L_IA,0);
  analogWrite(L_IB,0);   
   
}

void l_motor_on(int speed)
{
  analogWrite(R_IA,0); 
  analogWrite(R_IB,0);
  analogWrite(L_IA,speed);   // left motor (- direction)
  analogWrite(L_IB,0);    
}
void forward(int speed)
{
  analogWrite(R_IA,speed);    
  analogWrite(R_IB,0);
  analogWrite(L_IA,0);
  analogWrite(L_IB,speed);  
}

void back(int speed)
{
  analogWrite(R_IA,0);    
  analogWrite(R_IB,speed);
  analogWrite(L_IA,speed);
  analogWrite(L_IB,0);  
}

void left_turn(int speed)
{
  analogWrite(R_IA,speed);    // right motor in forward direction
  analogWrite(R_IB,0);
  analogWrite(L_IA,speed);    // left motor in backward direction
  analogWrite(L_IB,0);
}

void r_turn(int speed)
{
  analogWrite(R_IA,0);
  analogWrite(R_IB,speed);
  analogWrite(L_IA,0);
  analogWrite(L_IB,speed);
}

//초음파
bool is_obstacle(){
  int distanceCM = get_ultrasonic_cm();

  if(2 <= distanceCM && distanceCM <= OBS)
  {
    return 1;
  }
  return 0;
}

int get_ultrasonic_cm()
{
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 11765);
  int distanceCM = duration * 0.017;
  
  return distanceCM;
}



