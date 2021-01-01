// oengl
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// stb_image library
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

// Kapsule, a simple opengl wrapper created by me
#include <Kapsule/Kapsule.h>

// imgui, a light-weighted gui library
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// C++ std
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
float zMovement = 0.0f; 
float xMovement = 0.0f;
float fov = 45.0f;
float forwardStep = 0.0f;
float rightStep = 0.0f;
float waving;
float leftUpperArmRotationAngle = 0.0f;
float rightUpperArmRotationAngle = 0.0f;
float leftLowerArmRotationAngle = 0.0f;
float rightLowerArmRotationAngle = 0.0f;
float headRotationAngle = 0.0f;
float axeRotationAngle = 0.0f;
int reflectSwitch = 1;

glm::vec3 initRobotFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 robotFront = initRobotFront;
glm::vec3 robotRight = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 lightPos(1.0f, 7.0f, -1.0f);
glm::vec3 ballSize(0.3f, 0.3f, 0.3f);

bool mouseFirst = true;
bool cameraCanMove = true;
bool freeMode = false;
bool isWalking = false;
bool controlling = false;
bool ballControl = false;
bool fullScreen = false;
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
    } else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        camera.processKeybooard(Kapsule::DOWN, deltaTime);
    } else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        camera.processKeybooard(Kapsule::UP, deltaTime);
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        forwardStep = 0.06f;
        isWalking = true;
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        forwardStep = -0.06f;
        isWalking = true;
    } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        rightStep = -0.06f; 
        isWalking = true;
    } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rightStep = 0.06f;
        isWalking = true;
    } else {
        forwardStep = 0.0f;
        rightStep = 0.0f;
        isWalking = false;
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

    glm::mat4 robotFrontRotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(camera.yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    robotFront = glm::normalize(glm::vec3(robotFrontRotationMat * glm::vec4(initRobotFront, 1.0f)));
    robotRight = glm::normalize(glm::cross(robotFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    //DEBUGX(camera.yaw)
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f) {
        fov -= yoffset;
    }
    fov = max(1.0f, fov);
    fov = min(fov, 45.0f);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            cameraCanMove ^=1;
            if (cameraCanMove) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetCursorPosCallback(window, NULL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetCursorPosCallback(window, mouseCallback);
                mouseFirst = true;
            }
        } else if (key == GLFW_KEY_T) {
            Kapsule::getSnapshot(WIDTH, HEIGHT, "sreenshot.png");
        } else if (key == GLFW_KEY_F2) {
            freeMode ^= 1;
            if (freeMode) {
                camera.setEyePosition(glm::vec3(0.0f, 1.5f, 0.0f));
            }
        } else if (key == GLFW_KEY_F11) {
            fullScreen ^= 1;
            glfwSetWindowMonitor(window, fullScreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, WIDTH, HEIGHT, GLFW_DONT_CARE);
        }
    }
}

void imguiSetup(GLFWwindow* window)
{
    // imgui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

}

void imguiNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imguiRender()
{
    static bool showRobotControl = false;
    ImGui::Begin("Control Panel");
    ImGui::SetWindowSize(ImVec2(400.0f, 280.0f));
    ImGui::Text("FPS: %.1f frames/s", ImGui::GetIO().Framerate);
    ImGui::Text("light position: (%.3f, %.3f, %.3f)", lightPos.x, lightPos.y, lightPos.z);
    ImGui::SliderFloat("light.x", &lightPos.x, -2.0f, 2.0f);
    ImGui::SliderFloat("light.y", &lightPos.y, 5.0f, 12.0f);
    ImGui::SliderFloat("light.z", &lightPos.z, -2.0f, 2.0f);
    if (ImGui::Button("robot control")) {
        showRobotControl = true;
        controlling = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("ball control")) {
        ballControl = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("reflect/refract")) {
        reflectSwitch ^= 1;
    }
    ImGui::Text("Control Guide");
    ImGui::Text("F2 to swtich mode");
    ImGui::Text("W/A/S/D to control camera in free mode");
    ImGui::Text("arrow keys to control the robot");
    ImGui::Text("T key to capture a screenshot");
    ImGui::Text("F11 to switch on/off full screen");
    ImGui::Text("Q key to quit");
    ImGui::End();
    
    if (showRobotControl) {
        ImGui::Begin("robot control panel");
        ImGui::SetWindowSize(ImVec2(500.0f, 200.0f));
        ImGui::SliderFloat("leftUpperArmRotation", &leftUpperArmRotationAngle, 0.0f, 360.0f);
        ImGui::SliderFloat("rightUpperArmRotation", &rightUpperArmRotationAngle, 0.0f, 360.0f);
        ImGui::SliderFloat("leftLowerArmRotation", &leftLowerArmRotationAngle, 0.0f, 360.0f);
        ImGui::SliderFloat("rightLowerArmRotation", &rightLowerArmRotationAngle, 0.0f, 360.0f);
        ImGui::SliderFloat("headRotation", &headRotationAngle, 0.0f, 360.0f);
        ImGui::SliderFloat("axeRotation", &axeRotationAngle, 0.0f, 360.0f);
        if (ImGui::Button("close")) {
            showRobotControl = false;
            controlling = false;
        }
        ImGui::End();
    }

    if (ballControl) {
        ImGui::Begin("ball control panel");
        ImGui::SetWindowSize(ImVec2(500.0f, 200.0f));
        ImGui::SliderFloat("ballSize.x", &ballSize.x, 0.05f, 1.0f);
        ImGui::SliderFloat("ballSize.y", &ballSize.y, 0.05f, 1.0f);
        ImGui::SliderFloat("ballSize.z", &ballSize.z, 0.05f, 1.0f);
        if (ImGui::Button("close")) {
            ballControl = false;
        }
        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiCleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
    Model* ball = models[1];
    Model* woodenBox = models[2];
    Model* tree = models[3];
    Model* house = models[4];

    LayerModel* body = layerModels[0];
    LayerModel* head = layerModels[1];
    LayerModel* leftLowerArm = layerModels[2];
    LayerModel* leftUpperArm = layerModels[3];
    LayerModel* rightLowerArm = layerModels[4];
    LayerModel* rightUpperArm = layerModels[5];
    LayerModel* leftUpperLeg = layerModels[6];
    LayerModel* rightUpperLeg = layerModels[7];
    LayerModel* axe = layerModels[8];
    
    shadowShader->use();

    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(i * 2.0f, -0.4f, j * 2.0f));
            model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));
            shadowShader->setMat4("model", model);
            plane->draw(*shadowShader);
        }
    }

    glm::vec3 bodyPosition = body->getPosition();
    body->setPositionMatrix(glm::vec3(bodyPosition.x + rightStep * robotRight.x + forwardStep * robotFront.x, 0.8f, bodyPosition.z + rightStep * robotRight.z + forwardStep * robotFront.z));
    if (!freeMode) {
        body->setRotationMatrix(glm::radians(camera.yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    float eps = 1e-2;
    if (isWalking) {
        waving = sin(glfwGetTime() * 6) * 0.5f;
        leftUpperLeg->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperLeg->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        leftUpperArm->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperArm->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
    } else if (!controlling) {
        if (fabsf(waving) > eps) {
            if (waving > eps) {
                waving -= eps;
            } else {
                waving += eps;
            }
        } else {
            waving = 0.0f;
        }
        leftUpperLeg->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperLeg->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        leftUpperArm->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperArm->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    if (controlling) {
        leftUpperArm->setRotationMatrix(glm::radians(leftUpperArmRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperArm->setRotationMatrix(glm::radians(rightUpperArmRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        leftLowerArm->setRotationMatrix(glm::radians(leftLowerArmRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        rightLowerArm->setRotationMatrix(glm::radians(rightLowerArmRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        head->setRotationMatrix(glm::radians(headRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        axe->setRotationMatrix(glm::radians(axeRotationAngle - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    body->draw(*shadowShader);

    glm::mat4 ballModel(1.0f);
    ballModel = glm::translate(ballModel, glm::vec3(2.0f, 0.3f, 2.0f));
    ballModel = glm::scale(ballModel, glm::vec3(ballSize.x, ballSize.y, ballSize.z));
    shadowShader->setMat4("model", ballModel);
    ball->draw(*shadowShader);

    glm::mat4 boxModel(1.0f);
    boxModel = glm::translate(boxModel, glm::vec3(2.0f, 0.0f, 4.0f));
    boxModel = glm::scale(boxModel, glm::vec3(1.0f, 1.0f, 1.0f));
    shadowShader->setMat4("model", boxModel);
    woodenBox->draw(*shadowShader);

    boxModel = glm::mat4(1.0f);
    boxModel = glm::translate(boxModel, glm::vec3(2.0f, 0.0f, 5.3f));
    boxModel = glm::rotate(boxModel, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shadowShader->setMat4("model", boxModel);
    woodenBox->draw(*shadowShader);

    boxModel = glm::mat4(1.0f);
    boxModel = glm::translate(boxModel, glm::vec3(3.3f, 0.0f, 4.8f));
    boxModel = glm::rotate(boxModel, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shadowShader->setMat4("model", boxModel);
    woodenBox->draw(*shadowShader);

    for (int i = 0; i < 3; i++) {
        glm::mat4 treeModel(1.0f);
        treeModel = glm::translate(treeModel, glm::vec3(-7.0f - 0.3f * i, -0.5f, -4.0f + 4.4f * i));
        treeModel = glm::scale(treeModel, glm::vec3(2.0f, 2.0f, 2.0f));
        shadowShader->setMat4("model", treeModel);
        tree->draw(*shadowShader);
    }

    for (int i = 0; i < 2; i++) {
        glm::mat4 treeModel(1.0f);
        treeModel = glm::translate(treeModel, glm::vec3(7.0f + 0.3f * i, -0.5f, -4.0f + 4.4f * i));
        treeModel = glm::scale(treeModel, glm::vec3(2.0f, 2.0f, 2.0f));
        shadowShader->setMat4("model", treeModel);
        tree->draw(*shadowShader);
    }

    glm::mat4 houseModel = glm::mat4(1.0f);
    houseModel = glm::translate(houseModel, glm::vec3(-1.0f, 6.5f, -5.0f));
    houseModel = glm::scale(houseModel, glm::vec3(7.0f, 7.0f, 7.0f));
    shadowShader->setMat4("model", houseModel);
    house->draw(*shadowShader);
}



void renderScene(vector<LayerModel*> layerModels, vector<Model*> models, vector<Shader*> shaders)
{
    Shader* envShader = shaders[0];
    Shader* robotShader = shaders[1];
    Shader* ballShader = shaders[2];
    Shader* boxShader = shaders[3];

    Model* plane = models[0];
    Model* ball = models[1];
    Model* woodenBox = models[2];
    Model* tree = models[3];
    Model* house = models[4];

    LayerModel* body = layerModels[0];
    LayerModel* head = layerModels[1];
    LayerModel* leftLowerArm = layerModels[2];
    LayerModel* leftUpperArm = layerModels[3];
    LayerModel* rightLowerArm = layerModels[4];
    LayerModel* rightUpperArm = layerModels[5];
    LayerModel* leftUpperLeg = layerModels[6];
    LayerModel* rightUpperLeg = layerModels[7];
    LayerModel* axe = layerModels[8];

    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
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
            model = glm::translate(model, glm::vec3(i * 2.0f, -0.4f, j * 2.0f));
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

    if (!freeMode) {
        glm::vec3 robotHeadPosition = head->getPosition();
        glm::vec3 cameraPos = robotHeadPosition;
        cameraPos += robotFront;
        camera.setEyePosition(cameraPos);
    }

    glm::vec3 bodyPosition = body->getPosition();
    body->setPositionMatrix(glm::vec3(bodyPosition.x + rightStep * robotRight.x + forwardStep * robotFront.x, 0.8f, bodyPosition.z + rightStep * robotRight.z + forwardStep * robotFront.z));
    if (!freeMode) {
        body->setRotationMatrix(glm::radians(camera.yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    float eps = 1e-2;
    if (isWalking) {
        waving = sin(glfwGetTime() * 6) * 0.5f;
        leftUpperLeg->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperLeg->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        leftUpperArm->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperArm->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
    } else if(!controlling) {
        if (fabsf(waving) > eps) {
            if (waving > eps) {
                waving -= eps;
            } else {
                waving += eps;
            }
        } else {
            waving = 0.0f;
        }
        leftUpperLeg->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperLeg->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        leftUpperArm->setRotationMatrix(-waving, glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperArm->setRotationMatrix(waving, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    if (controlling) {
        leftUpperArm->setRotationMatrix(glm::radians(leftUpperArmRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        rightUpperArm->setRotationMatrix(glm::radians(rightUpperArmRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        leftLowerArm->setRotationMatrix(glm::radians(leftLowerArmRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        rightLowerArm->setRotationMatrix(glm::radians(rightLowerArmRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        head->setRotationMatrix(glm::radians(headRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        axe->setRotationMatrix(glm::radians(axeRotationAngle - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    body->draw(*robotShader);

    robotShader->setFloat("material.shininess", 0.25f * 128.0f);
    robotShader->setVec3("material.specular", 0.45, 0.55f, 0.45f);
    for (int i = 0; i < 3; i++) {
        glm::mat4 treeModel(1.0f);
        treeModel = glm::translate(treeModel, glm::vec3(-7.0f - 0.3f * i, -0.5f, -4.0f + 4.4f * i));
        treeModel = glm::scale(treeModel, glm::vec3(2.0f, 2.0f, 2.0f));
        robotShader->setMat4("model", treeModel);
        tree->draw(*robotShader);
    }

    for (int i = 0; i < 2; i++) {
        glm::mat4 treeModel(1.0f);
        treeModel = glm::translate(treeModel, glm::vec3(6.0f + 0.3f * i, -0.5f, -4.0f + 4.4f * i));
        treeModel = glm::scale(treeModel, glm::vec3(2.0f, 2.0f, 2.0f));
        robotShader->setMat4("model", treeModel);
        tree->draw(*robotShader);
    }

    ballShader->use();
    ballShader->setInt("reflectSwitch", reflectSwitch);
    glm::mat4 ballModel(1.0f);
    ballModel = glm::translate(ballModel, glm::vec3(2.0f, 0.3f, 2.0f));
    ballModel = glm::scale(ballModel, glm::vec3(ballSize.x, ballSize.y, ballSize.z));
    ballShader->setMat4("model", ballModel);
    ballShader->setMat4("projection", projection);
    ballShader->setMat4("view", view);
    ballShader->setVec3("viewPos", camera.eye);
    ballShader->setVec3("lightPos", lightPos);
    ball->draw(*ballShader);

    glm::mat4 boxModel(1.0f);
    boxModel = glm::translate(boxModel, glm::vec3(2.0f, 0.0f, 4.0f));
    boxShader->use();
    boxShader->setMat4("projection", projection);
    boxShader->setMat4("view", view);
    boxShader->setMat4("model", boxModel);
    boxShader->setVec3("lightPos", lightPos);
    boxShader->setVec3("viewPos", camera.eye);
    woodenBox->draw(*boxShader);

    boxModel = glm::mat4(1.0f);
    boxModel = glm::translate(boxModel, glm::vec3(2.0f, 0.0f, 5.3f));
    boxModel = glm::rotate(boxModel, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    boxShader->setMat4("model", boxModel);
    woodenBox->draw(*boxShader);

    boxModel = glm::mat4(1.0f);
    boxModel = glm::translate(boxModel, glm::vec3(3.3f, 0.0f, 4.8f));
    boxModel = glm::rotate(boxModel, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    boxShader->setMat4("model", boxModel);
    woodenBox->draw(*boxShader);

    glm::mat4 houseModel = glm::mat4(1.0f);
    houseModel = glm::translate(houseModel, glm::vec3(-1.0f, 6.5f, -5.0f));
    houseModel = glm::scale(houseModel, glm::vec3(7.0f, 7.0f, 7.0f));
    boxShader->setMat4("model", houseModel);
    house->draw(*boxShader);

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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "2018151004-liyuliang-Wondering", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create a GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    glfwSetScrollCallback(window, scrollCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    Shader envShader("shader/env_vshader.glsl", "shader/env_fshader.glsl");
    Shader robotShader("shader/robot_vshader.glsl", "shader/robot_fshader.glsl");
    //Shader shadowShader("shader/shadow_vshader.glsl", "shader/shadow_fshader.glsl");
    Shader depthShader("shader/depth_vshader.glsl", "shader/depth_fshader.glsl");
    Shader debugShader("shader/debug_vshader.glsl", "shader/debug_fshader.glsl");
    Shader skyboxShader("shader/skybox_vshader.glsl", "shader/skybox_fshader.glsl");
    Shader ballShader("shader/ball_vshader.glsl", "shader/ball_fshader.glsl");
    Shader boxShader("shader/wooden_box_vshader.glsl", "shader/wooden_box_fshader.glsl");

    Model plane((char*)"model/Plane/Plane.obj");
    Model ball((char*)"model/ball/Ball.obj");
    Model woodenBox((char*)"model/woodenBox/Wooden_stuff.obj");
    Model tree((char*)"model/tree/tree_2.obj");
    Model house((char*)"model/house/house.obj");
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
                                   .setPositionMatrix(glm::vec3(0.02f, -0.05f, 0.0f));

    LayerModel& rightUpperLeg = body.addSon()
                                    .setNodeModel(Model((char*)"model/man/Leg/Leg.obj"))
                                    .setPositionMatrix(glm::vec3(-0.02f, -0.05f, 0.0f));

    LayerModel& axe = rightLowerArm.addSon()
                                   .setNodeModel(Model((char*)"model/man/Axe/axe.obj"))
                                   .setPositionMatrix(glm::vec3(0.01f, -0.02f, 0.1f))
                                   .setRotationMatrix(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
                                   .setScaleMatrix(glm::vec3(0.01f, 0.005f, 0.01f));


    glEnable(GL_DEPTH_TEST);
    
    // imgui setup
    imguiSetup(window);

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
    LayerModels.push_back(&axe);

    vector<Model*> Models;
    Models.push_back(&plane);
    Models.push_back(&ball);
    Models.push_back(&woodenBox);
    Models.push_back(&tree);
    Models.push_back(&house);

    vector<Shader*> Shaders;
    Shaders.emplace_back(&envShader);
    Shaders.emplace_back(&robotShader);
    Shaders.emplace_back(&ballShader);
    Shaders.emplace_back(&boxShader);
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
        "skybox/iceRiver/right.jpg",
        "skybox/iceRiver/left.jpg",
        "skybox/iceRiver/top.jpg",
        "skybox/iceRiver/bottom.jpg",
        "skybox/iceRiver/front.jpg",
        "skybox/iceRiver/back.jpg",
    };
    skybox.loadCubemap(facesName);
    skybox.setupSkybox(50.0f);

    ballShader.use();
    ballShader.setInt("skybox", 0);

    boxShader.use();
    boxShader.setInt("diffuseMap", 0);
    boxShader.setInt("normalMap", 1);

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

        imguiNewFrame();
        imguiRender();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    imguiCleanup();
    glfwTerminate();
    return 0;
}
