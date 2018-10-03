#include "header.h"

#define index_error_value	5

extern void error_found(string);
extern void free_var(Variable*);
extern bool token_expected(int, string);
extern bool _typeof(p_Object, int);
extern int arguments(void);
extern Result expression(void);
extern Result * interp_block(void);

extern bool exec;
extern int ind_var;
extern Token * token, * runtime_error_detected;
extern std_function ARRAY stdfun;
extern p_Object _this, _virtual;

extern Variable _var[];
extern Result _arg[];

static Result _void = {type_void, .value.rt_pointer = NULL};
Except except;

void find_eop(void);

void find_eob(void)
{
	register int block = 0;
	
	do
	{
		++ token;
		
		if(token->type == tok_pontuation)
		{
			if(* token->value == '{')
				++ block;
			if(* token->value == '}')
				-- block;
		}
		else if(token->intern == kw_se)
		{
			find_eop();
			find_eob();
			
			++ token;
			
			if(token->intern == kw_senao)
			{
				find_eob();
			}
			else
			{
				-- token;
			}
		}
		else if(token->intern == kw_enquanto || token->intern == kw_para)
		{
			find_eop();
			find_eob();
		}
		else if(token->intern == kw_fazer)
		{
			find_eob();
			token_expected(tok_reserved, "enquanto");
			find_eop();
		}
		else if(token->intern == kw_tentar)
		{
			find_eob();
			
			do
			{
				++ token;
				find_eop();
				find_eob();
				++ token;
			}
			while(token->intern == kw_pegar);
			
			-- token;
		}
		else
		{
			while(token->type != tok_pontuation || * token->value != ';')
			{
				++ token;
			}
		}
	}
	while(block);
}

void find_eop(void)
{
	register int par = 0;
	
	do
	{
		++ token;
		
		if(token->type == tok_pontuation)
		{
			if(* token->value == '(')
				++ par;
			if(* token->value == ')')
				-- par;
		}
	}
	while(par);
}

Result * exec_return(void)
{
	static Result ret;
	
	++ token;
	
	if(token->type == tok_pontuation && * token->value == ':')
	{
		++ token;
		ret = expression();
	}
	else
	{
		-- token;
		ret.type = type_void;
		ret.value.rt_pointer = NULL;
	}
	
	token_expected(tok_pontuation, ";");
	exec = false;
	return &ret;
}

void exec_if(void)
{
	Result r;
	
	token_expected(tok_pontuation, "(");
	++ token;
	r = expression();
	token_expected(tok_pontuation, ")");
	
	if(r.type != type_bool)
	{
		error_found("A expressão condicional é incompatível com o tipo \"Logico\"");
	}
	
	if(r.value.rt_bool)
	{
		interp_block();
		
		++ token;
		
		if(token->intern == kw_senao)
		{
			find_eob();
		}
		else
		{
			-- token;
		}
	}
	else
	{
		find_eob();
		++ token;
		
		if(token->intern == kw_senao)
		{
			interp_block();
		}
		else
		{
			-- token;
		}
	}
}

void exec_while(void)
{
	bool ctrl = true;
	Result r;
	Token * begin;
	
	token_expected(tok_pontuation, "(");
	++ token;
	begin = token;
	
	while(ctrl && exec)
	{
		r = expression();
		token_expected(tok_pontuation, ")");
		
		if(r.type != type_bool)
		{
			error_found("A expressão condicional é incompatível com o tipo \"Logico\"");
		}
		
		if(ctrl = r.value.rt_bool)
		{
			interp_block();
			token = begin;
		}
		else
		{
			find_eob();
		}
	}
}

void exec_do(void)
{
	bool ctrl = true;
	Result r;
	Token * begin = token;
	
	interp_block();
	if(!exec)
	{
		return;
	}
	token_expected(tok_reserved, "enquanto");
	token_expected(tok_pontuation, "(");
	++ token;
	r = expression();
	token_expected(tok_pontuation, ")");
	token_expected(tok_pontuation, ";");
	
	if(r.type != type_bool)
	{
		error_found("A expressão condicional é incompatível com o tipo \"Logico\"");
	}
	
	while(r.value.rt_bool)
	{
		token = begin;
		interp_block();
		
		if(!exec)
		{
			return;
		}
		
		token += 3;
		r = expression();
		token += 2;
	}
}

