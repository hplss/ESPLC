/*
 * time.cpp
 *
 * Created: 2/22/2018 3:33:40 PM
 *  Author: Andrew Ward
 * This file contains the function definitions for the Time class.
 * Taken from a previous project of mine.
 */ 

#include "UICore.h"
#include <limits.h>
const String &str_0 PROGMEM = PSTR("0");
const String &str_colon PROGMEM = PSTR(":");

bool Time::operator<( const Time &T2 )
{
	return ( i_year < T2.i_year || i_month < T2.i_month || i_day < T2.i_day || i_hour < T2.i_hour || i_minute < T2.i_minute || i_second < T2.i_second );
}

Time& Time::operator=( const Time &T2 )
{
	this->i_year = T2.i_year;
	this->i_month = T2.i_month;
	this->i_day = T2.i_day;
	this->i_hour = T2.i_hour;
	this->i_minute = T2.i_minute;
	this->i_second = T2.i_second;
	this->i_timeZone = T2.i_timeZone;
	
	return *this;
}

bool Time::operator==( const Time &T2 )
{
	return ( i_year == T2.i_year && i_month == T2.i_month && i_day == T2.i_day && i_hour == T2.i_hour && i_minute == T2.i_minute && i_second == T2.i_second );
}

void Time::UpdateTime()
{
	uint8_t curSec = (millis()/1000)%2;
	
	if ( i_lastUpdateSecond == curSec )
		return; //Same second, do not update yet.
		
	i_lastUpdateSecond = curSec; //Save off our update second, since that's the fastest we update.	

	//System clock increase below.
	IncrementTime( 1, TIME_SECOND );// increment 1 second at a time.
}

String Time::GetTimeStr( bool decade )
{
	return ( ( (i_year < 10 && decade) ? str_0 + i_year : i_year ) + str_colon 
		+ ( ( i_month < 10 && decade ) ? str_0 + i_month: i_month ) + str_colon 
		+ ( ( i_day < 10 && decade ) ? str_0 + i_day : i_day ) + str_colon
		+ ( ( i_hour < 10 && decade ) ? str_0 + i_hour : i_hour ) + str_colon 
		+ ( ( i_minute < 10 && decade ) ? str_0 + i_minute : i_minute ) + str_colon
		+ ( ( i_second < 10 && decade ) ? str_0 + i_second : i_second ) );
}

void Time::SetNTPTime( unsigned long time )
{
	//Jan 1 1970 = base time
	i_year = 0; //reset
	i_month = 1;
	i_day = 1;
	//
	
	i_second = time%60; //Set seconds now
	i_minute = (time/60)%60; //Set the minutes now
	i_hour = (time/3600)%NUM_HOURS_DAY; //Just total these for now, we can use the existing container for temp storage
	i_day = AdvanceMonthYear( time/(3600*NUM_HOURS_DAY) );
	i_year -= 30; //meh
}

