#include <string.h>
#include <math.h>
#include "header.h"

extern void attrib_result(Result*, pointer, int);
extern void declare_class(Token*);
extern void runtime_error(int);
extern void error_found(String);
extern void op_attrib(pointer, int, Result*, register int);
extern void exec_func(Function*);
extern void exec_method(Method*, str_Object*);
extern bool token_expected(int, String);
extern bool methods_Arquivo(Result*, int);
extern bool methods_Texto(Result*, int);
extern bool methods_Matriz(Result*, int);
extern bool methods_Objeto(Result*, int);
extern int arguments(void);
extern Result exec_execute(void);
extern Variable * find_var(unsigned int);
extern Function * find_func(unsigned int);
extern Class * find_class(unsigned int);

extern int ind_var, ind_func, ind_arg, ind_class;
extern Token *token, unknow_class;

extern str_variable var_inf[];
extern Variable _var[];
extern Function _func[];
extern Result _arg[];
extern Class _class[];
extern String library_path;

static void expText(Result*);
static void expBool(Result*);
static void expRelac(Result*);
static void expArit0(Result*);
static void expArit1(Result*);
static void expArit2(Result*);
static void expElement(Result*);
static void expUnary(Result*);
static void expParent(Result*);
static void expValue(Result*);

int access;
p_Object _this = NULL, _virtual = NULL;

static double ret;

INLINE double char_value(char ch)
{
	return ch - '0';
}

INLINE double pow_10(int exp)
{
	for(ret = 1.0; exp; --exp)ret *= 10.0;
	return ret;
}

double stod(String str)
{
	double value = 0.0;
	
	while(* str && * str != '.')
	{
		value *= 10.0;
		value += char_value(* str ++);
	}
	
	if(* str == '.')
	{
		register int dec = 1;
		++ str;
		
		while(* str)
		{
			value += char_value(* str ++) / pow_10(dec ++);
		}
	}
	
	return value;
}

Result expression(void)
{
	Result result = {};
	p_Object bk_this = _this;
	p_Object bk_virtual = _virtual;
	
	access = mod_public;
	expText(&result);
	_this = bk_this;
	_virtual = bk_virtual;
	return result;
}

static void expText(Result *r)
{
	String str1, str2;
	
	expBool(r);
	++ token;
	
	while(token->intern == op_cat)
	{
		str1 = r->value.rt_String;
		++ token;
		expBool(r);
		
		switch(r->type)
		{
			case type_char:
				str2 = s_cs((char)r->value.rt_double);
				break;
			case type_bool:
				str2 = toString(r->value.rt_bool ? "verdadeiro" : "falso");
				break;
			case type_real:
				str2 = s_ds(r->value.rt_double);
				break;
			case type_string:
				str2 = toString(r->value.rt_String);
				break;
			case type_file:
			case type_object:
			case type_matrix:
			{
				char addr[17];
				sprintf(addr, "%p", r->value.rt_pointer);
				str2 = toString(addr);
				break;
			}
		}
		r->value.rt_String = concat(str1, str2, end);
		free(str1);
		free(str2);
		r->type = type_string;
		
		++ token;
	}
	-- token;
}

static void expBool(Result *r)
{
	bool b1, b2;
	int op;
	
	expRelac(r);
	++ token;
	
	while(token->intern >= op_and && token->intern <= op_only_if)
	{
		op = token->intern;
		b1 = r->value.rt_bool;
		
		++ token;
		expRelac(r);
		b2 = r->value.rt_bool;
		
		switch(op)
		{
			case op_and:
				r->value.rt_bool = b1 && b2;
				break;
			case op_or:
				r->value.rt_bool = b1 || b2;
				break;
			case op_xor:
				r->value.rt_bool = (b1 || b2) && !(b1 && b2);
				break;
			case op_if:
				r->value.rt_bool = !b1 && b2 ? false : true;
				break;
			case op_only_if:
				r->value.rt_bool = (bool)(b1 == b2);
				break;
		}
		
		++ token;
	}
	-- token;
}

