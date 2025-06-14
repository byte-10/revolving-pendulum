#include "wavefront_obj.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#if 0
#pragma region MY_OBJ_LOADER
bool obj_equalscmd(const char *p_currline, const char *p_startcmd)
{
	char buffer[256];
	strcpy_s(buffer, sizeof(buffer), p_currline);
	char *ptr = strchr(buffer, ' ');
	if (ptr) {
		*ptr = 0x0;
		return !strcmp(buffer, p_startcmd);
	}
	return false;
}

void obj_try_set_firstcmd(char *p_dst, int maxlen, char *p_line)
{
	char buffer[256];
	strcpy_s(buffer, sizeof(buffer), p_line);
	char *ptr = strchr(buffer, ' ');
	if (!p_dst[0] && ptr) {
		*ptr = 0x0;
		strcpy_s(p_dst, maxlen, buffer);
	}
}

#define obj_set_token_line(p_token, p_line) (p_token = p_line)

bool obj_token(char **p_dst_tok_addr, char **p_tokptr, int token)
{
	// no more tokens
	if (!(*p_tokptr))
		return false;

	*p_dst_tok_addr = *p_tokptr; //save previous token address
	(*p_tokptr) = strchr(*p_tokptr, token);
	if (!(*p_tokptr))
		return true;

	*(*p_tokptr) = '\0'; // null-terminate token
	(*p_tokptr)++; // increment pointer from '\0'
	return true;
}

enum OBJ_CMD {
	OBJ_CMD_GROUP = 0,
	OBJ_CMD_VERTEX,
	OBJ_CMD_VERTEX_NORMAL,
	OBJ_CMD_FACES
};

bool obj_save_data_to_group(std::vector<obj_group *> &dst_groups, char *p_currgroupname, int currgroup_maxlen,
	std::vector<glm::vec3> &temp_vertices, std::vector<glm::vec3> &temp_normals, std::vector<glm::vec2> &temp_uvs,
	std::vector<unsigned int> &vertex_indices, std::vector<unsigned int> &uv_indices, std::vector<unsigned int> &normal_indices)
{
	obj_group *p_meshgroup = new obj_group();
	if (!p_meshgroup) {
		DBG("Failed to allocate memory");
		return false;
	}

	strcpy_s(p_meshgroup->name, sizeof(p_meshgroup->name), p_currgroupname);

	for (size_t i = 0; i < vertex_indices.size(); i++)
		p_meshgroup->vertices.push_back(temp_vertices[vertex_indices[i] - 1]);

	for (size_t i = 0; i < uv_indices.size(); i++)
		p_meshgroup->uvs.push_back(temp_uvs[uv_indices[i] - 1]);

	for (size_t i = 0; i < normal_indices.size(); i++)
		p_meshgroup->normals.push_back(temp_normals[normal_indices[i] - 1]);

	DBG("Vertices count: %zd", p_meshgroup->vertices.size());
	DBG("UVs count: %zd", p_meshgroup->uvs.size());
	DBG("Normals count: %zd", p_meshgroup->normals.size());
	dst_groups.push_back(p_meshgroup);

	/* RESET TO DEFAULTS */
	strcpy_s(p_currgroupname, currgroup_maxlen, "new_group"); // reset value to default
	temp_vertices.clear();
	temp_uvs.clear();
	temp_normals.clear();

	vertex_indices.clear();
	uv_indices.clear();
	normal_indices.clear();
	return true;
}

#define IS_VALID_LINE(line) (line[0] != '\n' && line[0] != '#')

