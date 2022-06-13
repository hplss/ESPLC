#ifndef PLC_IO_OBJ_VAR
#define PLC_IO_OBJ_VAR

#include "../PLC_IO.h"

//Ladder_VARs can serve as both local variables to specific ladder objects (such as timers,counters,etc.), as well as independent values stored in memory, to be shared by multiple objects.
class Ladder_VAR : public Ladder_OBJ_Logical
{
	public:
	//These constructors are for pointers to existing variables
	//Ladder_VAR( VAR_PTR var, const String &id ) : Ladder_OBJ_Logical( id, var->getType() ){ values = var->values; b_usesPtr = var->b_usesPtr; }  //copy
	Ladder_VAR( int_fast32_t *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_INT ), b_usesPtr(true){ values.i.val_ptr = value; }
	Ladder_VAR( uint_fast32_t *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_UINT ), b_usesPtr(true){ values.ui.val_ptr = value; }
	Ladder_VAR( bool *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_BOOL ), b_usesPtr(true){ values.b.val_ptr = value; }
	Ladder_VAR( uint16_t *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_USHORT ), b_usesPtr(true){ values.us.val_ptr = value; }
	Ladder_VAR( double *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_FLOAT ), b_usesPtr(true){ values.d.val_ptr = value; }
	Ladder_VAR( uint64_t *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_ULONG ), b_usesPtr(true){ values.ul.val_ptr = value; }
	Ladder_VAR( int64_t *value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_LONG ), b_usesPtr(true){ values.l.val_ptr = value; }
	//
	//These constructors are for locally stored values
	Ladder_VAR( int_fast32_t value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_INT ), b_usesPtr(false){ values.i.val = value; }
	Ladder_VAR( uint_fast32_t value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_UINT ), b_usesPtr(false){ values.ui.val = value; }
	Ladder_VAR( bool value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_BOOL ), b_usesPtr(false){ values.b.val = value; }
	Ladder_VAR( uint16_t value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_USHORT ), b_usesPtr(false){ values.us.val = value; }
	Ladder_VAR( double value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_FLOAT ), b_usesPtr(false){ values.d.val = value; }
	Ladder_VAR( uint64_t value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_ULONG ), b_usesPtr(false){ values.ul.val = value; }
	Ladder_VAR( int64_t value, const String &id ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_LONG ), b_usesPtr(false){ values.l.val = value; }
	//
	virtual void updateObject()
	{ 
		Ladder_OBJ_Logical::updateObject(); 
	}

	//this function returns a string that represents the currently stored value in the variable object.
	String getValueStr();

	bool operator>(const VAR_PTR &);
	bool operator>=(const VAR_PTR &);
	bool operator<(const VAR_PTR &);
	bool operator<=(const VAR_PTR &);
	bool operator==(const VAR_PTR &);
	bool operator!=(const VAR_PTR &);
	Ladder_VAR operator=(const VAR_PTR &);
	template<typename T>
	Ladder_VAR operator=(const T &val)
	{
		this->setValue(val);
		return *this;
	}

	template <class T>
	T getValue()
	{
		switch(iType)
		{
			case OBJ_TYPE::TYPE_VAR_BOOL:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.b.val_ptr);
				else
					return static_cast<T>(values.b.val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.d.val_ptr);
				else
					return static_cast<T>(values.d.val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_USHORT:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.us.val_ptr);
				else
					return static_cast<T>(values.us.val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_INT:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.i.val_ptr);
				else
					return static_cast<T>(values.i.val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_UINT: 
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.ui.val_ptr);
				else
					return static_cast<T>(values.ui.val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_LONG:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.l.val_ptr);
				else
					return static_cast<T>(values.l.val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_ULONG:
			{
				if ( b_usesPtr )
					return static_cast<T>(*values.ul.val_ptr);
				else
					return static_cast<T>(values.ul.val);
			}
			break;
			default:
			break;
		}

		return static_cast<T>(0);
	}

