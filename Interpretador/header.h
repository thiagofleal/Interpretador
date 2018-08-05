#ifdef _WIN32
#	include <Tonight\tonight.h>
#	ifdef INCLUDE_LIST
#		include <Tonight\list.h>
#	endif
#	ifdef INCLUDE_STD_FUNC
#		include "..\DLL\add_std_func.h"
#	endif
#	define	interpreter_name	"Interpretador\\Interpretador.exe"
#	define	library_folder	"Bibliotecas\\"
#else
#	include <Tonight/tonight.h>
#	ifdef INCLUDE_LIST
#		include <Tonight/list.h>
#	endif
#	ifdef INCLUDE_STD_FUNC
#		include "../SO/add_std_func.h"
#	endif
#	define	interpreter_name	"Interpretador/Interpretador"
#	define	library_folder	"Bibliotecas/"
#endif

#define	interpreter_reference	"Interpretador"
#define	library_name	"padrao.bx"

extern EXCEPTION InterpreterException;

enum num_resources{
	num_var = 300,
	num_func = 1000,
	num_dim = 10,
	num_arg = 30,
	num_class = 200,
	num_attr = 50,
	num_meth = 100,
	default_matrix_dimension = -1
};

enum{
	int_keyword = 1,
	int_operator = 100,
	int_identifier = 200
}intern_types;

enum{
	
	kw_Caractere = int_keyword,
	kw_Logico,
	kw_Inteiro,
	kw_Real,
	kw_Texto,
	kw_Arquivo,
	kw_Objeto,
	
	kw_se,
	kw_senao,
	kw_fazer,
	kw_enquanto,
	kw_para,
	
	kw_verdadeiro,
	kw_falso,
	kw_Nulo,
	
	kw_Incluir,
	kw_retornar,
	kw_funcionalidade_interna,
	
	kw_Classe,
	kw_Herdar,
	kw_Publico,
	kw_Protegido,
	kw_Privado,
	kw_construtor,
	kw_destrutor,
	
	kw_tentar,
	kw_pegar,
	kw_disparar,
	
	kw_novo,
	kw_este,
	kw_virtual,
	kw_base
	
}key_word;

enum{
	tok_operator,
	tok_pontuation,
	tok_constant,
	tok_string,
	tok_character,
	tok_reserved,
	tok_identifier,
	tok_eof
}tok_type;

typedef struct{
	int line;
	String file;
	String value;
	int type;
	int intern;
}Token;

enum{
	type_char,
	type_bool,
	type_int,
	type_real,
	type_string,
	type_file,
	type_object,
	type_matrix,
	type_null,
	type_void
}result_type;

enum{
	
	op_attr = int_operator,
	op_incr,
	op_decr,
	op_incr_value,
	op_decr_value,
	op_incr_mul,
	op_decr_div,
	op_decr_mod,
	op_change,
	
	op_cat,
	
	op_not,
	op_and,
	op_or,
	op_xor,
	op_if,
	op_only_if,
	
	op_add,
	op_sub,
	op_mul,
	op_div,
	op_mod,
	op_pow,
	op_sqr,
	
	op_equal,
	op_differ,
	op_larg,
	op_less,
	op_larg_equal,
	op_less_equal,
	op_str_equal,
	op_str_equal_not_identic,
	
	op_dot,
	op_colch_open,
	op_colch_close,
	
	op_at
	
}enum_operator;

typedef struct{
	pointer value;
	int type;
	size_t size;
	int dimensions;
	int length;
}str_matrix;

typedef union{
	bool rt_bool;
	double rt_double;
	String rt_String;
	pointer rt_pointer;
	str_matrix rt_matrix;
}result_value;

typedef struct{
	int type;
	result_value value;
}Result;

typedef Result (* std_function)(void);

typedef struct{
	unsigned int id;
	int type;
	pointer value;
}Variable;

typedef struct{
	unsigned int id;
	Token *enter;
	Result ret_value;
}Function;

enum{
	mod_public,
	mod_protected,
	mod_private
}modifier;

typedef struct{
	unsigned int id;
	int type;
	pointer value;
	int mode;
}Attribute;

typedef struct{
	unsigned int id;
	Token *enter;
	Result ret_value;
	int mode;
}Method;

typedef struct str_class{
	unsigned int id;
	char name[31];
	struct str_class * p_base;
	Method constructor;
	Method destructor;
	int n_attr;
	int n_met;
	Attribute attr[num_attr];
	Method met[num_meth];
}Class;

typedef struct _str_Object{
	Class * p_class;
	struct _str_Object * base_object;
	Attribute * attr;
}str_Object, *p_Object;

typedef struct{
	int type;
	size_t size;
}str_variable;

typedef struct{
	jmp_buf * p_jmp_buf;
	p_Object thrown;
}Except;

enum{
	meth_id_escrever,
	meth_id_escreverLinha,
	meth_id_descarregar,
	meth_id_reiniciar,
	meth_id_fim,
	meth_id_fechar,
	meth_id_caractere,
	meth_id_inteiro,
	meth_id_real,
	meth_id_palavra,
	meth_id_linha,
	meth_id_tamanho,
	meth_id_formatar,
	meth_id_liberar,
	meth_id_destruir,
	meth_id_classe,
	meth_id_possui,
	meth_id_pertence,
	meth_id_number
}enum_meth_id;

enum{
	null_pointer,
	invalid_index,
	division_by_zero,
	negative_radix
}enum_error;
