#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "header.h"

extern void pre_scan(void);
extern void init_list(void);
extern void delete_list(void);
extern void parameters(int);
extern void init_meth_id(void);
extern void initFileList(void);
extern void deleteFileList(void);
extern int arguments(void);
extern unsigned int intern_identifier(string);
extern Result expression(void);
extern std_function ARRAY init_functions(void);
extern Token * tokenMaker(string, string);
extern Result * interp_block(void);
extern Function * find_func(unsigned int);

extern Except except;

int ind_var, ind_func, ind_arg, ind_class, ind_glob;
string library_path;
Token *token, *default_constructor, *default_destructor, *runtime_error_detected, unknow_class;
std_function ARRAY stdfun;

Variable _var[num_var];
Function _func[num_func];
Result _arg[num_arg];
Class _class[num_class];

str_variable var_inf[] = {
	{type_char, sizeof(char)},
	{type_bool, sizeof(bool)},
	{type_int, sizeof(int)},
	{type_real, sizeof(double)},
	{type_string, sizeof(string)},
	{type_file, sizeof(file)},
	{type_object, sizeof(p_Object)},
	{type_matrix, sizeof(str_matrix)},
	{type_null, sizeof NULL},
	{type_void, sizeof NULL}
};

int meth_id[meth_id_number];

Define_Exception(InterpreterException, "Erro na execução do código", GenericException);

void error_found(string str)
{
	throw(
		InterpreterException, concat(
			"Arquivo: ",
			token->file,
			"\nErro na linha ",
			getText(is(token->line)),
			":\n\t>> ",
			str,
			$end
		)
	);
}

bool token_expected(int type, string value)
{
	++ token;
	if(token->type == type && !strcmp(token->value, value))
		return true;
	else
	{
		static string tok_type_name[] = {
			"operador",
			"pontuação",
			"constante",
			"cadeia de caracteres",
			"caractere",
			"palavra-reservada",
			"identificador",
			"fim de arquivo"
		};
		string str_err = concat(
			"Espera-se ", tok_type_name[type],
			" \"", value, "\" após \"", (token - 1)->value, "\"", $end
		);
		error_found(str_err);
		Memory.free(str_err);
		return false;
	}
}

void free_var(Variable * var)
{
	if(var->type == type_string)
	{
		Memory.free(*(string*)var->value);
	}
	
	free(var->value);
}

string $throws open_file(string name)
{
	Scanner read = new Scanner(Tonight.Std.File.Input);
	
	int num_char = 0;
	char *prog, *begin;
	file arq;
	
	try
	{
		arq = File.open(name, File.Mode.read);
		
		while(true)
		{
			++ num_char;
			read.nextChar(arq);
		}
	}
	catch(FileOpenException)
	{
		string error = concat("Não foi possível abrir o arquivo \"", name, "\"", $end);
		error_found(error);
		Memory.free(error);
	}
	catch(InputException)
	{}
	
	rewind(arq);
	prog = begin = Array.Char(num_char);
	
	try
	{
		while(true)
		{
			* prog ++ = read.nextChar(arq);
		}
	}
	catch(InputException)
	{
		* -- prog = 0;
	}
	
	File.close(arq);
	return begin;
}

string get_library_path(string argv)
{
	int i, j;
	string interp = interpreter_name;
	string library = library_folder;
	string url_library = Array.Char(strlen(argv) - strlen(interp) + strlen(library) + 1);
	
	for(i = 0; i < strlen(argv) - strlen(interp); i++)
	{
		url_library[i] = argv[i];
	}
	for(j = i; i < j + strlen(library); i++)
	{
		url_library[i] = library[i - j];
	}
	url_library[i] = '\0';
	return url_library;
}

int main(int argc, string argv[])
{
	Writer error = new Writer(Tonight.Std.Error.Output);
	
	string prog, library;
	Function * f;
	Result *ret = NULL;
	jmp_buf buf;
	
	Locale.category = Locale.Category.Collate;
	Locale.set();
	Locale.category = Locale.Category.Monetary;
	Locale.set();
	Locale.category = Locale.Category.Numeric;
	Locale.set();
	Locale.category = Locale.Category.Time;
	Locale.set();
	
	Tonight.initRandom();
	
	try
	{
		init_list();
		init_meth_id();
		
		unknow_class = (Token){0, interpreter_reference, "(anônima)", tok_identifier, intern_identifier("Anonimo")};
		
		default_constructor = tokenMaker(interpreter_reference, "construtor(){}");
		default_destructor = tokenMaker(interpreter_reference, "destrutor(){}");
		runtime_error_detected = tokenMaker(interpreter_reference, ";__objeto_reservado__.erro_detectado(0);");
		
		stdfun = init_functions();
		except.p_jmp_buf = &buf;
		if(setjmp(*except.p_jmp_buf))
		{
			throw(InterpreterException, "Erro disparado fora de um bloco tentar-pegar");
		}
		
		initFileList();
		
		//Open the standard library
		library_path = get_library_path(argv[0]);
		library = concat(library_path, library_name, $end);
		prog = open_file(library);
		token = tokenMaker(library, prog);
		Array.free(prog);
		pre_scan();
		
		// Open file and separate in tokens
		prog = open_file(argv[1]);
		token = tokenMaker(argv[1], prog);
		Array.free(prog);
		
		// Initialize functions, classes and global variables
		pre_scan();
		ind_glob = ind_var;
		
		deleteFileList();
		
		// Indentify the main function
		f = find_func(intern_identifier("Principal"));
		
		delete_list();
		
		if(!f)
		{
			throw(InterpreterException, "A função \"Principal\" não foi declarada");
		}
		
		token = f->enter;
		
		token_expected(tok_pontuation, "(");
		++ token;
		
		if(token->type == tok_pontuation && * token->value == ')')
		{
			token = f->enter;
			parameters(0);
		}
		else
		{
			token = f->enter;
			_arg[0].type = type_matrix;
			_arg[0].value.rt_matrix.dimensions = 1;
			_arg[0].value.rt_matrix.length = argc - 1;
			_arg[0].value.rt_matrix.size = sizeof(string);
			_arg[0].value.rt_matrix.type = type_string;
			_arg[0].value.rt_matrix.value = argv + 1;
			parameters(1);
		}
		
		ret = interp_block();
		
		if(ret->type == type_real)
		{
			return (int)ret->value.rt_double;
		}
	}
	catch(FileOpenException)
	{
		error.println("Não foi possível abrir o arquivo...\nstrerror(): ", strerror(errno), $end);
	}
	catch(InterpreterException)
	{
		Exception e = getException();
		error.printargln(Error(e), Message(e), $end);
	}
	catch(GenericException)
	{
		Exception e = getException();
		error.println("Erro inesperado: ", Error(e), "\nMensagem de erro: ", Message(e), $end);
		
		if(errno)
		{
			error.println("strerror(errno): ", strerror(errno), $end);
		}
	}
	finally
	{
		Tonight.getKey();
		return Exit.Failure;
	}
	
	return Exit.Success;
}
