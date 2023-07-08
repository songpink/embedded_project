#include <stdlib.h>
#include <string.h>
#include "mbed.h"
#include "LM75B.h"
#include "C12832.h"
#include "ESP-call.h"
#include "project_header.h"

void check_snake_length_thread_function(void){
    int max_length=0;
    sem_difficulty_information.acquire();
    max_length = difficulty_information.max_length;
    sem_difficulty_information.release();
    while(true){
        sem_snake_status.acquire();
        if(snake_status.fullness>=70){
            snake_status.length++;
        }
        else if(snake_status.fullness<=30){
            snake_status.length--;
            if(snake_status.length<=0){
                game_over();
            }
        }
        if(snake_status.length>=max_length){ 
            game_over();
        }
        sem_snake_status.release();

        sem_print_thread.release();
        sem_update_esp_thread.release();
        sem_pwm_thread.release();
        
        thread_sleep_for(10000);
    }
}

void check_tem_thread_function(void){
    float max_tem=0.0f;
    float min_tem=0.0f;
    float max_hum=0.0f;
    float min_hum=0.0f;
    int error;
    sem_difficulty_information.acquire();
    max_tem = difficulty_information.max_tem;
    min_tem = difficulty_information.min_tem;
    sem_difficulty_information.release();

    while(true){
        sem_snake_status.acquire();
        snake_status.tem = tem.read();
        if(snake_status.tem + snake_status.tem_offset > max_tem||snake_status.tem + snake_status.tem_offset < min_tem){
                game_over();
        }
        sem_snake_status.release();

        sem_print_thread.release();
        sem_update_esp_thread.release();
        sem_pwm_thread.release();

        thread_sleep_for(1000);
    }
}

void eat_foods_thread_function(void){
    
    while(true){

        sem_change_fullness.acquire();
        sem_snake_status.acquire();
        sem_the_number_of_foods.acquire();

        snake_status.fullness = snake_status.fullness + the_number_of_foods;
        the_number_of_foods = 0;
        if(snake_status.fullness > FULLNESS_MAX){
            game_over();
        }
        sem_the_number_of_foods.release();
        sem_snake_status.release();

        sem_print_thread.release();
        sem_update_esp_thread.release();
        sem_pwm_thread.release();

    }
}

void increase_the_number_of_foods_thread_function(void){
    while(true){
        sem_increase_the_number_of_foods.acquire();
        sem_the_number_of_foods.acquire();

        if(the_number_of_foods<99)
            the_number_of_foods++;

        sem_the_number_of_foods.release();
    }
}

void decrease_the_number_of_foods_thread_function(void){
    while(true){
        sem_decrease_the_number_of_foods.acquire();
        sem_the_number_of_foods.acquire();

        if(the_number_of_foods > 0)
            the_number_of_foods--;

        sem_the_number_of_foods.release();
    }
}


void decrease_fullness_thread_function(void){
    sem_difficulty_information.acquire();
    int time_interval_to_decrease_fullness = difficulty_information.time_interval_to_decrease_fullness;
    sem_difficulty_information.release(); 
    while(true){
        sem_snake_status.acquire();
        snake_status.fullness--;
        if(snake_status.fullness<=0){
            game_over();
        }
        sem_snake_status.release();
        
        sem_print_thread.release();
        sem_update_esp_thread.release();
        sem_pwm_thread.release();

        thread_sleep_for(time_interval_to_decrease_fullness);
    }
}


