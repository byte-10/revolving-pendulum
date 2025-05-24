#pragma once

//
//#pragma pack(push, 1)
//struct triangle_data {
//  glm::vec3 normal;
//  glm::vec3 verts[3];
//  glm::u16  byte_count;
//};
//#pragma pack(pop)
//
//void flip_zy(glm::vec3& vec)
//{
//  float tmp = vec.z;
//  vec.z = vec.y;
//  vec.y = tmp;
//}
//
//
///**
//* source: https://en.wikipedia.org/wiki/STL_(file_format)
//*/
//class stl
//{
//public:
//  struct triangle {
//    glm::vec3 normal;
//    glm::vec3 verts[3];
//  };
//
//  std::vector<triangle> triangles;
//
//public:
//  bool load(const char* ppath) {
//    FILE* fp;
//    char  buf[90];
//    uint32_t count_triangles;
//#ifdef WIN32
//    fopen_s(&fp, ppath, "rb");
//#else
//    fp = fopen(ppath, "rb");
//#endif
//    if (!fp) {
//      printf("failed to open file\n");
//      return false;
//    }
//
//    constexpr size_t STL_header_size = 80;
//    if (fread(buf, 1, STL_header_size, fp) != STL_header_size) {
//      printf("read header error\n");
//      fclose(fp);
//      return false;
//    }
//
//    if (!memcmp(buf, "solid", sizeof("solid") - 1)) {
//      printf("this is ASCII format!\n");
//      fclose(fp);
//      return false;
//    }
//
//    if (fread(&count_triangles, 1, sizeof(count_triangles), fp) != sizeof(count_triangles)) {
//      printf("read error\n");
//      fclose(fp);
//      return false;
//    }
//
//    printf("count_triangles=%u\n", count_triangles);
//    if (!count_triangles) {
//      printf("no triangles\n");
//      fclose(fp);
//      return false;
//    }
//
//    triangles.reserve((size_t)count_triangles);
//    for (uint32_t i = 0; i < count_triangles; i++) {
//      triangle_data tri_data;
//      if (fread(&tri_data, 1, sizeof(tri_data), fp) != sizeof(tri_data)) {
//        printf("triangle read error\n");
//        fclose(fp);
//        return false;
//      }
//      printf("tri{(%f %f %f), (%f %f %f), (%f %f %f)} normal(%f %f %f) bytes_count: %hu\n",
//        tri_data.verts[0].x, tri_data.verts[0].y, tri_data.verts[0].z,
//        tri_data.verts[1].x, tri_data.verts[1].y, tri_data.verts[1].z,
//        tri_data.verts[2].x, tri_data.verts[2].y, tri_data.verts[2].z,
//        tri_data.normal.x, tri_data.normal.y, tri_data.normal.z,
//        tri_data.byte_count);
//
//      triangle tri;
//      const float scale = 0.1f;
//      tri.verts[0] = tri_data.verts[0] * scale;
//      tri.verts[1] = tri_data.verts[1] * scale;
//      tri.verts[2] = tri_data.verts[2] * scale;
//      flip_zy(tri.verts[0]);
//      flip_zy(tri.verts[1]);
//      flip_zy(tri.verts[2]);
//
//      tri.normal = tri_data.normal;
//      triangles.push_back(tri);
//    }
//    fclose(fp);
//    printf("Model loaded!  %zu triangles\n", triangles.size());
//    return true;
//  }
//
//  void draw() {
//    glBegin(GL_TRIANGLES);
//    for (size_t i = 0; i < triangles.size(); i++) {
//      triangle& tri = triangles[i];
//      glNormal3fv(glm::value_ptr(tri.normal));
//      glVertex3fv(glm::value_ptr(tri.verts[0]));
//      glVertex3fv(glm::value_ptr(tri.verts[1]));
//      glVertex3fv(glm::value_ptr(tri.verts[2]));
//    }
//    glEnd();
//  }
//};