/* SUPPORT ONLY TRIANGLES */
bool load_wavefront_obj(std::vector<obj_group *> &dst_groups, const char *p_filename)
{
	char line[256], group[64], firstcmd[64];
	strcpy_s(group, sizeof(group), "new_group");
	firstcmd[0] = 0;
	union {
		glm::vec3 temp_vec3;
		glm::vec2 temp_vec2;
	};

	FILE *fp = NULL;
	fopen_s(&fp, p_filename, "rt");
	if (!fp) {
		DBG("Failed to open file '%s'", p_filename);
		return false;
	}

	bool vertices_block_broken = false;
	obj_group *p_object = NULL;
	std::vector<unsigned int> vertex_indices, uv_indices, normal_indices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	while (!feof(fp) && !ferror(fp)) {
		fgets(line, sizeof(line), fp);
		if (!IS_VALID_LINE(line))
			continue;

		else if (line[0] == 'g' && sscanf(line, "g %64s", group) == 1) {
			obj_try_set_firstcmd(firstcmd, sizeof(firstcmd), line);
		}

		/* READING UVS */
		else if (line[0] == 'v' && line[1] == 't') {
			if (sscanf(line, "vt %f %f", &temp_vec2.x, &temp_vec2.y) == 2) {
				vertices_block_broken = true;
				temp_uvs.push_back(temp_vec2);
				obj_try_set_firstcmd(firstcmd, sizeof(firstcmd), line);
				while (!feof(fp) && !ferror(fp) && (line[0] == 'v' && line[1] == 't')) {
					fgets(line, sizeof(line), fp);
					if (!IS_VALID_LINE(line))
						continue;

					/* IGNORE W COMPONENT IN COORDS */
					if (sscanf(line, "vt %f %f", &temp_vec2.x, &temp_vec2.y) == 2) {
						temp_uvs.push_back(temp_vec2);						
					}
				}
				DBG("UVS: %zd", temp_uvs.size());
			}
		}

		/* READING VERTEX NORMALS */
		else if (line[0] == 'v' && line[1] == 'n') {
			if (sscanf(line, "vn %f %f %f", &temp_vec3.x, &temp_vec3.y, &temp_vec3.z) == 3) {
				temp_normals.push_back(temp_vec3);
				vertices_block_broken = true;
				obj_try_set_firstcmd(firstcmd, sizeof(firstcmd), line);
				while (!feof(fp) && !ferror(fp) && (line[0] == 'v' && line[1] == 'n')) {
					fgets(line, sizeof(line), fp);
					if (!IS_VALID_LINE(line))
						continue;

					if (sscanf(line, "vn %f %f %f", &temp_vec3.x, &temp_vec3.y, &temp_vec3.z) == 3) {
						temp_normals.push_back(temp_vec3);	
					}
				}
				DBG("NORMALS: %zd", temp_normals.size());
			}
		}

		/* READING VERTICES */
		else if (line[0] == 'v') {
			if (sscanf(line, "v %f %f %f", &temp_vec3.x, &temp_vec3.y, &temp_vec3.z) == 3) {

				/* IF FOUND NEW SHAPE VERTICES BLOCK, SAVE OLD DATA TO GROUP */
				if (vertices_block_broken) {
					if (!obj_save_data_to_group(dst_groups, group, sizeof(group), temp_vertices, temp_normals, temp_uvs, vertex_indices, uv_indices, normal_indices)) {
						DBG("saving group failed");
						fclose(fp);
						return false;
					}
					vertices_block_broken = false; // mark as no-broken
				}

				temp_vertices.push_back(temp_vec3);
				obj_try_set_firstcmd(firstcmd, sizeof(firstcmd), line);
				while (!feof(fp) && !ferror(fp) && line[0] == 'v') {
					fgets(line, sizeof(line), fp);
					if (!IS_VALID_LINE(line))
						continue;

					if (sscanf(line, "v %f %f %f", &temp_vec3.x, &temp_vec3.y, &temp_vec3.z) == 3) {
						DBG("%s", line);
						temp_vertices.push_back(temp_vec3);
					}
				}
				DBG("VERTS: %zd", temp_vertices.size());
			}
		}

		/* READING FACES */
		else if (line[0] == 'f') {
			unsigned int vertidxs[3], uvidxs[3], normalidxs[3];
			if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertidxs[0], &uvidxs[0], &normalidxs[0], &vertidxs[1], &uvidxs[1], &normalidxs[1], &vertidxs[2], &uvidxs[2], &normalidxs[2]) != 9) {
				fclose(fp);
				return false;
			}
			vertex_indices.push_back(vertidxs[0]);
			vertex_indices.push_back(vertidxs[1]);
			vertex_indices.push_back(vertidxs[2]);
			uv_indices.push_back(uvidxs[0]);
			uv_indices.push_back(uvidxs[1]);
			uv_indices.push_back(uvidxs[2]);
			normal_indices.push_back(normalidxs[0]);
			normal_indices.push_back(normalidxs[1]);
			normal_indices.push_back(normalidxs[2]);
			obj_try_set_firstcmd(firstcmd, sizeof(firstcmd), line);
			while (!feof(fp) && !ferror(fp) && (line[0] == 'f' || line[0] == 's')) {
				fgets(line, sizeof(line), fp);
				if (!IS_VALID_LINE(line))
					continue;

				if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertidxs[0], &uvidxs[0], &normalidxs[0], &vertidxs[1], &uvidxs[1], &normalidxs[1], &vertidxs[2], &uvidxs[2], &normalidxs[2]) == 9) {
					vertex_indices.push_back(vertidxs[0]);
					vertex_indices.push_back(vertidxs[1]);
					vertex_indices.push_back(vertidxs[2]);
					uv_indices.push_back(uvidxs[0]);
					uv_indices.push_back(uvidxs[1]);
					uv_indices.push_back(uvidxs[2]);
					normal_indices.push_back(normalidxs[0]);
					normal_indices.push_back(normalidxs[1]);
					normal_indices.push_back(normalidxs[2]);
				}
			}
			DBG("FACES: vertex_indices=%zd uv_indices=%zd normal_indices=%zd", vertex_indices.size(), uv_indices.size(), normal_indices.size());
		}
	}
	return true;
}
#pragma endregion
#endif

