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
	MathBlockOBJ(const String &id, OBJ_TYPE type, VAR_PTR A, VAR_PTR B = 0, VAR_PTR dest = 0) : Ladder_OBJ_Logical(id, type)
    { 
        sourceA = A; //must always have a valid pointer
        getObjectVARs().emplace_back(sourceA); //add to the storage vector for local VAR object pointers.
        sourceB = B;

        if ( type != OBJ_TYPE::TYPE_MATH_INC && type != OBJ_TYPE::TYPE_MATH_DEC ) //These are the only types of math functions that act on their source, hence need no DEST.
        {
            if ( sourceB )
                getObjectVARs().emplace_back(sourceB);

            if (!dest ) //no destination object given so create one for later reference by other objects. This is also the equivalent to an output stored in memory.
            {
                if ( usesFloat() || type == OBJ_TYPE::TYPE_MATH_COS || type == OBJ_TYPE::TYPE_MATH_SIN || type == OBJ_TYPE::TYPE_MATH_TAN ) //floating point operation
                    dest = make_shared<Ladder_VAR>( static_cast<double>(0), bitTagDEST );
                else if ( usesUnsignedInt() ) //both have unsigned integers, so default to unsigned long for storage
                {
                    dest = make_shared<Ladder_VAR>( static_cast<uint64_t>(0), bitTagDEST );
                }
                else //default to signed integer (long)
                {
                    dest = make_shared<Ladder_VAR>( static_cast<int64_t>(0), bitTagDEST );
                }
            } 
            
            destination = dest; //store it off
            getObjectVARs().emplace_back(destination); //push to storage vector for all variables
        }
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
    //Outputs the ArcTangent of Source A to DEST
    void computeATAN();
    //Outputs the ArcSine of Source A to DEST
    void computeASIN();
    //Outputs the ArcCosine of Source A to DEST
    void computeACOS();
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
    //Not equals function - Checks to see if A is not equal to B.
    bool computeNEQ();
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
    virtual VAR_PTR getObjectVAR( const String &id )
	{
		return Ladder_OBJ_Logical::getObjectVAR(id); //default case. -- probably an error
	}

    //returns true if either Ladder_Var object uses float type vairables. This is important for some math operations.
    bool usesFloat()
    {
        return (sourceA->iType == OBJ_TYPE::TYPE_VAR_FLOAT || ( sourceB && sourceB->iType == OBJ_TYPE::TYPE_VAR_FLOAT));
    }

    //returns true if there are exclusively unsigned numbers at play.
    bool usesUnsignedInt()
    {
        OBJ_TYPE AType = sourceA->iType;

        if ( sourceB )
        {
            OBJ_TYPE BType = sourceB->iType;
            return (AType == OBJ_TYPE::TYPE_VAR_UINT || AType == OBJ_TYPE::TYPE_VAR_ULONG || AType == OBJ_TYPE::TYPE_VAR_USHORT) 
            && ( BType == OBJ_TYPE::TYPE_VAR_UINT || BType == OBJ_TYPE::TYPE_VAR_ULONG || BType == OBJ_TYPE::TYPE_VAR_USHORT);
        }
        
        return (AType == OBJ_TYPE::TYPE_VAR_UINT || AType == OBJ_TYPE::TYPE_VAR_ULONG);
    }
	
	private:
	VAR_PTR sourceA,
			sourceB,
			destination;
};

#endif /* MATH_BASIC */