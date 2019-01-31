#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "header.h"

#define method(_meth)	meth_id[meth_id_##_meth]

extern void exec_method(Method*, str_Object*);
extern void error_found(string);
extern void free_var(Variable*);
extern bool token_expected(int, string);
extern int arguments(void);
extern unsigned int intern_identifier(string);
extern Result expression(void);
extern strClass * find_class(unsigned int);
extern Attribute * find_attr(p_Object*, unsigned int);
extern Method * find_met(p_Object*, unsigned int);

extern Token * token;
extern int meth_id[];
extern Result _arg[];

void init_meth_id(void)
{
	method(escrever) = intern_identifier("escrever");
	method(escreverLinha) = intern_identifier("escreverLinha");
	method(descarregar) = intern_identifier("descarregar");
	method(reiniciar) = intern_identifier("reiniciar");
	method(fim) = intern_identifier("fim");
	method(fechar) = intern_identifier("fechar");
	method(caractere) = intern_identifier("caractere");
	method(inteiro) = intern_identifier("inteiro");
	method(real) = intern_identifier("real");
	method(palavra) = intern_identifier("palavra");
	method(linha) = intern_identifier("linha");
	method(tamanho) = intern_identifier("tamanho");
	method(formatar) = intern_identifier("formatar");
	method(liberar) = intern_identifier("liberar");
	method(destruir) = intern_identifier("destruir");
	method(classe) = intern_identifier("classe");
	method(liberar) = intern_identifier("liberar");
	method(possui) = intern_identifier("possui");
	method(pertence) = intern_identifier("pertence");
}

int expected_arguments(int num_args_min, int num_args_max)
{
	int arg;
	
	token_expected(tok_pontuation, "(");
	arg = arguments();
	
	if(arg < num_args_min)
	{
		error_found("Argumentos insuficientes");
		return false;
	}
	if(arg > num_args_max)
	{
		error_found("Excesso de Argumentos");
		return false;
	}
	
	return arg;
}

string format_string(const register string _format, Result *args, register int args_length)
{
	register bool zero = false;
	register int integer = 0, decimal = 5;
	static char str[1001];
	string format = _format;
	
	*str = 0;
	
	while(* format)
	{
		if(* format == '%')
		{
			if(isdigit(* ++ format))
			{
				if(* format == '0')
				{
					zero = true;
				}
				integer = strtol(format, &format, 10);
			}
			if(* format == '*')
			{
				if(* format == '0')
				{
					zero = true;
				}
				integer = (int)args->value.rt_double;
				++ args;
				++ format;
			}
			if(* format == '.')
			{
				++ format;
				
				if(isdigit(* format))
				{
					decimal = strtol(format, &format, 10);
				}
				if(* format == '*')
				{
					decimal = (int)args->value.rt_double;
					++ args;
					++ format;
				}
			}
			
			switch(* format)
			{
				case 'l':
					if(args->value.rt_bool)
						sprintf(str, "%sverdadeiro", str);
					else
						sprintf(str, "%sfalso", str);
					break;
				case 'c':{
					char cstr[] = {(char)args->value.rt_double, 0};
					strcat(str, cstr);
					break;
				}
				case 'i':
					if(zero)
						sprintf(str, "%s%0*d", str, integer, (int)args->value.rt_double);
					else
						sprintf(str, "%s%*d", str, integer, (int)args->value.rt_double);
					break;
				case 'r':
					if(zero)
						sprintf(str, "%s%0*.*lf", str, integer, decimal, args->value.rt_double);
					else
						sprintf(str, "%s%*.*lf", str, integer, decimal, args->value.rt_double);
					break;
				case 't':
					sprintf(str, "%s%*s", str, integer, args->value.rt_String);
					break;
				case 'a':
				case 'o':
					sprintf(str, "%s%*p", str, integer, args->value.rt_pointer);
					break;
				
				default:{
					char cstr[] = {* format, 0};
					strcat(str, cstr);
					break;
				}
			}
			
			if(args_length)
			{
				++ args;
				-- args_length;
			}
			else
			{
				args->type = type_void;
				args->value.rt_pointer = NULL;
			}
		}
		else
		{
			char cstr[] = {* format, 0};
			strcat(str, cstr);
		}
		
		++ format;
	}
	
	return toString(str);
}

