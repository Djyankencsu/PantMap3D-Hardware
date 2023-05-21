#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

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
#define JET_ON 15
#define BUILT_IN_LED 25
#define mask 0xffffffe0

#define InitialPower 0x00000000
#define MainRelay 0x00000004
#define CompAndSwitch 0x00000003
int32_t current_state = 0;
int32_t output_pins = 0x1c7c901f;
bool debug = false;

void evaluate_state(){
    int temp = 0;
}

void state_enforce(int32_t encoded_state, int32_t gpio_pins){
    int32_t geo_counter = 1;
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

int main(){
    stdio_init_all();
    gpio_init(BUILT_IN_LED);
    gpio_init(IN0);
    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(OUT0);
    gpio_init(OUT1);
    gpio_init(OUT2);
    gpio_init(LEDA);
    gpio_init(LEDB);
    gpio_init(SWITCH_PWR_EN);
    gpio_init(COMP_PWR_EN);
    gpio_init(MAIN_RELAY);
    gpio_init(LIGHT_A);
    gpio_init(LIGHT_B);
    gpio_init(MUX_S2);
    gpio_init(MUX_S1);
    gpio_init(MUX_S0);
    gpio_init(ADC_MUX);
    gpio_init(JET_ON);
    gpio_set_dir(BUILT_IN_LED, GPIO_OUT);
    gpio_set_dir(SWITCH_PWR_EN,GPIO_OUT);
    while (1) {
        gpio_put(BUILT_IN_LED, 1);
        current_state = 0x1fffffff;
        state_enforce(current_state,output_pins);
        sleep_ms(1000);
        gpio_put(BUILT_IN_LED, 0);
        current_state = 0;
        state_enforce(current_state,output_pins);
        sleep_ms(1000);
        //asm(
        //    "BL state_enforce"
        //);
    }
}