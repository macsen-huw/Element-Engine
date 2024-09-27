#pragma once
#include "glm.hpp"
#include <vector>
#include <string>
#include "Model.hpp"

namespace elmt {

    /*
     * An L system is defined as a tuple G = (V, w, P), where:
     * V is the alphabet
     * w is the axiom (initial state)
     * P is the set of production rules
     *
     * For this L system:
     * V = {R, G, B}, where:
     * R rotates the current turtle direction by some angle in some direction
     * G "grows" the current branch (turtle)
     * B creates a new branch (turtle), with its state being the current state of the active turtle
     *
     * w = GGGB
     * P = {(B --> G R B R B)}
     *
     */




    /*
     * each joint defines a circle in space, connecting 2 joints together will produce a (possibly bent) cylinder
     */
    struct Joint{
        glm::vec3 centre;
        float radius;
    };

    struct TreeNode{
        std::vector<TreeNode> children;
        Joint joint;
    };

    struct Turtle{
        glm::vec3 position;
        glm::vec3 orientation;
        uint32_t depth;
    };



    class L_system {


        public:
            explicit L_system(std::string &string);
            Model* createCylinder(Joint &j1, Joint &j2, size_t partitions);


        private:
            std::string string;
            void parseString();

    };

}