bool methods_Arquivo(Result * r, int _meth_id)
{
	register int ret = true;
	file file = r->value.rt_pointer;
	Result bk_arg[num_arg];
	
	memcpy(bk_arg, _arg, sizeof bk_arg);
	
	if(_meth_id == method(escrever) || _meth_id == method(escreverLinha))
	{
		string s;
		
		expected_arguments(1, 1);
		
		switch(_arg[0].type)
		{
			case type_char:
				s = s_cs((char)_arg[0].value.rt_double);
				break;
			case type_bool:
				s = _arg[0].value.rt_bool ? toString("verdadeiro") : toString("falso");
				break;
			case type_real:
				s = s_ds(_arg[0].value.rt_double);
				break;
			case type_string:
				s = toString(_arg[0].value.rt_String);
				break;
			default:
				s = Memory.alloc(sizeof(char) * 17);
				sprintf(s, "%p", _arg[0].value.rt_pointer);
				break;
		}
		
		fputs(s, (FILE*)file);
		
		if(_meth_id == method(escreverLinha))
		{
			fputc('\n', (FILE*)file);
		}
		
		r->type = type_void;
		r->value.rt_pointer = NULL;
		
		Memory.free(s);
	}
	else if(_meth_id == method(descarregar))
	{
		expected_arguments(0, 0);
		r->type = type_bool;
		r->value.rt_bool = fflush((FILE*)file) ? true : false;
	}
	else if(_meth_id == method(reiniciar))
	{
		expected_arguments(0, 0);
		r->type = type_void;
		r->value.rt_pointer = NULL;
		File.rewind(file);
	}
	else if(_meth_id == method(fim))
	{
		expected_arguments(0, 0);
		r->type = type_bool;
		r->value.rt_bool = File.end(file) ? true : false;
	}
	else if(_meth_id == method(fechar))
	{
		expected_arguments(0, 0);
		r->type = type_bool;
		r->value.rt_bool = fclose((FILE*)file) ? true : false;
	}
	else if(_meth_id == method(caractere))
	{
		char c;
		
		expected_arguments(0, 0);
		fscanf((FILE*)file, "%c", &c);
		r->type = type_char;
		r->value.rt_double = (double)c;
	}
	else if(_meth_id == method(inteiro))
	{
		int i;
		
		expected_arguments(0, 0);
		fscanf((FILE*)file, "%i", &i);
		r->type = type_real;
		r->value.rt_double = (double)i;
	}
	else if(_meth_id == method(real))
	{
		double d;
		
		expected_arguments(0, 0);
		fscanf((FILE*)file, "%lf", &d);
		r->type = type_real;
		r->value.rt_double = d;
	}
	else if(_meth_id == method(palavra))
	{
		char s[301];
		
		expected_arguments(0, 0);
		fscanf((FILE*)file, "%s", s);
		r->type = type_string;
		r->value.rt_String = toString(s);
	}
	else if(_meth_id == method(linha))
	{
		char s[1001];
		
		expected_arguments(0, 0);
		fscanf((FILE*)file, " %[^\n]s", s);
		r->type = type_string;
		r->value.rt_String = toString(s);
	}
	else
	{
		ret = false;
	}
	
	memcpy(_arg, bk_arg, sizeof bk_arg);
	return ret;
}