bool Time::IncrementTime( unsigned int inc, uint8_t unit )
{
	unsigned int tempMinutes = 0;
	unsigned int tempHours = 0;
	
	switch ( unit )
	{
		case TIME_SECOND: //Yeah, we've gotta go all the way up the chain. woohoo, on the plus side, we can't possibly roll over multiple days with seconds here.
			if ( long(inc + i_second) >= UINT_MAX )
				inc = UINT_MAX - i_second;
			
			tempMinutes = (((inc + i_second)/60) + i_minute); //Total minutes + existing minutes
			i_second = (inc + i_second)%60; //Set seconds now
		
			if ( tempMinutes > i_minute ) //Should we proceed beyond this? (saving CPU)
			{
				i_minute = tempMinutes%60; //Set the minutes now
				i_hour = ((tempMinutes/60) + i_hour); //Just total these for now, we can use the existing container for temp storage
				i_day = AdvanceMonthYear( (i_hour/NUM_HOURS_DAY + i_day) );
				i_hour = i_hour%NUM_HOURS_DAY; //Finally, Get the remainder from our stored hours, now that we've calculated the days and such
			}
			break; //END TIME_SECOND
			
		case TIME_MINUTE:
			if ( long(inc + i_minute) >= UINT_MAX )
				inc = UINT_MAX - i_minute; //prevent overflow

			tempHours = ((inc + i_minute)/60 + i_hour); //Save using old minutes
			i_minute = (inc + i_minute)%60; //Set new minutes
			if ( tempHours > i_hour )
			{
				i_day = AdvanceMonthYear( i_day + ((tempHours/NUM_HOURS_DAY) )%GetMonthDays( i_month ) );
				i_hour = tempHours%NUM_HOURS_DAY;
			}
			break; //END TIME_MINUTE
			
		case TIME_HOUR:
			if ( long(inc + i_hour) >= UINT_MAX )
				inc = UINT_MAX - i_hour; //prevent overflow
		
			i_day = AdvanceMonthYear((inc + i_hour/NUM_HOURS_DAY) + i_day)%GetMonthDays(i_month); //Set the final day count, based on final month.
			i_hour = (inc + i_hour)%NUM_HOURS_DAY;	
			break; //END TIME_HOUR
		
		case TIME_DAY:
			if ( ((inc + i_day)/365) + i_year >= MAX_YEAR ) //Should never reach UINT_MAX
				inc = MAX_YEAR*365; //Max number of days
			else 
				inc += i_day; //Add our current days
			
			i_day = AdvanceMonthYear(inc)%GetMonthDays( i_month ); //using final modulus
			break; //END TIME_DAY
		
		case TIME_MONTH: 
			if ( ((inc + i_month)/NUM_MONTHS_YEAR + i_year) > MAX_YEAR )
				inc = MAX_YEAR*NUM_MONTHS_YEAR; //cap
			else
				inc += i_month; //Add existing month
				
			i_month = inc%NUM_MONTHS_YEAR; //Just apply this here
			while ( inc > NUM_MONTHS_YEAR )
			{
				inc -= NUM_MONTHS_YEAR;
				i_year++;
				if ( i_year >= MAX_YEAR )
					return false; //end, we've hit the max year
			}
			break; //END TIME_MONTH
			
		case TIME_YEAR:
			if ( (inc + i_year) > MAX_YEAR ) //Since we're using unsigned byte
				i_year = MAX_YEAR; //cap
			else
				i_year += inc; //just add this, easy peasy
			break; //END TIME_YEAR
			
		default:	
			return false; //No valid unit, just end.
	}
	return true;
}


uint8_t Time::GetMonthDays( uint8_t month )
{
	uint16_t totalYear = 1970 + i_year; //leap year stuff
	switch( month )
	{
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
		case 2:
			if ( !(totalYear%4) && totalYear%400 ) //for NTP stuff
				return 29;
				
			return 28;
		default: //all others
			return 31;
	}
}

uint8_t Time::AdvanceMonthYear( unsigned int days )
{
	while ( days > GetMonthDays(i_month) )
	{
		days -= GetMonthDays(i_month);
		i_month++; //Just keep adding the months up, we shouldn't be able to roll over here.
		if ( i_month > NUM_MONTHS_YEAR )
		{
			i_month = 1; //Roll over
			if ( i_year < MAX_YEAR )
				i_year++; //Happy new year
		}
	}
	return days; //Should always be 31 or less
}

bool Time::SetTime( const uint8_t &yr, const uint8_t &mo, const uint8_t &day, const uint8_t &hr, const uint8_t &min, const uint8_t &sec )
{
	if ( yr > MAX_YEAR || mo > NUM_MONTHS_YEAR || day > GetMonthDays(mo) || hr >= NUM_HOURS_DAY || min >= 60 || sec >= 60)
		return false; //We went out of bounds somewhere
		
	i_year = yr;
	i_month = mo;
	i_day = day;
	i_hour = hr;
	i_minute = min;
	i_second = sec;
	
	return true;
}

bool Time::SetTime( const Time &T2 )
{
	i_year = T2.i_year;
	i_month = T2.i_month;
	i_day = T2.i_day;
	i_hour = T2.i_hour;
	i_minute = T2.i_minute;
	i_second = T2.i_second;
	i_timeZone = T2.i_timeZone;
	
	return true;
}

bool Time::SetTime( const Time *T2 )
{
	i_year = T2->i_year;
	i_month = T2->i_month;
	i_day = T2->i_day;
	i_hour = T2->i_hour;
	i_minute = T2->i_minute;
	i_second = T2->i_second;
	i_timeZone = T2->i_timeZone;
	
	return true;
}

bool Time::IsAhead( const Time *T2 )
{
	if ( i_year > T2->i_year ) //year is ahead of the other's
		return true; //A year ahead.
	else if ( i_month > T2->i_month && i_year == T2->i_year )
		return true; //Month is ahead, and year is equal or greater
	else if ( i_day > T2->i_day && i_month == T2->i_month )
		return true;
	else if ( i_hour > T2->i_hour && i_day == T2->i_day )
		return true;
	else if ( i_minute > T2->i_minute && i_hour == T2->i_hour )
		return true;
	else if ( i_second > T2->i_second && i_minute == T2->i_minute )
		return true;
	
	
	return false;
}