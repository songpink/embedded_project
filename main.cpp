#include <stdlib.h>
#include <string.h>
#include "mbed.h"
#include "LM75B.h"
#include "C12832.h"
#include "ESP-call.h"
#include "project_header.h"

C12832 lcd(D11, D13, D12, D7, D10);
LM75B tem(D14, D15);
PwmOut speaker(D6);

Semaphore sem_snake_status(1, 1);
Semaphore sem_difficulty_information(1, 1);
Semaphore sem_get_start_information(0, 1);
Semaphore sem_game_over(0, 1);
Semaphore sem_change_fullness(0, 1);
Semaphore sem_the_number_of_foods(1, 1);
Semaphore sem_increase_the_number_of_foods(0, 1);
Semaphore sem_decrease_the_number_of_foods(0, 1);
Semaphore sem_print_thread(0, 1);
Semaphore sem_uart_rx_thread(1, 1);
Semaphore sem_update_esp_thread(0, 1);
Semaphore sem_pwm_thread(0, 1);

Difficulty_information difficulty_information;
Snake_status snake_status;
int the_number_of_foods;
char difficulty;
int current_score;
int select_screen=0;

InterruptIn joystick_up(A2);
InterruptIn joystick_down(A3);
InterruptIn joystick_center(D4);
InterruptIn joystick_left(A4);
InterruptIn joystick_right(A5);



Timer timer;

