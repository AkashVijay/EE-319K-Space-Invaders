// SpaceInvaders.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/12/2022 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"
#include "DAC.h"
#include "PLL.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void endScreen(void);
void createEnemy(void);
void rules(void);
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
uint32_t ADCStatus;
uint32_t pos;
uint32_t score = 0;
uint8_t numenemy = 5;
uint8_t lives = 3;
uint8_t counter = 0;
uint8_t level = 1;
uint8_t language = 0;

//structs
struct PlayerShip{
    uint32_t xpos;
	  uint8_t ypos;
    uint8_t alive; 
		uint8_t dead;
    int8_t lives;
    uint32_t score;
};
typedef struct PlayerShip Player;
Player p1;

struct Laser{
    uint32_t xpos;
	  uint8_t ypos;
		uint8_t linex;
    uint8_t exist;
		uint8_t lasermove;
};
typedef struct Laser Laser;
Laser lizard;

struct Enemy{
    int32_t xpos;
	  int32_t ypos;
    uint8_t exist;
    uint32_t score;
};
typedef struct Enemy Enemy;
Enemy smallEnemy[5];

//void Timer1A_Handler(void){ // can be used to perform tasks in background
//  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
//   // execute user task
//}
uint8_t checkCollision(int8_t laserx, uint8_t lasery){
	for(int i = 0; i < numenemy; i++){
		if(smallEnemy[i].exist == 1){
			if((smallEnemy[i].xpos+12 >= laserx-2) && (smallEnemy[i].xpos <= laserx+2) && (smallEnemy[i].ypos+10 >= lasery)){
				score += smallEnemy[i].score;
				ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, clearsmallenemy, 16, 10);
				smallEnemy[i].exist = 0;
				Sound_Killed();
				return(1);
			}
		}
	}
	return(0);
}

void moveEnemy(void){
	for(int i = 0; i < numenemy; i++){
		if(smallEnemy[i].exist == 1){
			if(i%2==0){
				if(counter % (5-level) == 0){
					ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, clearsmallenemy, 16, 10);
					ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, SmallEnemy10pointB, 16,10);
					smallEnemy[i].ypos += 1;
					if(smallEnemy[i].ypos >= 149){
						for(int i = 0; i < numenemy; i++){
							ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, clearsmallenemy, 16, 10);
							smallEnemy[i].exist = 0;
						}
						lives--;
						if(lives == 0){
							endScreen();
						} else {
							createEnemy();
						}
					}
				}
			} else {
			  if(counter % (4-level) == 0){
					ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, clearsmallenemy, 16, 10);
					ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, SmallEnemy20pointA, 16,10);
					smallEnemy[i].ypos += 1;
					if(smallEnemy[i].ypos >= 149){
						for(int i = 0; i < numenemy; i++){
							ST7735_DrawBitmap(smallEnemy[i].xpos, smallEnemy[i].ypos, clearsmallenemy, 16, 10);
							smallEnemy[i].exist = 0;
						}
						lives--;
						if(lives == 0){
							endScreen();
						} else {
							createEnemy();
						}
					}
				}
			}
		}
	}
}

void createlaser(void){
	if(lizard.exist == 0){
		ST7735_DrawBitmap(lizard.xpos, lizard.ypos, laser, 5,8);
		lizard.linex = lizard.xpos;
		lizard.lasermove = 1;
		lizard.exist = 1;
		Sound_Shoot();
	}
}

void laserF(void){
	if(lizard.exist == 1){
		ST7735_DrawBitmap(lizard.linex, p1.ypos-(7*lizard.lasermove), clearlaser, 5,8);
		lizard.lasermove++;
		ST7735_DrawBitmap(lizard.linex, p1.ypos-(7*lizard.lasermove+1), laser, 5,8);
		//If laser reaches top, remove existence
		if(lizard.lasermove >= 23 || checkCollision(lizard.linex, p1.ypos-(7*lizard.lasermove+1))){
			ST7735_DrawBitmap(lizard.linex, p1.ypos-(7*lizard.lasermove+1), clearlaser, 5,8);
			lizard.exist = 0;
		}
		lizard.xpos = p1.xpos+6;
		lizard.ypos = p1.ypos-7;
	} 
	lizard.xpos = p1.xpos+6;
	lizard.ypos = p1.ypos-7;
}

