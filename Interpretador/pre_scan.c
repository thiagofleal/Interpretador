#define INCLUDE_LIST

#include <string.h>
#include "header.h"

extern void error_found(String);
extern bool token_expected(int, String);
extern String throws open_file(String);
extern Result expression(void);
extern Token * tokenMaker(String, String);
extern Class * find_class(unsigned int);

extern int ind_var, ind_func, ind_arg, ind_class;
extern Token *token, *default_constructor, *default_destructor;

extern Variable _var[];
extern Function _func[];
extern Result _arg[];
extern Class _class[];

void pre_scan(void);

static Object fileList = NULL;

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
						Class * p_base;
						
						token_expected(tok_pontuation, ":");
						++ token;
						p_base = find_class(token->intern);
						
						if(!p_base)
						{
							String str_err = concat("A classe \"", token->value, "\" não foi declarada", end);
							error_found(str_err);
							free(str_err);
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
		fileList = new Object(List);
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

static void registerFile(String file)
{
	if(fileList)
	{
		initIList().add(fileList, file);
	}
}

bool checkFile(String file)
{
	if(fileList)
	{
		IList iList = initIList();
		register int i, size = iList.size(fileList);
		
		for(i = 0; i < size; i++)
		{
			if(!strcmp(iList.get(fileList, i), file))
			{
				return true;
			}
		}
		
		registerFile(file);
		return false;
	}
}

static void include_file(String file)
{
	String prog;
	Token * bk_tok;
	
	if(!checkFile(file))
	{
		prog = open_file(file);
		bk_tok = token;
		token = tokenMaker(file, prog);
		free(prog);
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
						String file;
						
						token_expected(tok_pontuation, ":");
						++ token;
						file = expression().value.rt_String;
						token_expected(tok_pontuation, ";");
						include_file(file);
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
