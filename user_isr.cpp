#include <stdlib.h>
#include <string.h>
#include "mbed.h"
#include "LM75B.h"
#include "C12832.h"
#include "ESP-call.h"
#include "project_header.h"

void joystick_up_ISR_function(void){
    sem_increase_the_number_of_foods.release();
}
void joystick_down_ISR_function(void){
    sem_decrease_the_number_of_foods.release();
}
void joystick_center_ISR_function(void){
    sem_change_fullness.release();
}

void joystick_left_ISR_function(void){
    if(select_screen<1){
        select_screen++;
    }
    sem_print_thread.release();
}
void joystick_right_ISR_function(void){
    if(select_screen>0){
        select_screen--;
    }
    sem_print_thread.release();
}


void uart_ISR_function(void){
    while(ESP.readable()!=0){
        enqueue(&queue, ESP.getc());
    }
    sem_uart_rx_thread.release();
}
