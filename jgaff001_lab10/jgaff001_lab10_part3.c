/*	Partner 1's Name & E-mail: Justin Gafford jgaff001@ucr.edu
*  Partner 2's Name & E-mail: Wang Sitao 934415487@qq.com
*	Lab Section: 022
*	Assignment: Lab #10 Exercise #3
*
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#define button (PINA & 0x01)
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char threeLEDs = 0x00;
unsigned char blinkingLED = 0x00;
unsigned char sound = 0x00;
unsigned char output = 0x00;


typedef struct task{
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
}task;
const unsigned char tasksSize = 4;
const unsigned long tasksPeriodGCD = 2;
const unsigned long periodBlinkLED = 1000;
const unsigned long periodThreeLEDs = 300;
const unsigned long periodspeaker = 2;
const unsigned long periodCombine = 2;
task tasks[4];



enum TL_State{TL_SMStart,TL_Seq1,TL_Seq2};
enum BL_State{BL_LEDOff,BL_LEDOn};
enum speaker{off,on};
enum cb{start,combine};

    
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
    return (b ? x | (0x01 << k) : x & ~(0x01 << k));
} 

unsigned char GetBit(unsigned char x, unsigned char k){
    return ((x & (0x01 << k)) !=0);
}   

int Three(int state){
    switch(state){
        case TL_SMStart:
            state = TL_Seq1;
            break;
        case TL_Seq1:
            state = TL_Seq2;
            break;
        case TL_Seq2:
            state = TL_SMStart;
            break;
        default:
            state = TL_SMStart;
    }    
    switch(state){
        case TL_SMStart:
            threeLEDs = 0x01;
            break;
        case TL_Seq1:
            threeLEDs = 0x02;
            break;
        case TL_Seq2:
            threeLEDs = 0x04;
            break;
    }
return state;
}

int Blink(int state){
    switch(state){
        case BL_LEDOff:
            state = BL_LEDOn;
            break;
        case BL_LEDOn:
            state = BL_LEDOff;
            break;
        default:
            state = BL_LEDOff;
    }
    switch(state){
        case BL_LEDOff:
            blinkingLED = 0x00;
            break;
        case BL_LEDOn:
            blinkingLED = 0x08;
            break;
    }
return state;
}

int Speaker(int state){
    if (!button)
    {
    switch(state){
        case off:
            state = on;
            break;
        case on:
            state = off;   
            break;
        default:
            state = off;
    }
    switch(state){
        case off:
            sound = 0x00;
            break;
        case on:
            sound = 0x10;
            break;
    }
    }    
    return state;
}    
int CombineLEDs(int state){
    switch(state){
        case start:
            state = combine;
            break;
        case combine:
            //state = start;
            break;
        default:
            state = start;    
    }
    switch(state){
        case start:
            break;
        case combine:
            output = threeLEDs;
            //output = output | blinkingLED;
            output = SetBit(output,3,GetBit(blinkingLED,3));
            output = SetBit(output,4,GetBit(sound,4));
            //output = blinkingLED;
            PORTB = output;
            break;
    }
return state;
}   
void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff(){
    TCCR1B = 0X00;
}

void TimerISR(){
    unsigned char i;
    //task tasks[tasksSize];
    for (i = 0;i < tasksSize;++i) {		
        if (tasks[i].elapsedTime >= tasks[i].period) {			
            tasks[i].state = tasks[i].TickFct(tasks[i].state);			
            tasks[i].elapsedTime = 0;		
            }		
            tasks[i].elapsedTime += tasksPeriodGCD;	
            }
}

ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}



void Tick(){
    while(!TimerFlag);
    TimerFlag = 0;
}

int main() {
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;    
    unsigned char i = 0;	
    //task tasks[tasksSize];
    tasks[i].state = BL_LEDOff;	
    tasks[i].period = periodBlinkLED;	
    tasks[i].elapsedTime = tasks[i].period;	
    tasks[i].TickFct = &Blink;	
    i++;	
    tasks[i].state = TL_SMStart;	
    tasks[i].period = periodThreeLEDs;	
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Three;
    i++;
    tasks[i].state = off;
    tasks[i].period = periodspeaker;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Speaker;
    i++;
    tasks[i].state = start;
    tasks[i].period = periodCombine;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &CombineLEDs;
    
    TimerSet(tasksPeriodGCD); 
    TimerOn();   
    while(1) {
       
    }
    return 0;
}