NodeMCU_PGM scripts[] = {
    "", 1,
    
    "wifi.setmode(wifi.STATION)", 1,
    
    "station_cfg={}", 1, 
    "station_cfg.ssid=\"TestNet\"", 1,
    "station_cfg.pwd=\"\"", 1,
    "station_cfg.save=false", 1,
    "wifi.sta.config(station_cfg)", 1,
    "wifi.sta.connect()", 100,
    "print(wifi.sta.status())", 1,
    "print(wifi.sta.getip())", 1,

    "name=\"default\"", 1,
    "level=\"0\"", 1,
    "game_started=false", 1,
    "now_loading=true", 1,
    "cel=0.0", 1,
    //"hum=0.0", 1,
    "fullness=0", 1,
    "length=0", 1,

    "http_resp = \"HTTP/1.0 200 OK\\r\\nContent-Type: text/html\\r\\n\\r\\n\"", 1,
    "html_main_1 = \"<form method=\\\"POST\\\">\"", 1,
    "html_main_2 = \"Snake's Name <input type=\\\"text\\\" name=\\\"snake_name\\\" placeholder=\\\"name\\\" value=\\\"default\\\" minlength=\\\"1\\\" maxlength=\\\"8\\\"> \"", 1,
    "html_main_3 = \"<h1>Select the difficulty level!</h1>\"", 1,
    "html_main_4 = \"EASY <input type=\\\"radio\\\" name=\\\"level\\\" value=\\\"0\\\" checked> \"", 1,
    "html_main_5 = \"NORMAL <input type=\\\"radio\\\" name=\\\"level\\\" value=\\\"1\\\"> \"", 1,
    "html_main_6 = \"HARD <input type=\\\"radio\\\" name=\\\"level\\\" value=\\\"2\\\"> \"", 1,
    "html_main_7 = \"<input type=\\\"hidden\\\" name=\\\"dummy\\\" value=\\\"dummy\\\">\"", 1,
    "html_main_8 = \"<input type=\\\"submit\\\">\"", 1,
    


    "html_loading = \"<h1>now loading..</h1>\"", 1,
    "                   html_main_2_input_1 = \"<form method=\\\"POST\\\">\"", 1,
    "                   html_main_2_input_2 = \"Enter the number of foods: <input type=\\\"number\\\" min=\\\"0\\\" max=\\\"99\\\" name=\\\"the_number_of_foods\\\" placeholder=\\\"number\\\" value=\\\"0\\\"> \"", 1,
    "                   html_main_2_input_3 = \"<input type=\\\"hidden\\\" name=\\\"dummy\\\" value=\\\"dummy\\\">\"", 1,
    "                   html_main_2_input_4= \"<input type=\\\"submit\\\">\"", 1,
    "                   html_main_2_input_5 = \"</form>\"", 1,
    "                   html_main_2_input_6 = \"<form method=\\\"POST\\\">\"", 1,
    "                   html_main_2_input_7 = \"<input type=\\\"hidden\\\" name=\\\"up\\\" value=\\\"up\\\">\"", 1,
    "                   html_main_2_input_8 = \"Air Conditioner: <input type=\\\"submit\\\" value=\\\"Up\\\"> \"", 1,
    "                   html_main_2_input_9 = \"</form>\"", 1,
    "                   html_main_2_input_10 = \"<form method=\\\"POST\\\">\"", 1,
    "                   html_main_2_input_11 = \"<input type=\\\"hidden\\\" name=\\\"down\\\" value=\\\"down\\\">\"", 1,
    "                   html_main_2_input_12 = \"<input type=\\\"submit\\\" value=\\\"Down\\\"> \"", 1,
    "                   html_main_2_input_13 = \"</form>\"", 1,

    "http_redirect = \"HTTP1.0/ 302 Found\\r\\nLocation: /\"", 1,

    "html_good = \"<img src=\\\"https://i.ibb.co/Fx4pqKP/image.gif\\\" alt=\\\"good\\\" border=\\\"0\\\"></a>\"", 1,
    "html_so_so = \"<img src=\\\"https://i.ibb.co/KbXtKV5/image.gif\\\" alt=\\\"so_so\\\" border=\\\"0\\\"></a>\"", 1,
    "html_bad = \"<img src=\\\"https://i.ibb.co/v1qhtxF/image.gif\\\" alt=\\\"bad\\\" border=\\\"0\\\"></a>\"", 1,


    "function send_html_main(sck)", 1,
    "                   sck:send(http_resp)", 1,
    "                   sck:send(html_main_1)", 1,
    "                   sck:send(html_main_2)", 1,
    "                   sck:send(html_main_3)", 1,
    "                   sck:send(html_main_4)", 1,
    "                   sck:send(html_main_5)", 1,
    "                   sck:send(html_main_6)", 1,
    "                   sck:send(html_main_7)", 1,
    "                   sck:send(html_main_8)", 1,
    "end", 1,

    "function send_html_input(sck)", 1,
    "                   sck:send(html_main_2_input_1)", 1,
    "                   sck:send(html_main_2_input_2)", 1,
    "                   sck:send(html_main_2_input_3)", 1,
    "                   sck:send(html_main_2_input_4)", 1,
    "                   sck:send(html_main_2_input_5)", 1,
    "                   sck:send(html_main_2_input_6)", 1,
    "                   sck:send(html_main_2_input_7)", 1,
    "                   sck:send(html_main_2_input_8)", 1,
    "                   sck:send(html_main_2_input_9)", 1,
    "                   sck:send(html_main_2_input_10)", 1,
    "                   sck:send(html_main_2_input_11)", 1,
    "                   sck:send(html_main_2_input_12)", 1,
    "                   sck:send(html_main_2_input_13)", 1,
    "end", 1,

    "srv = net.createServer(net.TCP)", 1,
    "srv:listen(80, function(conn)", 1,
    "   conn:on(\"receive\", function(sck, payload)", 1,
    "       if game_started ~= true then" , 1,
    "           if now_loading~=true then", 1,
    "              local snake_name_start=0", 1,
    "              local snake_name_end=0", 1,
    "              local level_start=0", 1,
    "              local level_end=0", 1,
    "              local between=0", 1,
    "              local post_flag_1=0", 1,
    "              local post_flag_2=0", 1,
    "              post_flag_1, post_flag_2 = string.find(payload, \"POST\")", 1,
    "              if post_flag_2 ~= nil then", 1,
    "                   snake_name_start, snake_name_end = string.find(payload, \"snake_name=\")", 1,
    "                   if snake_name_end~=nil then", 1,
    "                       between = string.find(payload, '&', snake_name_end)", 1,
    "                       if between ~= nil then", 1,
    "                           name=string.sub(payload, snake_name_end+1, between-1)", 1,
    "                           level_start, level_end = string.find(payload, \"level=\")", 1,
    "                           if level_end ~= nil then", 1,
    "                               between = string.find(payload, '&', level_end)", 1,
    "                               if between ~= nill then", 1,
    "                                   level = string.sub(payload, level_end + 1, between - 1)", 1,
    "                                   uart.write(0, '\\001'..name..'\\001'..level..'\\001')", 1,
    "                               end", 1,
    "                           end", 1,                     
    "                       end", 1,
    "                   end", 1,
    "                   sck:send(http_redirect)", 1,
    "              else", 1,  
    "                   send_html_main(sck)", 1,
    "              end", 1,
    "           else", 1,
    "               sck:send(http_resp)", 1,
    "               sck:send(html_loading)", 1,
    "           end", 1,
    "       else", 1,
    "           if now_loading~=true then", 1,
    "               local post_flag_1=0", 1,
    "               local post_flag_2=0", 1,
    "               post_flag_1, post_flag_2 = string.find(payload, \"POST /\")", 1,
    "               if post_flag_1 ~= nil then", 1,
    "                   local the_number_of_foods_start=0", 1,
    "                   local the_number_of_foods_end=0", 1,
    "                   the_number_of_foods_start, the_number_of_foods_end = string.find(payload, \"the_number_of_foods=\")", 1,
    "                   if the_number_of_foods_end~=nil then", 1,
    "                       local between=string.find(payload, '&', the_number_of_foods_end)", 1,
    "                       if between~=nil then", 1,
    "                           local foods = string.sub(payload, the_number_of_foods_end+1, between-1)", 1,
    "                           uart.write(0, '\\002'..foods..'\\002')", 1,
    "                           now_loading=true", 1,
    "                       end", 1,
    "                   else", 1,  
    "                       local up_start=0", 1,
    "                       local up_end=0", 1,
    "                       up_start, up_end = string.find(payload, \"up=up\")", 1,
    "                       if up_end ~= nil then", 1,
    "                           uart.write(0, '\\003')", 1,
    "                           now_loading=true", 1,
    "                       else", 1,
    "                           local down_start=0", 1,
    "                           local down_end=0", 1,
    "                           down_start, down_end = string.find(payload, \"down=down\")", 1,
    "                           if down_end ~= nil then", 1,
    "                               uart.write(0, '\\004')", 1,
    "                               now_loading=true", 1,
    "                           end", 1,
    "                       end", 1,
    "                   end", 1,
    "                   sck:send(http_redirect)", 1, 
    "               else", 1,
    "                   sck:send(http_resp)", 1,
    "                   if fullness > 70 then", 1,
    "                       sck:send(html_good)", 1,
    "                   else", 1,
    "                       if fullness < 30 then", 1,
    "                           sck:send(html_bad)", 1,
    "                       else", 1,
    "                           sck:send(html_so_so)", 1,
    "                       end", 1,
    "                   end", 1,
    "                   sck:send(string.format(\"<h2>name: %s, length: %d, fullness: %d, cel: %.2f</h2>\", name, length, fullness, cel))", 1,
    "                   send_html_input(sck)", 1,
    "               end", 1,
    "           else", 1,
    "               sck:send(http_resp)", 1,
    "               sck:send(html_loading)", 1,
    "           end", 1,
    "       end", 1,
    "   end)", 1,
    "   conn:on(\"sent\", function(sck) conn:close() end) ", 1,
    "end)", 1,
    NULL, 0,
};

