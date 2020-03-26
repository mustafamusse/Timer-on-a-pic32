
#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"
#include <string.h>
#define TMR2PERIOD ((80000000 )/ 256) / 10 //prescalar till 256, vi ska ha en timeout var tiondel sekund.
//vi väljer prescalar till 256 eftersom det finns 8-bits register i timer 2 och 2^8 = 256 register
int mytime = 0x0000;
int b = 0;
char textstring[] = "text, more text, and even more text!";
int timeoutcount = 0;
int countlaps = 1;
int mytimes[1000];
int tottimes[1000];


void labinit( void )
{
  PORTE = ~0xFF; //tilldelar lamporna till outputs
  TRISD = 0xfe0; //tilldelar index 5-11 till inputs.
  TRISF = 0x2; //tilldelar index 2 till inputs.

  PR2 = TMR2PERIOD; //perioden för timer 2 tilldelas till PR2
  T2CONSET = 0x70; // setting the prescale, man sätter prescale värdet 1:256
  TMR2 = 0; // reset timer to 0
   // sätter bit 15 to "1", startar timer 2

  IPC(2) = 0x7;         // set timer 2 int priority = 4, IPC2<4:2>
  IEC(0) = 0x100;          // Gör så att bit8 blir 1
  enable_interrupt();

  mytimes[0] = 0;
  tottimes[0] = 0;
}


int time2seconds (int mytime) {
  int seconds = 0;
  seconds += ((mytime >> 12) & 0xF) *10*60; //multiplying the first number in the clock
  seconds += ((mytime >> 8) & 0xF) *60;     //multyplying the secount number
  seconds += ((mytime >> 4) & 0xF) *10;     //multiplying the thrid bit and adding it to "secounds"
  seconds += mytime & 0xF;

  return seconds;
}

int seconds2time ( int seconds ) {
  int mytime = 0x0000;
  int temp = 0x0;

  // tens of minutes
  temp = seconds/(10*60); //sparas som int!!!!!!
  seconds -= temp*10*60;  //minskar värdet
  mytime |= (temp << 12); //lägger till det som första siffran

  // single minutes
  temp = seconds/60;
  seconds -= temp*60;
  mytime |= (temp << 8); //placerar nästa siffra

  // tens of seconds
  temp = seconds/10;
  seconds -= temp*10;
  mytime |= (temp << 4); //tiotalet för minutvisaren

  // single seconds
  temp = seconds;
  mytime |= temp; //entalet för minutvisaren
}

int avg ()
{
  int i;
  int seconds = 0;
  for(i = 1; i < countlaps; i++){
    seconds += time2seconds(mytimes[i]); //adderar allt i mytimevektorn --> lapen vi är på
  }
  return seconds2time(seconds/(countlaps-1));
}

void labwork( void )
{
  int btns = getbtns(); //tar emot outputsen av buttons
  int btn1 = getbtn1();
  int sw = getsw(); //vi kollar vilken port påslagen och skickar tillbaka värdet.

  if (sw == 1)
  {
    T2CONSET = 0x8000; //Startar timern
  }

  if (sw == 0)
  {
    T2CON = 0x0; //stoppa timern
    T2CONSET = 0x70; // setting the prescale, man sätter prescale värdet 1:256
    mytime = 0x0000;  //sätter mytime till 00:00
    TMR2 = 0;         //clear time register
    display_string(1, "");
    display_string1(1, "");
    display_string2(1, "");
    display_string(2, "");
    display_string1(2, "");
    display_string2(2, "");
    display_string(3, "");
    display_string1(3, "");
    display_string2(3, "");
    int i;
    for(i = 1; i < countlaps; i++){
      mytimes[i] = 0; //reset alla värden i vektorerna
      tottimes[i] = 0;
    }
    countlaps = 1;
  }


  if (btn1 == 1)
  {
    int avrage = avg(mytimes[countlaps]);
    time2string (textstring, avrage);
    display_string2 (0, "avg:");
    display_string2 (1, textstring);
  }

  if(btns == 4){
    if(b == 0){
      PORTE=0xff;
      int linenumber = (countlaps - 1) % 3 + 1;  //linenumber är vilken rad på skärmen vi ska displaya "lapx" på
      tottimes[countlaps] = mytime; //sparar tiden i vektorn
      mytimes[countlaps] = time2seconds(tottimes[countlaps]) - time2seconds(tottimes[countlaps-1]);
      mytimes[countlaps] = seconds2time(mytimes[countlaps]);
      display_lap(linenumber, countlaps); //displayar numret
      time2string(textstring, mytimes[countlaps]); //tiden --> string
      display_string1(linenumber, textstring); //skriver ut strängen på önskad rad
      countlaps++;
      b = 1;
    }
  }
  else{
      PORTE=0x0; //låter lamporna lysa så länge knappen är intryckt.
  }
}

void user_isr( void )
{
  if(IFS(0) & 0x100)
  { //om Interruptbiten blir en 1 ökar timeoutcount med 1,
    timeoutcount++;
    // reseting flag
    IFSCLR(0) = 0x100; //tilldelar Interrupt bitten till bit 8 för timer 2 och sedan ska vi reseta den.
    //Med hjälp prescale kan vi kontrollera hur snabbt den ska flagga.
    // configuring the priority
  }

  // only execute on every tenth time-out cycle
  if (timeoutcount == 10)
  { //när timecount blir 10 då ska utföra följande instruktioner.
    time2string( textstring, mytime );
    display_string ( 0, textstring );
    display_update();
    tick( &mytime );
      b = 0;
    // display_image(96, icon);
    timeoutcount = 0; //tilldelar timeoutcount till 0
  }
  return;
}
