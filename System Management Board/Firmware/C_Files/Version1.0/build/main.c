#include "stdio.h"
#include "string.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"

#define IN0 7
#define IN1 10
#define IN2 11
#define OUT0 12
#define OUT1 27
#define OUT2 28
#define LEDA 22
#define LEDB 21
#define SWITCH_PWR_EN 0
#define COMP_PWR_EN 1
#define MAIN_RELAY 2
#define LIGHT_A 3
#define LIGHT_B 4
#define MUX_S2 20
#define MUX_S1 19
#define MUX_S0 18
#define ADC_MUX 26
#define ADC_MUX_CHANNEL 0
#define JET_ON 15
#define BUILT_IN_LED 25
#define mask 0xffffffe0

#define InitialPower 0x00000000
#define MainRelay 0x00000004
#define CompAndSwitch 0x00000003
uint32_t current_state = 0;
uint32_t all_pins = 0x1f7c9c9f;
uint32_t output_pins = 0x1c7c901f;
uint32_t input_pins = 0x04000c80;

int volt_threshold = (1 << 9);

typedef struct bit_holder{
    int S2, S1, S0;
} bits;
const bits AUX_Voltage = {0,0,1};
const bits Temp_Sensor1 = {1,1,0};
const bits Temp_Sensor2 = {1,1,1};

typedef struct process_monitor{
    bool in_process;
    uint64_t start_time;
} monitor;
monitor sd_now = {false,0};
monitor debug = {false, 0};
uint64_t debug_time = 0;

const bool American = true;

void set_mux(bits pins){
  sleep_ms(10);
  gpio_put(MUX_S2,pins.S2);
  gpio_put(MUX_S1,pins.S1);
  gpio_put(MUX_S0,pins.S0);
  sleep_ms(10);
}

uint read_ADC_MUX(bits pins){
  adc_select_input(ADC_MUX_CHANNEL);
  set_mux(pins);
  uint data = adc_read();
  return data;
}

int check_pow(){
    adc_select_input(ADC_MUX_CHANNEL);
    uint voltage = read_ADC_MUX(AUX_Voltage);
    if (voltage > volt_threshold){
        return 1;
    }
    else {
        return 0;
    }
}

void evaluate_state(uint64_t time){
    bool holder = (bool)check_pow();
    if (!holder){
        sd_now.start_time = time_us_64();
        sd_now.in_process = true;
    }
    else if ((time > 10000000) && holder) {
        current_state = MainRelay;
    }
    else if ((time > 20000000) && holder){
        current_state = MainRelay + CompAndSwitch;
    }
}

void shutdown_process(uint64_t input_time){
    uint64_t relative_time = input_time - (sd_now.start_time + debug_time);
    if (relative_time > 20000000){
        current_state = InitialPower;
        watchdog_enable(5000,1);
        //Once this passes to the gpio_puts_masked, this will be end of program.
        //If it does not shutdown, then a watchdog is enabled
        //to force a reboot because an error has likely occured. 
    }
    else if (relative_time > 11000000){
        gpio_put(JET_ON,0);
    }
    else if (relative_time > 10000000){
        gpio_put(JET_ON,1);
    }
}


/* This function was rendered unnecessary
by the gpio_put_masked built-in function, 
but I left it here temporarily. 

void state_enforce(uint32_t encoded_state, uint32_t gpio_pins){
    uint32_t geo_counter = 1;
    bool cond_holder;
    for (int i = 0; i <= 29; i++){
        if ((geo_counter & gpio_pins)>0){
            //Bool type casting should make any non-zero result true here.
            cond_holder = (bool)(encoded_state & geo_counter);
            gpio_put(i, cond_holder);
            printf("%d:%d ",i,cond_holder);
        }
        geo_counter *= 2;
    }
    printf("\n");
}
*/

