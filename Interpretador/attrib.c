#include "header.h"

extern void error_found(string);
extern void runtime_error(int);
extern bool token_expected(int, string);
extern Result expression(void);

void attrib(pointer dest, int type, Result * r)
{
	switch(type)
	{
		case type_char:
			*(char*)dest = (char)r->value.rt_double;
			return;
		case type_bool:
			*(bool*)dest = r->value.rt_bool;
			return;
		case type_int:
			*(int*)dest = (int)r->value.rt_double;
			return;
		case type_real:
			*(double*)dest = r->value.rt_double;
			return;
		case type_string:
			if(*(string*)dest)
			{
				free(*(string*)dest);
			}
			if(r->value.rt_String)
				*(string*)dest = toString(r->value.rt_String);
			else
				*(string*)dest = toString("");
			return;
		case type_object:
		case type_file:
			*(pointer*)dest = r->value.rt_pointer;
			return;
		case type_matrix:
			
			if(!r->value.rt_pointer)
			{
				((str_matrix*)dest)->value = NULL;
				((str_matrix*)dest)->length = 0;
				return;
			}
			if(((str_matrix*)dest)->dimensions != r->value.rt_matrix.dimensions && r->value.rt_matrix.dimensions != default_matrix_dimension)
			{
				error_found("Impossível atribuir matrizes de diferentes dimensões");
				return;
			}
			
			((str_matrix*)dest)->value = r->value.rt_matrix.value;
			((str_matrix*)dest)->length = r->value.rt_matrix.length;
			return;
	}
}

void op_attrib(pointer dest, int type, Result * r, register int op)
{
	switch(op)
	{
		case op_attr:
			*r = expression();
			break;
		case op_incr:
			r->value.rt_double = r->value.rt_double + 1;
			break;
		case op_decr:
			r->value.rt_double = r->value.rt_double - 1;
			break;
		case op_incr_value:
			r->value.rt_double = r->value.rt_double + expression().value.rt_double;
			break;
		case op_decr_value:
			r->value.rt_double = r->value.rt_double - expression().value.rt_double;
			break;
		case op_incr_mul:
			r->value.rt_double = r->value.rt_double * expression().value.rt_double;
			break;
		case op_decr_div:{
			double div = expression().value.rt_double;
			
			if(div)
			{
				r->value.rt_double = r->value.rt_double / div;
			}
			else
			{
				runtime_error(division_by_zero);
			}
			break;
		}
		case op_decr_mod:
			{
				double fv = r->value.rt_double;
				double sv = expression().value.rt_double;
				
				r->value.rt_double = (double)((int)fv - ((int)fv / (int)sv) * (int)sv);
				break;
			}
		case op_change:
			r->value.rt_bool = (r->value.rt_bool ? false : true);
			break;
	}
	
	attrib(dest, type, r);
}

void attrib_result(Result * r, pointer v, int type)
{
	switch(r->type = type)
	{
		case type_char:
			r->value.rt_double = (double)(*(char*)v);
			break;
		case type_bool:
			r->value.rt_bool = *(bool*)v;
			break;
		case type_int:
			r->type = type_real;
			r->value.rt_double = (double)(*(int*)v);
			break;
		case type_real:
			r->value.rt_double = *(double*)v;
			break;
		case type_string:
			if(*(string*)v)
				r->value.rt_String = toString(*(string*)v);
			else
				r->value.rt_String = toString("");
			break;
		case type_file:
		case type_object:
			r->value.rt_pointer = *(pointer*)v;
			break;
		case type_matrix:
			r->value.rt_matrix = *(str_matrix*)v;
			break;
	}
}
