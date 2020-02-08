/*
 * PLC_Rung.h
 *
 * Created: 9/24/2019 2:33:31 PM
 *  Author: Andrew
 * The purpose of a rung is to serve as a manager of the objects stored within. 
 * - All rungs must have a dedicated output.
 */ 


#ifndef PLC_RUNG_H_
#define PLC_RUNG_H_

#include "PLC_IO.h"
#include <memory>

class Ladder_Rung
{
	public:	
	Ladder_Rung(){ };
	~Ladder_Rung();
	bool addRungObject( shared_ptr<Ladder_OBJ> ); //These objects should be added in sequential order at they are read by the parser
	shared_ptr<Ladder_OBJ>getRungObjectByIndex(uint16_t x){ return rungObjects[x]; }
	shared_ptr<Ladder_OBJ>getRungObjectByID(uint16_t id);
	uint16_t getNumRungObjects(){ return rungObjects.size(); }
	uint8_t getNumInitialRungObjects() { return firstRungObjects.size(); }
	bool addInitialRungObject( shared_ptr<Ladder_OBJ> );
	vector<shared_ptr<Ladder_OBJ>> &getRungObjects(){ return rungObjects; }
		
	void processRung( uint16_t );
		
	private:
	vector<shared_ptr<Ladder_OBJ>> rungObjects;
	vector<shared_ptr<Ladder_OBJ>> firstRungObjects; //used to store the objects that are first checked in the rung when it comes time to scan.
};




#endif /* PLC_RUNG_H_ */