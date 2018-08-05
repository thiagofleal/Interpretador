#define INCLUDE_LIST

#include <string.h>
#include <ctype.h>
#include "header.h"

static char *p_prog, *begin;
static String file;
static Object idList = NULL;

static char character(void)
{
	if(*p_prog == '\\')
	{
		switch(* ++ p_prog)
		{
			case 'a':
				return '\a';
			case 'b':
				return '\b';
			case 'n':
				return '\n';
			case 'r':
				return '\r';
			case 't':
				return '\t';
			case 'v':
				return '\v';
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				char c = (char)strtol(p_prog, &p_prog, 10);
				-- p_prog;
				return c;
			}
			default:
				return *p_prog;
		}
	}
	return *p_prog;
}

static int line(void)
{
	register int ret = 1;
	register char *p;
	
	for(p = begin; p != p_prog; ++ p)
		if(* p == '\n')
			++ ret;
	
	return ret;
}

static int reserved(String word)
{
	if(!strcmp(word, "Caractere"))
		return kw_Caractere;
	if(!strcmp(word, "Logico"))
		return kw_Logico;
	if(!strcmp(word, "Inteiro"))
		return kw_Inteiro;
	if(!strcmp(word, "Real"))
		return kw_Real;
	if(!strcmp(word, "Texto"))
		return kw_Texto;
	if(!strcmp(word, "Arquivo"))
		return kw_Arquivo;
	if(!strcmp(word, "Objeto"))
		return kw_Objeto;
	if(!strcmp(word, "se"))
		return kw_se;
	if(!strcmp(word, "senao"))
		return kw_senao;
	if(!strcmp(word, "fazer"))
		return kw_fazer;
	if(!strcmp(word, "enquanto"))
		return kw_enquanto;
	if(!strcmp(word, "para"))
		return kw_para;
	if(!strcmp(word, "verdadeiro"))
		return kw_verdadeiro;
	if(!strcmp(word, "falso"))
		return kw_falso;
	if(!strcmp(word, "Nulo"))
		return kw_Nulo;
	if(!strcmp(word, "Incluir"))
		return kw_Incluir;
	if(!strcmp(word, "retornar"))
		return kw_retornar;
	if(!strcmp(word, "funcionalidade_interna"))
		return kw_funcionalidade_interna;
	if(!strcmp(word, "Classe"))
		return kw_Classe;
	if(!strcmp(word, "Herdar"))
		return kw_Herdar;
	if(!strcmp(word, "Privado"))
		return kw_Privado;
	if(!strcmp(word, "Protegido"))
		return kw_Protegido;
	if(!strcmp(word, "Publico"))
		return kw_Publico;
	if(!strcmp(word, "construtor"))
		return kw_construtor;
	if(!strcmp(word, "destrutor"))
		return kw_destrutor;
	if(!strcmp(word, "tentar"))
		return kw_tentar;
	if(!strcmp(word, "pegar"))
		return kw_pegar;
	if(!strcmp(word, "disparar"))
		return kw_disparar;
	if(!strcmp(word, "novo"))
		return kw_novo;
	if(!strcmp(word, "este"))
		return kw_este;
	if(!strcmp(word, "virtual"))
		return kw_virtual;
	if(!strcmp(word, "base"))
		return kw_base;
	return 0;
}

unsigned int intern_identifier(String id)
{
	register int i = 0, size;
	IList iList = initIList();
	
	size = iList.size(idList);
	
	for(; i < size; i++)
	{
		if(!strcmp(id, iList.get(idList, i)))
		{
			return (unsigned int)((i + 1) + int_identifier);
		}
	}
	
	iList.add(idList, toString(id));
	return (unsigned int)((i + 1) + int_identifier);
}

void init_list(void)
{
	if(!idList)
	{
		idList = new Object(List);
	}
}

void delete_list(void)
{
	if(idList)
	{
		pointer p;
		IList iList = initIList();
		
		while(iList.size(idList))
		{
			p = iList.get(idList, 0);
			iList.remove(idList, 0);
			free(p);
		}
		
		delete(idList);
		idList = NULL;
	}
}

