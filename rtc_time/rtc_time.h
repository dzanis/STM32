/*
 * rtc_time.h
 * Библиотека RTC для stm32f1 серии. Проверено только на stm32f103
 * В HAL библиотеке RTC не востанавливает дату после сброса,в этой библиотек убран этот недостаток.
 * К тому же  можно откалибровать часы в методе RTC_Init() https://spec-project.ucoz.ru/index/kalibrovka_rtc_na_stm32f1/0-14
 * Используется unix time.
 * В RTC счётчике записывается и инкременируется 32-x битное число секунд,
 * которые тикают от 1 января 1970 года 00:00:00  до  19 января 2038 года  03:14:08
 * Пример использования:
 * rtc_time_t time = {0};// создаём структуру для времени
 * RTC_Init();// незабываем в начале инициализировать
 * RTC_GetTime(&time); // получаем время
 * Чтобы не пересчитывать дату лишний раз,отделил расчёт даты от времени
 * RTC_GetDate(&time);// получаем дату
 * Дальше в цикле можно получать время хоть каждую секунду,это не так долго как расчёт даты
 * Но  есть недостаток,когда произойдёт переход суток дата не обновится в структуре времени,это нужно делать в ручную.
 *
 * while(1)
 * {
 * RTC_GetTime(&time);
 *  print(time.hour ... и т.д
 * //например можно сделать так если нужно обновить дату при переходе суток
 * if(time.hour == 0 && time.minute == 0)RTC_GetDate(&time);
 * }
 *
 *
 * Чтоб настроить время просто заполняем структуру (день недели wday не нужно указывать,он расчитывается автоматом через RTC_GetDate )
 * (кстати день недели от 0 до 6 ,понедельник это 0,вторник 1, ... воскресенье 6.
 * time.year = 2019;time.month = 9;time.day = 21;
 * time.hour = 11;time.minute = 59;
 * RTC_SetTime(&time);
 *
 * Так же можно дату установить из строки,например мне это нужно для настройки через uart
 * Формат даты в виде строки такой  year-month-day,hour:minute
 * RTC_SetTimeFromString("2019-09-19,11:59");RTC_GetTime(&time);RTC_GetDate(&time);//и обновить нашу структуру времени
 *
 * Оригинал здесь
 * https://spec-project.ucoz.ru/index/zapusk_rtc_na_stm32_i_realizacija_kalendarja_s_rabochej_bibliotekoj/0-13
 *  Created on: 21 сент. 2019 г.
 *      Author: zan
 */

#ifndef RTC_TIME_H_
#define RTC_TIME_H_


typedef struct{
uint16_t year;
uint8_t month;
uint8_t day;
uint8_t wday;

uint8_t hour;
uint8_t minute;
uint8_t second;
} rtc_time_t;

void RTC_Init();
void RTC_SetTime(rtc_time_t * time);
void RTC_GetTime(rtc_time_t * time);
void RTC_GetDate(rtc_time_t * time);
uint8_t RTC_SetTimeFromString(char * time_str);


#endif /* RTC_TIME_H_ */
