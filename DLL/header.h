#define default_matrix_dimension -1

enum{
	type_char,
	type_bool,
	type_int,
	type_real,
	type_string,
	type_file,
	type_object,
	type_matrix,
	type_null,
	type_void
}var_type;

typedef struct{
	void *value;
	int type;
	size_t size;
	int dimensions;
	int length;
}str_matrix;

typedef union{
	bool rt_bool;
	double rt_double;
	String rt_String;
	pointer rt_pointer;
	str_matrix rt_matrix;
}result_type;

typedef struct{
	int type;
	result_type value;
}Result;
