#pragma once
#include "wavefront_obj.h"
#include <vector>
#include "node.h"

bool dump_model(obj_importer& objimp,
	model_node* pnodes,
	size_t count,
	const char* pfilename);

