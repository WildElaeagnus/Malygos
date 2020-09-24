
enum BR_STATES { BR_CHECK = 0,
                 BR_INIT = 1,
                 BR_WAIT = 2,
                 BR_TIMER = 3,
                 BR_PAUSE_3SEC = 4,
                 BR_PAUSE = 5,
                 BR_TIMEOUT = 6,
                 BR_FALSE_START = 7,
                 BR_FIRST = 8,
};
bool buttons_detect = 0;
bool DISABLE_BUTTONS, ENABLE_BUTTONS;

#define TIMER_1SEC 1

u8 BR_Step()
{
    static BR_STATES BR_state = BR_FIRST; // текущее состояние
    static u8 repeat_answer_time;
    static int8_t BR_timer;
    static u16 ms_timer;
    static u8 button_state;
    static u8 try_number;
    
    if ( resetBtn.isSingle() ) // выход в меню по нажатию кнопки СБРОС
    {
        short_tone(); //MakeSound(12, 60);
        BR_state = BR_FIRST;
        return 0;
    } //if
    
    if ( resetBtn.isDouble() ) // остановка игры по нажатию кнопки НАЗАД
    {
        short_tone(); //MakeSound(8, 30);
        BR_state = BR_INIT;
    } //if
    
    switch ( BR_state )
    {
        case BR_INIT:
            BR_timer = 60;
            ms_timer = 0;
            try_number = 1;
//            //UpdateSSD("- - - - ");
            led_strip_display(0,0); //ClearScreen(); // очистка табло
             for(u8 i = 0; i < BUTTONS_QUANTITY; i++){
              buttons_state[i] = 1;  
              leds_state[i] = 0;            
              } //SetLEDs(0x00); // тушение всех светодиодов
            buttons_detect = false;
            PetrifyQueue(); //ClearButtonsQueue(1); // очистка очереди нажатий кнопок
            //!!!ENABLE_BUTTONS; // включение кнопок
            BR_state = BR_WAIT;
            break;
            
        case BR_WAIT:
            button_state = ButtonsQueue.getHead(); //GetButtonsQueue(try_number); // получение номера кнопки стоящей в очереди первой
            if ( button_state != 0 ) // если обнаружено нажатие кнопки - фальстарт
            {
                //!!!DISABLE_BUTTONS; // отключение кнопок
                SetLEDs(button_state);
                ////UpdateSSD("  F S   ");
                //!!!//UpdateScreen(button_state); вывести на СД ленту значения кнопок в очереди
                long_tone();  //MakeWarningSound();
                ms_timer = 0;
                BR_timer = 3; // установка таймера задержки фальстарта на 3 секунды
                BR_state = BR_FALSE_START;
            } //if
            else if ( selectBtn.isSingle() ) // запуск игры по нажатию кнопки ПУСК
            {
//                //UpdateSSD(BR_timer);
                //!!!//UpdateScreenTime(BR_timer, RED);
                MakeLongSound();
                //!buttons_detect = true; хз что это
                BR_state = BR_TIMER;
            } //if
            break;
            
        case BR_TIMER:
            button_state = ButtonsQueue.getHead(); //GetButtonsQueue(try_number); // получение номера кнопки стоящей в очереди
            if ( button_state != 0 ) // если обнаружено нажатие кнопки - пауза для ответа
            {
                SetLEDs(button_state); //отобразить на прикнопочных светодиодах состояния кнопок
//                //UpdateSSD(" . . . .");
                //UpdateScreen(button_state);
                short_tone();   //MakeShortSound();
                ms_timer = 0;
                BR_state = BR_PAUSE_3SEC;
            } //if
            else if ( ++ms_timer >= TIMER_1SEC )
            {
                ms_timer = 0;
                BR_timer--;
//                //UpdateSSD(BR_timer);
                ////UpdateScreenTime(BR_timer, RED); вывод времени на 4 битсс походу
                if ( BR_timer == 10 )
                {
                    if ( try_number == 1 ) MakeWarningSound(); //
                } //if
                else if ( BR_timer == 3 ) MakeShortSound();
                else if ( BR_timer == 2 ) MakeShortSound();
                else if ( BR_timer == 1 ) MakeShortSound();
                else if ( BR_timer == 0 )
                {
                    MakeLongSound();
                    BR_state = BR_TIMEOUT;
                } //elif
            } //elif
            break;
            
        case BR_PAUSE_3SEC:
            if ( ++ms_timer >= (TIMER_1SEC * 3) ) // 3 секунды
            {
                DISABLE_BUTTONS; // отключение кнопок
                buttons_detect = false;
                u16 first_button_delay = GetButtonsDelay(try_number);
                if ( first_button_delay != 0 )
                {
                    Serial.println("t1");//ConvertDelayToText(first_button_delay, delay_text1);
                    u16 second_button_delay = GetButtonsDelay(try_number + 1);
                    if ( second_button_delay != 0 )
                    {
                    Serial.println("t2");    //ConvertDelayToText(second_button_delay, delay_text2);
                        //UpdateScreen(delay_text1, DecodeButtonColor(GetButtonsQueue(try_number)), 0, NULL, delay_text2, DecodeButtonColor(GetButtonsQueue(try_number + 1)), 0, NULL);
                    } //if
                    //else //UpdateScreen(delay_text1, DecodeButtonColor(GetButtonsQueue(try_number)), 0, NULL, "", NO_COLOR, 0, NULL);
                } //if
                BR_state = BR_PAUSE;
            } //if
            break;
            
        case BR_PAUSE: 
            if ( selectBtn.isSingle() ) // продолжение игры по нажатию кнопки ПУСК
            {
                try_number++;
                ClearButtonsQueue(try_number); // сброс состояний тех кнопок, которые еще не отвечали
                BR_timer = repeat_answer_time;
                ms_timer = 0;
                //UpdateSSD(BR_timer);
                //UpdateScreenTime(BR_timer, RED);
                MakeLongSound();
                SetLEDs(0x00); // тушение всех светодиодов
                buttons_detect = true;
                ENABLE_BUTTONS; // включение кнопок
                BR_state = BR_TIMER;
            } //if
            break;
            
        case BR_TIMEOUT:
            if ( ++ms_timer >= TIMER_1SEC )  // 1 секунда
            {
                ms_timer = 0;
                BR_timer--;
                if ( BR_timer <= -5 ) BR_state = BR_INIT;
            } //if
            break;
            
       case BR_FALSE_START:
            if ( BR_timer == 0) // если таймер задержки фальстарта истек
            {
              if ( selectBtn.isSingle() ) // продолжение игры по нажатию кнопки ПУСК
              {
                  try_number++;
                  ClearButtonsQueue(try_number); // сброс состояний тех кнопок, которые еще не отвечали
                  BR_timer = repeat_answer_time;
                  ms_timer = 0;
                  //UpdateSSD("- - - - ");
                  ClearScreen(); // очистка табло
                  MakeShortSound();
                  SetLEDs(0x00); // тушение всех светодиодов
                  ENABLE_BUTTONS; // включение кнопок
                  BR_state = BR_WAIT;
                  break;
              } //if
            } //if
            // мигание светодиода игрока сделавшего фальстарт
            ms_timer++;
            if ( ms_timer == ( TIMER_1SEC / 2 ) ) // с частотой 1 Гц
                {
                    SetLEDs(0x00);
                    //UpdateScreen(0x00);
                } //if
            else if ( ms_timer >= TIMER_1SEC ) // 1 секунда
            {
                ms_timer = 0;
                SetLEDs(button_state);
                //UpdateScreen(button_state);
                if ( BR_timer != 0 ) BR_timer--; // декремент секунд таймера задержки фальстарта, пока не истек
            } //elif
            break;
            
        case BR_CHECK: // выбор времени таймера повторного ответа ( 10 сек, 15 сек, 20 сек )
            if ( selectBtn.isSingle() ) // переключение выбора времени по нажатию кнопки ПУСК
            {
                MakeShortSound();
                repeat_answer_time -= 5;
                if ( repeat_answer_time < 10 ) repeat_answer_time = 20;
                if ( repeat_answer_time == 20 )
                {
                    //UpdateSSD("2 0 c.  ");
                    //UpdateScreen(TEXT8L, "20сек.", GREEN, 1, NULL);
                } //if
                else if ( repeat_answer_time == 15 )
                {
                    //UpdateSSD("1 5 c.  ");
                    //UpdateScreen(TEXT8L, "15сек.", YELLOW, 1, NULL);
                } //elif
                else if ( repeat_answer_time == 10 )
                {
                    //UpdateSSD("1 0 c.  ");
                    //UpdateScreen(TEXT8L, "10сек.", RED, 1, NULL);
                } //elif
            } //if
            else if ( modeBtn.isSingle() ) // продолжение игры по удержанию кнопки ПУСК
            {
                MakeShortSound();
                BR_state = BR_INIT;
            } //elif
            break;
            
        case BR_FIRST:
            repeat_answer_time = 20;
//            //UpdateSSD("2 0 c.  ");
            //UpdateScreen("Повторный ответ:", WHITE, 32, 6, "20сек.", GREEN, 1, NULL);
            BR_state = BR_CHECK;
            break;
            
        default:
            break;
    } //switch
    return 1;
} //BR_Step()

void MakeShortSound(){
  short_tone();
}

void ClearScreen(){
  led_strip_display(0,0);
}

void SetLEDs (u8 var){
  Serial.println(var);
}

void MakeLongSound() {
  long_tone(); 
}

void MakeWarningSound() {
  long_tone();
}
u8 GetButtonsDelay(u8 var) {
  ButtonsQueue.getHead();
}

void ClearButtonsQueue(u8 try_number){
  PetrifyQueue();
}
