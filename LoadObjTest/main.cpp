#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Kapsule/Buffers.h>
#include <Kapsule/VertexArray.h>
#include <Kapsule/Shader.h>
#include <Kapsule/Mesh.h>
#include <Kapsule/Model.h>
#include <Kapsule/Camera.h>
#include <Kapsule/Utility.h>
#include <Kapsule/LayerModel.h>
#include <Kapsule/Snapshot.h>
#include <Kapsule/Video.h>

#include <string>
#include <iostream>
#include <vector>
#include <thread>

const int WIDTH = 1280;
const int HEIGHT = 720;
const GLuint SHADOW_WIDTH = 1024;
const GLuint SHADOW_HEIGHT = 1024;
float theta = 0.0f;
float deltaTime = 0.0f;
const double blockSize = 0.125;
float lastTime = 0.0f;
float curTime;
int frameCnt = 0;
float bodyRotationAngle = 0.0f;
float armRotationAngle = 0.0f;
float movement = 0.0f; 
glm::vec3 lightPos(1.0f, 5.0f, 0.0f);

bool mouseFirst = true;
bool cameraCanMove = true;
float lastX, lastY;

double gameSpeed;

using namespace std;
using namespace Kapsule;

Camera camera(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

void resizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processKeyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeybooard(Kapsule::LEFT, deltaTime);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeybooard(Kapsule::RIGHT, deltaTime);
    } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeybooard(Kapsule::FORWARD, deltaTime);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeybooard(Kapsule::BACKWARD, deltaTime);
    } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        cameraCanMove ^= 1;
    } else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        Kapsule::getSnapshot(WIDTH, HEIGHT, "snapshot.png");
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (mouseFirst) {
        lastX = xpos;
        lastY = ypos;
        mouseFirst = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos, lastY = ypos;
    camera.processMouse(xoffset, yoffset);
}

void renderShadowMap(vector<LayerModel*> layerModels, vector<Model*> models, Shader* shadowShader)
{
    Model* plane = models[0];

    LayerModel* body = layerModels[0];
    LayerModel* head = layerModels[1];
    LayerModel* leftLowerArm = layerModels[2];
    LayerModel* leftUpperArm = layerModels[3];
    LayerModel* rightLowerArm = layerModels[4];
    LayerModel* rightUpperArm = layerModels[5];
    LayerModel* leftUpperLeg = layerModels[6];
    LayerModel* rightUpperLeg = layerModels[7];
    
    shadowShader->use();

    for (int i = -10; i < 10; i++) {
        for (int j = -10; j < 10; j++) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(i * 2.0f, -0.5f, j * 2.0f));
            model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));
            shadowShader->setMat4("model", model);
            plane->draw(*shadowShader);
        }
    }

    bodyRotationAngle = glfwGetTime();
    armRotationAngle = 1.5 * glfwGetTime();
    movement = 2.0 * sin(glfwGetTime());
    
    body->setPositionMatrix(glm::vec3(0.0f, 1.2f, movement));
    rightUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    rightLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    leftUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    leftLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    body->draw(*shadowShader);
}

void renderScene(vector<LayerModel*> layerModels, vector<Model*> models, vector<Shader*> shaders)
{
    Shader* envShader = shaders[0];
    Shader* robotShader = shaders[1];
    //Shader* shadowShader = shaders[2];

    Model* plane = models[0];

    LayerModel* body = layerModels[0];
    LayerModel* head = layerModels[1];
    LayerModel* leftLowerArm = layerModels[2];
    LayerModel* leftUpperArm = layerModels[3];
    LayerModel* rightLowerArm = layerModels[4];
    LayerModel* rightUpperArm = layerModels[5];
    LayerModel* leftUpperLeg = layerModels[6];
    LayerModel* rightUpperLeg = layerModels[7];

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.getViewMatrix();
    
    // draw floor
    envShader->use();
    // configure environment shader
    envShader->setVec3("light.position", lightPos);
    envShader->setVec3("viewPos", camera.eye);

    envShader->setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
    envShader->setVec3("light.diffuse", 0.9f, 0.9f, 0.9f);
    envShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    
    envShader->setFloat("material.shiness", 128.0f);
    envShader->setVec3("material.specular", 0.2f, 0.2f, 0.2f);
    envShader->setInt("material.diffuse", 0);

    envShader->setMat4("projection", projection);
    envShader->setMat4("view", view);

    for (int i = -10; i < 10; i++) {
        for (int j = -10; j < 10; j++) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(i * 2.0f, -0.5f, j * 2.0f));
            model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));
            envShader->setMat4("model", model);
            plane->draw(*envShader);
        }
    }

    // draw robot
    robotShader->use();
    // configure robot shader
    robotShader->setVec3("light.position", lightPos);
    robotShader->setVec3("viewPos", camera.eye);

    robotShader->setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
    robotShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    robotShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    
    robotShader->setFloat("material.shininess", 128.0f);
    robotShader->setVec3("material.specular", 0.727811, 0.626959f, 0.626959f);
    robotShader->setInt("material.diffuse", 0);

    robotShader->setMat4("projection", projection);
    robotShader->setMat4("view", view);

    body->setPositionMatrix(glm::vec3(0.0f, 1.2f, movement));
    rightUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    rightLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    leftUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    leftLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    body->draw(*robotShader);
    
    //shadowShader->use();
    //body->drawShadow(*shadowShader, lightPos, view, projection, glm::mat4(1.0f));
    //	 export video
    //if (!Kapsule::isCompleteRecording()) {
    //    Kapsule::saveVideoFrame(HEIGHT, WIDTH);
    //} else {
    //    Kapsule::saveAsVideo();
    //}
}

