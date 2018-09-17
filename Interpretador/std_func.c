#define INCLUDE_LIST
#define INCLUDE_STD_FUNC

#include "header.h"

extern void error_found(string);

std_function ARRAY init_functions(void)
{
	object list = new Object(List);
	std_function ARRAY ret;
	
	set_error_function(error_found);
	add_std_func(list);
	ret = iList.toArray(list);
	delete(list);
	return ret;
}
