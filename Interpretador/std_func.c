#define INCLUDE_LIST
#define INCLUDE_STD_FUNC

#include "header.h"

extern void error_found(string);

std_function ARRAY init_functions(void)
{
	return add_std_func();
}
