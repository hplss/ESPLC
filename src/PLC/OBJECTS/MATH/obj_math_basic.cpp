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
            case OBJ_TYPE::TYPE_MATH_SIN:
            {
                computeSIN();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_COS:
            {
                computeCOS();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_TAN:
            {
                computeTAN();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_ASIN:
            {
                computeASIN();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_ACOS:
            {
                computeACOS();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_ATAN:
            {
                computeATAN();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_MUL:
            {
                computeMUL();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_DIV:
            {
                computeDIV();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_ADD:
            {
                computeADD();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_SUB:
            {
                computeSUB();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_EQ:
            {
                state = computeEQ();
                destination->setValue(state);
            }
            break;
            case OBJ_TYPE::TYPE_MATH_NEQ:
            {
                state = computeNEQ();
                destination->setValue(state);
            }
            break;
            case OBJ_TYPE::TYPE_MATH_LES:
            {
                state = computeLES();
                destination->setValue(state);
            }
            break;
            case OBJ_TYPE::TYPE_MATH_LEQ:
            {
                state = computeLEQ();
                destination->setValue(state);
            }
            break;
            case OBJ_TYPE::TYPE_MATH_GRT:
            {
                state = computeGRT();
                destination->setValue(state);
            }
            break;
            case OBJ_TYPE::TYPE_MATH_GRQ:
            {
                state = computeGRQ();
                destination->setValue(state);
            }
            break;
            case OBJ_TYPE::TYPE_MATH_INC:
            {
                computeINC();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_DEC:
            {
                computeDEC();
            }
            break;
            case OBJ_TYPE::TYPE_MATH_MOV:
            {
                computeMOV();
            }
            break;
            default:
            break;
        }
    }
    Ladder_OBJ_Logical::setLineState(state, bNot); 
}

void MathBlockOBJ::computeMUL()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 * val2);
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 * val2);
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 * val2);
    }
}

void MathBlockOBJ::computeDIV()
{
    double val1 = sourceA->getValue<double>();
    double val2 = sourceB->getValue<double>();
    destination->setValue( val1 / val2); //always compute as float for now.
}

void MathBlockOBJ::computeADD()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 + val2);
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 + val2);
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 + val2);
    }
}

void MathBlockOBJ::computeSUB()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 - val2 );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 - val2);
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 - val2);
    }
}

bool MathBlockOBJ::computeEQ()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 == val2 ? true : false );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 == val2 ? true : false );
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 == val2 ? true : false );
    }
    return destination->getValue<bool>();
}

bool MathBlockOBJ::computeNEQ()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 != val2 ? true : false );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 != val2 ? true : false );
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 != val2 ? true : false );
    }

    return destination->getValue<bool>();
}

bool MathBlockOBJ::computeGRT()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 > val2 ? true : false );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 > val2 ? true : false );
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 > val2 ? true : false );
    }

    return destination->getValue<bool>();
}

bool MathBlockOBJ::computeGRQ()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 >= val2 ? true : false );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 >= val2 ? true : false );
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 >= val2 ? true : false );
    }
    return destination->getValue<bool>();
}

bool MathBlockOBJ::computeLES()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 < val2 ? true : false );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 < val2 ? true : false );
    }
    else //assume signed int
    {
        int64_t val1 = sourceA->getValue<int64_t>();
        int64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 < val2 ? true : false );
    }
    return destination->getValue<bool>();
}

bool MathBlockOBJ::computeLEQ()
{
    if ( usesUnsignedInt() )
    {
        uint64_t val1 = sourceA->getValue<uint64_t>();
        uint64_t val2 = sourceB->getValue<uint64_t>();
        destination->setValue( val1 <= val2 ? true : false );
    }
    else if ( usesFloat() )
    {
        double val1 = sourceA->getValue<double>();
        double val2 = sourceB->getValue<double>();
        destination->setValue( val1 <= val2 ? true : false );
    }
    else //assume signed int
    {
        uint64_t val1 = sourceA->getValue<int64_t>();
        uint64_t val2 = sourceB->getValue<int64_t>();
        destination->setValue( val1 <= val2 ? true : false );
    }
    return destination->getValue<bool>();
}

void MathBlockOBJ::computeINC()
{
    if(usesUnsignedInt())
    {
        sourceA->setValue(sourceA->getValue<uint64_t>() + 1);
    }
    else if ( usesFloat() )
    {
        sourceA->setValue(sourceA->getValue<double>() + 1);
    }
    else
    {
        sourceA->setValue(sourceA->getValue<int64_t>() + 1);
    }
}

void MathBlockOBJ::computeDEC()
{
    if(usesUnsignedInt())
    {
        sourceA->setValue(sourceA->getValue<uint64_t>() - 1);
    }
    else if ( usesFloat() )
    {
        sourceA->setValue(sourceA->getValue<double>() - 1);
    }
    else
    {
        sourceA->setValue(sourceA->getValue<int64_t>() - 1);
    }
}

void MathBlockOBJ::computeMOV()
{
    if(usesUnsignedInt())
    {
        uint64_t value = sourceA->getValue<uint64_t>();
        destination->setValue(value);
    }
    if ( usesFloat() )
    {
        double value = sourceA->getValue<double>();
        destination->setValue(value);
    }
    else
    {
        int64_t value = sourceA->getValue<int64_t>();
        destination->setValue(value);
    }
}

void MathBlockOBJ::computeTAN()
{
    double val = sourceA->getValue<double>();
    destination->setValue(tan(val));
}

void MathBlockOBJ::computeSIN()
{
    double val = sourceA->getValue<double>();
    destination->setValue(sin(val));
}

void MathBlockOBJ::computeCOS()
{
    double val = sourceA->getValue<double>();
    destination->setValue(cos(val));
}

void MathBlockOBJ::computeATAN()
{
    double val = sourceA->getValue<double>();
    destination->setValue(atan(val));
}

void MathBlockOBJ::computeASIN()
{
    double val = sourceA->getValue<double>();
    destination->setValue(asin(val));
}

void MathBlockOBJ::computeACOS()
{
    double val = sourceA->getValue<double>();
    destination->setValue(acos(val));
}
