#ifndef _ADD_STD_FUNC_H_
#	define _ADD_STD_FUNC_H_

#	ifdef _WIN32
#		if BUILDING_DLL
#			define SHARED __declspec(dllexport)
#		else
#			define SHARED __declspec(dllimport)
#		endif
#	else
#		define SHARED
#	endif

	SHARED void set_error_function(P_void);
	SHARED pointer add_std_func(void);

#endif
