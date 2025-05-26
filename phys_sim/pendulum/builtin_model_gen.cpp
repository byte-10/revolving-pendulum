#include "builtin_model_gen.h"

/*
* print meshes info
*/
void print_meshes(obj_importer& objimp)
{
	printf("=== meshes  ===\n");
	for (size_t i = 0; i < objimp.get_num_meshes(); i++) {
		obj_importer::mesh* pmesh = objimp.get_mesh(i);
		printf("%s\n", pmesh->get_name());
	}
	printf("======================\n");
}

/**
* log nodes
*/
void print_nodes(model_node* pnodes,
	size_t count)
{
	printf("=== logging nodes ===\n");
	for (size_t i = 0; i < count; i++) {
		model_node* pnode = &pnodes[i];
		
	}
}

bool dump_model(obj_importer& objimp,
	model_node* pnodes,
	size_t count,
	const char* pfilename)
{
	print_meshes(objimp);



}