obj_importer::obj_importer()
{
}

bool obj_importer::load(const char* pmodel)
{
	mesh* pmesh;
	std::string error;
	tinyobj::attrib_t m_attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	/* parsing model */
	if (!tinyobj::LoadObj(&m_attrib, &shapes, &materials, nullptr, &error, pmodel)) {
		printf("failed to load OBJ: '%s'\n", error.c_str());
		return false;
	}

	/* prepare meshes */
	m_meshes.reserve(shapes.size());
	for (size_t i = 0; i < shapes.size(); i++) {
		tinyobj::shape_t* pshape = &shapes[i];
		pmesh = new (std::nothrow)mesh();
		if (!pmesh) {
			printf("memory allocation for mesh failed!\n");
			return false;
		}
		printf("preparing mesh: \"%s\"\n", pshape->name.c_str());
		if (!pmesh->prepare_data(&m_attrib, pshape)) {
			printf("preparing data failed!\n");
			return false;
		}
		m_meshes.push_back(pmesh);
	}
	return true;
}

void obj_importer::unload()
{
	for (size_t i = 0; i < get_num_meshes(); i++) {
		mesh* pmesh = get_mesh(i);
		if (pmesh) {
			delete pmesh;
		}
	}
}

inline size_t obj_importer::find_node_by_name(const char* pname)
{
	for (size_t i = 0; i < get_num_meshes(); i++) {
		mesh* pmesh = get_mesh(i);
		if (pmesh && !strcmp(pmesh->get_name(), pname)) {
			return i;
		}
	}
	return bad_index;
}

bool obj_importer::mesh::prepare_data(
	const tinyobj::attrib_t* attrib,
	const tinyobj::shape_t* pshape)
{
	m_name = pshape->name;
	for (size_t f = 0; f < pshape->mesh.num_face_vertices.size(); ++f) {
		if (pshape->mesh.num_face_vertices[f] != 3) {
			assert(false && "face is not triangulated!");
			return false;
		}
	}

	for (const tinyobj::index_t& idx : pshape->mesh.indices) {
		int vi = idx.vertex_index * 3;
		m_verts.emplace_back(
			attrib->vertices[vi + 0],
			attrib->vertices[vi + 1],
			attrib->vertices[vi + 2]
		);

		if (idx.normal_index >= 0) {
			int ni = idx.normal_index * 3;
			m_normals.emplace_back(
				attrib->normals[ni + 0],
				attrib->normals[ni + 1],
				attrib->normals[ni + 2]
			);
		}

		if (idx.texcoord_index >= 0) {
			int ti = idx.texcoord_index * 2;
			m_texcoords.emplace_back(
				attrib->texcoords[ti + 0],
				attrib->texcoords[ti + 1]
			);
		}
	}
	return true;
}