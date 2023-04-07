#define IN0 7
#define IN1 10
#define IN2 11
#define OUT0 12
#define OUT1 27
#define OUT2 28
#define LEDA 21
#define LEDB 22
#define SWITCH_PWR_EN 0
#define COMP_PWR_EN 1
#define MAIN_RELAY 2
#define LIGHT_A 3
#define LIGHT_B 4
#define MUX_S2 20
#define MUX_S1 19
#define MUX_S0 18
#define ADC_MUX 26
#define JET_ON 15

//Since there are several 3-bit interfaces, a dedicated structure
//seemed useful. 
struct bits{
  int S2,S1,S0;
};

int read_ADC_MUX(struct bits pins){
  digitalWrite(MUX_S2, pins.S2);
  digitalWrite(MUX_S1, pins.S1);
  digitalWrite(MUX_S0, pins.S0);
  int data = analogRead(ADC_MUX);
  return data;
}

void parser(int input_char){
  
}

void shutdown(){
  pinMode(JET_ON,OUTPUT);
  digitalWrite(JET_ON,1);
  delay(1000);
  digitalWrite(JET_ON,0);
  //Wait before cutting power
  delay(30000);
  digitalWrite(SWITCH_PWR_EN,0);
  delay(50);
  digitalWrite(COMP_PWR_EN,0);
  delay(50);
  digitalWrite(LIGHT_A,0);
  delay(50);
  digitalWrite(LIGHT_B,0);
  delay(200);
  digitalWrite(MAIN_RELAY,0);
  //End of code
}

void setup() {
  // put your setup code here, to run once:
  pinMode(IN0, INPUT);
  pinMode(IN1, INPUT);
  pinMode(IN2, INPUT);
  pinMode(OUT0, OUTPUT);
  digitalWrite(OUT0,0);
  pinMode(OUT1, OUTPUT);
  digitalWrite(OUT1,0);
  pinMode(OUT2, OUTPUT);
  digitalWrite(OUT2,0);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1,0);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2,0);
  pinMode(MAIN_RELAY, OUTPUT);
  digitalWrite(MAIN_RELAY,0);
  pinMode(SWITCH_PWR_EN, OUTPUT);
  digitalWrite(SWITCH_PWR_EN,0);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i<10;i++){
    if (Serial.available()>0){
      int data = Serial.read();
      parser(data);
      delay(1000);
    }
  }
  digitalWrite(MAIN_RELAY,1);
  //Pi is in control
  delay(10000);
  
  shutdown();
}
