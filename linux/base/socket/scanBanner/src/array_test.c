#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "array.h"
#include "request.h"

int main (int argc, char **argv) {
 	array *a;
	data_string *ds;
	data_count *dc;
    data_config *dg;

	UNUSED(argc);
	UNUSED(argv);

	a = array_init();

	ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("abc"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("alfrag"));

	array_insert_unique(a, (data_unset *)ds);

	ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("abc"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("hameplman"));

	array_insert_unique(a, (data_unset *)ds);
    
    ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("abc"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("xiangshouding"));
    
	array_insert_unique(a, (data_unset *)ds);

	ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("123"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("alfrag"));

	array_insert_unique(a, (data_unset *)ds);

	dc = data_count_init();
	buffer_copy_string_len(dc->key, CONST_STR_LEN("def"));

	array_insert_unique(a, (data_unset *)dc);

	dc = data_count_init();
	buffer_copy_string_len(dc->key, CONST_STR_LEN("def"));

	array_insert_unique(a, (data_unset *)dc);
    
    //get element
    ds = (data_string *)array_get_element(a, "abc");
    fprintf(stdout, "%s\n", ds->value->ptr);
	array_print(a, 0);

	array_free(a);

	fprintf(stderr, "%d\n",
	       buffer_caseless_compare(CONST_STR_LEN("Content-Type"), CONST_STR_LEN("Content-type")));

	return 0;
}
