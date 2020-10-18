#ifndef PLC_IO_OBJ_VAR
#define PLC_IO_OBJ_VAR

#include "../PLC_IO.h"
#include "obj_var.h"

//Ladder_VARs can serve as both local variables to specific ladder objects (such as timers,counters,etc.), as well as independent values stored in memory, to be shared by multiple objects.
class Ladder_VAR : public Ladder_OBJ_Logical
{
	public:
	//These constructors are for pointers to existing variables
	Ladder_VAR( int_fast32_t *value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_INT ){ values.i.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( uint_fast32_t *value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_UINT ){ values.ui.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( bool *value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_BOOL ){ values.b.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( double *value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_FLOAT ){ values.d.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( uint64_t *value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_ULONG ){ values.ul.val_ptr = value; b_usesPtr = true; }
	Ladder_VAR( int64_t *value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_LONG ){ values.l.val_ptr = value; b_usesPtr = true; }
	//
	//These constructors are for locally stored values
	Ladder_VAR( int_fast32_t value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_INT ){ values.i.val = value; b_usesPtr = false; }
	Ladder_VAR( uint_fast32_t value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_UINT ){ values.ui.val = value; b_usesPtr = false; }
	Ladder_VAR( bool value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_BOOL ){ values.b.val = value; b_usesPtr = false; }
	Ladder_VAR( double value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_FLOAT ){ values.d.val = value; b_usesPtr = false; }
	Ladder_VAR( uint64_t value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_ULONG ){ values.ul.val = value; b_usesPtr = false; }
	Ladder_VAR( int64_t value, const String &id = "" ) : Ladder_OBJ_Logical( id, OBJ_TYPE::TYPE_VAR_LONG ){ values.l.val = value; b_usesPtr = false; }
	//
	virtual void updateObject()
	{ 
		Ladder_OBJ_Logical::updateObject(); 
	}
	bool getBoolValue(){ if (b_usesPtr && values.b.val_ptr) return *values.b.val_ptr; else return values.b.val; }
	int64_t getLongValue(){ if (b_usesPtr && values.l.val_ptr) return *values.l.val_ptr; else return values.l.val; }
	uint64_t getULongValue(){ if (b_usesPtr && values.ul.val_ptr) return *values.ul.val_ptr; else return values.ul.val; }
	float getDoubleValue(){ if (b_usesPtr && values.d.val_ptr) return *values.d.val_ptr; else return values.d.val; }
	int_fast32_t getIntValue(){ if (b_usesPtr && values.i.val_ptr) return *values.i.val_ptr; else return values.i.val; }
	uint_fast32_t getUIntValue(){ if (b_usesPtr && values.ui.val_ptr) return *values.ui.val_ptr; else return values.ui.val; }

	template <class T>
	T getValue();

	template <typename T>
	void setValue( const T );

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
		group<double> d;
		group<bool> b;
	} values;

	bool b_usesPtr; //tells us if we're using a pointer to an object of the same type, or if we're using a locally stored value.
};

#endif //PLC_IO_OBJ_VAR