bool game_started=false;

QueueType queue;

int main(void){ // osPriorityNormal
    PC.baud(115200);
    ESP.baud(115200);

    speaker.period(1.0 / 1319);

    init_queue(&queue);


    ESP_reset();
    ESP_noecho();
    ESP_call_multi(scripts);

    joystick_up.rise(joystick_up_ISR_function);
    joystick_down.rise(joystick_down_ISR_function);
    joystick_center.rise(joystick_center_ISR_function);
    joystick_left.rise(joystick_left_ISR_function);
    joystick_right.rise(joystick_right_ISR_function);


    



    while(true){
        ESP.printf("now_loading=false\r\n");thread_sleep_for(100);
        get_game_information();
        ESP.printf("now_loading=true\r\n");thread_sleep_for(100);

        set_difficulty();
        set_status();

        Thread check_snake_length_thread(osPriorityLow);
        Thread check_tem_thread(osPriorityLow);
        Thread eat_foods_thread(osPriorityLow);
        Thread decrease_fullness_thread(osPriorityLow);
        Thread update_esp_thread(osPriorityLow);
        Thread increase_the_number_of_foods_thread(osPriorityLow);
        Thread decrease_the_number_of_foods_thread(osPriorityLow);
        Thread print_thread(osPriorityLow);
        Thread pwm_thread(osPriorityLow);
        Thread uart_rx_thread(osPriorityLow);

        check_snake_length_thread.start(check_snake_length_thread_function);
        check_tem_thread.start(check_tem_thread_function);
        eat_foods_thread.start(eat_foods_thread_function);
        decrease_fullness_thread.start(decrease_fullness_thread_function);
        increase_the_number_of_foods_thread.start(increase_the_number_of_foods_thread_function);
        decrease_the_number_of_foods_thread.start(decrease_the_number_of_foods_thread_function);
        update_esp_thread.start(update_esp_thread_function);
        decrease_fullness_thread.start(decrease_fullness_thread_function);
        print_thread.start(print_thread_function);
        pwm_thread.start(pwm_thread_function);
        uart_rx_thread.start(uart_rx_thread_function);

        lcd.cls();

        ESP.attach(uart_ISR_function);
        ESP.printf("game_started=true\r\n");thread_sleep_for(100);
        ESP.printf("now_loading=false\r\n");thread_sleep_for(100);

        sem_game_over.acquire();

        check_snake_length_thread.terminate();
        check_tem_thread.terminate();
        eat_foods_thread.terminate();
        decrease_fullness_thread.terminate();
        update_esp_thread.terminate();
        increase_the_number_of_foods_thread.terminate();
        decrease_the_number_of_foods_thread.terminate();
        decrease_fullness_thread.terminate();
        print_thread.terminate();
        pwm_thread.terminate();
        uart_rx_thread.terminate();

        ESP.printf("\r\n");
        ESP.printf("now_loading=true\r\n");
        ESP.printf("game_started=false\r\n"); // set ESP program to loading state

        lcd.cls();
        lcd.locate(0,6);
        lcd.printf("GAME OVER");




        re_initialize_resource();
    }
    return 0;
}