static void expRelac(Result *r)
{
	int op;
	Result sr;
	
	expArit0(r);
	++ token;
	
	while(token->intern >= op_equal && token->intern <= op_str_equal_not_identic)
	{
		op = token->intern;
		++ token;
		expRelac(&sr);
		
		switch(r->type)
		{
			case type_char:
			case type_int:
			case type_real:
				switch(op)
				{
					case op_equal:
						r->value.rt_bool = (bool)(r->value.rt_double == sr.value.rt_double);
						break;
					case op_differ:
						r->value.rt_bool = (bool)(r->value.rt_double != sr.value.rt_double);
						break;
					case op_larg:
						r->value.rt_bool = (bool)(r->value.rt_double > sr.value.rt_double);
						break;
					case op_less:
						r->value.rt_bool = (bool)(r->value.rt_double < sr.value.rt_double);
						break;
					case op_larg_equal:
						r->value.rt_bool = (bool)(r->value.rt_double >= sr.value.rt_double);
						break;
					case op_less_equal:
						r->value.rt_bool = (bool)(r->value.rt_double <= sr.value.rt_double);
						break;
					default:
						error_found("Operador relacional inválido para valores numéricos");
				}
				break;
			
			case type_bool:
				switch(op)
				{
					case op_equal:
						r->value.rt_bool = (bool)(r->value.rt_bool == sr.value.rt_bool);
						break;
					case op_differ:
						r->value.rt_bool = (bool)(r->value.rt_bool != sr.value.rt_bool);
						break;
					default:
						error_found("Operador relacional inválido para valores lógicos");
				}
				break;
			
			case type_string:
			{
				bool res;
				
				switch(op)
				{
					case op_equal:
						res = (strcmp(r->value.rt_String, sr.value.rt_String) ? false : true);
						free(r->value.rt_String);
						free(sr.value.rt_String);
						break;
					case op_differ:
						res = (strcmp(r->value.rt_String, sr.value.rt_String) ? true : false);
						free(r->value.rt_String);
						free(sr.value.rt_String);
						break;
					case op_str_equal:
						res = equal(r->value.rt_String, sr.value.rt_String);
						free(r->value.rt_String);
						free(sr.value.rt_String);
						break;
					case op_str_equal_not_identic:
						res = equal(r->value.rt_String, sr.value.rt_String) && strcmp(r->value.rt_String, sr.value.rt_String);
						free(r->value.rt_String);
						free(sr.value.rt_String);
						break;
					default:
						error_found("Operador relacional inválido para textos");
				}
				
				r->value.rt_bool = res;
				break;
			}
			
			case type_matrix:
				switch(op)
				{
					case op_equal:
						r->value.rt_bool = (bool)(r->value.rt_matrix.value == sr.value.rt_matrix.value);
						break;
					case op_differ:
						r->value.rt_bool = (bool)(r->value.rt_matrix.value != sr.value.rt_matrix.value);
						break;
					default:
						error_found("Operador relacional inválido para matrizes");
				}
				break;
			
			case type_file:
			case type_object:
			case type_null:
				switch(op)
				{
					case op_equal:
						r->value.rt_bool = (bool)(r->value.rt_pointer == sr.value.rt_pointer);
						break;
					case op_differ:
						r->value.rt_bool = (bool)(r->value.rt_pointer != sr.value.rt_pointer);
						break;
					default:
						error_found("Operador relacional inválido para ponteros");
				}
				break;
			
			default:
				error_found("Tipo de valor desconhecido");
		}
		r->type = type_bool;
		++ token;
	}
	-- token;
}

static void expArit0(Result *r)
{
	int op;
	double fv, sv;
	
	expArit1(r);
	++ token;
	
	while(token->intern == op_add || token->intern == op_sub)
	{
		op = token->intern;
		
		fv = r->value.rt_double;
		++ token;
		expArit1(r);
		sv = r->value.rt_double;
		
		switch(op)
		{
			case op_add:
				r->value.rt_double = fv + sv;
				break;
			
			case op_sub:
				r->value.rt_double = fv - sv;
				break;
		}
		
		++ token;
	}
	
	-- token;
}

static void expArit1(Result *r)
{
	int op;
	double fv, sv;
	
	expArit2(r);
	++ token;
	
	while(token->intern >= op_mul && token->intern <= op_mod)
	{
		op = token->intern;
		fv = r->value.rt_double;
		++ token;
		expArit2(r);
		sv = r->value.rt_double;
		
		switch(op)
		{
			case op_mul:
				r->value.rt_double = fv * sv;
				break;
			
			case op_div:
				if(!sv)
					runtime_error(division_by_zero);
				r->value.rt_double = fv / sv;
				break;
			
			case op_mod:
				r->value.rt_double = (double)((int)fv - ((int)fv / (int)sv) * (int)sv);
				break;
		}
		
		++ token;
	}
	
	-- token;
}

static void expArit2(Result *r)
{
	int op;
	double fv, sv;
	
	expUnary(r);
	++ token;
	
	while(token->intern == op_pow || token->intern == op_sqr)
	{
		op = token->intern;
		fv = r->value.rt_double;
		++ token;
		expUnary(r);
		sv = r->value.rt_double;
		
		switch(op)
		{
			case op_pow:
				r->value.rt_double = pow(fv, sv);
				break;
			
			case op_sqr:
				if(fv < 0)
				{
					if(((int)sv % 2))
						r->value.rt_double = -pow(-fv, 1.0 / sv);
					else
						runtime_error(negative_radix);
				}
				else
				{
					r->value.rt_double = pow(fv, 1.0 / sv);
				}
				break;
		}
		
		++ token;
	}
	
	-- token;
}

