#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tiny_obj_loader.h"

class obj_importer
{
public:
	class mesh {
		friend class obj_importer;
		std::string            m_name;
		std::vector<glm::vec3> m_verts;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec2> m_texcoords;

		bool prepare_data(const tinyobj::attrib_t *attrib, const tinyobj::shape_t* pshape);

	public:
		inline size_t get_num_verts() const { return m_verts.size(); }
		inline size_t get_num_normals() const { return m_normals.size(); }
		inline size_t get_num_texcoords() const { return m_texcoords.size(); }
		inline glm::vec3* get_verts() { return m_verts.data(); }
		inline glm::vec3* get_normals() { return m_normals.data(); }
		inline glm::vec2* get_texcoords() { return m_texcoords.data(); }
		inline const char* get_name() const { return m_name.c_str(); }
	};

private:
	std::vector<mesh*> m_meshes;
public:
	obj_importer();

	/* load/unload */
	bool   load(const char *pmodel);
	void   unload();

	/* get data */
	constexpr static size_t bad_index = (size_t)-1;
	inline size_t find_node_by_name(const char* pname);
	inline size_t get_num_meshes() const { return m_meshes.size(); }
	inline mesh* get_mesh(size_t idx) const {
		assert(idx < m_meshes.size() && "idx out of bounds");
		return m_meshes[idx];
	}

	inline std::vector<mesh*>& get_meshes() { return m_meshes; }
};