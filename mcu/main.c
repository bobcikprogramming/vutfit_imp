#include <fitkitlib.h>
#include <lcd/display.h>
#include <keyboard/keyboard.h>
#include<stdio.h>
#include <string.h>

char last_ch; //naposledy precteny znak
int pressCnt; //poèet stisknutí tlaèítka za vteøinu
int rpmList[10] = {0,0,0,0,0,0,0,0,0,0}; //pole posledních 10 hodnot poètu zmáèknutí tlaèítka, na poèátku nulové hodnoty
char result[50]; //pomocné pole znakù pro výpis RPM na obrazovku
int position; //pozice v poli rpmList dle sekundy

// Funkce pro výpis nápovìdy do konzole.
void print_user_help(void)
{
  term_send_str("Pro simulaci otáèek je potøeba tisknout klávesu 1 nebo 9");
}

// Funkce reagující na stisknutí tlaèítka.
// Jedná-li se o tlaèítko '1', tak bere jeden stisk jako jednu hodnotu RPM.
// V pøípadì tlaèítka '9' se jeden stisk bere jako desetinásobná hodnota RPM.
int keyboard_idle()
{
  char ch;
  ch = key_decode(read_word_keyboard_4x4());
  if (ch != last_ch) // stav se zmnenil
  {
    last_ch = ch;
    if (ch != 0) // vylucime pusteni klavesy
    {
      if(ch == '1'){
        pressCnt ++;
      }else if(ch == '9'){
        pressCnt += 10;
      }
    }
  }
  return 0;
}


// Funkce reagující na pøíkazy z konzole. Aplikace nepodporuje jiné pøíkazy než-li help, proto funkce vrací
// reakci na neznámý pøíkaz.
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

// Funkce inicializující FPGA.
void fpga_initialized()
{
  LCD_init();                          // inicializuj LCD
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0); // zapni LCD, vypni kurzor
}

// Funkce pro získání hodnoty RPM.
// Funkce seète hodnoty z posledních 10 sekund a následnì je vynásobí šesti, aby se získala prùmìrná hodnota za minutu.
int getRpm(){
  int i = 0;
  int rmp = 0;
  while(i < 10){
    rmp += rpmList[i];
    i++;
  }
  return (rmp * 6);
}

int main(void)
{
  last_ch = 0;
  pressCnt = 0;
  position = 0;

  initialize_hardware();
  keyboard_init();
  WDG_stop();                               // zastav watchdog

  CCTL0 = CCIE;                             // povol preruseni pro casovac (rezim vystupni komparace) 
  CCR0 = 0x8000;                            // nastav po kolika ticich (32768  = 0x8000, tj. za 1 s) ma dojit k preruseni
  TACTL = TASSEL_1 + MC_2;                  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

  while (1)
  {
    terminal_idle();                   // obsluha terminalu
    keyboard_idle();
  }         
}

// Funkce vykonávající akci pøi pøerušení èasovaèe.
// Pøi pøerušení dojde k aktualizování listu s RPM, získání aktuální hodnoty RPM a zobrazení na obrazovku.
// Je-li hodnota RPM nulová, tak se na obrazovku vypíše "Rotor stoji".
interrupt (TIMERA0_VECTOR) Timer_A (void)
{
  rpmList[position % 10] = pressCnt;
  int rpm = getRpm();
  LCD_clear();	
  
  if(rpm == 0){
    LCD_write_string("Rotor stoji");
  }else{
    memset(result,0,50);
    sprintf(result, "%d", rpm);
    LCD_write_string("RPM: ");
    LCD_append_string(result);
  }

  position ++;
  if(position == 10){
    position = 0;
  }

  pressCnt = 0;
  CCR0 += 0x8000;    // nastav po kolika ticich (32768  = 0x8000, tj. za 1 s) ma dojit k preruseni
}