static void expUnary(Result *r)
{
	if(token->intern == op_add || token->intern == op_sub || token->intern == op_not)
	{
		int op = token->intern;
		
		++ token;
		expElement(r);
		
		switch(op)
		{
			case op_add:
				return;
			
			case op_sub:
				r->value.rt_double *= (-1.0);
				return;
			
			case op_not:
				r->value.rt_bool = !r->value.rt_bool;
				return;
		}
	}
	expElement(r);
}

static void expElement(Result *r)
{
	expParent(r);
	++ token;
	
	while(token->intern == op_dot || token->intern == op_colch_open)
	{
		if(token->intern == op_dot)
		{
			int id;
			String name, str_err = NULL;
			Result bk_arg[num_arg];
	
			memcpy(bk_arg, _arg, sizeof bk_arg);
			++ token;
			id = token->intern;
			name = token->value;
			
			switch(r->type)
			{
				case type_string:
					if(!methods_Texto(r, id))
					{
						str_err = concat("O tipo de dados \"Texto\" não possui o método/atibuto \"", name, "\"", end);
					}
					break;
				case type_file:
					if(!r->value.rt_pointer)
					{
						runtime_error(null_pointer);
					}
					if(!methods_Arquivo(r, id))
					{
						str_err = concat("O tipo de dados \"Arquivo\" não possui o método/atributo \"", name, "\"", end);
					}
					break;
				case type_object:
				{
					p_Object bk_virtual = _virtual;
					
					if(!r->value.rt_pointer)
					{
						runtime_error(null_pointer);
					}
					_virtual = r->value.rt_pointer;
					
					if(!methods_Objeto(r, id))
					{
						str_err = concat(
							"A classe \"",
							&_virtual->p_class->name[0],
							"\" não possui o método/atributo \"",
							name,
							"\"", end
						);
					}
					_virtual = bk_virtual;
					break;
				}
				case type_matrix:
					if(!methods_Matriz(r, id))
					{
						str_err = concat("O tipo de dados \"Vetor\" não possui o método/atributo \"", name, "\"", end);
					}
					break;
			}
			
			memcpy(_arg, bk_arg, sizeof bk_arg);
			
			if(str_err)
			{
				error_found(str_err);
				free(str_err);
			}
		}
		else
		{
			size_t size;
			register int index, type, lenght = r->value.rt_matrix.length;
			pointer value = r->value.rt_matrix.value;
			
			if(r->type != type_matrix)
			{
				error_found("O operador \"[]\" somente pode ser aplicado a matrizes");
			}
			
			if(r->value.rt_matrix.dimensions - 1)
			{
				type = type_matrix;
				size = sizeof(str_matrix);
			}
			else
			{
				type = r->value.rt_matrix.type;
				size = r->value.rt_matrix.size;
			}
			
			++ token;
			index = (int)expression().value.rt_double;
			
			if(index >= lenght || index < 0)
			{
				runtime_error(invalid_index);
			}
			
			value += (size_t)index * size;
			attrib_result(r, value, type);
			token_expected(tok_operator, "]");
			++ token;
			
			if(token->intern >= op_attr && token->intern <= op_change)
			{
				int op = token->intern;
				
				if(op != op_incr && op != op_decr && op != op_change)
				{
					++ token;
				}
				op_attrib(value, type, r, op);
			}
			else
			{
				-- token;
			}
		}
		++ token;
	}
	-- token;
}

static void expParent(Result *r)
{
	if(token->type == tok_pontuation && * token->value == '(')
	{
		++ token;
		expText(r);
		token_expected(tok_pontuation, ")");
	}
	expValue(r);
}

void create_matrix(str_matrix * matrix, int type, int dimensions, int *length)
{
	matrix->type = var_inf[type].type;
	matrix->length = *length;
	
	if((matrix->dimensions = dimensions) - 1)
	{
		int i;
		
		matrix->value = calloc(*length, sizeof(str_matrix));
		matrix->size = sizeof(str_matrix);
		
		for(i = 0; i < *length; i++)
		{
			create_matrix(matrix->value + i * sizeof(str_matrix), type, dimensions - 1, length + 1);
		}
	}
	else
	{
		matrix->value = calloc(*length, var_inf[type].size);
		matrix->size = var_inf[type].size;
	}
}

p_Object instance_class(Class * p_class)
{
	register int i;
	p_Object _new = malloc(sizeof(str_Object));
	
	if(!p_class)
	{
		return NULL;
	}
	
	_new->p_class = p_class;
	_new->base_object = instance_class(p_class->p_base);
	_new->attr = new Memory(p_class->n_attr * sizeof(Attribute));
	
	for(i = 0; i < p_class->n_attr; i++)
	{
		_new->attr[i].id = p_class->attr[i].id;
		_new->attr[i].mode = p_class->attr[i].mode;
		_new->attr[i].type = p_class->attr[i].type;
		_new->attr[i].value = new Memory(var_inf[p_class->attr[i].type - type_char].size);
		memcpy(_new->attr[i].value, p_class->attr[i].value, var_inf[p_class->attr[i].type - type_char].size);
	}
	
	return _new;
}

