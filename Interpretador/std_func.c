#define INCLUDE_LIST
#define INCLUDE_STD_FUNC

#include "header.h"

extern void error_found(string);

std_function ARRAY init_functions(void)
{
	object list = new(List.class);
	std_function ARRAY ret = NULL;
	
	set_error_function(error_found);
	add_std_func(list);
	ret = $(list $as List).toArray();
	delete(list);
	return ret;
}
