#include "header.h"

extern int ind_var, ind_func, ind_arg, ind_class, access;

extern Variable _var[];
extern Function _func[];
extern strClass _class[];

Variable * find_var(unsigned int id)
{
	register int i;
	
	for(i = ind_var - 1; i >= 0; i--)
		if(_var[i].id == id)
			return _var + i;
	
	return NULL;
}

Function * find_func(unsigned int id)
{
	register int i;
	
	for(i = ind_func - 1; i >= 0; i--)
		if(_func[i].id == id)
			return _func + i;
	
	return NULL;
}

strClass * find_class(unsigned int id)
{
	register int i;
	
	for(i = ind_class - 1; i >= 0; i--)
		if(_class[i].id == id)
			return _class + i;
	
	return NULL;
}

Attribute * find_attr(p_Object * p_obj, unsigned int id)
{
	register int i;
	
	if(!*p_obj)
		return NULL;
	
	for(i = (*p_obj)->p_class->n_attr - 1; i >= 0; i--)
		if((*p_obj)->attr[i].id == id && (*p_obj)->attr[i].mode <= access)
			return (*p_obj)->attr + i;
	
	if(access == mod_private)
	{
		access = mod_protected;
	}
	
	*p_obj = (*p_obj)->base_object;
	return find_attr(p_obj, id);
}

Method * find_met(p_Object * p_obj, unsigned int id)
{
	register int i;
	strClass * p_class;
	
	if(!*p_obj)
	{
		return NULL;
	}
	
	p_class = (*p_obj)->p_class;
	
	for(i = p_class->n_met - 1; i >= 0; i--)
		if(p_class->met[i].id == id && p_class->met[i].mode <= access)
			return p_class->met + i;
	
	if(access == mod_private)
	{
		access = mod_protected;
	}
	
	*p_obj = (*p_obj)->base_object;
	return find_met(p_obj, id);
}
