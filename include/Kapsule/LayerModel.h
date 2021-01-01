#pragma once
#ifndef __LAYER_MODEL_H__
#define __LAYER_MODEL_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Kapsule/Model.h>
#include <Kapsule/Shader.h>

#include <vector>

namespace Kapsule {
    using namespace std;
    
    class LayerModel {
    public:
        LayerModel() 
        {
            positionMatrix = glm::mat4(1.0f);
            scaleMatrix = glm::mat4(1.0f);
            rotationMatrix = glm::mat4(1.0f);
            position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        void draw(Shader& shader)
        {
            position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            __render(shader, this, glm::mat4(1.0f));
        }

        LayerModel& setNodeModel(Kapsule::Model nodeModel)
        {
            this->nodeModel = nodeModel;
            return *this;
        }

        LayerModel& setScaleMatrix(glm::vec3 scaleVec)
        {
            glm::mat4 model(1.0f);
            this->scaleMatrix = glm::scale(model, scaleVec);
            return *this;
        }

        LayerModel& setPositionMatrix(glm::vec3 positionVec)
        {
            glm::mat4 model(1.0f);
            this->positionMatrix = glm::translate(model, positionVec);
            return *this;
        }

        LayerModel& setRotationMatrix(float angle, glm::vec3 axis)
        {
            glm::mat4 model(1.0f);
            this->rotationMatrix = glm::rotate(model, angle, axis);
            return *this;
        }

        LayerModel& move(float movement, glm::vec3 dir)
        {
            
        }

        LayerModel& addSon()
        {
            LayerModel* newNode = new LayerModel();
            this->sons.push_back(newNode);
            return *newNode;
        }

        void drawShadow(Shader& shadowShader, 
                          glm::vec3 lightPos,
                          glm::mat4 viewMatrix,
                          glm::mat4 projMatrix,
                          glm::mat4 modelMatrix)
        {
            __drawShadow(shadowShader, this, lightPos, viewMatrix, projMatrix, modelMatrix);
        }
        
        glm::vec3 getPosition()
        {
            return glm::vec3(position.x, position.y, position.z);
        }
        
    private:
        vector<LayerModel*> sons;
        Kapsule::Model nodeModel;
        glm::mat4 scaleMatrix;
        glm::mat4 positionMatrix;
        glm::mat4 rotationMatrix;
        glm::vec4 position;

        void __render(Shader& shader, LayerModel* curNode, glm::mat4 modelMatrix)
        {
            modelMatrix *= curNode->positionMatrix * curNode->scaleMatrix * curNode->rotationMatrix;
            shader.setMat4("model", modelMatrix);
            curNode->position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            curNode->position = modelMatrix * curNode->position;
            curNode->nodeModel.draw(shader);
            size_t siz = curNode->sons.size();
            for (size_t i = 0; i < siz; i++) {
                __render(shader, curNode->sons[i], modelMatrix);
            }
        }

        void __drawShadow(Shader& shadowShader, 
                          LayerModel* curNode,
                          glm::vec3 lightPos,
                          glm::mat4 viewMatrix,
                          glm::mat4 projMatrix,
                          glm::mat4 modelMatrix)
        {
            modelMatrix *= curNode->positionMatrix * curNode->scaleMatrix * curNode->rotationMatrix;
            curNode->nodeModel.drawShadow(shadowShader, lightPos, viewMatrix, projMatrix, modelMatrix);
            size_t siz = curNode->sons.size();
            for (size_t i = 0; i < siz; i++) {
                __drawShadow(shadowShader, curNode->sons[i], lightPos, viewMatrix, projMatrix, modelMatrix);
            }
        }

    };
}

#endif // !__LAYER_MODEL_H__
