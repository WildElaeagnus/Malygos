

void SVOYA_GAME_function(){ 
if (!SWOYA_GAME_init) {  //инициализация режима
  #ifdef DEBUG
  Serial.println("SVOYA_GAME");
  #endif
  //вывод 4
  disp_1bit_7seg(4);
  SWOYA_GAME_init = true;
}
}
