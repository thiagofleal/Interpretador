#include <string.h>
#include "header.h"

extern void free_var(Variable*);
extern void attrib(pointer, int, Result*);
extern bool token_expected(int, String);
extern bool error_found(String);
extern bool declare_prmtr(int);
extern Result expression(void);
extern Result * interp_block(void);

extern int ind_var, ind_glob, ind_arg;
extern p_Object _this, _virtual;
extern Token * token;

extern Variable _var[];
extern Result _arg[];
extern str_variable var_inf[];

bool exec = true;

int arguments(void)
{
	register int count = 0;
	
	++ token;
	
	if(token->type == tok_pontuation && * token->value == ')')
	{
		return 0;
	}
	
	-- token;
	
	do
	{
		++ token;
		_arg[count ++] = expression();
		++ token;
	}
	while(token->type == tok_pontuation && * token->value == ',');
	
	-- token;
	token_expected(tok_pontuation, ")");
	return count;
}

void parameters(int num_arg)
{
	register int num = 0;
	
	token_expected(tok_pontuation, "(");
	
	if(num_arg)
	{
		do
		{
			++ token;
			
			if(token->type == tok_pontuation && * token->value == '{')
			{
				register int i, type;
				str_matrix mtrx;
				
				++ token;
				mtrx.type = token->intern - kw_Caractere;
				mtrx.dimensions = 1;
				mtrx.length = num_arg - num;
				mtrx.size = var_inf[token->intern - kw_Caractere].size;
				mtrx.value = calloc((num_arg - num), mtrx.size);
				++ token;
				
				if(!mtrx.value)
				{
					throw(MemoryAllocException, "Memória insuficiente para alocar o vetor");
				}
				
				while(token->intern == op_colch_open)
				{
					++ mtrx.dimensions;
					token_expected(tok_operator, "]");
					++ token;
				}
				-- token;
				
				for(i = 0; i < num_arg - num; i++)
				{
					attrib(
						mtrx.value + (size_t)i * mtrx.size,
						mtrx.dimensions == 1
							? mtrx.type
							: type_matrix,
						_arg + num + (size_t)i
					);
				}
				
				_arg[num].type = type_matrix;
				_arg[num].value.rt_matrix = mtrx;
				token_expected(tok_pontuation, "}");
				++ token;
				
				num_arg = num + 1;
			}
			
			if(!declare_prmtr(num ++))
			{
				error_found("Excesso de argumentos");
				return;
			}
			
			++ token;
		}
		while(token->type == tok_pontuation && * token->value == ',');
	}
	else
	{
		++ token;
		
		if(token->type == tok_pontuation && * token->value == '{')
		{
			++ token;
			++ token;
			_arg[0].value.rt_matrix.dimensions = 1;
			while(token->intern == op_colch_open)
			{
				++ _arg[num].value.rt_matrix.dimensions;
				token_expected(tok_operator, "]");
				++ token;
			}
			_arg[0].type = type_matrix;
			_arg[0].value.rt_matrix.length = 0;
			_arg[0].value.rt_matrix.size = 0;
			_arg[0].value.rt_matrix.type = type_void;
			_arg[0].value.rt_matrix.value = NULL;
			
			-- token;
			token_expected(tok_pontuation, "}");
			++ token;
			
			if(!declare_prmtr(num = 0))
			{
				error_found("Sintaxe incorreta");
			}
			++ token;
		}
	}
	
	if(num > num_arg)
	{
		error_found("Argumentos insuficientes");
	}
	if(num < num_arg)
	{
		error_found("Excesso de argumentos");
	}
	
	if(token->type != tok_pontuation || * token->value != ')')
	{
		if(token->type == tok_reserved)
		{
			error_found("Argumentos insuficientes");
		}
		error_found("Sintaxe incorreta");
	}
}

Result * redirect_program(Token * enter, P_void func, pointer arg)
{
	register int i, bk_ind_var = ind_var;
	Token * bk_tok = NULL;
	static Result * ret;
	Variable * bk_var = new Memory((ind_var - ind_glob) * sizeof(Variable));
	
	for(i = ind_glob; i < ind_var; i++)
	{
		bk_var[i - ind_glob] = _var[i];
	}
	
	ind_var = ind_glob;
	bk_tok = token;
	token = enter;
	func(arg);
	ret = interp_block();
	exec = true;
	token = bk_tok;
	
	for(i = ind_glob; i < ind_var; i++)
	{
		free_var(_var + i);
	}
	
	ind_var = ind_glob;
	
	for(i = 0; i < bk_ind_var - ind_glob; i++)
	{
		_var[ind_var ++] = bk_var[i];
	}
	
	free(bk_var);
	return ret;
}

INLINE void alloc_parameters(int * n_prmt)
{
	parameters(*n_prmt);
}

void exec_func(Function * func)
{
	int n_arg;
	
	++ token;
	n_arg = arguments();
	func->ret_value = * redirect_program(func->enter, alloc_parameters, &n_arg);
}

struct prmt_meth{
	int n_prmt;
	pointer _this;
};

INLINE void func_meth(struct prmt_meth * arg)
{
	parameters(arg->n_prmt);
	_this = arg->_this;
}

void exec_method(Method * meth, str_Object * obj_this)
{
	static struct prmt_meth arg;
	pointer bk_this = _this;
	
	++ token;
	arg.n_prmt = arguments();
	arg._this = obj_this;
	meth->ret_value = * redirect_program(meth->enter, func_meth, &arg);
	_this = bk_this;
}
