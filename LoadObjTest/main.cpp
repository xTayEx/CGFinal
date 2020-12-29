#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

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
#include <Kapsule/Skybox.h>

#include <string>
#include <iostream>
#include <vector>
#include <thread>

const int WIDTH = 1280;
const int HEIGHT = 720;
const GLuint SHADOW_WIDTH = 2048;
const GLuint SHADOW_HEIGHT = 2048;
float theta = 0.0f;
float deltaTime = 0.0f;
const double blockSize = 0.125;
float lastTime = 0.0f;
float curTime;
int frameCnt = 0;
float bodyRotationAngle = 0.0f;
float armRotationAngle = 0.0f;
float movement = 0.0f; 
glm::vec3 lightPos(1.0f, 3.0f, 1.0f);

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

// for shadow map debug
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void renderShadowMap(vector<LayerModel*> layerModels, vector<Model*> models, Shader* shadowShader)
{
    Model* plane = models[0];
    Model* grassBlock = models[1];
    Model* bunny = models[2];
    Model* monkey = models[3];

    LayerModel* body = layerModels[0];
    LayerModel* head = layerModels[1];
    LayerModel* leftLowerArm = layerModels[2];
    LayerModel* leftUpperArm = layerModels[3];
    LayerModel* rightLowerArm = layerModels[4];
    LayerModel* rightUpperArm = layerModels[5];
    LayerModel* leftUpperLeg = layerModels[6];
    LayerModel* rightUpperLeg = layerModels[7];
    
    shadowShader->use();

    glm::mat4 blockModel(1.0f);
    blockModel = glm::translate(blockModel, glm::vec3(-2.0f, 1.0f, 0.0f));
    blockModel = glm::scale(blockModel, glm::vec3(20.0f, 20.0f, 20.0f));
    shadowShader->setMat4("model", blockModel);
    grassBlock->draw(*shadowShader);

    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
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
    
    body->setPositionMatrix(glm::vec3(0.0f, 0.7f, movement));
    rightUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    rightLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    leftUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    leftLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    body->draw(*shadowShader);

    glm::mat4 bunnyModel(1.0f);
    bunnyModel = glm::translate(bunnyModel, glm::vec3(2.0f, 0.3f, 2.0f));
    bunnyModel = glm::scale(bunnyModel, glm::vec3(0.3f, 0.3f, 0.3f));
    shadowShader->setMat4("model", bunnyModel);
    bunny->draw(*shadowShader);

    glm::mat4 monkeyModel(1.0f);
    monkeyModel = glm::translate(monkeyModel, glm::vec3(2.0f, 0.3f, 4.0f));
    monkeyModel = glm::scale(monkeyModel, glm::vec3(0.3f, 0.3f, 0.3f));
    shadowShader->setMat4("model", monkeyModel);
    monkey->draw(*shadowShader);
}

