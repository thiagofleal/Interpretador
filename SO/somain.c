#include <Tonight/tonight.h>
#include <Tonight/list.h>
#include "add_std_func.h"
#include "header.h"

static Result *_arg;

Result func0(void)
{
	Result ret;
	
	ret.type = type_file;
	ret.value.rt_pointer = fopen(_arg[1].value.rt_String, _arg[2].value.rt_String);
	
	return ret;
}

Result func1(void)
{
	Result ret;
	const String cmd = _arg[1].value.rt_String;
	
	ret.type = type_real;
	ret.value.rt_double = (double)system(cmd);
	
	return ret;
}

Result func2(void)
{
	Result ret;
	static Painter paint;
	
	paint = new Painter(Tonight.resources.Color);
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	paint.both((int)_arg[1].value.rt_double, (int)_arg[2].value.rt_double);
	
	return ret;
}

Result func3(void)
{
	Result ret;
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	Tonight.sleep((unsigned int)_arg[1].value.rt_double);
	
	return ret;
}

Result func4(void)
{
	Result ret;
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	Tonight.position((int)_arg[1].value.rt_double, (int)_arg[2].value.rt_double);
	
	return ret;
}

Result func5(void)
{
	Result ret;
	
	ret.type = type_void;
	ret.value.rt_pointer = NULL;
	exit((int)_arg[1].value.rt_double);
	
	return ret;
}

Result func6(void)
{
	Result ret;
	
	ret.type = type_real;
	ret.value.rt_double = (double)Tonight.getKey();
	
	return ret;
}

Result func7(void)
{
	Result ret;
	
	ret.type = type_bool;
	ret.value.rt_bool = Tonight.pressKey();
	
	return ret;
}

Result func8(void)
{
	Result ret;
	static Random rand;
	
	double arg1 = _arg[1].value.rt_double;
	double arg2 = _arg[2].value.rt_double;
	int arg3 = (int)_arg[3].value.rt_double;
	
	rand = new Random(Tonight.std.random.range);
	
	ret.type = type_real;
	ret.value.rt_double = rand.nextDouble(arg1, arg2, arg3);
	
	return ret;
}

Result func9(void)
{
	String str = _arg[1].value.rt_String, p;
	Result ret;
	
	strtod(str, &p);
	ret.type = type_bool;
	ret.value.rt_bool = (bool)(!*p);
	
	return ret;
}

Result func10(void)
{
	Result ret;
	
	ret.type = type_real;
	ret.value.rt_double = atof(_arg[1].value.rt_String);
	
	return ret;
}

Result func11(void)
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

void set_arg(void *arg)
{
	_arg = arg;
}

void add_std_func(Object list)
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
}