void exec_for(void)
{
	bool ctrl = true;
	register int par, bk_ind_var = ind_var;
	Result r;
	Token *p1, *p2;
	
	token_expected(tok_pontuation, "(");
	++ token;
	
	if(token->type == tok_reserved)
	{
		declare_var();
	}
	else
	{
		expression();
		token_expected(tok_pontuation, ";");
	}
	
	++ token;
	p1 = token;
	
	while(ctrl && exec)
	{
		r = expression();
		token_expected(tok_pontuation, ";");
		
		if(r.type != type_bool)
		{
			error_found("A expressão condicional é incompatível com o tipo \"Logico\"");
		}
		
		++ token;
		p2 = token;
		par = 1;
		
		while(par)
		{
			++ token;
			
			if(token->type == tok_pontuation)
			{
				if(* token->value == '(')
					++ par;
				if(* token->value == ')')
					-- par;
			}
		}
		
		if(exec && (ctrl = r.value.rt_bool))
		{
			interp_block();
			token = p2;
			expression();
			token = p1;
		}
		else
		{
			find_eob();
		}
	}
	
	while(ind_var > bk_ind_var)
	{
		free_var(_var + -- ind_var);
	}
}

void exec_try(void)
{
	register int i, bk_ind_var = ind_var;
	jmp_buf buf, *bk = except.p_jmp_buf;
	Token * begin = token;
	Variable * bk_var = Memory.alloc(ind_var * sizeof(Variable));
	p_Object bk_this = _this, bk_virtual = _virtual;
	
	for(i = 0; i < ind_var; i++)
	{
		bk_var[i] = _var[i];
	}
	
	except.p_jmp_buf = &buf;
	
	if(setjmp(buf))
	{
		bool ctrl = true;
		int id;
		
		token = begin;
		find_eob();
		except.p_jmp_buf = bk;
		
		for(i = 0; i < bk_ind_var; i++)
		{
			_var[i] = bk_var[i];
		}
		ind_var = bk_ind_var;
		_this = bk_this;
		_virtual = bk_virtual;
		
		while((++ token)->intern == kw_pegar)
		{
			token_expected(tok_pontuation, "(");
			++ token;
			id = token->intern;
			token_expected(tok_pontuation, ":");
			++ token;
			
			if(ctrl && _typeof(*(p_Object*)except.thrown, id))
			{
				_var[ind_var].id = token->intern;
				_var[ind_var].type = type_object;
				_var[ind_var].value = except.thrown;
				++ ind_var;
				token_expected(tok_pontuation, ")");
				ctrl = false;
				interp_block();
				Memory.free(except.thrown);
			}
			else
			{
				token_expected(tok_pontuation, ")");
				find_eob();
			}
		}
		
		-- token;
	}
	else
	{
		token = begin;
		
		interp_block();
		
		while((++ token)->intern == kw_pegar)
		{
			token_expected(tok_pontuation, "(");
			++ token;
			token_expected(tok_pontuation, ":");
			++ token;
			token_expected(tok_pontuation, ")");
			find_eob();
		}
		-- token;
		except.p_jmp_buf = bk;
	}
	
	Memory.free(bk_var);
}

void exec_throw(void)
{
	token_expected(tok_pontuation, ":");
	++ token;
	except.thrown = new Pointer(expression().value.rt_pointer);
	token_expected(tok_pontuation, ";");
	longjmp(*except.p_jmp_buf, 1);
}

void runtime_error(int value)
{
	Memory.free(runtime_error_detected[index_error_value].value);
	runtime_error_detected[index_error_value].value = s_is(value);
	token = runtime_error_detected;
	interp_block();
}

Result exec_execute(void)
{
	static int n;
	token_expected(tok_pontuation, "(");
	n = arguments() - 1;
	return stdfun[(int)_arg[0].value.rt_double](_arg + 1, n);
}
