#include "header.h"

extern void find_eob(void);
extern void free_var(Variable*);
extern void exec_if(void);
extern void exec_while(void);
extern void exec_do(void);
extern void exec_for(void);
extern void exec_try(void);
extern void exec_throw(void);
extern void error_found(string);
extern bool token_expected(int, string);
extern Result expression(void);
extern Result * exec_return(void);

extern bool exec;
extern int ind_var;
extern Token * token;
extern Variable _var[];

static Result _void = {type_void, .value.rt_pointer = NULL};

Result * interp_block(void)
{
	bool block = false;
	const int bk_ind_var = ind_var;
	static Result res, *ret = &_void;
	
	do
	{
		++ token;
		
		if(token->type == tok_pontuation)
		{
			ret = &_void;
			if(* token->value == '{')
			{
				block = true;
			}
			if(* token->value == '}')
			{
				break;
			}
		}
		else switch(token->intern)
		{
			case kw_Caractere:
			case kw_Logico:
			case kw_Inteiro:
			case kw_Real:
			case kw_Texto:
			case kw_Arquivo:
			case kw_Objeto:
				declare_var();
				break;
			
			case kw_se:
				exec_if();
				break;
			case kw_senao:
				error_found("Sintaxe incorreta");
				break;
			case kw_fazer:
				exec_do();
				break;
			case kw_enquanto:
				exec_while();
				break;
			case kw_para:
				exec_for();
				break;
			
			case kw_retornar:
				ret = exec_return();
				break;
			
			case kw_Incluir:
			case kw_Classe:
			case kw_Herdar:
			case kw_Privado:
			case kw_Protegido:
			case kw_Publico:
			case kw_construtor:
			case kw_destrutor:
				error_found("Sintaxe incorreta");
				break;
			
			case kw_tentar:
				exec_try();
				break;
			case kw_pegar:
				break;
			case kw_disparar:
				exec_throw();
				break;
			
			default:
				ret = &_void;
				res = expression();
				token_expected(tok_pontuation, ";");
				
				if(res.type == type_string)
				{
					Memory.free(res.value.rt_String);
				}
		}
	
	}while(block && exec);
	
	while(ind_var > bk_ind_var)
	{
		free_var(_var + -- ind_var);
	}
	
	return ret;
}
