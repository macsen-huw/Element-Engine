#include "L_system.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

using namespace elmt;

L_system::L_system(std::string &string) {
    this->string = string;
    parseString();
}


void L_system::parseString() {

    for (int i = 0; i < 10; i++) {
        std::string oldString = string;
        string.clear();

        for (char token: oldString) {
            if (token == 'B') {
                string.append("GRBRB");
            }
        }
    }
}

Model* L_system::createCylinder(elmt::Joint &j1, elmt::Joint &j2, size_t partitions) {


    float angle = (2 * glm::radians(180.f)) / (float) partitions;

    glm::mat3x3 rot = glm::mat3x3(
            cosf(angle), 0, sinf(angle),
            0, 1, 0,
            -sinf(angle), 0, cosf(angle)
    );

    std::vector<glm::vec3> directions;

    glm::vec3 dir = glm::vec3(0, 0, 1);

    for (size_t i = 0; i < partitions; i++) {
        directions.push_back(dir);
        dir = rot * dir;
    }

    Mesh mesh;

    for (size_t i = 0; i < partitions; i++) {

        Vertex vert1{};
        vert1.Position = directions[i] * j1.radius;
        vert1.Normal = glm::normalize(directions[i]);

        Vertex vert2{};
        vert2.Position = directions[(i + 1) % partitions] * j1.radius;
        vert2.Normal = glm::normalize(directions[(i + 1) % partitions]);


        Vertex vert3{};
        vert3.Position = directions[i] * j2.radius + glm::vec3(0, 1, 0);
        vert3.Normal = glm::normalize(directions[i]);

        Vertex vert4{};
        vert4.Position = directions[(i + 1) % partitions] * j2.radius + glm::vec3(0, 1, 0);
        vert4.Normal = glm::normalize(directions[(i + 1) % partitions]);



        printf("\nVerts = \n"
               "(%f, %f, %f), (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)\n",
               vert1.Position.x, vert1.Position.y, vert1.Position.z,
               vert2.Position.x, vert2.Position.y, vert2.Position.z,
               vert3.Position.x, vert3.Position.y, vert3.Position.z,
               vert4.Position.x, vert4.Position.y, vert4.Position.z
        );

        mesh.vertices.push_back(vert1);
        mesh.vertices.push_back(vert2);
        mesh.vertices.push_back(vert3);
        mesh.vertices.push_back(vert4);

        mesh.indices.push_back(mesh.vertices.size() - 3);
        mesh.indices.push_back(mesh.vertices.size() - 4);
        mesh.indices.push_back(mesh.vertices.size() - 2);

        mesh.indices.push_back(mesh.vertices.size() - 2);
        mesh.indices.push_back(mesh.vertices.size() - 1);
        mesh.indices.push_back(mesh.vertices.size() - 3);
    }

    Material material;
    material.diffuse = glm::vec3(1, 0, 0);
    material.specular = glm::vec3(0, 0, 0);

    std::vector<Mesh> meshes = {mesh};
    std::vector<Material> mats = {material};

    Model *model = new Model(meshes, mats, core::getRenderManager());
    return model;
}