bool methods_Texto(Result * r, int _meth_id)
{
	register int ret = true;
	string str = r->value.rt_String;
	Result bk_arg[num_arg];
	
	memcpy(bk_arg, _arg, sizeof bk_arg);
	
	if(_meth_id == method(caractere))
	{
		register int ind;
		
		expected_arguments(1, 1);
		ind = (int)_arg[0].value.rt_double;
		r->type = type_char;
		
		if(ind >= 0 && ind < strlen(str))
		{
			r->value.rt_double = (double)((uchar)str[ind]);
		}
		else
		{
			r->value.rt_double = 0.0;
		}
		
		Memory.free(str);
	}
	else if(_meth_id == method(formatar))
	{
		register int nargs = expected_arguments(0, num_arg);
		
		r->type = type_string;
		r->value.rt_String = format_string(str, _arg, nargs);
		Memory.free(str);
	}
	else if(_meth_id == method(tamanho))
	{
		r->type = type_real;
		r->value.rt_double = (double)strlen(str);
		Memory.free(str);
	}
	else
	{
		ret = false;
	}
	
	memcpy(_arg, bk_arg, sizeof bk_arg);
	return ret;
}

bool methods_Matriz(Result * r, int _meth_id)
{
	register bool ret = true;
	str_matrix mtrx = r->value.rt_matrix;
	Result bk_arg[num_arg];
	
	memcpy(bk_arg, _arg, sizeof bk_arg);
	
	if(_meth_id == method(liberar))
	{
		expected_arguments(0, 0);
		free(mtrx.value);
		mtrx.length = 0;
		
		r->type = type_void;
		r->value.rt_pointer = NULL;
	}
	else if(_meth_id == method(tamanho))
	{
		r->type = type_real;
		r->value.rt_double = (double)mtrx.length;
	}
	else
	{
		ret = false;
	}
	
	memcpy(_arg, bk_arg, sizeof bk_arg);
	return ret;
}

void delete_object(p_Object obj)
{
	register int i, n_attr = obj->p_class->n_attr;
	Token * bk_token = token;
	
	exec_method(&obj->p_class->destructor, obj);
	
	if(obj->base_object)
	{
		token = bk_token;
		delete_object(obj->base_object);
	}
	
	for(i = 0; i < n_attr; i++)
	{
		free_var((Variable*)(obj->attr + i));
	}
	
	Memory.free(obj->attr);
	obj->attr = NULL;
	obj->p_class = NULL;
	free(obj);
}

bool _typeof(p_Object obj, int classId)
{
	if(obj->p_class->id == classId)
	{
		return true;
	}
	if(obj->base_object)
	{
		return _typeof(obj->base_object, classId);
	}
	return false;
}

bool methods_Objeto(Result * r, int _meth_id)
{
	p_Object obj = r->value.rt_pointer;
	static string name;
	
	name = (string)&obj->p_class->name;
	
	if(_meth_id == method(destruir))
	{
		delete_object(obj);
		r->type = type_void;
		r->value.rt_pointer = NULL;
		return true;
	}
	
	if(_meth_id == method(classe))
	{
		expected_arguments(0, 0);
		r->type = type_string;
		r->value.rt_String = name;
		return true;
	}
	
	if(_meth_id == method(pertence))
	{
		int id;
		
		token_expected(tok_pontuation, "(");
		++ token;
		id = token->intern;
		token_expected(tok_pontuation, ")");
		r->type = type_bool;
		r->value.rt_bool = _typeof(obj, id);
		return true;
	}
	
	++ token;
	if(token->type == tok_pontuation && * token->value == '(')
	{
		Method * m = find_met(&obj, _meth_id);
		-- token;
		
		if(!m)
		{
			string str = concat("A classe \"", name, "\" não possui o método \"", token->value, "\"", $end);
			error_found(str);
			Memory.free(str);
			return false;
		}
		
		exec_method(m, obj);
		
		*r = m->ret_value;
		return true;
	}
	else
	{
		Attribute *a = find_attr(&obj, _meth_id);
		-- token;
		
		if(!a)
		{
			string str = concat("A classe \"", name, "\" não possui o atributo \"", token->value, "\"", $end);
			error_found(str);
			Memory.free(str);
			return false;
		}
		attrib_result(r, a->value, a->type);
		++ token;
		
		if(token->intern >= op_attr && token->intern <= op_change)
		{
			int op = token->intern;
			
			if(op != op_incr && op != op_decr && op != op_change)
			{
				++ token;
			}
			
			op_attrib(a->value, a->type, r, op);
		}
		else
		{
			-- token;
		}
		
		return true;
	}
	
	return false;
}
