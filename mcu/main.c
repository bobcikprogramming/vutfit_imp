#include <fitkitlib.h>
#include <lcd/display.h>
#include <keyboard/keyboard.h>
#include<stdio.h>
#include <string.h>

char last_ch; //naposledy precteny znak
int pressCnt; //po�et stisknut� tla��tka za vte�inu
int rpmList[10] = {0,0,0,0,0,0,0,0,0,0}; //pole posledn�ch 10 hodnot po�tu zm��knut� tla��tka, na po��tku nulov� hodnoty
char result[50]; //pomocn� pole znak� pro v�pis RPM na obrazovku
int position; //pozice v poli rpmList dle sekundy

// Funkce pro v�pis n�pov�dy do konzole.
void print_user_help(void)
{
  term_send_str("Pro simulaci ot��ek je pot�eba tisknout kl�vesu 1 nebo 9");
}

// Funkce reaguj�c� na stisknut� tla��tka.
// Jedn�-li se o tla��tko '1', tak bere jeden stisk jako jednu hodnotu RPM.
// V p��pad� tla��tka '9' se jeden stisk bere jako desetin�sobn� hodnota RPM.
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


// Funkce reaguj�c� na p��kazy z konzole. Aplikace nepodporuje jin� p��kazy ne�-li help, proto funkce vrac�
// reakci na nezn�m� p��kaz.
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

// Funkce inicializuj�c� FPGA.
void fpga_initialized()
{
  LCD_init();                          // inicializuj LCD
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0); // zapni LCD, vypni kurzor
}

// Funkce pro z�sk�n� hodnoty RPM.
// Funkce se�te hodnoty z posledn�ch 10 sekund a n�sledn� je vyn�sob� �esti, aby se z�skala pr�m�rn� hodnota za minutu.
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

// Funkce vykon�vaj�c� akci p�i p�eru�en� �asova�e.
// P�i p�eru�en� dojde k aktualizov�n� listu s RPM, z�sk�n� aktu�ln� hodnoty RPM a zobrazen� na obrazovku.
// Je-li hodnota RPM nulov�, tak se na obrazovku vyp�e "Rotor stoji".
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