void renderScene(vector<LayerModel*> layerModels, vector<Model*> models, vector<Shader*> shaders)
{
    Shader* envShader = shaders[0];
    Shader* robotShader = shaders[1];
    Shader* bunnyShader = shaders[2];
    //Shader* shadowShader = shaders[2];

    Model* plane = models[0];
    Model* grassBlock = models[1];
    Model* bunny = models[2];
    Model* monkey = models[3];

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

    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
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

    body->setPositionMatrix(glm::vec3(0.0f, 0.7f, movement));
    rightUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    rightLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    leftUpperArm->setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    leftLowerArm->setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    body->draw(*robotShader);

    // draw grass block
    glm::mat4 blockModel(1.0f);
    blockModel = glm::translate(blockModel, glm::vec3(-2.0f, 1.0f, 0.0f));
    blockModel = glm::scale(blockModel, glm::vec3(20.0f, 20.0f, 20.0f));
    robotShader->setMat4("model", blockModel);
    grassBlock->draw(*robotShader);

    bunnyShader->use();
    bunnyShader->setInt("reflectSwitch", 0);
    glm::mat4 bunnyModel(1.0f);
    bunnyModel = glm::translate(bunnyModel, glm::vec3(2.0f, 0.3f, 2.0f));
    bunnyModel = glm::scale(bunnyModel, glm::vec3(0.3f, 0.3f, 0.3f));
    bunnyShader->setMat4("model", bunnyModel);
    bunnyShader->setMat4("projection", projection);
    bunnyShader->setMat4("view", view);
    bunnyShader->setVec3("viewPos", camera.eye);
    bunnyShader->setVec3("lightPos", lightPos);
    bunny->draw(*bunnyShader);

    glm::mat4 monkeyModel(1.0f);
    bunnyShader->setInt("reflectSwitch", 1);
    monkeyModel = glm::translate(monkeyModel, glm::vec3(2.0f, 0.3f, 4.0f));
    monkeyModel = glm::scale(monkeyModel, glm::vec3(0.3f, 0.3f, 0.3f));
    bunnyShader->setMat4("model", monkeyModel);
    monkey->draw(*bunnyShader);

    
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
    //stbi_set_flip_vertically_on_load(true);

    Shader envShader("shader/env_vshader.glsl", "shader/env_fshader.glsl");
    Shader robotShader("shader/robot_vshader.glsl", "shader/robot_fshader.glsl");
    //Shader shadowShader("shader/shadow_vshader.glsl", "shader/shadow_fshader.glsl");
    Shader depthShader("shader/depth_vshader.glsl", "shader/depth_fshader.glsl");
    Shader debugShader("shader/debug_vshader.glsl", "shader/debug_fshader.glsl");
    Shader skyboxShader("shader/skybox_vshader.glsl", "shader/skybox_fshader.glsl");
    Shader bunnyShader("shader/bunny_vshader.glsl", "shader/bunny_fshader.glsl");

    Model plane((char*)"model/Plane/Plane.obj");
    Model grassBlock((char*)"model/dirt/grassBlock.obj");
    Model ball((char*)"model/ball/Ball.obj");
    Model monkey((char*)"model/monkey/Monkey.obj");
    LayerModel body = LayerModel();
    body.setNodeModel(Model((char*)"model/man/Body/Body.obj"))
        .setPositionMatrix(glm::vec3(0.0f, 0.7f, 0.0f))
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    Models.push_back(&grassBlock);
    Models.push_back(&ball);
    Models.push_back(&monkey);

    vector<Shader*> Shaders;
    Shaders.emplace_back(&envShader);
    Shaders.emplace_back(&robotShader);
    Shaders.emplace_back(&bunnyShader);
    //Shaders.emplace_back(&shadowShader);

    envShader.use();
    envShader.setInt("material.diffuse", 0);
    envShader.setInt("shadowMap", 1);

    robotShader.use();
    robotShader.setInt("material.diffuse", 0);
    robotShader.setInt("shadowMap", 1);

    // set up skybox
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    Kapsule::Skybox skybox;
    vector<string> facesName = {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg",
    };
    skybox.loadCubemap(facesName);
    skybox.setupSkybox(50.0f);

    bunnyShader.use();
    bunnyShader.setInt("skybox", 0);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render depth map
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        // nearPlane and farPlane may be modified.
        float nearPlane = -15.0f;
        float farPlane = 15.0f;
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

        envShader.use();
        envShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        robotShader.use();
        robotShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderScene(LayerModels, Models, Shaders);
        // export video
        //if (!Kapsule::isCompleteRecording()) {
        //    Kapsule::saveVideoFrame(HEIGHT, WIDTH);
        //} else {
        //    Kapsule::saveAsVideo();
        //}

        // for shadow map debug
        //debugShader.use();
        //debugShader.setFloat("near_plane", nearPlane);
        //debugShader.setFloat("far_plane", farPlane);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, depthMap);
        //renderQuad();
        // end

        // render skybox
        glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        skybox.renderSkybox(skyboxShader, view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