int main(int argc, char** argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "EasyAnimationMaker", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create a GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    Shader envShader("shader/env_vshader.glsl", "shader/env_fshader.glsl");
    Shader robotShader("shader/robot_vshader.glsl", "shader/robot_fshader.glsl");
    //Shader shadowShader("shader/shadow_vshader.glsl", "shader/shadow_fshader.glsl");
    Shader depthShader("shader/depth_vshader.glsl", "shader/depth_fshader.glsl");
    Model plane((char*)"model/Plane/Plane.obj");
    LayerModel body = LayerModel();
    body.setNodeModel(Model((char*)"model/man/Body/Body.obj"))
        .setPositionMatrix(glm::vec3(0.0f, 1.2f, 0.0f))
        .setScaleMatrix(glm::vec3(12.0f, 15.0f, 12.0f));


    LayerModel& rightUpperArm = body.addSon()
                                    .setNodeModel(Model((char*)"model/man/UpperArm/UpperArm.obj"))
                                    .setPositionMatrix(glm::vec3(-0.0475f, 0.01f, 0.0f))
                                    .setScaleMatrix(glm::vec3(0.5f, 0.5f, 0.5f));
    LayerModel& rightLowerArm = rightUpperArm.addSon()
                                             .setNodeModel(Model((char*)"model/man/LowerArm/LowerArm.obj"))
                                             .setPositionMatrix(glm::vec3(0.0f, -0.04f, 0.0f))
                                             .setScaleMatrix(glm::vec3(0.5f, 1.0f, 0.5f));

    LayerModel& leftUpperArm = body.addSon()
                                   .setNodeModel(Model((char*)"model/man/UpperArm/UpperArm.obj"))
                                   .setPositionMatrix(glm::vec3(0.0475f, 0.01f, 0.0f))
                                   .setScaleMatrix(glm::vec3(0.5f, 0.5f, 0.5f));

    LayerModel& leftLowerArm = leftUpperArm.addSon()
                                           .setNodeModel(Model((char*)"model/man/LowerArm/LowerArm.obj"))
                                           .setPositionMatrix(glm::vec3(0.0f, -0.04f, 0.0f))
                                           .setScaleMatrix(glm::vec3(0.5f, 1.0f, 0.5f));

    LayerModel& head = body.addSon()
                           .setNodeModel(Model((char*)"model/man/Head/Head.obj"))
                           .setPositionMatrix(glm::vec3(0.0f, 0.05f, 0.0f))
                           .setScaleMatrix(glm::vec3(0.8f, 0.8f, 0.8f));

    LayerModel& leftUpperLeg = body.addSon()
                                   .setNodeModel(Model((char*)"model/man/Leg/Leg.obj"))
                                   .setPositionMatrix(glm::vec3(-0.02f, -0.05f, 0.0f))
                                   .setScaleMatrix(glm::vec3(0.3f, 1.0f, 0.3f));

    LayerModel& rightUpperLeg = body.addSon()
                                    .setNodeModel(Model((char*)"model/man/Leg/Leg.obj"))
                                    .setPositionMatrix(glm::vec3(0.02f, -0.05f, 0.0f))
                                    .setScaleMatrix(glm::vec3(0.3f, 1.0f, 0.3f));

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    
    // generate depth framebuffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //Kapsule::openVideo(1280, 720, 300);
    vector<LayerModel*> LayerModels;
    LayerModels.push_back(&body);
    LayerModels.push_back(&head);
    LayerModels.push_back(&leftLowerArm);
    LayerModels.push_back(&leftUpperArm);
    LayerModels.push_back(&rightLowerArm);
    LayerModels.push_back(&rightUpperArm);
    LayerModels.push_back(&leftUpperLeg);
    LayerModels.push_back(&rightUpperLeg);

    vector<Model*> Models;
    Models.push_back(&plane);

    vector<Shader*> Shaders;
    Shaders.emplace_back(&envShader);
    Shaders.emplace_back(&robotShader);
    //Shaders.emplace_back(&shadowShader);

    envShader.use();
    envShader.setInt("material.diffuse", 0);
    envShader.setInt("shadowMap", 1);

    robotShader.use();
    robotShader.setInt("material.diffuse", 0);
    robotShader.setInt("shadowMap", 1);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render depth map
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        // nearPlane and farPlane may be modified.
        float nearPlane = 1.0f;
        float farPlane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            renderShadowMap(LayerModels, Models, &depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        curTime = glfwGetTime();
        deltaTime = curTime - lastTime;
        lastTime = curTime;
        processKeyboardInput(window);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderScene(LayerModels, Models, Shaders);
        // export video
        //if (!Kapsule::isCompleteRecording()) {
        //    Kapsule::saveVideoFrame(HEIGHT, WIDTH);
        //} else {
        //    Kapsule::saveAsVideo();
        //}
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}