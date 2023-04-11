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

bool American = true; 

//Since there are several 3-bit interfaces, a dedicated structure
//seemed useful. 
struct bits{
  int S2,S1,S0;
};

void set_mux(struct bits pins){
  delay(10);
  digitalWrite(MUX_S2,pins.S2);
  digitalWrite(MUX_S1,pins.S1);
  digitalWrite(MUX_S0,pins.S0);
  delay(10);
}

int read_ADC_MUX(struct bits pins){
  set_mux(pins);
  int data = analogRead(ADC_MUX);
  set_mux({0,0,0});
  return data;
}

float convt_temp(float temp){
  //The MCP9700T is supposed to have temp coeff Vc = 10mV/˚C and V(0˚) = 400mv. 
  //The datasheet gives the output equation Vout = Va*Vc+V(0˚), where Va = 
  //the ambient temperature. 
  //4096 is based on the RPi Pico's 12-bit ADC readings
  float Vc = 10.0/1000.0;
  float V0 = 400.0/1000.0;
  temp = temp*(5.0/4096.0);
  temp = temp-V0;
  temp = temp/Vc;
  if (American){
    //Fahrenheit conversion
    temp = (temp*1.8)+32.0;
  }
  return temp;
}

int check_temp(){
  struct bits temp_sensor1_mux = {1,1,0};
  struct bits temp_sensor2_mux = {1,1,1};
  float temp1 = (float)read_ADC_MUX(temp_sensor1_mux);
  float temp2 = (float)read_ADC_MUX(temp_sensor2_mux);
  temp1 = convt_temp(temp1);
  temp2 = convt_temp(temp2);
  float holder = [temp1,temp2];
  return holder;
}

void parser(int input_char){
  
}

void check_pow(){
  struct bits mux_selector = {0,0,1]};
  int voltage = Read_ADC_MUX(mux_selector);
  int threshold = 2048; //an element in [0,4096]
  if (voltage >= threshold){
    digitalWrite(SWITCH_PWR_EN,1);
    delay(50);
    digitalWrite(COMP_PWR_EN,1);
  }
  else {
    shutdown();
  }
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
  pinMode(LIGHT_A,OUTPUT);
  digitalWrite(LIGHT_A,0);
  pinMode(LIGHT_B,OUTPUT);
  digitalWrite(LIGHT_B,0);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1,0);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2,0);
  pinMode(MAIN_RELAY, OUTPUT);
  digitalWrite(MAIN_RELAY,0);
  pinMode(SWITCH_PWR_EN, OUTPUT);
  digitalWrite(SWITCH_PWR_EN,0);
  Serial.begin(115200);
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
  check_pow();
}

void loop() {
  // put your main code here, to run repeatedly:
  struct bits input_bits;
  input_bits = {digitalRead(IN0),digitalRead(IN1),digitalRead(IN2)};
  //Input 11* Light A, 1*1 Light B, 01* LED A, 0*1 LED B
  digitalWrite(input_bits.S0 && input_bits.S1, LIGHT_A);
  digitalWrite(input_bits.S0 && input_bits.S2, LIGHT_B);
  digitalWrite(!input_bits.S0&&input_bits.S1,LED_A);
  digitalWrite(!input_bits.S0&&input_bits.S2,LED_B);

  struct bits mux_selector;
  mux_selector = {0,0,1};
  int voltage = read_ADC_MUX(mux_selector);

  shutdown();
}
