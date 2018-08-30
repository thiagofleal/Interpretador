#define INCLUDE_LIST
#define INCLUDE_STD_FUNC

#include "header.h"

extern void error_found(String);

std_function ARRAY init_functions(void)
{
	Object list = new Object(List);
	std_function ARRAY ret;
	
	set_error_function(error_found);
	add_std_func(list);
	ret = initIList().toArray(list);
	delete(list);
	return ret;
}
