#define INCLUDE_LIST

#include <string.h>
#include "header.h"

extern void error_found(string);
extern bool token_expected(int, string);
extern string $throws open_file(string);
extern Result expression(void);
extern Token * tokenMaker(string, string);
extern strClass * find_class(unsigned int);

extern int ind_var, ind_func, ind_arg, ind_class;
extern Token *token, *default_constructor, *default_destructor;

extern Variable _var[];
extern Function _func[];
extern Result _arg[];
extern strClass _class[];

void pre_scan(void);

static object fileList = NULL;

void ignore_block(void)
{
	int block = 0;
	
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
	}
	while(block);
}

bool declare_method(Method * meth, int mode)
{
	if((token + 1)->type == tok_pontuation && *(token + 1)->value == '(')
	{
		meth->id = token->intern;
		meth->enter = token;
		meth->mode = mode;
		
		while(token->type != tok_pontuation || *token->value != ')')
			++ token;
		ignore_block();
		return true;
	}
	else
	{
		error_found("Sintaxe incorreta");
		return false;
	}
}

void declare_class(Token * id_class)
{
	register int n_attr = 0, n_met = 0;
	register int bk_ind_var = ind_var, bk_ind_func = ind_func, mode = mod_public;
	
	strcpy(_class[ind_class].name, id_class->value);
	_class[ind_class].id = id_class->intern;
	
	token_expected(tok_pontuation, "{");
	++ token;
	++ ind_class;
	
	_class[ind_class - 1].constructor.enter = default_constructor;
	_class[ind_class - 1].destructor.enter = default_destructor;
	
	do
	{
		switch(token->type)
		{
			case tok_reserved:
				switch(token->intern)
				{
					case kw_Caractere:
					case kw_Logico:
					case kw_Inteiro:
					case kw_Real:
					case kw_Texto:
					case kw_Arquivo:
					case kw_Objeto:
						declare_var();
						-- token;
						token_expected(tok_pontuation, ";");
						
						while(ind_var > bk_ind_var)
						{
							-- ind_var;
							_class[ind_class - 1].attr[n_attr].id = _var[ind_var].id;
							_class[ind_class - 1].attr[n_attr].mode = mode;
							_class[ind_class - 1].attr[n_attr].type = _var[ind_var].type;
							_class[ind_class - 1].attr[n_attr].value = _var[ind_var].value;
							++ n_attr;
						}
						
						break;
					case kw_Publico:
					case kw_Protegido:
					case kw_Privado:
						mode = token->intern - kw_Publico;
						token_expected(tok_pontuation, ":");
						break;
					case kw_construtor:
						declare_method(&_class[ind_class - 1].constructor, mod_public);
						break;
					case kw_destrutor:
						declare_method(&_class[ind_class - 1].destructor, mod_public);
						break;
					case kw_Herdar:
					{
						strClass * p_base;
						
						token_expected(tok_pontuation, ":");
						++ token;
						p_base = find_class(token->intern);
						
						if(!p_base)
						{
							string str_err = concat("A classe \"", token->value, "\" não foi declarada", $end);
							error_found(str_err);
							Memory.free(str_err);
						}
						
						_class[ind_class - 1].p_base = p_base;
						token_expected(tok_pontuation, ";");
						break;
					}
					default:
						error_found("Sintaxe incorreta");
				}
				break;
			case tok_identifier:
				declare_method(_class[ind_class - 1].met + n_met, mode);
				++ n_met;
				break;
		}
		++ token;
	}
	while(token->type != tok_pontuation || * token->value != '}');
	
	_class[ind_class - 1].n_attr = n_attr;
	_class[ind_class - 1].n_met = n_met;
}

void initFileList(void)
{
	if(!fileList)
	{
		fileList = new(List.class);
	}
}

void deleteFileList(void)
{
	if(fileList)
	{
		delete(fileList);
		fileList = NULL;
	}
}

static void registerFile(string _file)
{
	if(fileList)
	{
		$(fileList $as List).add(_file);
	}
}

bool checkFile(string _file)
{
	if(fileList)
	{
		register int i, size = $(fileList $as List).size();
		
		for(i = 0; i < size; i++)
		{
			if(!strcmp($(fileList $as List).get(i), _file))
			{
				return true;
			}
		}
		
		registerFile(_file);
		return false;
	}
}

static void include_file(string _file)
{
	string prog;
	Token * bk_tok;
	
	if(!checkFile(_file))
	{
		prog = open_file(_file);
		bk_tok = token;
		token = tokenMaker(_file, prog);
		Array.free(prog);
		pre_scan();
		token = bk_tok;
	}
}

void pre_scan(void)
{
	do
	{
		switch(token->type)
		{
			case tok_reserved:
				switch(token->intern)
				{
					case kw_Incluir:
					{
						string _file;
						
						token_expected(tok_pontuation, ":");
						++ token;
						_file = expression().value.rt_String;
						token_expected(tok_pontuation, ";");
						include_file(_file);
						break;
					}
					case kw_Caractere:
					case kw_Logico:
					case kw_Inteiro:
					case kw_Real:
					case kw_Texto:
					case kw_Arquivo:
					case kw_Objeto:
						declare_var();
						-- token;
						token_expected(tok_pontuation, ";");
						break;
					case kw_Classe:
					{
						Token * tok;
						
						token_expected(tok_pontuation, ":");
						tok = ++ token;
						declare_class(tok);
						break;
					}
				}
				break;
			case tok_identifier:
				if((token + 1)->type == tok_pontuation && *(token + 1)->value == '(')
				{
					_func[ind_func].id = token->intern;
					_func[ind_func].enter = token;
					
					while(token->type != tok_pontuation || *token->value != ')')
						++ token;
					
					++ ind_func;
					ignore_block();
				}
				else
				{
					error_found("Sintaxe incorreta");
				}
		}
		++ token;
	}
	while(token->type != tok_eof);
}
