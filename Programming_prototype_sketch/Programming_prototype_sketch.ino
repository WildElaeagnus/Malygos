//CODE FOR arduino nano

#include "GyverButton.h"
#include <TM1637Display.h>
#include "GyverTimer.h" 
#include <Adafruit_NeoPixel.h>
#include <TM1637Display.h>
#include <ArduinoQueue.h>

#define DEBUG 

#define BUTTONS_QUANTITY 15
//#define QUEUE_SIZE_ITEMS BUTTONS_QUANTITY 

//gyver button настройки для обработки кнопок меню
#define MODE_BTN_PIN 15 //пины кнопок
#define SELECT_BTN_PIN 16
#define RESET_BTN_PIN 17
#define DEBUG_BTN_PIN 14

#define DEBOUNCE 50 //антидребезг
#define HOLD_TIMEOUT 300
#define CLICK_TIMEOUT 600


//настройки адресной светодиодной ленты
#define PIN_1 5     //пины для вывода данных на ленту  
#define PIN_2 3 
#define PIN_3 4 

#define NUM_LEDS 30   // число диодо
#define STRIP_LED_NUM 15 //чило секций на ленте(должно соответствовать числу кнопок)
#define INTERRUPT_3 2 //прерывание кнопок просто настраиваем этот пин на чтение оно не на что особо не влияет потому что в функции стоит 0 пин но работает пин 2

#define CLK 52 //пины для 4битового 7сегм
#define DIO 53 //JOTORO
#define SOUND_PIN 19

//константы с игровых режимов
#define CGK_main_time 15
#define CGK_sub_time 10

#define BR_main_time 15
#define BR_sub_time_10 10
#define BR_sub_time_5 5

//настройки звуковых сигналов
#define SHORT_TONE_DUR 100
#define LONG_TONE_DUR 100
#define TONE_FREQ 500


int8_t DispMSG[] = {1, 2, 3, 4}; 
 
// кнопки для игроков
#define buttonPin_0 37
#define buttonPin_1 38
#define buttonPin_2 39
#define buttonPin_3 40
#define buttonPin_4 41
#define buttonPin_5 42
#define buttonPin_6 43
#define buttonPin_7 44
#define buttonPin_8 45
#define buttonPin_9 46
#define buttonPin_10 47
#define buttonPin_11 48
#define buttonPin_12 49
#define buttonPin_13 50
#define buttonPin_14 51

const u8 button_pins[]{
  buttonPin_0, 
  buttonPin_1,
  buttonPin_2, 
  buttonPin_3, 
  buttonPin_4, 
  buttonPin_5, 
  buttonPin_6, 
  buttonPin_7, 
  buttonPin_8, 
  buttonPin_9, 
  buttonPin_10, 
  buttonPin_11, 
  buttonPin_12, 
  buttonPin_13, 
  buttonPin_14 
};

volatile u8 buttons_state[15]; // массив который хранит информацию собранную после опроса кнопок по прерыванию

//светодиодные лампы над кнопками
#define ledPin_0 22
#define ledPin_1 23
#define ledPin_2 24
#define ledPin_3 25
#define ledPin_4 26
#define ledPin_5 27
#define ledPin_6 28
#define ledPin_7 29
#define ledPin_8 30
#define ledPin_9 31
#define ledPin_10 32
#define ledPin_11 33
#define ledPin_12 34
#define ledPin_13 35
#define ledPin_14 36

const u8 led_pins[] = {
  ledPin_0, 
  ledPin_1, 
  ledPin_2, 
  ledPin_3, 
  ledPin_4, 
  ledPin_5, 
  ledPin_6, 
  ledPin_7, 
  ledPin_8, 
  ledPin_9, 
  ledPin_10, 
  ledPin_11, 
  ledPin_12, 
  ledPin_13, 
  ledPin_14 
};
u8 leds_state[BUTTONS_QUANTITY]; //массив с состояниями светодиодов над кнопками
u8 inQueue[BUTTONS_QUANTITY]; //массив который помнит, кто есть в очереди уже

//пины семисегментного индикатора для отображения номера режима
#define seg_1bit_pin_A 6
#define seg_1bit_pin_B 7
#define seg_1bit_pin_C 8
#define seg_1bit_pin_D 9
#define seg_1bit_pin_E 10
#define seg_1bit_pin_F 11
#define seg_1bit_pin_G 12
#define seg_1bit_pin_DP 13

