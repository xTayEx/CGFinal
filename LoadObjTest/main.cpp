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
float theta = 0.0f;
float deltaTime = 0.0f;
const double blockSize = 0.125;

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
    Shader shadowShader("shader/shadow_vshader.glsl", "shader/shadow_fshader.glsl");
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
    
    float lastTime = 0.0f;
    float curTime;
    int frameCnt = 0;
    float bodyRotationAngle = 0.0f;
    float armRotationAngle = 0.0f;
    float movement = 0.0f; 

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //Kapsule::openVideo(1280, 720, 300);
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        curTime = glfwGetTime();
        deltaTime = curTime - lastTime;
        lastTime = curTime;
        bodyRotationAngle = glfwGetTime();
        armRotationAngle = 1.5 * glfwGetTime();
        movement = 2.0 * sin(glfwGetTime());
        processKeyboardInput(window);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        
        // draw floor
        envShader.use();
        // configure environment shader
        envShader.setVec3("light.position", lightPos);
        envShader.setVec3("viewPos", camera.eye);

        envShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
        envShader.setVec3("light.diffuse", 0.9f, 0.9f, 0.9f);
        envShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        
        envShader.setFloat("material.shiness", 128.0f);
        envShader.setVec3("material.specular", 0.2f, 0.2f, 0.2f);
        envShader.setInt("material.diffuse", 0);

        envShader.setMat4("projection", projection);
        envShader.setMat4("view", view);

        for (int i = -10; i < 10; i++) {
            for (int j = -10; j < 10; j++) {
                glm::mat4 model(1.0f);
                model = glm::translate(model, glm::vec3(i * 2.0f, -0.5f, j * 2.0f));
                model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));
                envShader.setMat4("model", model);
                plane.draw(envShader);
            }
        }

        // draw robot
        robotShader.use();
        // configure robot shader
        robotShader.setVec3("light.position", lightPos);
        robotShader.setVec3("viewPos", camera.eye);

        robotShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
        robotShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        robotShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        
        robotShader.setFloat("material.shininess", 128.0f);
        robotShader.setVec3("material.specular", 0.727811, 0.626959f, 0.626959f);
        robotShader.setInt("material.diffuse", 0);

        robotShader.setMat4("projection", projection);
        robotShader.setMat4("view", view);

        body.setPositionMatrix(glm::vec3(0.0f, 1.2f, movement));
        rightUpperArm.setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        rightLowerArm.setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        leftUpperArm.setRotationMatrix(armRotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        leftLowerArm.setRotationMatrix(armRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        body.draw(robotShader);
        
        shadowShader.use();
        //DEBUG
        body.drawShadow(shadowShader, lightPos, view, projection, glm::mat4(1.0f));
        //DEBUG
   //     if (!Kapsule::isCompleteRecording()) {
            //Kapsule::saveVideoFrame(HEIGHT, WIDTH);
   //     } else {
   //         Kapsule::saveAsVideo();
   //     }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}