/*
 * PLC_VAR.h
 *
 * Created: 10/11/2019 7:06:14 PM
 *  Author: Andrew
 This header contains information regarding the VAR class. This type of object is used to store data to be communicated between different logic blocks (such as math or compute blocks). 
 */ 


#ifndef PLC_VAR_H_
#define PLC_VAR_H_

#include "../CORE/GlobalDefs.h"

class Ladder_VAR
{
	public:
	Ladder_VAR( uint16_t id, int value )
	{ 
		varID = id;
		iVarType = TYPE_VAR_INT;
		values.iValue = value;
	}
	Ladder_VAR( uint16_t id, bool value )
	{
		varID = id;
		iVarType = TYPE_VAR_BOOL;
		values.bValue = value;
	}
	Ladder_VAR( uint16_t id, float value )
	{
		varID = id;
		iVarType = TYPE_VAR_FLOAT;
		values.fValue = value;
	}
	Ladder_VAR( uint16_t id, uint64_t value )
	{
		varID = id;
		iVarType = TYPE_VAR_LONG;
		values.lValue = value;
	}
	uint8_t getVarType(){ return iVarType; }
	template <typename T>
	T getValue()
	{
		switch(iVarType)
		{
			case TYPE_VAR_FLOAT:
				return values.fValue;
			case TYPE_VAR_INT:
				return values.iValue;
			case TYPE_VAR_LONG:
				return values.lValue;
			case TYPE_VAR_BOOL:
				return values.bValue;
			default:
				return 0;
		}
	}
		
	private:
	union
	{
		bool bValue : 1;
		float fValue;
		int_fast32_t iValue;
		int64_t lValue;
	} values;
	uint8_t iVarType : 3;
	uint16_t varID;
};

#endif /* PLC_VAR_H_ */