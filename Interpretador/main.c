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
extern unsigned int intern_identifier(String);
extern Result expression(void);
extern std_function ARRAY init_functions(void);
extern Token * tokenMaker(String, String);
extern Result * interp_block(void);
extern Function * find_func(unsigned int);

extern Except except;

int ind_var, ind_func, ind_arg, ind_class, ind_glob;
String library_path;
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
	{type_string, sizeof(String)},
	{type_file, sizeof(File)},
	{type_object, sizeof(p_Object)},
	{type_matrix, sizeof(str_matrix)},
	{type_null, sizeof NULL},
	{type_void, sizeof NULL}
};

int meth_id[meth_id_number];

Define_Exception(InterpreterException, "Erro na execução do código", GenericException);

void error_found(String str)
{
	throw(InterpreterException, concat("Arquivo: ", token->file, "\nErro na linha ", getText(is(token->line)), ":\n\t>> ", str, end));
}

bool token_expected(int type, String value)
{
	++ token;
	if(token->type == type && !strcmp(token->value, value))
		return true;
	else
	{
		static String tok_type_name[] = {
			"operador",
			"pontuação",
			"constante",
			"cadeia de caracteres",
			"caractere",
			"palavra-reservada",
			"identificador",
			"fim de arquivo"
		};
		String str_err = concat("Espera-se ", tok_type_name[type], " \"", value, "\" após \"", (token - 1)->value, "\"", end);
		error_found(str_err);
		free(str_err);
		return false;
	}
}

void free_var(Variable * var)
{
	if(var->type == type_string)
	{
		free(*(String*)var->value);
	}
	
	free(var->value);
}

String throws open_file(String name)
{
	Scanner read = new Scanner(Tonight.std.file.input);
	
	int num_char = 0;
	char *prog, *begin;
	File arq;
	
	try
	{
		arq = new File(name, "r");
		
		while(true)
		{
			++ num_char;
			read.nextChar(arq);
		}
	}
	catch(FileOpenException)
	{
		String error = concat("Não foi possível abrir o arquivo \"", name, "\"", end);
		error_found(error);
		free(error);
	}
	catch(InputException)
	{}
	
	rewind(arq);
	prog = begin = new array.Char(num_char);
	
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
	
	fclose(arq);
	return begin;
}

void alloc_pointer(String id, int type, pointer value)
{
	_var[ind_var].id = intern_identifier(id);
	_var[ind_var].type = type;
	_var[ind_var].value = new Pointer(value);
	++ ind_var;
}

String get_library_path(String argv)
{
	int i, j;
	String interp = interpreter_name;
	String library = library_folder;
	String url_library = new array.Char(strlen(argv) - strlen(interp) + strlen(library) + 1);
	
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

int main(int argc, String argv[])
{
	Writer error = new Writer(Tonight.std.error);
	
	String prog, library;
	Function * f;
	Result *ret = NULL;
	jmp_buf buf;
	
	Tonight.locale();
	Tonight.initRandom();
	
	try
	{
		init_list();
		init_meth_id();
		
		unknow_class = (Token){0, interpreter_reference, "(anônima)", tok_identifier, intern_identifier("Anonimo")};
		
		alloc_pointer("Tela", type_file, stdout);
		alloc_pointer("Teclado", type_file, stdin);
		
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
		library = concat(library_path, library_name, end);
		prog = open_file(library);
		token = tokenMaker(library, prog);
		free(prog);
		pre_scan();
		
		// Open file and separate in tokens
		prog = open_file(argv[1]);
		token = tokenMaker(argv[1], prog);
		free(prog);
		
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
			_arg[0].value.rt_matrix.size = sizeof(String);
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
		error.println("Não foi possível abrir o arquivo...\nstrerror(): ", strerror(errno), end);
	}
	catch(InterpreterException)
	{
		Exception e = getException();
		error.printargln(Error(e), Message(e), end);
	}
	catch(GenericException)
	{
		Exception e = getException();
		error.println("Erro inesperado: ", Error(e), "\nMensagem de erro: ", Message(e),end);
		
		if(errno)
		{
			error.println("strerror(errno): ", strerror(errno), end);
		}
	}
	finally
	{
		Tonight.getKey();
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