void get_game_information(void){
    bool got=false;
    int state=0;
    char temp_name[NAME_LENGTH];
    int temp_name_index=0;
    char temp_difficulty=0;
    while(got!=true){
        char read = ESP.getc();
        PC.putc(read);
        if(state==0){
            if(read=='\001'){
                state=1;
            }
        }
        else if(state==1){ // \001~name~\001~difficulty~\001
            if(read=='\001'){
                temp_name[temp_name_index]='\0';
                state=2;
            }
            else{
                temp_name[temp_name_index]=read;
                temp_name_index++;
            }
        }
        else if(state==2){
            if(read=='\001'){
                strcpy(snake_status.name, temp_name);
                difficulty = temp_difficulty;
                got=true;
                state=0;
            }
            else{
                temp_difficulty=read;
            }
        }
    }
}

void set_difficulty(void){

    if(difficulty==EASY){
        difficulty_information.max_length=50;
        difficulty_information.time_interval_to_decrease_fullness=3000;
        difficulty_information.max_tem=40;
        difficulty_information.min_tem=10;
    }
    else if(difficulty==NORMAL){
        difficulty_information.max_length=35;
        difficulty_information.time_interval_to_decrease_fullness=2000;
        difficulty_information.max_tem=35;
        difficulty_information.min_tem=15;
    }
    else if(difficulty==HARD){
        difficulty_information.max_length=20;
        difficulty_information.time_interval_to_decrease_fullness=1000;
        difficulty_information.max_tem=30;
        difficulty_information.min_tem=20;
    }
    else{
        printf("difficulty set error\n");
    }
}

void set_status(void){
    snake_status.fullness=50;
    snake_status.length=5;
    snake_status.tem_offset=0;
}

void game_over(void){
    sem_game_over.release();
}

void re_initialize_resource(void){

    sem_snake_status.release();
    sem_difficulty_information.release();
    sem_change_fullness.try_acquire();
    sem_the_number_of_foods.release();


    sem_increase_the_number_of_foods.try_acquire();
    sem_decrease_the_number_of_foods.try_acquire();
    sem_print_thread.try_acquire();
    sem_uart_rx_thread.release();
    sem_update_esp_thread.try_acquire();
    sem_pwm_thread.try_acquire();

    select_screen=0;
    the_number_of_foods=0;
    game_started=false;


    ESP.attach(NULL); // prevent from updating queue
    init_queue(&queue);
    
    the_number_of_foods=0;
    speaker=0;
}


void init_queue(QueueType* q) {
	q->front = q->rear = 0;
}


int is_empty_queue(QueueType* q) {
	return (q->front == q->rear);
}


int is_full(QueueType* q) {
	return (q->front == ((q->rear+1)%MAX_QUEUE_SIZE));
}

int enqueue(QueueType* q, char data) {
	if (is_full(q)) {
		return -1;
	}
	else {
		q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
		q->data[q->rear] = data;
        return 0;
	}
}

char dequeue(QueueType* q) {
	if (is_empty_queue(q)) {
		return -1;
	}
	else {
		q->front = (q->front + 1) % MAX_QUEUE_SIZE;
		char data = q->data[q->front];
		return data;
	}
}
