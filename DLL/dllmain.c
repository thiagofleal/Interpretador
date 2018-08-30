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
	const String cmd = _arg[0].value.rt_String;
	
	ret.type = type_real;
	ret.value.rt_double = (double)system(cmd);
	
	return ret;
}

Result func2(Result *_arg, int numArgs)
{
	Result ret;
	static Painter paint;
	
	paint = new Painter(Tonight.resources.Color);
	
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
	
	rand = new Random(Tonight.std.random.range);
	
	ret.type = type_real;
	ret.value.rt_double = rand.nextDouble(arg1, arg2, arg3);
	
	return ret;
}

Result func9(Result *_arg, int numArgs)
{
	String str = _arg[0].value.rt_String, p;
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
	
	String arg1 = _arg[0].value.rt_String;
	String arg2 = _arg[1].value.rt_String;
	
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
			error_found(concat("Não foi possível encontrar a função \"", arg2, "\"", end));
		}
		
		FreeLibrary(library);
	}
	else
	{
		error_found(concat("Não foi possível abrir o arquivo \"", arg1, "\"", end));
	}
	
	return ret;
}

DLLIMPORT void set_error_function(P_void function)
{
	error_found = function;
}

DLLIMPORT void add_std_func(Object list)
{
	IList iL = initIList();
	
	iL.add(list, func0);
	iL.add(list, func1);
	iL.add(list, func2);
	iL.add(list, func3);
	iL.add(list, func4);
	iL.add(list, func5);
	iL.add(list, func6);
	iL.add(list, func7);
	iL.add(list, func8);
	iL.add(list, func9);
	iL.add(list, func10);
	iL.add(list, func11);
	iL.add(list, func12);
}