static void expValue(Result *r)
{
	switch(token->type)
	{
		case tok_constant:
			
			r->type = type_real;
			r->value.rt_double = stod(token->value);
			return;
		
		case tok_character:
			
			r->type = type_char;
			r->value.rt_double = (double)*token->value;
			return;
		
		case tok_string:
			
			r->type = type_string;
			r->value.rt_String = toString(token->value);
			return;
		
		case tok_reserved:
			
			switch(token->intern)
			{
				case kw_verdadeiro:
					r->type = type_bool;
					r->value.rt_bool = true;
					return;
				
				case kw_falso:
					r->type = type_bool;
					r->value.rt_bool = false;
					return;
				
				case kw_Nulo:
					r->type = type_null;
					r->value.rt_pointer = NULL;
					return;
				
				case kw_novo:
				{
					++ token;
					
					if(token->intern >= kw_Caractere && token->intern <= kw_Objeto)
					{
						register int type, i = 0;
						int dimensions[num_dim];
						
						type = token->intern - kw_Caractere;
						r->type = type_matrix;
						
						while((++ token)->intern == op_colch_open)
						{
							++ token;
							dimensions[i++] = (int)expression().value.rt_double;
							token_expected(tok_operator, "]");
						}
						-- token;
						
						create_matrix(&r->value.rt_matrix, type, i, dimensions);
					}
					if(token->intern == kw_Classe)
					{
						Class * id = new Memory(sizeof(Class));
						p_Object obj;
						
						declare_class(&unknow_class);
						
						memcpy(id, _class + -- ind_class, sizeof(Class));
						obj = instance_class(id);
						
						if(!obj)
						{
							String str_err = concat("A classe \"", token->value, "\" não foi declarada", end);
							error_found(str_err);
							free(str_err);
						}
						
						exec_method(&id->constructor, obj);
						attrib_result(r, &obj, type_object);
					}
					if(token->type == tok_identifier)
					{
						Class * id = find_class(token->intern);
						p_Object obj = instance_class(id);
						
						if(!obj)
						{
							String str_err = concat("A classe \"", token->value, "\" não foi declarada", end);
							error_found(str_err);
							free(str_err);
						}
						
						exec_method(&id->constructor, obj);
						attrib_result(r, &obj, type_object);
					}
					
					return;
				}
				case kw_este:
					r->type = type_object;
					r->value.rt_pointer = _this;
					access = mod_private;
					return;
				case kw_virtual:
					r->type = type_object;
					r->value.rt_pointer = _virtual;
					access = mod_private;
					return;
				case kw_base:
					++ token;
					access = mod_protected;
					if(token->type == tok_pontuation && * token->value == '(')
					{
						-- token;
						exec_method(&_this->base_object->p_class->constructor, _this->base_object);
						r->type = type_void;
						r->value.rt_pointer = NULL;
					}
					else
					{
						-- token;
						r->type = type_object;
						r->value.rt_pointer = _this->base_object;
					}
					return;
				case kw_funcionalidade_interna:
					*r = exec_execute();
					return;
			}
		
		case tok_identifier:
			
			++ token;
			
			if(token->type == tok_pontuation && * token->value == '(')
			{
				Function *f;
				Result bk_arg[num_arg];
				
				memcpy(bk_arg, _arg, sizeof bk_arg);
				-- token;
				f = find_func(token->intern);
				
				if(!f)
				{
					String str = concat("A função \"", token->value, "\" não foi declarada", end);
					error_found(str);
					free(str);
				}
				
				exec_func(f);
				memcpy(_arg, bk_arg, sizeof bk_arg);
				
				*r = f->ret_value;
			}
			else
			{
				Variable *v;
				-- token;
				
				v = find_var(token->intern);
				
				if(!v)
				{
					String str = concat("A variável \"", token->value, "\" não foi declarada", end);
					error_found(str);
					free(str);
					return;
				}
				attrib_result(r, v->value, v->type);
				++ token;
				
				if(token->intern >= op_attr && token->intern <= op_change)
				{
					int op = token->intern;
					
					if(op != op_incr && op != op_decr && op != op_change)
					{
						++ token;
					}
					
					op_attrib(v->value, v->type, r, op);
				}
				else
				{
					-- token;
				}
			}
			break;
		
		case tok_operator:
			
			switch(token->intern)
			{
				case op_at:
					
					r->type = type_string;
					++ token;
					r->value.rt_String = concat(library_path, token->value, end);
					break;
			}
	}
}
