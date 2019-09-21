/*
 * rtc_time.c
 *
 *  Created on: 21 сент. 2019 г.
 *      Author: zan
 */
#include <stm32f1xx.h>

#include "rtc_time.h"

#include "string.h" // for strncpy
#include "stdlib.h" // for atoi


void  RTC_Init()//Инициализация RTC
{
  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)      //Проверка работы часов, если не включены, то инициализировать
  {
   RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  	//Включить тактирование PWR и Backup
   PWR->CR |= PWR_CR_DBP;                                   //Разрешить доступ к Backup области
   RCC->BDCR |= RCC_BDCR_BDRST;                             //Сбросить Backup область
   RCC->BDCR &= ~RCC_BDCR_BDRST;
   RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;       //Выбрать LSE источник (кварц 32768) и подать тактирование
   RCC->BDCR |= RCC_BDCR_LSEON;                             //Включить LSE
   while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){} //Дождаться включения
   BKP->RTCCR |= 0;                                         //калибровка RTC
   while (!(RTC->CRL & RTC_CRL_RTOFF));                     //проверить закончены ли изменения регистров RTC
   RTC->CRL  |=  RTC_CRL_CNF;                               //Разрешить Запись в регистры RTC
   RTC->PRLL  = 0x7FFF;                                     //Настроит делитель на 32768 (32767+1)
   RTC->CRL  &=  ~RTC_CRL_CNF;                              //Запретить запись в регистры RTC
   while (!(RTC->CRL & RTC_CRL_RTOFF));                     //Дождаться окончания записи
   RTC->CRL &= (uint16_t)~RTC_CRL_RSF;                      //Синхронизировать RTC
   while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF){}         //Дождаться синхронизации
   PWR->CR &= ~PWR_CR_DBP;                                  //запретить доступ к Backup области
  }
}


#define SEC_A_DAY 86400

void RTC_CounterToTime (unsigned long timer, rtc_time_t * unix_time)
{
	unsigned long days;
	days = timer%SEC_A_DAY;
	unix_time->hour=days/3600;
	unix_time->minute=(days%3600)/60;
	unix_time->second=(days%3600)%60;
}


void RTC_CounterToDate (unsigned long timer, rtc_time_t * unix_time)
{
	unsigned long a;
	char b;
	char c;
	char d;
	a = ((timer+43200)/(86400>>1)) + (2440587<<1) + 1;
	a>>=1;
	unix_time->wday = a%7;
	a+=32044;
	b=(4*a+3)/146097;
	a=a-(146097*b)/4;
	c=(4*a+3)/1461;
	a=a-(1461*c)/4;
	d=(5*a+2)/153;
	unix_time->day=a-(153*d+2)/5+1;
	unix_time->month=d+3-12*(d/10);
	unix_time->year=100*b+c-4800+(d/10);
}



unsigned long RTC_TimeToCounter (rtc_time_t * unix_time)
{
	char a;
	int y;
	char m;
	unsigned long Uday;
	unsigned long time;

	a=((14-unix_time->month)/12);
	y=unix_time->year+4800-a;
	m=unix_time->month+(12*a)-3;
	Uday=(unix_time->day+((153*m+2)/5)+365*y+(y/4)-(y/100)+(y/400)-32045)-2440588;
	time=Uday*86400;
	time+=unix_time->second+unix_time->minute*60+unix_time->hour*3600;
	return time;
}

uint32_t RTC_GetCounter(void)//Получить значение счетчика
{
     return  (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
}

void RTC_SetCounter_(uint32_t count)//Записать новое значение счетчика
{
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  	//включить тактирование PWR и Backup
  PWR->CR |= PWR_CR_DBP;									//разрешить доступ к Backup области
  while (!(RTC->CRL & RTC_CRL_RTOFF));                     //проверить закончены ли изменения регистров RTC
  RTC->CRL |= RTC_CRL_CNF;                                 //Разрешить Запись в регистры RTC
  RTC->CNTH = count>>16;                                   //записать новое значение счетного регистра
  RTC->CNTL = count;
  RTC->CRL &= ~RTC_CRL_CNF;                                //Запретить запись в регистры RTC
  while (!(RTC->CRL & RTC_CRL_RTOFF));                     //Дождаться окончания записи
  PWR->CR &= ~PWR_CR_DBP;                                  //запретить доступ к Backup области
}

void RTC_SetTime(rtc_time_t * time)
{
	RTC_SetCounter_(RTC_TimeToCounter(time));

}

void RTC_GetTime(rtc_time_t * time)
{
	RTC_CounterToTime(RTC_GetCounter(),time);
}

void RTC_GetDate(rtc_time_t * time)
{
	RTC_CounterToDate(RTC_GetCounter(),time);
}

void rtc_substr(char * str, char * sub, int from, int to) {

   int count = to-from;
   strncpy(sub, str+from, count);
   sub[count] = '\0';
}

// проверка на правильность формата времени
void rtc_check_time_format(char * time_str)
{

}

/**  date save from string 2019-02-28,03:14
 *  year-month-day,hour:min
 * */
uint8_t RTC_SetTimeFromString(char * time_str)
{
	 //TODO добавить проверку на правильность формата времени
	 //rtc_check_time_format(time_str);

	 char c[4];
	 char * sub = (char*)c;

	 rtc_time_t t= {0};

	 rtc_substr(time_str,sub,0, 4);
	t.year = atoi(sub);

	rtc_substr(time_str,sub,5, 7);
	t.month = atoi(sub);

	rtc_substr(time_str,sub,8, 10);
	t.day = atoi(sub);

	rtc_substr(time_str,sub,11, 13);
	t.hour = atoi(sub);

	rtc_substr(time_str,sub,14,16);
	t.minute = atoi(sub);

	RTC_SetTime(&t);
	return 1;
}


