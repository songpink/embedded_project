#define MAX_QUEUE_SIZE 100
#define EASY '0'
#define NORMAL '1'
#define HARD '2'
#define MAX_HISTORY 5
#define NAME_LENGTH 9
#define FULLNESS_MAX 99

typedef struct {
    int max_length;
    int time_interval_to_decrease_fullness;
    float max_tem;
    float min_tem;
} Difficulty_information;

typedef struct {
    int length;
    int fullness;
    float tem;
    float tem_offset;
    char name[NAME_LENGTH];
} Snake_status;

typedef struct QueueType {
	char data[MAX_QUEUE_SIZE];
	int front, rear;
}QueueType;


void check_snake_length_thread_function(void);
void check_tem_thread_function(void);
void eat_foods_thread_function(void);
void decrease_fullness_thread_function(void);
void increase_the_number_of_foods_thread_function(void);
void decrease_the_number_of_foods_thread_function(void);
void print_thread_function(void);
void uart_rx_thread_function(void);
void update_esp_thread_function(void);
void print_snake_appearance_thread_function(void);
void pwm_thread_function(void);

void joystick_up_ISR_function(void);
void joystick_down_ISR_function(void);
void joystick_center_ISR_function(void);
void joystick_left_ISR_function(void);
void joystick_right_ISR_function(void);
void uart_ISR_function(void);
void uart_ISR_function_2(void);

void get_game_information(void);

void set_difficulty(void);
void set_status(void);

void re_initialize_resource(void);

void game_over(void);
void print_game_over(void);

extern C12832 lcd;
extern LM75B tem;
extern PwmOut speaker;

extern Semaphore sem_snake_status;
extern Semaphore sem_difficulty_information;
extern Semaphore sem_game_over;
extern Semaphore sem_change_fullness;
extern Semaphore sem_the_number_of_foods;
extern Semaphore sem_increase_the_number_of_foods;
extern Semaphore sem_decrease_the_number_of_foods;
extern Semaphore sem_lcd;
extern Semaphore sem_increase_the_number_of_foods;
extern Semaphore sem_decrease_the_number_of_foods;
extern Semaphore sem_get_start_information;
extern Semaphore sem_select_screen;
extern Semaphore sem_print_thread;
extern Semaphore sem_uart_rx_thread;
extern Semaphore sem_update_esp_thread;
extern Semaphore sem_pwm_thread;



extern Difficulty_information difficulty_information;
extern Snake_status snake_status;
extern int the_number_of_foods;
extern char difficulty;
extern int select_screen;

extern bool game_started;
extern QueueType queue;

extern InterruptIn joystick_up;
extern InterruptIn joystick_down;
extern InterruptIn joystick_center;
extern InterruptIn joystick_left;
extern InterruptIn joystick_right;


void init_queue(QueueType* q);
int is_empty_queue(QueueType* q);
int is_full(QueueType* q);
int enqueue(QueueType* q, char data);
char dequeue(QueueType* q);
