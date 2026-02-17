#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <ranges>
#include <miniaudio.h>
#include "core/AudioSystem.h"

#include "core/mesh.h"
#include "core/assimpLoader.h"
#include "core/texture.h"
#include "core/Camera.h"
#include "core/scene.h"

//#define MAC_CLION
#define VSTUDIO

#ifdef MAC_CLION
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif

#ifdef VSTUDIO
// Note: install imgui with:
//     ./vcpkg.exe install imgui[glfw-binding,opengl3-binding]
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

int g_width = 800;
int g_height = 600;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

glm::vec3 CameraMovement(GLFWwindow *window) {
    // Camera movement
    glm::vec3 input = glm::vec3(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        input.x = -1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        input.x = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        input.z = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        input.z = -1;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        input.y = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        input.y = -1;
    }

    return input;
}
glm::vec2 lastMousePosition = glm::vec2(0.0f, 0.0f);
glm::vec2 CameraRotation(GLFWwindow *window) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 newMousePosition = glm::vec2(xpos, ypos);
    glm::vec2 diff = glm::vec2(newMousePosition.x - lastMousePosition.x, newMousePosition.y - lastMousePosition.y);
    lastMousePosition = newMousePosition;

    return diff;
}

void framebufferSizeCallback(GLFWwindow *window,
                             int width, int height) {
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

std::string readFileToString(const std::string &filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        printf("Could not open file: %s\n", filePath.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

GLuint generateShader(const std::string &shaderPath, GLuint shaderType) {
    printf("Loading shader: %s\n", shaderPath.c_str());
    const std::string shaderText = readFileToString(shaderPath);
    const GLuint shader = glCreateShader(shaderType);
    const char *s_str = shaderText.c_str();
    glShaderSource(shader, 1, &s_str, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Error! Shader issue [%s]: %s\n", shaderPath.c_str(), infoLog);
    }
    return shader;
}

//framebuffers
unsigned int sceneFBO;
unsigned int sceneColor;
unsigned int sceneRBO;
unsigned int postFBO;
unsigned int postColor;

int width = g_width;
int height = g_height;
int oldWidth = g_width;
int oldHeight = g_width;

void DestroyFBOs() {
    // cleanup framebuffer resources
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneColor);
    glDeleteRenderbuffers(1, &sceneRBO);

    glDeleteFramebuffers(1, &postFBO);
    glDeleteTextures(1, &postColor);
}

void CreateFBOs() {
    width = g_width;
    height = g_height;

    // Scene framebuffer
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    glGenTextures(1, &sceneColor);
    glBindTexture(GL_TEXTURE_2D, sceneColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, sceneColor, 0);

    // Depth + stencil
    glGenRenderbuffers(1, &sceneRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                          width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, sceneRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR:: Scene framebuffer incomplete\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Post-process framebuffer
    glGenFramebuffers(1, &postFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

    glGenTextures(1, &postColor);
    glBindTexture(GL_TEXTURE_2D, postColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, postColor, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR:: Post-process framebuffer incomplete\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(g_width, g_height, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    //Setup platforms
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    const GLuint modelVertexShader = generateShader("shaders/modelVertex.vs", GL_VERTEX_SHADER);
    const GLuint fragmentShader = generateShader("shaders/fragment.fs", GL_FRAGMENT_SHADER);
    const GLuint textureShader = generateShader("shaders/texture.fs", GL_FRAGMENT_SHADER);
    const GLuint lightFragmentShader = generateShader("shaders/lightFragment.fs", GL_FRAGMENT_SHADER);
    const GLuint lightVertexShader = generateShader("shaders/lightVertex.vs", GL_VERTEX_SHADER);

    int success;
    char infoLog[512];
    const unsigned int modelShaderProgram = glCreateProgram();
    const unsigned int textureShaderProgram = glCreateProgram();
    const unsigned int lightShaderProgram = glCreateProgram();
    // shader loading happens here:
    {
        glAttachShader(modelShaderProgram, modelVertexShader);
        glAttachShader(modelShaderProgram, fragmentShader);
        glLinkProgram(modelShaderProgram);
        glGetProgramiv(modelShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(modelShaderProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(textureShaderProgram, modelVertexShader);
        glAttachShader(textureShaderProgram, textureShader);
        glLinkProgram(textureShaderProgram);
        glGetProgramiv(textureShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(textureShaderProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(lightShaderProgram, lightVertexShader);
        glAttachShader(lightShaderProgram, lightFragmentShader);
        glLinkProgram(lightShaderProgram);
        glGetProgramiv(lightShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(lightShaderProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }
    }

    GLint lightModelLoc = glGetUniformLocation(lightShaderProgram, "model");
    GLint lightViewLoc = glGetUniformLocation(lightShaderProgram, "view");
    GLint lightProjLoc = glGetUniformLocation(lightShaderProgram, "projection");
    GLint tex_mvp_loc = glGetUniformLocation(textureShaderProgram, "mvpMatrix");
    GLint tex_sampler_loc = glGetUniformLocation(textureShaderProgram, "text");

    glDeleteShader(modelVertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(textureShader);
    glDeleteShader(lightVertexShader);
    glDeleteShader(lightFragmentShader);

    core::Mesh screenQuadMesh = core::Mesh::generateScreenQuad();

    core::Model suzanne = core::AssimpLoader::loadModel("models/nonormalmonkey.obj");
    suzanne.SetBaseColor(glm::vec3(0.65f, 0.4f, 0.0f));
    suzanne.SetName("Suzanne");
    core::Model suzanne2 = core::AssimpLoader::loadModel("models/nonormalmonkey.obj");
    suzanne2.SetBaseColor(glm::vec3(1, 1, 0));
    suzanne2.SetName("Suzanne");
    core::Texture texture("textures/object_65_baseColor.png");

    glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glClearColor(clearColor.r,
                 clearColor.g, clearColor.b, clearColor.a);

    //VP
    glm::mat4 view;
    glm::mat4 projection;

    GLint mvpMatrixUniform = glGetUniformLocation(modelShaderProgram, "mvpMatrix");
    GLint modelMatrixUniform = glGetUniformLocation(modelShaderProgram, "modelMatrix");
    GLint textureModelUniform = glGetUniformLocation(textureShaderProgram, "mvpMatrix");
    GLint texture0Uniform = glGetUniformLocation(textureShaderProgram, "text");
    GLint texture1Uniform = glGetUniformLocation(textureShaderProgram, "text");

    double currentTime = glfwGetTime();
    double finishFrameTime = 0.0;
    float deltaTime = 0.0f;
    float rotationStrength = 5.0f;

    // very basic scenes:
    std::vector<Scene*> sceneList;

    Scene* scene1 = new Scene("Basic Scene");
    scene1->AddObject(&suzanne);
    sceneList.push_back(scene1);

    Scene* currentScene = sceneList[0];
    Camera* camera = currentScene->GetCamera();

    // IMGui:
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(2.0f, 1.0f, 2.0f);
    glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
    float lightStrength = 5.0f;
    float ambientStrength = 0.1f;
    float specularStrength = 0.5f;
    float cameraSpeed = 2.0f;

    CreateFBOs();

    while (!glfwWindowShouldClose(window)) {
        width = g_width;
        height = g_height;
        if (width != oldWidth || height != oldHeight) {
            DestroyFBOs();
            CreateFBOs();
            oldWidth = g_width;
            oldHeight = g_height;
            printf("Resize!\n");
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Raw Engine v2");

        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat3("Light color", &lightColor[0],0,1);
            ImGui::SliderFloat("Light Strength", &lightStrength, 0, 10);
            ImGui::SliderFloat3("Light Position", &lightPos[0],-10, 10);
            ImGui::SliderFloat3("Ambient Color", &ambientColor[0],0,1);
            ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0, 1);
            ImGui::SliderFloat("Specular Strength", &specularStrength, 0, 1);
        }

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0, 10);
        }
        ImGui::End();

        ImGui::Begin("Models");
        int i = 0;
        for (core::Model* model : currentScene->GetObjects()) {
            std::string name = model->GetName();
            if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 oldPos = model->GetTranslation();
                glm::vec3 pos = oldPos;

                ImGui::PushID(i);
                ImGui::InputFloat3("Position", glm::value_ptr(pos));
                ImGui::PopID();

                if (oldPos != pos) {
                    model->translate(pos - oldPos);
                }
            }
            i++;
        }
        ImGui::End();

        processInput(window);

        glUseProgram(lightShaderProgram);

        glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(projection));


        if (currentScene->GetSceneName() == "Basic Scene") suzanne.rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rotationStrength * 100) * static_cast<float>(deltaTime));

        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Scene stuff
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(texture1Uniform, 1);
        glBindTexture(GL_TEXTURE_2D, texture.getId());

        glUseProgram(modelShaderProgram);
        for (core::Model* model : currentScene->GetObjects())
        {
            unsigned int currentShader = 0;
            if (model == &suzanne || model == &suzanne2) {
                glUseProgram(lightShaderProgram);
                currentShader = lightShaderProgram;
                glm::vec3 cameraPos = camera->GetPosition();  // from your camera

                GLint lightPosLoc   = glGetUniformLocation(lightShaderProgram, "lightPos");
                GLint lightColorLoc = glGetUniformLocation(lightShaderProgram, "lightColor");
                GLint cameraPosLoc  = glGetUniformLocation(lightShaderProgram, "cameraPos");
                GLint lightStrengthLoc = glGetUniformLocation(lightShaderProgram, "lightStrength");
                GLint ambientLightColorLoc = glGetUniformLocation(lightShaderProgram, "ambientLightColor");
                GLint ambientLightStrengthLoc = glGetUniformLocation(lightShaderProgram, "ambientLightStrength");
                GLint specularStrengthLocation = glGetUniformLocation(lightShaderProgram, "specularStrength");

                glUniform3fv(lightPosLoc,   1, glm::value_ptr(lightPos));
                glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
                glUniform3fv(cameraPosLoc,  1, glm::value_ptr(cameraPos));
                glUniform3fv(ambientLightColorLoc, 1, glm::value_ptr(ambientColor));
                glUniform1f(lightStrengthLoc, lightStrength);
                glUniform1f(ambientLightStrengthLoc, ambientStrength);
                glUniform1f(specularStrengthLocation, specularStrength);

                glUniform3f(glGetUniformLocation(lightShaderProgram, "baseColor"), model->GetBaseColor().x, model->GetBaseColor().y, model->GetBaseColor().z);
            }
            else {
                glUseProgram(modelShaderProgram);
                currentShader = modelShaderProgram;
            }

            glm::mat4 modelMatrix = model->getModelMatrix();

            if (currentShader == lightShaderProgram)
            {
                glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
                glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
            }
            else if (currentShader == modelShaderProgram)
            {
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection * view * model->getModelMatrix()));
                glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(model->getModelMatrix()));
            }
            else if (currentShader == textureShaderProgram)
            {
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection * view * model->getModelMatrix()));
            }
            model->render();
        }

        glEnable(GL_BLEND);
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postFBO);
            glBlitFramebuffer(0,0,g_width,g_height,0,0,g_width,g_height,GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // screen
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(textureShaderProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, postColor);
            glUniform1i(glGetUniformLocation(textureShaderProgram, "text"), 0);

            screenQuadMesh.render();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        finishFrameTime = glfwGetTime();
        deltaTime = static_cast<float>(finishFrameTime - currentTime);
        currentTime = finishFrameTime;

        //Camera Control
        float speed = deltaTime * cameraSpeed;
        glm::vec3 movement = CameraMovement(window);
        movement = glm::vec3(movement.x, movement.y, movement.z);
        camera->MoveCamera(movement * speed);

        glm::vec2 rotation = CameraRotation(window);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
            camera->RotateCamera(glm::vec2(rotation.x, -rotation.y));
        }

        //VP
        view = camera->GetViewMatrix();
        projection = camera->GetProjectionMatrix(static_cast<float>(g_width), static_cast<float>(g_height));
    }

    for (Scene* scene : sceneList) {
        delete scene;
    }

    // cleanup shaders
    glDeleteProgram(textureShaderProgram);
    glDeleteProgram(modelShaderProgram);
    glDeleteProgram(lightShaderProgram);

    // cleanup framebuffer resources
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneColor);
    glDeleteRenderbuffers(1, &sceneRBO);

    glDeleteFramebuffers(1, &postFBO);
    glDeleteTextures(1, &postColor);

    // cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // terminate
    glfwTerminate();
    return 0;
}