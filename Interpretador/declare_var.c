#include <stdlib.h>
#include "header.h"

extern void error_found(string);
extern void attrib(pointer, int, Result*);
extern bool token_expected(int, string);
extern Result expression(void);

extern int ind_var, ind_func, ind_arg, ind_class;
extern Token *token;

extern str_variable var_inf[];
extern Variable _var[];
extern Function _func[];
extern Result _arg[];
extern Class _class[];

int count_dimensions(void)
{
	register int count = 0;
	
	++ token;
	while(token->intern == op_colch_open)
	{
		token_expected(tok_operator, "]");
		++ token;
		++ count;
	}
	-- token;
	return count;
}

void declare_matrix(int dimensions, int type, size_t size)
{
	_var[ind_var].type = type_matrix;
	_var[ind_var].value = calloc(1, sizeof(str_matrix));
	((str_matrix*)_var[ind_var].value)->type = type;
	((str_matrix*)_var[ind_var].value)->size = size;
	((str_matrix*)_var[ind_var].value)->dimensions = dimensions;
	((str_matrix*)_var[ind_var].value)->value = NULL;
}

void attrib_matrix(str_matrix * dest, str_matrix base)
{
	int i;
	
	if(dest->dimensions != base.dimensions && base.dimensions != default_matrix_dimension)
	{
		error_found("Impossível atribuir matrizes de diferentes dimensões");
	}
	
	dest->length = base.length;
	dest->value = base.value;
}

bool declare_var(void)
{
	register const int var_type = token->intern - kw_Caractere, dimensions = count_dimensions();
	
	if(!token_expected(tok_pontuation, ":"))
		return false;
	
	do
	{
		register int d = dimensions + count_dimensions();
		
		++ token;
		_var[ind_var].id = token->intern;
		
		if(d)
		{
			declare_matrix(d, var_inf[var_type].type, var_inf[var_type].size);
		}
		else
		{
			_var[ind_var].type = var_inf[var_type].type;
			_var[ind_var].value = calloc(1, var_inf[var_type].size);
		}
		
		++ ind_var;
		++ token;
		
		if(token->intern == op_attr)
		{
			++ token;
			
			if(d)
			{
				attrib_matrix(_var[ind_var - 1].value, expression().value.rt_matrix);
			}
			else
			{
				Result r = expression();
				attrib(_var[ind_var - 1].value, var_type, &r);
			}
			++ token;
		}
	}
	while(token->type == tok_pontuation && * token->value == ',');
	
	-- token;
	token_expected(tok_pontuation, ";");
	return true;
}

bool declare_prmtr(int index)
{
	register const int var_type = token->intern - kw_Caractere, d = count_dimensions();
	
	if(!token_expected(tok_pontuation, ":"))
		return false;
	
	++ token;
	_var[ind_var].id = token->intern;
	
	if(d)
	{
		declare_matrix(d, var_inf[var_type].type, var_inf[var_type].size);
		((str_matrix*)_var[ind_var].value)->length = _arg[index].value.rt_matrix.length;
		((str_matrix*)_var[ind_var].value)->value = _arg[index].value.rt_matrix.value;
	}
	else
	{
		_var[ind_var].type = var_inf[var_type].type;
		_var[ind_var].value = calloc(1, var_inf[var_type].size);
		attrib(_var[ind_var].value, var_type, _arg + index);
	}
	
	++ ind_var;
	return true;
}
