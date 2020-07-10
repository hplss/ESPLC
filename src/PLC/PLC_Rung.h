/*
 * PLC_Rung.h
 *
 * Created: 9/24/2019 2:33:31 PM
 *  Author: Andrew Ward
 * The purpose of a rung is to serve as a manager of the objects stored within. 
 * - All rungs must have a dedicated output.
 */ 


#ifndef PLC_RUNG_H_
#define PLC_RUNG_H_

#include "PLC_IO.h"
#include <memory>

/*The Ladder_Rung object serves to represent each "rung" of a "ladder" in PLC programming. 
Each rung has its own set of logic statements (set by the end-user) that ultimately determine how the associated inputs operate on a given output(s).
*/
class Ladder_Rung
{
	public:	
	Ladder_Rung(){ };
	~Ladder_Rung();
	//Adds a new ladder logic object to the rung, provided it passes the appropriate tests.
	bool addRungObject( shared_ptr<Ladder_OBJ_Wrapper> ); 
	//Returns the ladder object associated within this rung object, based on the object's index in the storage vector.
	shared_ptr<Ladder_OBJ_Wrapper>getRungObjectByIndex(uint16_t x){ return rungObjects[x]; }
	//Returns the ladder object associated with this rung object, based on the object's unique ID number.
	shared_ptr<Ladder_OBJ_Wrapper>getRungObjectByID(uint16_t id);
	//Returns the total number of objects stored in the rung object.
	uint16_t getNumRungObjects(){ return rungObjects.size(); }
	//Returns the number of rung objects stored in the container for ladder objects that are referenced at the beginning of each ladder logic scan.
	uint8_t getNumInitialRungObjects() { return firstRungObjects.size(); }
	//Adds an inputted object to the container for the ladder objects that are referenced at the beginning of each ladder logic scan.
	bool addInitialRungObject( shared_ptr<Ladder_OBJ_Wrapper> );
	//Returns a reference to the container for the rung object's ladder objects.
	vector<shared_ptr<Ladder_OBJ_Wrapper>> &getRungObjects(){ return rungObjects; }
	//Begins the process of setting the line state to HIGH for the appropriate ladder object from the initial objects container, and iteratively determining the state for each subsequently associated object and applying changes as needed.
	void processRung( uint16_t );

		
	private:
	vector<shared_ptr<Ladder_OBJ_Wrapper>> rungObjects; //Container used to store all of the pointers to objects associated with a given rung.
	vector<shared_ptr<Ladder_OBJ_Wrapper>> firstRungObjects; //Container used to store the objects that are first checked in the rung when it comes time to scan.
};




#endif /* PLC_RUNG_H_ */