static Token nextToken(void)
{
	int i = 0;
	String v = NULL;
	Token ret = {};
	
	ret.file = file;
	/* Ignore white spaces */
	while(isspace(*p_prog))
		++ p_prog;
	
	/* Ignore comentary */
	if(*p_prog == '/' && * (p_prog + 1) == '*')
	{
		++ p_prog;
		do
		{
			while(*p_prog != '*')
				++ p_prog;
			++ p_prog;
		}
		while(*p_prog != '/');
		++ p_prog;
		return nextToken();
	}
	
	if(*p_prog == '/' && * (p_prog + 1) == '/')
	{
		while(* p_prog != '\n')
			++ p_prog;
		return nextToken();
	}
	
	ret.line = line();
	
	/* End of File */
	if(*p_prog == 0)
	{
		ret.type = tok_eof;
		ret.value = new Char(0);
		return ret;
	}
	
	/* Pontuation */
	if(strchr("(){};:,", * p_prog))
	{
		ret.type = tok_pontuation;
		ret.value = new array.Char(2);
		ret.value[0] = *p_prog;
		ret.value[1] = 0;
		++ p_prog;
		return ret;
	}
	
	/* Characters */
	if(*p_prog == '\'')
	{
		++ p_prog;
		ret.type = tok_character;
		ret.value = new array.Char(2);
		ret.value[0] = character();
		ret.value[1] = 0;
		++ p_prog;
		if(*p_prog != '\'')
			fprintf(stderr, "\t-> Erro: Esperado aspas simples(\"'\") após \"%c\"\n", *ret.value);
		++ p_prog;
		return ret;
	}
	
	v = new array.Char(1001);
	
	/* Operators */
	if(strchr("+-*/^\\=~!><&|@#$%[].", *p_prog))
	{
		ret.type = tok_operator;
		
		switch(*p_prog)
		{
			case '=':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_equal;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_attr;
				}
				break;
			
			case '!':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_differ;
						break;
					
					case '!':
						v[1] = *p_prog;
						ret.intern = op_change;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_not;
				}
				break;
			
			case '+':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '+':
						v[1] = *p_prog;
						ret.intern = op_incr;
						break;
					
					case '=':
						v[1] = *p_prog;
						ret.intern = op_incr_value;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_add;
				}
				break;
			
			case '-':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '-':
						v[1] = *p_prog;
						ret.intern = op_decr;
						break;
					
					case '=':
						v[1] = *p_prog;
						ret.intern = op_decr_value;
						break;
					
					case '>':
						v[1] = *p_prog;
						ret.intern = op_if;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_sub;
				}
				break;
			
			case '*':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_incr_mul;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_mul;
				}
				break;
			
			case '/':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_decr_div;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_div;
				}
				break;
			
			case '%':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_decr_mod;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_mod;
				}
				break;
			
			case '^':
				v[0] = *p_prog;
				ret.intern = op_pow;
				v[1] = 0;
				break;
			
			case '\\':
				v[0] = *p_prog;
				ret.intern = op_sqr;
				v[1] = 0;
				break;
			
			case '>':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_larg_equal;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_larg;
				}
				break;
			
			case '<':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '<':
						v[1] = *p_prog;
						ret.intern = op_cat;
						break;
					
					case '>':
						v[1] = *p_prog;
						ret.intern = op_only_if;
						break;
					
					case '=':
						v[1] = *p_prog;
						ret.intern = op_less_equal;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = op_less;
				}
				break;
			
			case '&':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '&':
						v[1] = *p_prog;
						ret.intern = op_and;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = 0;
				}
				break;
			
			case '|':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '|':
						v[1] = *p_prog;
						ret.intern = op_or;
						break;
					
					case '&':
						v[1] = *p_prog;
						ret.intern = op_xor;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = 0;
				}
				break;
			
			case '~':
				v[0] = *p_prog;
				switch(* ++ p_prog)
				{
					case '=':
						v[1] = *p_prog;
						ret.intern = op_str_equal;
						break;
					
					case '~':
						v[1] = *p_prog;
						ret.intern = op_str_equal_not_identic;
						break;
					
					default:
						-- p_prog;
						v[1] = 0;
						ret.intern = 0;
				}
				break;
			
			case '.':
				v[0] = *p_prog;
				ret.intern = op_dot;
				v[1] = 0;
				break;
			
			case '[':
				v[0] = *p_prog;
				ret.intern = op_colch_open;
				v[1] = 0;
				break;
			
			case ']':
				v[0] = *p_prog;
				ret.intern = op_colch_close;
				v[1] = 0;
				break;
			
			case '@':
				v[0] = *p_prog;
				ret.intern = op_at;
				v[1] = 0;
				break;
		}
		++ p_prog;
		v[2] = 0;
		ret.value = toString(v);
		free(v);
		return ret;
	}
	
	/* Number constants */
	if(strchr("0123456789", *p_prog))
	{
		register bool dot = true;
		
		ret.type = tok_constant;
		
		while(strchr("0123456789", *p_prog) || (*p_prog == '.' && dot))
		{
			if(* p_prog == '.')
				dot = false;
			
			v[i] = *p_prog;
			++ i;
			++ p_prog;
		}
		v[i] = 0;
		ret.value = toString(v);
		free(v);
		return ret;
	}
	
	/* Strings */
	if(*p_prog == '\"')
	{
		++ p_prog;
		ret.type = tok_string;
		*v = 0;
		
		while(* p_prog != '\"')
		{
			v[i] = character();
			++ p_prog;
			++ i;
		}
		v[i] = 0;
		++ p_prog;
		ret.value = toString(v);
		free(v);
		return ret;
	}
	
	while(isalnum(*p_prog) || *p_prog == '_')
	{
		v[i] = *p_prog;
		++ p_prog;
		++ i;
	}
	v[i] = 0;
	
	if(ret.intern = reserved(v))
		ret.type = tok_reserved;
	else
	{
		ret.intern = intern_identifier(v);
		ret.type = tok_identifier;
	}
	ret.value = toString(v);
	free(v);
	return ret;
}

Token * throws tokenMaker(String _file, String prog)
{
	int num_tok = 0;
	Token tok, *p_ret, *ret;
	
	file = _file;
	begin = p_prog = prog;
	
	do
	{
		tok = nextToken();
		free(tok.value);
		++ num_tok;
	}
	while(tok.type != tok_eof);
	
	p_ret = ret = new Memory(num_tok * sizeof(Token));
	p_prog = begin;
	
	do
	{
		tok = nextToken();
		* ret ++ = tok;
	}
	while(tok.type != tok_eof);
	
	return p_ret;
}