void print_thread_function(void){
    int fullness=0;
    float tem=0;
    int foods=0;
    int length=0;
    int temp_select_screen=0;
    int previous_select_screen=0;

    bool front=false;
    while(true){
        sem_print_thread.acquire();
        CriticalSectionLock::enable();
        temp_select_screen=select_screen;
        CriticalSectionLock::disable();
        
        if(temp_select_screen==0){
            if(previous_select_screen!=temp_select_screen){
                previous_select_screen = temp_select_screen;
                lcd.cls();
            }
            sem_snake_status.acquire();

            length=snake_status.length;
            fullness = snake_status.fullness;
            tem = snake_status.tem + snake_status.tem_offset;

            sem_snake_status.release();

            lcd.locate(0, 6);
            lcd.printf("full: %2d, tem: %2.0f  ", fullness, tem);
            lcd.locate(0, 16);
            lcd.printf("length: %2d", length);
        }
        else if(temp_select_screen==1){
            if(previous_select_screen!=temp_select_screen){
                previous_select_screen = temp_select_screen;
                lcd.cls();
            }
            sem_the_number_of_foods.acquire();
            foods = the_number_of_foods;
            sem_the_number_of_foods.release();
            lcd.locate(0, 6);
            lcd.printf("the_number_of_foods: %2d", the_number_of_foods);

            if(front==false){
                lcd.line(32,21,33,21,0); // left eye
                lcd.line(36,21,37,21,0); // right eye

                lcd.line(32,17,36,17,1);
                lcd.line(31,18,31,19,1);
                lcd.line(30,20,30,22,1);
                lcd.line(32,20,32,21,1); // left eye
                lcd.line(32,24,38,24,1);
                lcd.line(38,23,38,22,1);
                lcd.line(39,22,39,21,1);
                lcd.line(38,20,38,19,1);
                lcd.pixel(37,18,1);
                lcd.line(36,20,36,21,1); // right eye
                lcd.pixel(31,23,1);
                lcd.pixel(37,23,1);    //8-pixel-snake-head
                lcd.line(30,25,31,25,1);
                lcd.pixel(29,26,1);
                lcd.line(28,27,28,29,1);
                lcd.pixel(29,30,1);
                lcd.line(30,31,38,31,1);
                lcd.pixel(39,30,1);
                lcd.line(40,30,40,28,1);
                lcd.pixel(41,31,1);
                lcd.line(42,30,42,28,1);
                lcd.pixel(41,27,1);
                lcd.pixel(40,26,1);
                lcd.pixel(39,25,1);
                lcd.line(38,25,38,26,1);
                lcd.pixel(37,27,1);
                lcd.line(37,28,32,28,1);
                lcd.pixel(31,27,1);
                lcd.pixel(33,26,1);
                lcd.pixel(34,25,1);     //8-pixel-snake-body
                front=true;
            }
            else{
                lcd.line(32,20,32,21,0); // left eye
                lcd.line(36,20,36,21,0); // right eye

                lcd.line(32,17,36,17,1);
                lcd.line(31,18,31,19,1);
                lcd.line(30,20,30,22,1);
                lcd.line(32,21,33,21,1); // left eye
                lcd.pixel(31,23,1);
                lcd.line(32,24,38,24,1);
                lcd.line(38,23,38,22,1);
                lcd.line(39,22,39,21,1);
                lcd.line(38,20,38,19,1);
                lcd.pixel(37,18,1);
                lcd.line(36,21,37,21,1); // right eye
                lcd.pixel(31,23,1);
                lcd.pixel(37,23,1);    //8-pixel-snake-head
                lcd.line(30,25,31,25,1);
                lcd.pixel(29,26,1);
                lcd.line(28,27,28,29,1);
                lcd.pixel(29,30,1);
                lcd.line(30,31,38,31,1);
                lcd.pixel(39,30,1);
                lcd.line(40,30,40,28,1);
                lcd.pixel(41,31,1);
                lcd.line(42,30,42,28,1);
                lcd.pixel(41,27,1);
                lcd.pixel(40,26,1);
                lcd.pixel(39,25,1);
                lcd.line(38,25,38,26,1);
                lcd.pixel(37,27,1);
                lcd.line(37,28,32,28,1);
                lcd.pixel(31,27,1);
                lcd.pixel(33,26,1);
                lcd.pixel(34,25,1);      
                front=false;
            }
        }
    }
}



