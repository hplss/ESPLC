#ifndef PLC_IO_OBJ_MATH_BASIC
#define PLC_IO_OBJ_MATH_BASIC

#include <vector>
#include <memory>
#include "PLC/PLC_IO.h"
#include "CORE/GlobalDefs.h"
#include "PLC/OBJECTS/obj_var.h"
#include <math.h>

//Basic math operations block. Math function are Addition, multiplication, division, clear (set to zero), square root, absolute value, cosine, sine, tangent, greater than, Less than, equal to
//Also implement constructors that allow for constants to be passed in as default arguments. Basically just have the constructor create a set of ladder variables.
class MathBlockOBJ : public Ladder_OBJ_Logical 
{
	public:
    template <typename A, typename B>
    MathBlockOBJ(const String &id, OBJ_TYPE type, A var1,  B var2 = 0, shared_ptr<Ladder_VAR> dest = 0) : MathBlockOBJ(id, type, make_shared<Ladder_VAR>(var1, bitTagSRCA), make_shared<Ladder_VAR>(var2, bitTagSRCB), dest ) {}
	MathBlockOBJ(const String &id, OBJ_TYPE type, shared_ptr<Ladder_VAR> A, shared_ptr<Ladder_VAR> B = 0, shared_ptr<Ladder_VAR> dest = 0) : Ladder_OBJ_Logical(id, type)
    { 
        sourceA = A; //must always have a valid pointer
        getObjectVARs().emplace_back(sourceA); //add to the storage vector for local VAR object pointers.
        sourceB = B;

        if ( sourceB )
            getObjectVARs().emplace_back(sourceB);

        if (!dest) //no destination object given so create one for later reference by other objects. This is also the equivalent to an output stored in memory.
        {
            if ( usesFloat() || type == OBJ_TYPE::TYPE_MATH_COS || type == OBJ_TYPE::TYPE_MATH_SIN || type == OBJ_TYPE::TYPE_MATH_TAN ) //floating point operation
                dest = make_shared<Ladder_VAR>( &destValues.dValue, bitTagDEST );
            else if ( usesUnsignedInt() ) //both have unsigned integers, so default to unsigned long for storage
            {
                dest = make_shared<Ladder_VAR>( &destValues.ulValue, bitTagDEST );
            }
            else //default to signed integer (long)
            {
                dest = make_shared<Ladder_VAR>( &destValues.lValue, bitTagDEST );
            }
        } 

        destination = dest; //store it off
        getObjectVARs().emplace_back(destination); //push to storage vector for all variables
    }
	~MathBlockOBJ() //deconstructor
    {}
	virtual void setLineState(bool &, bool);
    
    //Outputs the tangent of Source A to DEST
    void computeTAN();
    //Outputs the sine of Source A to DEST
    void computeSIN();
    //Outputs the cosine of Source A to DEST
    void computeCOS();
    //Multiplication function - multiplies Source A by Source B, then outputs to DEST
    void computeMUL();
    //Division function - divides Source A by Source B, then outputs to DEST
    void computeDIV();
    //Addition function - adds Source A to source B, then outputs to DEST 
    void computeADD();
    //Subtraction function - subtracts Source B from Source A, then outputs to DEST
    void computeSUB();
    //Equals function - Checks to see if A is equals to B.
    bool computeEQ();
    //Greater than or Equal to function - Checks to see if Source A is >= Source B. 
    bool computeGRQ();
    //Greater than function - checks to see if Source A is > Source B
    bool computeGRT();
    //Less than function - checks to see if Source A is < Source B
    bool computeLES();
    //Less than or equal to function - checks to see if Source A is <= Source B
    bool computeLEQ();
    //increment function - adds 1 to Source A value
    void computeINC();
    //decrement function - decrements 1 from source A value
    void computeDEC();
    //Move function - copies the stored value from Source A into DEST
    void computeMOV();

	virtual void updateObject(){}
    virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		for ( uint8_t x = 0; x < getObjectVARs().size(); x++ )
        {
            if ( id == getObjectVARs()[x]->getID() )
                return getObjectVARs()[x]; //found the stored var, so return it
        }

		return Ladder_OBJ_Logical::getObjectVAR(id); //default case. -- probably an error
	}
    //returns true if either Ladder_Var object uses float type vairables. This is important for some math operations.
    bool usesFloat()
    {
        return (sourceA->getType() == OBJ_TYPE::TYPE_VAR_FLOAT || ( sourceB && sourceB->getType() == OBJ_TYPE::TYPE_VAR_FLOAT));
    }
    //returns true if there are exclusively unsigned numbers at play.
    bool usesUnsignedInt()
    {
        if ( sourceB )
            return (sourceA->getType() == OBJ_TYPE::TYPE_VAR_UINT || sourceA->getType() == OBJ_TYPE::TYPE_VAR_ULONG) && ( sourceB->getType() == OBJ_TYPE::TYPE_VAR_UINT || sourceB->getType() == OBJ_TYPE::TYPE_VAR_ULONG );
        
        return (sourceA->getType() == OBJ_TYPE::TYPE_VAR_UINT || sourceA->getType() == OBJ_TYPE::TYPE_VAR_ULONG);
    }
	
    /*
    template <class T>
	T getResult();
    */
	private:
	shared_ptr<Ladder_VAR> sourceA,
						   sourceB,
						   destination;
    union
	{
		bool bValue;
		double dValue;
		int_fast32_t iValue; //signed int
		uint_fast32_t uiValue; //unsigned int
		int64_t lValue;
		uint64_t ulValue;
	} destValues;
};

#endif /* MATH_BASIC */