void createEnemy(void){
	for(int i = 0; i < numenemy; i++){
		smallEnemy[i].exist = 1;
		smallEnemy[i].xpos = 25*i + 5;
		smallEnemy[i].ypos = 19;
		if(i%2==0){
			ST7735_DrawBitmap(smallEnemy[i].xpos, 19, SmallEnemy10pointB, 16,10);
			smallEnemy[i].score = 10;
		} else {
			ST7735_DrawBitmap(smallEnemy[i].xpos, 19, SmallEnemy20pointA, 16,10);
			smallEnemy[i].score = 20;
		}
	}
}

void nextLevel(void){
	uint8_t enemyalive = 0;
	for(int i = 0; i < numenemy; i++){
		if(smallEnemy[i].exist == 1){
			enemyalive++;
		}
	}
	if(enemyalive == 0){
		level++;
		if(level > 3){
			endScreen();
		} else {
			createEnemy();
		}
	}
}

void GPIO_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x10; 
    volatile uint32_t delay = SYSCTL_RCGCGPIO_R;
    // Fire Button and Pause Button || English/Spanish button on start screen
    GPIO_PORTE_DEN_R |= 0x03;
    GPIO_PORTE_AFSEL_R &= ~0x01;
    GPIO_PORTE_AMSEL_R &= ~0x01;
    GPIO_PORTE_DIR_R &= ~0x03;
    GPIO_PORTE_PCTL_R &= ~0x0000000F;
    GPIO_PORTE_IS_R &= ~0x01; // Begin configuration for edge-triggered interrupts
    GPIO_PORTE_IBE_R &= ~0x01;
    GPIO_PORTE_IEV_R |= 0x01;
    GPIO_PORTE_ICR_R = 0x01;
    GPIO_PORTE_IM_R |= 0x01; // Configured for rising-edge interrupts
    NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF00) | 0x00000040; // Make priority 5
    NVIC_EN0_R = 0x00000010; // enable the interrupt in NVIC
}

void GPIOPortE_Handler(void) {
	GPIO_PORTE_ICR_R = 0x01;
	createlaser();
}

void SysTick_Init(uint32_t period){
	NVIC_ST_CTRL_R=0x00000000;
	NVIC_ST_RELOAD_R = period-1;
	NVIC_ST_CURRENT_R=0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; 
	NVIC_ST_CTRL_R=7;
}

void SysTick_Handler(void){
	//GPIO_PORTF_DATA_R ^= 0x4;
	ADCStatus = 1;
}

uint32_t Convert(uint32_t x){
  return ((111*x)/4096);
}

void StartScreen(void){
	language = 0;
	ST7735_FillScreen(0x0000);	// set screen to black
	ST7735_SetCursor(6, 1);
  ST7735_OutString("Welcome to");
  ST7735_SetCursor(4,4);
  ST7735_OutString("SPACE INVADERS!");
  ST7735_SetCursor(5, 7);
  ST7735_OutString("Press PE0 for");
	ST7735_SetCursor(7, 8);
  ST7735_OutString("English");
  ST7735_SetCursor(5, 10);
	ST7735_OutString("Press PE1 for");
	ST7735_SetCursor(7, 11);
	ST7735_OutString("Spanish");
	while((GPIO_PORTE_DATA_R & 0x01) != 0x01 && (GPIO_PORTE_DATA_R & 0x02) != 0x02){
		while((GPIO_PORTE_DATA_R & 0x02) == 0x02){
			//if((GPIO_PORTE_DATA_R & 0x02) == 0x02){
				language = 1;
				break;
			//}
		}
	}
	rules();
	ST7735_FillScreen(0x0000);
	createEnemy();
	EnableInterrupts();
}