void uart_rx_thread_function(void){
    char read=0;
    int state=0;
    int foods_string_index = 0;
    char foods_string[3]={0};
    
    float max_tem=0.0;
    float min_tem=0.0;

    sem_difficulty_information.acquire();
    max_tem = difficulty_information.max_tem;
    min_tem = difficulty_information.min_tem;
    sem_difficulty_information.release();

    while(true){
        sem_uart_rx_thread.acquire();
        while(is_empty_queue(&queue)!=true){ 
            CriticalSectionLock::enable();
            read = dequeue(&queue);
            CriticalSectionLock::disable();
            PC.putc(read);
            if(state==0){
                if(read=='\002'){
                    state=1;
                }
                else if(read=='\003'){
                    sem_snake_status.acquire();
                    snake_status.tem_offset++;
                    if( (snake_status.tem + snake_status.tem_offset > max_tem) || (snake_status.tem + snake_status.tem_offset < min_tem)){
                        game_over();
                    }
                    sem_snake_status.release();

                    thread_sleep_for(1000);
                    CriticalSectionLock::enable();
                    ESP.printf("now_loading=false\r\n");
                    CriticalSectionLock::disable();

                    sem_print_thread.release();
                    sem_update_esp_thread.release();
                    sem_pwm_thread.release();
                }
                else if(read=='\004'){
                    sem_snake_status.acquire();
                    snake_status.tem_offset--;
                    if( (snake_status.tem + snake_status.tem_offset > max_tem) || (snake_status.tem + snake_status.tem_offset < min_tem)){
                        game_over();
                    }
                    sem_snake_status.release();
                    thread_sleep_for(1000);
                    CriticalSectionLock::enable();
                    ESP.printf("now_loading=false\r\n");
                    CriticalSectionLock::disable();
                    sem_print_thread.release();
                    sem_update_esp_thread.release();
                    sem_pwm_thread.release();
                }
            }
            else if(state==1){
                if(read=='\002'){
                    foods_string[foods_string_index]='\0';
                    foods_string_index=0;

                    int temp = atoi(foods_string);

                    sem_snake_status.acquire();
                    snake_status.fullness = snake_status.fullness + temp;
                    if(snake_status.fullness>FULLNESS_MAX){
                        game_over();
                    }
                    sem_snake_status.release();
                    thread_sleep_for(1000);
                    CriticalSectionLock::enable();
                    ESP.printf("now_loading=false\r\n");
                    CriticalSectionLock::disable();

                    sem_print_thread.release();
                    sem_update_esp_thread.release();
                    sem_pwm_thread.release();

                    state=0;
                }
                else{
                    foods_string[foods_string_index]=read;
                    foods_string_index++;
                }
            }
        }
    }
}

void update_esp_thread_function(void){
    int temp_fullness=0;
    int temp_length=0;
    float temp_cel=0.0;
    while(true){
        sem_update_esp_thread.acquire();

        sem_snake_status.acquire();
        temp_fullness = snake_status.fullness;
        temp_length = snake_status.length;
        temp_cel = snake_status.tem + snake_status.tem_offset;
        sem_snake_status.release();

        CriticalSectionLock::enable();
        ESP.printf("fullness=%d\r\n", temp_fullness);
        CriticalSectionLock::disable();
        CriticalSectionLock::enable();
        ESP.printf("length=%d\r\n", temp_length);
        CriticalSectionLock::disable();
        CriticalSectionLock::enable();
        ESP.printf("cel=%f\r\n", temp_cel);
        CriticalSectionLock::disable();

    }
}



void pwm_thread_function(void){
    int PWM_fullness=0;
    float PWM_tem=0;
    float max_tem=0;
    float min_tem=0;
    sem_difficulty_information.acquire();
    max_tem = difficulty_information.max_tem;
    min_tem = difficulty_information.min_tem;
    sem_difficulty_information.release();

    while(true){
        sem_pwm_thread.acquire();

        sem_snake_status.acquire();
        sem_the_number_of_foods.acquire();
        PWM_fullness = snake_status.fullness ;
        PWM_tem = snake_status.tem + snake_status.tem_offset;

        sem_snake_status.release();
        sem_the_number_of_foods.release();

        if((PWM_tem >= max_tem-3) || (PWM_tem <= min_tem+3) || (PWM_fullness <= 15)){	
            speaker = 0.5;
        }
        else{
            speaker = 0.0;
        }
    }
}