const char seg_1bit[] = {
  seg_1bit_pin_A, 
  seg_1bit_pin_B, 
  seg_1bit_pin_C,
  seg_1bit_pin_D,
  seg_1bit_pin_E,
  seg_1bit_pin_F,
  seg_1bit_pin_G,
  seg_1bit_pin_DP,
  };


GTimer myTimer(MS, 3000);
GTimer stripTimer(MS, 1000);

GTimer led_timer(MS);

//3 линии адресной СД ленты
Adafruit_NeoPixel strip_1 = Adafruit_NeoPixel(NUM_LEDS, PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_2 = Adafruit_NeoPixel(NUM_LEDS, PIN_2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_3 = Adafruit_NeoPixel(NUM_LEDS, PIN_3, NEO_GRB + NEO_KHZ800);


GButton modeBtn(MODE_BTN_PIN);
GButton selectBtn(SELECT_BTN_PIN);
GButton resetBtn(RESET_BTN_PIN);
GButton debugBtn(DEBUG_BTN_PIN);

ArduinoQueue<u8> ButtonsQueue(BUTTONS_QUANTITY);

// глобальные переменные
//int buttonState = 0;         //TODO написать за что отвечают переменные
u8 led;
bool state = 1;
uint8_t tru_state;
u8 seg_1bit_pin = 6;
u8 button_state = 0;
u8 led_state = 0;
u8 current_button_pin = 37;

TM1637Display display(CLK, DIO);

//Константя для прерываний
volatile bool interrupt_state = 0;

u8 menu_state;

//таймеры
uint32_t test_timer;
uint32_t final_timer;
uint32_t tone_timer;

u8 THE_FINAL_COUNTDOWN;
u8 CGK_state;
enum {
   start_CGK,
   CGK_50,
   CGK_10,
   end_CGK,
   null_CGK,
} CGK_states;

enum {
  BR_init,
  BR_null,
  BR_timer_50,
  BR_timer_55,
  BR_timer_60,
  BR_answer,
  BR_endgame,
  BR_false_start,
} BR_states;

bool CGK_init_flag, BR_init_flag, W_KILLER_init, SWOYA_GAME_init;
u16 strip_mp = 2;//множитель для отношения времени к числу зажженных сд на ленте

u8 BR_state;


void setup() {
  //инициализация массива с состояниями кнопок, в массиве должны быть все 1 если ни одна кнопка не нажата
  for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
      buttons_state[i] = 1;              
  }
{ //buttons and led setup
 for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
      pinMode(led_pins[i], OUTPUT);                 
  }
      
  // initialize the pushbutton pin as an INPUT_PULLUP:
  for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
      pinMode(button_pins[i], INPUT_PULLUP);                 
  }
  
  //инициализация пинов для однобитового 7сегментного индикатора для отображения режима игры
    for(u8 i = 0; i < 8; i++){
      pinMode(seg_1bit[i], OUTPUT);                 
  }

  //инициализация СД ленты
  pinMode( PIN_1, OUTPUT);
  pinMode( PIN_2, OUTPUT);
  pinMode( PIN_3, OUTPUT);
}
  //4 ьитовый 7сегм дисплей 
  display.setBrightness(0x0f);
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  display.setSegments(data);
  //пин аудио
  pinMode(SOUND_PIN, OUTPUT);
  Serial.begin(9600);

  { //led strip setup
  strip_1.begin();
  strip_1.setBrightness(255);    // яркость, от 0 до 255
  strip_1.clear();                          // очистить
  strip_1.show();                           // отправить на ленту

  strip_2.begin();
  strip_2.setBrightness(255);    // яркость, от 0 до 255
  strip_2.clear();                          
  strip_2.show();
  
  strip_3.begin();
  strip_3.setBrightness(255);    // яркость, от 0 до 255
  strip_3.clear();                          // очистить
  strip_3.show();
  }
  //прерывание по которыму триггерится опрос кнопок
  pinMode( INTERRUPT_3, INPUT);
  attachInterrupt(3, buttonTick, RISING); //buttonTick это функция прерывания никто не знает почему там должен быть 0 но иначе не работает 0/3
{
  //кнопки меню
//пока что не работает надо либо настроить либо убрать совсем
//  modeBtn.setDebounce(DEBOUNCE);        // настройка антидребезга (по умолчанию 80 мс)
//  modeBtn.setTimeout(SELECT_BTN_PIN);        // настройка таймаута на удержание (по умолчанию 500 мс)
//  modeBtn.setClickTimeout(CLICK_TIMEOUT);   // настройка таймаута между кликами (по умолчанию 300 мс)
//
//  selectBtn.setDebounce(DEBOUNCE);        // настройка антидребезга (по умолчанию 80 мс)
//  selectBtn.setTimeout(SELECT_BTN_PIN);        // настройка таймаута на удержание (по умолчанию 500 мс)
//  selectBtn.setClickTimeout(CLICK_TIMEOUT);   // настройка таймаута между кликами (по умолчанию 300 мс)
////
//  resetBtn.setDebounce(DEBOUNCE);        // настройка антидребезга (по умолчанию 80 мс)
//  resetBtn.setTimeout(SELECT_BTN_PIN);        // настройка таймаута на удержание (по умолчанию 500 мс)
//  resetBtn.setClickTimeout(CLICK_TIMEOUT);   // настройка таймаута между кликами (по умолчанию 300 мс)
}
    test_timer = millis();
    led_timer.setTimeout(1500);
    strip_mp = CGK_main_time/STRIP_LED_NUM;
}

