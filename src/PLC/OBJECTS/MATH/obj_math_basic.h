#ifndef PLC_IO_OBJ_MATH_BASIC
#define PLC_IO_OBJ_MATH_BASIC

#include <vector>
#include <memory>
#include "PLC/PLC_IO.h"
#include "CORE/GlobalDefs.h"
#include "../obj_var.h"
#include <math.h>

//Basic math operations block. Math function are Addition, multiplication, division, clear (set to zero), square root, absolute value, cosine, sine, tangent, greater than, Less than, equal to
//Also implement constructors that allow for constants to be passed in as default arguments. Basically just have the constructor create a set of ladder variables.
class MathBlockOBJ : public Ladder_OBJ 
{
	public:
    template <typename A, typename B>
    MathBlockOBJ(const String &id, uint8_t type, A var1,  B var2 = 0, shared_ptr<Ladder_VAR> dest = 0) : MathBlockOBJ(id, type, make_shared<Ladder_VAR>(var1), make_shared<Ladder_VAR>(var2), dest ) {}
	MathBlockOBJ(const String &id, uint8_t type, shared_ptr<Ladder_VAR> A, shared_ptr<Ladder_VAR> B = 0, shared_ptr<Ladder_VAR> dest = 0) : Ladder_OBJ(id, type)
    { 
        sourceA = A; //must always have a valid pointer
        sourceB = B;

        if (!dest) //no destination object given so create one for later reference by other objects. This is also the equivalent to an output stored in memory.
        {
            if ( usesFloat() || type == TYPE_MATH_COS || type == TYPE_MATH_SIN || type == TYPE_MATH_TAN ) //floating point operation
                destination = make_shared<Ladder_VAR>( &destValues.fValue );
            else if ( usesUnsignedInt() ) //both have unsigned integers, so default to unsigned long for storage
            {
                destination = make_shared<Ladder_VAR>( &destValues.ulValue );
            }
            else //default to signed integer (long)
            {
                destination = make_shared<Ladder_VAR>( &destValues.lValue );
            }
        } 
    }
	~MathBlockOBJ() //deconstructor
    {}
	virtual void setLineState(bool &, bool);
    //Outputs the tangent of Source A to DEST
    void computeTAN()
    {
        destination->setValue(tan(sourceA->getValue<float>()));
    }
    //Outputs the sine of Source A to DEST
    void computeSIN()
    {
        destination->setValue(sin(sourceA->getValue<float>()));
    }
    //Outputs the cosine of Source A to DEST
    void computeCOS()
    {
        destination->setValue(cos(sourceA->getValue<float>()));
    }
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

	virtual void updateObject();
    virtual shared_ptr<Ladder_VAR> getObjectVAR( const String &id )
	{
		if (id == bitTagDEST) //DEST is the only available bit operator tag for this object type.
    	{
			return destination;
    	}
		else
		{
			return Ladder_OBJ::getObjectVAR(id); //default case. -- probably an error
		}
	}
    //returns true if either Ladder_Var object uses float type vairables. This is important for some math operations.
    bool usesFloat()
    {
        return (sourceA->getType() == TYPE_VAR_FLOAT || ( sourceB && sourceB->getType() == TYPE_VAR_FLOAT));
    }
    //returns true if there are exclusively unsigned numbers at play.
    bool usesUnsignedInt()
    {
        if ( sourceB )
            return (sourceA->getType() == TYPE_VAR_UINT || sourceA->getType() == TYPE_VAR_ULONG) && ( sourceB->getType() == TYPE_VAR_UINT || sourceB->getType() == TYPE_VAR_ULONG );
        
        return (sourceA->getType() == TYPE_VAR_UINT || sourceA->getType() == TYPE_VAR_ULONG);
    }
	
	private:
	shared_ptr<Ladder_VAR> sourceA,
						   sourceB,
						   destination;
    union
	{
		bool bValue;
		float fValue;
		int_fast32_t iValue; //signed int
		uint_fast32_t uiValue; //unsigned int
		int64_t lValue;
		uint64_t ulValue;
	} destValues;
};

#endif /* MATH_BASIC */