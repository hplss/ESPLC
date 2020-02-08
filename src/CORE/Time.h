/*
 * time.h
 *
 * Created: 2/22/2018 3:34:20 PM
 *  Author: Andrew Ward
 * This header contains all of the definitions for the Time class
 */ 

#include "GlobalDefs.h"

#ifndef TIME_H_
#define TIME_H_

#define MAX_YEAR 99 //Only working within bounds of last two digits (NOT Year 2100 compliant, as if that matters)
#define NUM_MONTHS_YEAR 12 //Number of months to a year
#define NUM_HOURS_DAY 24 //Number of hours to a day

#define TIME_SECOND 1
#define TIME_MINUTE 2
#define TIME_HOUR 3
#define TIME_DAY 4
#define TIME_MONTH 5
#define TIME_YEAR 6

class Time
{
	public: 

	Time( uint8_t yr = 0, uint8_t mo = 1, uint8_t da = 1, uint8_t hr = 1, uint8_t min = 0, uint8_t sec = 0, uint8_t tz = 0 )
	{
		if ( mo > 12 )
			mo = 12;
		if ( hr > 24 )
			hr = 24;
		if ( min > 60 )
			min = 60;
		if ( sec > 60 )
			sec = 60;
		i_year = yr; i_second = sec; i_minute = min; i_timeZone = tz; //Init to 0
		i_hour = hr; i_day = da; i_month = mo; //Default to 1, as we cannot have a day 0, or month 0, etc.
	}
	Time( const Time &T2 )
	{
		SetTime( T2 );
	}
	
	void UpdateTime(); //used to update the time in the time object
	bool SetTime( const uint8_t &yr, const uint8_t &mo, const uint8_t &day, const uint8_t &hr, const uint8_t &min, const uint8_t &sec );
	bool SetTime( const Time &T2 ); //For copying values over
	bool SetTime( const Time *T2 ); //For copying values over
	void SetNTPTime( unsigned long ); //For translating NTP to a valid date/time
	bool IncrementTime( uint32_t, uint8_t ); //Input increment amount, along with time unit, can also be used to set.
	bool IsAhead( const Time * );
	bool IsBehind( const Time *T2 ){ return !IsAhead(T2); }
	uint8_t GetMonthDays( uint8_t ); //Used to determine the proper number of days in a specific month.
	bool SetTimeZone( const uint8_t &zone ){ i_timeZone = zone; return true; }
	uint8_t GetTimeZone(){ return i_timeZone; }
	String GetTimeStr( bool decade = true ); //Returns a formatted string, representing the current saved time.
	
	//Operator stuff.
	bool operator< ( const Time &T2 ); //see time.cpp
	bool operator> ( const Time &T2 ){ return !(*this < T2 ); }
	bool operator<= ( const Time &T2 ){ return *this < T2; }
	bool operator>= ( const Time &T2 ){ return !(*this < T2 ); }
	bool operator== ( const Time &T2 ); //see time.cpp
	bool operator!= ( const Time &T2 ){ return !(*this == T2);}
	Time& operator= ( const Time &T2 );
	//
	
	private:
	uint8_t AdvanceMonthYear( unsigned int ); //Used to advance the month, and possibly the year, using a value of days as an input. Returns the remainder of days.
	
	uint8_t i_second : 6,
		i_minute : 6,
		i_hour : 5,
		i_day : 5,
		i_month : 4,
		i_year,
		i_timeZone : 5,
		i_lastUpdateSecond : 1; //only need one bit for this
};



#endif /* TIME_H_ */