void loop() {
  //часть кода которую надо выполнить не зависимо от игрового режима
  menu_buttons_tick(); // опрос кнопок меню
  if (modeBtn.isSingle()){ //перелистывание разделов меню
    menu_state++ ;
    BR_init_flag = false; //c этим надо что-то сделать так нельзя мб засунуть в стракт или в массив
    CGK_init_flag = false;
    W_KILLER_init = false;
    SWOYA_GAME_init = false;
    BR_state = BR_init; // верменное решение пока режим не доконца прописан
    disp_1bit_7seg(0); //petrify очистить однобитный семисегмент
    display.clear();// очистить 4 битный семисегмент
//    #ifdef DEBUG
//    printQueueStats();
//    #endif
  }

  if (debugBtn.isSingle()) DebugFunction();
  //записать состояния кнопок в светодиоды
//  for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
//    leds_state[i] = buttons_state[i];
//    if(leds_state[i] == !1) {
////      ButtonsQueue.enqueue(i + 1); // засунуть номер кнопки сд которой азгорелся в очередь 0 нельзя ибо это знак пустой очереди
//      digitalWrite(led_pins[i], 1);
//    }
//  }

//засунуть в очередь нажатые кнопки
//ReadQueue();

    //раз в 3 секунды обнулить состояния светодиодов и кнопок
// if(myTimer.isReady()){ 
//    for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
//      digitalWrite(led_pins[i], 0);
//    }
  
    

//    strip_1.setPixelColor(4, 0xffffff);            
//    strip_2.setPixelColor(3, 0xffffff);    
//    strip_3.setPixelColor(2, 0xffffff);  
     
// }
//отобразить состояние ленты раз в секунду
 if(stripTimer.isReady()){
  strip_1.show(); 
  strip_2.show(); 
  strip_3.show();
 }

 
//отображение на ленте нажатой кнопки
//  for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
//  
//    if (!buttons_state[i]) {
//      display_btn_strip(i, 3);
//    }
// }

 //разделы меню
  switch(menu_state) {      
  case 0:
  CGK_function(); //ЧГК -0
  break;
  case 1:
//  BRAIN_RING_function();  //Брейн ринг - 1
  BR_Step();
  break;
  case 2:
  W_KILLER_function();  //Ворошиловский снайперок - 2
  break;
  case 3:
  SVOYA_GAME_function();  //Своя игра - 3
  break;
  default:
  menu_state = 0;
  break;
  }
  
//  #ifdef DEBUG
//  if(resetBtn.isDouble()){
//    Serial.println("btn_states:");
//    for(u8 i = 0; i < 15; i++){
//      Serial.print(buttons_state[i]);
//    }
//    Serial.println(" ");
//  }
//  #endif
  
  if (interrupt_state == 1){ //выполнение действий по прерыванию

    interrupt_state = 0;
  }
}
