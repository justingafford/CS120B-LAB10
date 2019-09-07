/*	Partner 1's Name & E-mail: Justin Gafford jgaff001@ucr.edu
*  Partner 2's Name & E-mail: Wang Sitao 934415487@qq.com
*	Lab Section: 022
*	Assignment: Lab #10 Exercise #5
*
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#define button1 (PINA & 0x01)
#define button2 (PINA & 0x02)

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char output = 0x00;
unsigned char cnt = 0x00;

typedef struct task{
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
}task;
const unsigned char tasksSize = 1;
const unsigned long tasksPeriodGCD = 1;

const unsigned long led = 1000;
task tasks[1];



enum sm{start,increase,decrease,reset};


    
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
    return (b ? x | (0x01 << k) : x & ~(0x01 << k));
} 

unsigned char GetBit(unsigned char x, unsigned char k){
    return ((x & (0x01 << k)) !=0);
}   

int SM(int state){
    switch(state){
        case start:
            if (!button1)
            {
                
                state = increase;
                cnt++;
                break;
            }
            if (!button2)
            {
                cnt++;
                state = decrease;
                break;
            }
            cnt = 0;
        case increase:
            if (!button2)
            {
                state = reset;
                
                break;
            }
            
            state = start;
            break;
        case decrease:
            if (!button1)
            {
                state = reset;
                
                break;
            }
            
            state = start;
            break;
        case reset:
            state = start;
            break;
        default:
            state = start;
    }
    if (cnt == 3)
    {
        tasks[0].period = 400;
    }
        
    switch(state){
        case start:
            break;
        case increase:
            if (output < 9)
            {
                output++;
                PORTB = output;
            }
            break;
        case decrease:
            if (output > 0)
            {
                output--;
                PORTB = output;
            }
            break;
        case reset:
            output = 0;
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
    tasks[i].state = start;	
    tasks[i].period = led;	
    tasks[i].elapsedTime = tasks[i].period;	
    tasks[i].TickFct = &SM;	
   
    TimerSet(tasksPeriodGCD); 
    TimerOn();   
    while(1) {
       
    }
    return 0;
}


