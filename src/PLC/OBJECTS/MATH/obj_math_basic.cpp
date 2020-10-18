/* The purpose of this file is to hold the function definitions related to the MathBlockOBJ Ladder Object.
 -- Created by Andrew Ward - 7/15/2020
*/
#include "obj_math_basic.h"

void MathBlockOBJ::setLineState(bool &state, bool bNot)
{ 
    if (state) //must have a HIGH state before computing.
    {
        switch ( getType() )
        {
            case TYPE_MATH_SIN:
            {
                computeSIN();
            }
            break;
            case TYPE_MATH_COS:
            {
                computeCOS();
            }
            break;
            case TYPE_MATH_TAN:
            {
                computeTAN();
            }
            break;
            case TYPE_MATH_EQ:
            {
                state = computeEQ();
                destination->setValue(state);
            }
            break;
            case TYPE_MATH_LES:
            {
                state = computeLES();
                destination->setValue(state);
            }
            break;
            case TYPE_MATH_LEQ:
            {
                state = computeLEQ();
                destination->setValue(state);
            }
            break;
            case TYPE_MATH_GRT:
            {
                state = computeGRT();
                destination->setValue(state);
            }
            break;
            case TYPE_MATH_GRQ:
            {
                state = computeGRQ();
                destination->setValue(state);
            }
            break;
            case TYPE_MATH_MOV:
            {
                computeMOV();
            }
            break;
        }
    }
    Ladder_OBJ_Logical::setLineState(state, bNot); 
}

void MathBlockOBJ::computeMUL()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() * sourceB->getValue<uint64_t>());
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() * sourceB->getValue<float>());
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() * sourceB->getValue<int64_t>());
}
void MathBlockOBJ::computeDIV()
{
    destination->setValue( sourceA->getValue<float>() / sourceB->getValue<float>()); //always compute as float for now.
}
void MathBlockOBJ::computeADD()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() + sourceB->getValue<uint64_t>());
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() + sourceB->getValue<float>());
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() + sourceB->getValue<int64_t>());
}
void MathBlockOBJ::computeSUB()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() - sourceB->getValue<uint64_t>());
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() - sourceB->getValue<float>());
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() - sourceB->getValue<int64_t>());
}
bool MathBlockOBJ::computeEQ()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() == sourceB->getValue<uint64_t>() ? true : false );
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() == sourceB->getValue<float>() ? true : false );
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() == sourceB->getValue<int64_t>() ? true : false );

    return destination->getValue<bool>();
}
bool MathBlockOBJ::computeGRT()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() > sourceB->getValue<uint64_t>() ? true : false );
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() > sourceB->getValue<float>() ? true : false );
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() > sourceB->getValue<int64_t>() ? true : false );

    return destination->getValue<bool>();
}
bool MathBlockOBJ::computeGRQ()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() >= sourceB->getValue<uint64_t>() ? true : false );
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() >= sourceB->getValue<float>() ? true : false );
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() >= sourceB->getValue<int64_t>() ? true : false );

    return destination->getValue<bool>();
}
bool MathBlockOBJ::computeLES()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() < sourceB->getValue<uint64_t>() ? true : false );
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() < sourceB->getValue<float>() ? true : false );
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() < sourceB->getValue<int64_t>() ? true : false );

    return destination->getValue<bool>();
}
bool MathBlockOBJ::computeLEQ()
{
    if ( usesUnsignedInt() )
        destination->setValue( sourceA->getValue<uint64_t>() <= sourceB->getValue<uint64_t>() ? true : false );
    else if ( usesFloat() )
        destination->setValue( sourceA->getValue<float>() <= sourceB->getValue<float>() ? true : false );
    else //assume signed int
        destination->setValue( sourceA->getValue<int64_t>() <= sourceB->getValue<int64_t>() ? true : false );

    return destination->getValue<bool>();
}

void MathBlockOBJ::computeINC()
{
    if ( usesFloat() )
        sourceA->setValue(sourceA->getValue<float>() + 1);
    else
        sourceA->setValue(sourceA->getValue<int64_t>() + 1);
}

void MathBlockOBJ::computeDEC()
{
    if ( usesFloat() )
        sourceA->setValue(sourceA->getValue<float>() - 1);
    else
        sourceA->setValue(sourceA->getValue<int64_t>() - 1);
}

void MathBlockOBJ::computeMOV()
{
    
}

