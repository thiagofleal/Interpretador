#include <windows.h>
#include <Tonight\tonight.h>
#include <Tonight\list.h>
#include "add_std_func.h"
#include "header.h"

static P_void error_found = NULL;

Result func0(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_file;
	
	if(numArgs == 1)
	{
		switch((int)_arg[0].value.rt_double)
		{
			case 0:
				ret.value.rt_pointer = stdin;
				break;
			
			case 1:
				ret.value.rt_pointer = stdout;
				break;
			
			case 2:
				ret.value.rt_pointer = stderr;
				break;
		}
	}
	
	if(numArgs == 2)
	{
		ret.value.rt_pointer = fopen(_arg[0].value.rt_String, _arg[1].value.rt_String);
	}
	
	return ret;
}

Result func1(Result *_arg, int numArgs)
{
	Result ret;
	const string cmd = _arg[0].value.rt_String;
	
	ret.type = type_real;
	ret.value.rt_double = (double)system(cmd);
	
	return ret;
}

Result func2(Result *_arg, int numArgs)
{
	Result ret;
	static Painter paint;
	
	paint = new Painter(Tonight.Resources.Color);
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	paint.both((int)_arg[0].value.rt_double, (int)_arg[1].value.rt_double);
	
	return ret;
}

Result func3(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	Tonight.sleep((unsigned int)_arg[0].value.rt_double);
	
	return ret;
}

Result func4(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	Tonight.position((int)_arg[0].value.rt_double, (int)_arg[1].value.rt_double);
	
	return ret;
}

Result func5(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	exit((int)_arg[0].value.rt_double);
	
	return ret;
}

Result func6(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_real;
	ret.value.rt_double = (double)Tonight.getKey();
	
	return ret;
}

Result func7(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_bool;
	ret.value.rt_bool = Tonight.pressKey();
	
	return ret;
}

Result func8(Result *_arg, int numArgs)
{
	Result ret;
	static Random rand;
	
	double arg1 = _arg[0].value.rt_double;
	double arg2 = _arg[1].value.rt_double;
	int arg3 = (int)_arg[2].value.rt_double;
	
	rand = new Random(Tonight.Std.Random.Range);
	
	ret.type = type_real;
	ret.value.rt_double = rand.nextDouble(arg1, arg2, arg3);
	
	return ret;
}

Result func9(Result *_arg, int numArgs)
{
	string str = _arg[0].value.rt_String, p;
	Result ret;
	
	strtod(str, &p);
	ret.type = type_bool;
	ret.value.rt_bool = *p ? false : true;
	
	return ret;
}

Result func10(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_real;
	ret.value.rt_double = atof(_arg[0].value.rt_String);
	
	return ret;
}

Result func11(Result *_arg, int numArgs)
{
	Result ret;
	
	ret.type = type_matrix;
	ret.value.rt_matrix.dimensions = default_matrix_dimension;
	ret.value.rt_matrix.length = 0;
	ret.value.rt_matrix.size = 0;
	ret.value.rt_matrix.type = type_null;
	ret.value.rt_matrix.value = NULL;
	
	return ret;
}

Result func12(Result *_arg, int numArgs)
{
	Result ret;
	pointer library;
	
	string arg1 = _arg[0].value.rt_String;
	string arg2 = _arg[1].value.rt_String;
	
	Result (*func)(Result*, int);
	
	ret.type = type_object;
	ret.value.rt_pointer = NULL;
	
	if(library = LoadLibrary(arg1))
	{
		if(*(pointer*)&func = GetProcAddress(library, arg2))
		{
			ret = func(_arg + 2, numArgs - 2);
		}
		else
		{
			error_found(concat("N�o foi poss�vel encontrar a fun��o \"", arg2, "\"", $end));
		}
		
		FreeLibrary(library);
	}
	else
	{
		error_found(concat("N�o foi poss�vel abrir o arquivo \"", arg1, "\"", $end));
	}
	
	return ret;
}

DLLIMPORT void set_error_function(P_void function)
{
	error_found = function;
}

DLLIMPORT void add_std_func(object list)
{
	iList.add(list, func0);
	iList.add(list, func1);
	iList.add(list, func2);
	iList.add(list, func3);
	iList.add(list, func4);
	iList.add(list, func5);
	iList.add(list, func6);
	iList.add(list, func7);
	iList.add(list, func8);
	iList.add(list, func9);
	iList.add(list, func10);
	iList.add(list, func11);
	iList.add(list, func12);
}