	template <typename T>
	void setValue( const T val ) //Doesn't support String type
	{
		switch(iType)
		{
			case OBJ_TYPE::TYPE_VAR_BOOL:
			{
				if ( b_usesPtr )
					*values.b.val_ptr = static_cast<bool>(val);
				else
					values.b.val = static_cast<bool>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_FLOAT:
			{
				if ( b_usesPtr )
					*values.d.val_ptr = static_cast<double>(val);
				else
					values.d.val = static_cast<double>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_USHORT:
			{
				if ( b_usesPtr )
					*values.us.val_ptr = static_cast<uint16_t>(val);
				else
					values.us.val = static_cast<uint16_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_INT:
			{
				if ( b_usesPtr )
					*values.i.val_ptr = static_cast<int_fast32_t>(val);
				else
					values.i.val = static_cast<int_fast32_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_UINT:
			{
				if ( b_usesPtr )
					*values.ui.val_ptr = static_cast<uint_fast32_t>(val);
				else
					values.ui.val = static_cast<uint_fast32_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_LONG:
			{
				if ( b_usesPtr )
					*values.l.val_ptr = static_cast<int64_t>(val);
				else
					values.l.val = static_cast<int64_t>(val);
			}
			break;
			case OBJ_TYPE::TYPE_VAR_ULONG:
			{
				if ( b_usesPtr )
					*values.ul.val_ptr = static_cast<uint64_t>(val);
				else
					values.ul.val = static_cast<uint64_t>(val);
			}
			break;
			default:
			break;
		}
	}
	void setValue( const String & );

	virtual void setLineState(bool &, bool);

	private:
	template <typename T>
	union group
	{
		T *val_ptr;
		T val;
	};

	union
	{
		group<int64_t> l;
		group<uint64_t> ul;
		group<int_fast32_t> i;
		group<uint_fast32_t> ui;
		group<uint16_t> us;
		group<double> d;
		group<bool> b;
	} values;

	const bool b_usesPtr; //tells us if we're using a pointer to an object of the same type, or if we're using a locally stored value.
};

//Returns numeric type from operation between ladder variable object and numeric value
template<typename T>
const T operator-(const VAR_PTR &var, const T &val)
{
	return (var->getValue<double>() - val);
}
template<typename T>
const T operator+(const VAR_PTR &var, const T &val)
{
	return (var->getValue<double>() + val);
}
template<typename T>
const T operator*(const VAR_PTR &var, const T &val)
{
	return (var->getValue<double>() * val);
}
template<typename T>
const T operator/(const VAR_PTR &var, const T &val)
{
	return (var->getValue<double>() / val);
}
template<typename T>
bool operator==(const VAR_PTR &var, const T &val)
{
	return (var->getValue<T>() == val);
}
template<typename T>
bool operator>=(const VAR_PTR &var, const T &val)
{
	return (var->getValue<T>() >= val);
}
template<typename T>
bool operator<=(const VAR_PTR &var, const T &val)
{
	return (var->getValue<T>() <= val);
}
template<typename T>
bool operator>(const VAR_PTR &var, const T &val)
{
	return !operator<=(var,val);
}
template<typename T>
bool operator<(const VAR_PTR &var, const T &val)
{
	return !operator>=(var,val);
}
template<typename T>
bool operator!=(const VAR_PTR &var, const T &val)
{
	return !operator==(var,val);
}


//Returns numeric type from operation between numeric type and ladder variable object
template<typename T>
const T operator-(const T &val, const VAR_PTR &var)
{
	return (val - var->getValue<double>());
}
template<typename T>
const T operator+(const T &val, const VAR_PTR &var)
{
	return (val + var->getValue<double>());
}
template<typename T>
const T operator*(const T &val, const VAR_PTR &var)
{
	return (val * var->getValue<double>());
}
template<typename T>
const T operator/(const T &val, const VAR_PTR &var)
{
	return (val / var->getValue<double>());
}
template<typename T>
bool operator==(const T &val, const VAR_PTR &var)
{
	return (val == var->getValue<T>());
}
template<typename T>
bool operator>=(const T &val, const VAR_PTR &var)
{
	return (val >= var->getValue<T>());
}
template<typename T>
bool operator<=(const T &val, const VAR_PTR &var)
{
	return (val <= var->getValue<T>());
}
template<typename T>
bool operator>(const T &val, const VAR_PTR &var)
{
	return !operator<=(val,var);
}
template<typename T>
bool operator<(const T &val, const VAR_PTR &var)
{
	return !operator>=(val,var);
}
template<typename T>
bool operator!=(const T &val, const VAR_PTR &var)
{
	return !operator==(val,var);
}

//Returns numeric type from operation between two ladder variable objects
const double operator-(const VAR_PTR &var1, const VAR_PTR &var2 );
const double operator+(const VAR_PTR &var1, const VAR_PTR &var2 );
const double operator*(const VAR_PTR &var1, const VAR_PTR &var2 );
const double operator/(const VAR_PTR &var1, const VAR_PTR &var2 ); 
bool operator==(const VAR_PTR &var1, const VAR_PTR &var2 ); 
bool operator>=(const VAR_PTR &var1, const VAR_PTR &var2 ); 
bool operator<=(const VAR_PTR &var1, const VAR_PTR &var2 ); 
bool operator>(const VAR_PTR &var1, const VAR_PTR &var2 ); 
bool operator<(const VAR_PTR &var1, const VAR_PTR &var2 );
bool operator!=(const VAR_PTR &var1, const VAR_PTR &var2 ); 
#endif //PLC_IO_OBJ_VAR