void rules(void){
	ST7735_FillScreen(0x0000);
	if(language == 0){
		ST7735_SetCursor(1, 1);
		ST7735_OutString("Press PE0 to shoot");
		ST7735_SetCursor(1, 3);
		ST7735_OutString("Press PE1 to pause");
		ST7735_SetCursor(1, 5);
		ST7735_OutString("Enemies move faster");
		ST7735_SetCursor(1, 6);
		ST7735_OutString("as you progress");
		ST7735_SetCursor(1, 8);
		ST7735_OutString("Press PE0");
		ST7735_SetCursor(1, 9);
		ST7735_OutString("to continue");
	}
	if(language == 1){
		ST7735_SetCursor(0, 0);
		ST7735_OutString("Presiona PE0");
		ST7735_SetCursor(0, 1);
		ST7735_OutString("para disparar");
		ST7735_SetCursor(0, 3);
		ST7735_OutString("Presione PE1");
		ST7735_SetCursor(0, 4);
		ST7735_OutString("para pausar");
		ST7735_SetCursor(0, 6);
		ST7735_OutString("Los enemigos se");
		ST7735_SetCursor(0, 7);
		ST7735_OutString("mueven mas rapido a");
		ST7735_SetCursor(0, 8);
		ST7735_OutString("medida que avanzas");
		ST7735_SetCursor(0, 10);
		ST7735_OutString("Presione PE0");
		ST7735_SetCursor(0, 11);
		ST7735_OutString("para continuar");
	}
	while((GPIO_PORTE_DATA_R & 0x01) != 0x01){
	}
}

void endScreen(){
	DisableInterrupts();
	ST7735_FillScreen(0x0000);	// set screen to black
	if(language == 0){
		ST7735_SetCursor(6, 1);
		ST7735_OutString("GAME OVER");
		ST7735_SetCursor(4,4);
		ST7735_OutString("Your Score was");
		ST7735_SetCursor(9, 7);
		LCD_OutDec(score);
		ST7735_SetCursor(4, 10);
		ST7735_OutString("Press PE0 to");
		ST7735_SetCursor(5, 12);
		ST7735_OutString("play again");
	}
	if(language == 1){
		ST7735_SetCursor(6, 1);
		ST7735_OutString("FIN DEL JUEGO");
		ST7735_SetCursor(1,4);
		ST7735_OutString("Tu puntuacion fue");
		ST7735_SetCursor(9, 7);
		LCD_OutDec(score);
		ST7735_SetCursor(1, 10);
		ST7735_OutString("Presione PE0 para");
		ST7735_SetCursor(1, 11);
		ST7735_OutString("volver a jugar");
	}
	while((GPIO_PORTE_DATA_R & 0x01) != 0x01){
	}
	score = 0;
	lives = 3;
	counter = 0;
	level = 1;
	StartScreen();
}

void Pause(){
	while((GPIO_PORTE_DATA_R & 0x02) == 0x02){
		DisableInterrupts();
		while((GPIO_PORTE_DATA_R & 0x02) != 0x02){}
	}
	EnableInterrupts();
}

int main(void){
  DisableInterrupts();
	//Initializations
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
	//PLL_Init(Bus80MHz);
  Random_Init(1);
	SysTick_Init(80000000/30);
  ADC_Init();
	GPIO_Init();
	DAC_Init();
	Sound_Init();
  Output_Init();
  StartScreen();
	EnableInterrupts();
  pos = ADC_In();
	p1.xpos = pos;
	p1.ypos = 159;
	lizard.xpos = p1.xpos+6;
	lizard.ypos = p1.ypos-7;
	lizard.exist = 0;
  while(1){
		while(ADCStatus ==0){}	
			//GPIO_PORTF_DATA_R ^= 0x0;
			Pause();
			pos = ADC_In();
			pos = Convert(pos);	
			ST7735_DrawBitmap(p1.xpos, p1.ypos, clearship, 18,8);
			p1.xpos = pos;
			//p1.ypos = 129;
			//ST7735_DrawBitmap(p1.xpos, p1.ypos, coolspaceship, 40,32); // player ship bottom
			ST7735_DrawBitmap(p1.xpos, p1.ypos, PlayerShip, 18,8); // player ship bottom
			moveEnemy();
			laserF();
			ADCStatus = 0;
			ST7735_SetCursor(0, 0);
			if (language == 0){
				ST7735_OutString("Score: ");
				LCD_OutDec(score);
				ST7735_SetCursor(11, 0);
				ST7735_OutString("Lives: ");
			}
			if (language == 1){
				ST7735_OutString("Puntuacion:");
				LCD_OutDec(score);
				ST7735_SetCursor(14, 0);
				ST7735_OutString("Vida:");
			}
			LCD_OutDec(lives);
			nextLevel();
			counter++;
  }
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}
typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

int main1(void){ char l;
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Delay100ms(30);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Delay100ms(20);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }  
}
