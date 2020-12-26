#pragma once
#ifndef __MESH_H__
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Kapsule/Buffers.h>
#include <Kapsule/VertexArray.h>
#include <Kapsule/Shader.h>

#include <string>

namespace Kapsule {
    using namespace std;	
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct Texture {
        unsigned int textureID;
        string type;
        string path;
    };

    class Mesh {
    public:
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        Mesh(vector<Vertex> vertices,
            vector<unsigned int> indices,
            vector<Texture> textures);
        void draw(const Shader& shader);
		void drawShadow(const Shader& shadowShader, 
						glm::vec3 lightPos,
                        glm::mat4 shadowViewMatrix,
                        glm::mat4 projMatrix,
                        glm::mat4 modelMatrix);
    private:
        VertexArray vao;
        VertexBuffer vbo;
        ElementBuffer ebo;
        void setupMesh();
    };

    Mesh::Mesh(vector<Vertex> vertices,
        vector<unsigned int> indices,
        vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }

    void Mesh::setupMesh()
    {
        vao.create();
        vbo.create();
        ebo.create();

        vao.bind();
        vbo.bind();
        vbo.loadData(vertices, USAGE::STATIC_DRAW);

        ebo.bind();
        ebo.loadData(indices, USAGE::STATIC_DRAW);

        // vertex position
        vao.setAttributePointer(vbo, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normal
        vao.setAttributePointer(vbo, 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // texture coord
        vao.setAttributePointer(vbo, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        vao.unbind();
        //DEBUG
        //DEBUGX(textures.size())
    }

    void Mesh::draw(const Shader& shader)
    {
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string name = textures[i].type;
            if (name == "diffuse_texture") {
                number = to_string(diffuseNr++);
            } else if (name == "specular_texture") {
                number = to_string(specularNr++);
            }
            glUniform1i(glGetUniformLocation(shader.ShaderID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].textureID);
        }

        vao.bind();
        //DEBUGX(vao.id)
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        vao.unbind();
        glActiveTexture(GL_TEXTURE0);
    }

    void Mesh::drawShadow(const Shader& shadowShader, 
                          glm::vec3 lightPos,
                          glm::mat4 shadowViewMatrix,
                          glm::mat4 ProjMatrix,
                          glm::mat4 modelMatrix)
    {
        float lx = lightPos.x;
        float ly = lightPos.y;
        float lz = lightPos.z;
        glm::mat4 shadowProjMatrix = glm::mat4(-ly, 0.0,  0.0,  0.0,
                                                lx, 0.0,   lz,  1.0,
                                               0.0, 0.0,  -ly,  0.0,
                                               0.0, 0.0,  0.0, -ly);
        modelMatrix = shadowProjMatrix * modelMatrix;
        shadowShader.setMat4("viewMatrix", shadowViewMatrix);
        shadowShader.setMat4("modelMatrix", modelMatrix);
        shadowShader.setMat4("projMatrix", ProjMatrix);
        shadowShader.setVec3("fColor", glm::vec3(0.0f, 0.0f, 0.0f));
        vao.bind();
        //DEBUGX(vao.id)
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        vao.unbind();
    }


};
#endif // !__MESH_H__