void toggle_pin(int pin){
    uint32_t pin_mask = (1 << pin);
    uint32_t pin_state = !((current_state && pin_mask)>>pin);
    gpio_put(pin, pin_state);
    current_state = ((~pin_mask) | current_state) | (pin_state << pin);
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

float check_temp(int sensor){
  float temp = 0.0;
  if (sensor == 1){
    temp = (float)read_ADC_MUX(Temp_Sensor1);
  }
  else if (sensor == 2){
    temp = (float)read_ADC_MUX(Temp_Sensor2);
  }
  temp = convt_temp(temp);
  return temp;
}

void parser(int input_char){
  switch (input_char) {
    //"M" toggles the main relay
    case 77:
      toggle_pin(MAIN_RELAY);
    break;
    case 83:
      //"S" toggles the switch relay
      toggle_pin(SWITCH_PWR_EN);
    break;
    //"C" toggles the computer relay
    case 67:
      toggle_pin(COMP_PWR_EN);
    break;
    //"J" toggles the Jetson on pin
    case 74:
      toggle_pin(JET_ON);
    break;
    //"T" prints temperatures over serial
    case 84:
      printf("%f ",check_temp(1));
      printf("%f\n",check_temp(2));
    break;
    //"a" toggles LEDA
    case 97:
      toggle_pin(LEDA);
    break;
    //"b" toggles LEDB
    case 98:
      toggle_pin(LEDB);
    break;
    //"A" toggles LIGHT_A
    case 65:
      toggle_pin(LIGHT_A);
    break;
    //"B" toggles LIGHT_B
    case 66:
      toggle_pin(LIGHT_B);
    break;
    //"O" enables output pin control parsing expects 4 chars like "O001" MSB (2) to LSB (0)
    case 79:{
      uint32_t input_string[3];
      input_string[2] = getchar()-48;
      input_string[1] = getchar()-48;
      input_string[0] = getchar()-48;
      uint32_t state_update = (input_string[0] << OUT0) + (input_string[1] << OUT1) + (input_string[2] << OUT2);
      uint32_t outputs = (1 << OUT0) + (1 << OUT1) + (1 << OUT2);
      current_state = (current_state & (~outputs))+state_update;
      gpio_put_masked(output_pins,current_state);
      printf("\n");
    }
    break;
    //"I" enables input pin value reading
    case 73:{
      int holder[3];
      holder[0] = gpio_get(IN0);
      holder[1] = gpio_get(IN1);
      holder[2] = gpio_get(IN2);
      printf("I%1d%1d%1d\n",holder[2],holder[1],holder[0]);
    }
    break;
    //"K" runs shutdown procedure
    case 75:
      debug.in_process = false;
      sd_now.in_process = true;
    break;
    //"V" reads voltage of input from ADC mux
    case 86:
    {
      uint voltage = read_ADC_MUX(AUX_Voltage);
      printf("%d\n",voltage);
    }
    break;
    case 100:
      debug.in_process = false;
    break;
  }
  printf("Input \"%c\": done\n", input_char);
}

uint64_t debug_mode(){
    printf("Ready!\n");
    while (debug.in_process) {
        int holder;
        holder = getchar_timeout_us(0);
        if (holder != -1){
            parser(holder);
        }
    }
    printf("Exiting debug mode\n");
    return time_us_64() + debug_time;
}

int main(){
    stdio_init_all();
    //Found these neat predefined functions in the SDK
    //for setting pins from a bitmap.
    gpio_init_mask(all_pins);
    gpio_set_dir_out_masked(output_pins);
    gpio_set_dir_in_masked(input_pins);
    //Configuring ADC input is separate
    adc_init();
    adc_gpio_init(ADC_MUX);
    while (1) {
        uint64_t time_ref = time_us_64() + debug_time;
        if (debug.in_process) {
            printf("Entering debug mode\n");
            debug.start_time = time_ref;
            debug_time += debug_mode() - debug.start_time;
        }
        if (!sd_now.in_process){
            evaluate_state(time_ref);
        }
        else {
            shutdown_process(time_ref);
        }
        gpio_put_masked(output_pins,current_state);
        if (getchar_timeout_us(0)==100){
            debug.in_process = true;
        }
    }
    //Code should NEVER go beyond here. If it does, reboot. 
    watchdog_enable(1,1);
}