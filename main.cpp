#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

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
int pressed = 0;
int SceneInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (pressed == 0) {
            pressed = 1;
            return 1;
        }
    }

    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) pressed = 0;

    return 0;
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

unsigned int postFBO2;
unsigned int postColor2;

unsigned int postFBO3;
unsigned int postColor3;

// Bloom
unsigned int bloomFBO1;
unsigned int bloomColor;

unsigned int blurFBO[2];
unsigned int blurColor[2];

int width = g_width;
int height = g_height;

void DestroyFBOs() {
    // cleanup framebuffer resources
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneColor);
    glDeleteRenderbuffers(1, &sceneRBO);

    glDeleteFramebuffers(1, &bloomFBO1);
    glDeleteTextures(1, &bloomColor);

    glDeleteFramebuffers(2, blurFBO);
    glDeleteTextures(2, blurColor);

    glDeleteFramebuffers(1, &postFBO);
    glDeleteTextures(1, &postColor);

    glDeleteFramebuffers(1, &postFBO2);
    glDeleteTextures(1, &postColor2);

    glDeleteFramebuffers(1, &postFBO3);
    glDeleteTextures(1, &postColor3);
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

    // Bright-pass framebuffer
    glGenFramebuffers(1, &bloomFBO1);
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO1);

    glGenTextures(1, &bloomColor);
    glBindTexture(GL_TEXTURE_2D, bloomColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, bloomColor, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR:: Bloom framebuffer incomplete\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blur ping-pong buffers
    glGenFramebuffers(2, blurFBO);
    glGenTextures(2, blurColor);

    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);

        glBindTexture(GL_TEXTURE_2D, blurColor[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, blurColor[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR:: Blur framebuffer " << i << " incomplete\n";
    }

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

    glGenFramebuffers(1, &postFBO2);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO2);

    glGenTextures(1, &postColor2);
    glBindTexture(GL_TEXTURE_2D, postColor2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postColor2, 0);

    glGenFramebuffers(1, &postFBO3);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO3);

    glGenTextures(1, &postColor3);
    glBindTexture(GL_TEXTURE_2D, postColor3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postColor3, 0);

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
    const GLuint invertFragmentShader = generateShader("shaders/ColorInversion.fs", GL_FRAGMENT_SHADER);
    const GLuint screenQuadVertexShader = generateShader("shaders/ScreenQuad.vs", GL_VERTEX_SHADER);
    const GLuint vertexShader = generateShader("shaders/vertex.vs", GL_VERTEX_SHADER);
    const GLuint pixelFragmentShader = generateShader("shaders/pixelFragment.fs", GL_FRAGMENT_SHADER);
    const GLuint bloomExtractShader = generateShader("shaders/bloomExtract.fs", GL_FRAGMENT_SHADER);
    const GLuint blurShader = generateShader("shaders/blur.fs", GL_FRAGMENT_SHADER);
    const GLuint bloomCombineShader = generateShader("shaders/bloomCombine.fs", GL_FRAGMENT_SHADER);
    const GLuint hueShiftShader = generateShader("shaders/hueShift.fs", GL_FRAGMENT_SHADER);

    int success;
    char infoLog[512];
    const unsigned int modelShaderProgram = glCreateProgram();
    const unsigned int textureShaderProgram = glCreateProgram();
    const unsigned int lightShaderProgram = glCreateProgram();
    const unsigned int invertProgram = glCreateProgram();
    const unsigned int pixelateProgram = glCreateProgram();
    const unsigned int bloomExtractProgram = glCreateProgram();
    const unsigned int blurProgram = glCreateProgram();
    const unsigned int bloomCombineProgram = glCreateProgram();
    const unsigned int hueShiftProgram = glCreateProgram();
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

        glAttachShader(invertProgram, vertexShader);
        glAttachShader(invertProgram, invertFragmentShader);
        glLinkProgram(invertProgram);
        glGetProgramiv(invertProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(invertProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(pixelateProgram, vertexShader);
        glAttachShader(pixelateProgram, pixelFragmentShader);
        glLinkProgram(pixelateProgram);
        glGetProgramiv(pixelateProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(pixelateProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(bloomExtractProgram, vertexShader);
        glAttachShader(bloomExtractProgram, bloomExtractShader);
        glLinkProgram(bloomExtractProgram);
        glGetProgramiv(bloomExtractProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(bloomExtractProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(blurProgram, vertexShader);
        glAttachShader(blurProgram, blurShader);
        glLinkProgram(blurProgram);
        glGetProgramiv(blurProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(blurProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(bloomCombineProgram, vertexShader);
        glAttachShader(bloomCombineProgram, bloomCombineShader);
        glLinkProgram(bloomCombineProgram);
        glGetProgramiv(bloomCombineProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(bloomCombineProgram, 512, NULL, infoLog);
            printf("Error! Making Shader Program: %s\n", infoLog);
        }

        glAttachShader(hueShiftProgram, vertexShader);
        glAttachShader(hueShiftProgram, hueShiftShader);
        glLinkProgram(hueShiftProgram);
        glGetProgramiv(hueShiftProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(hueShiftProgram, 512, NULL, infoLog);
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
    glDeleteShader(invertFragmentShader);
    glDeleteShader(screenQuadVertexShader);
    glDeleteShader(vertexShader);
    glDeleteShader(pixelFragmentShader);
    glDeleteShader(bloomExtractShader);
    glDeleteShader(bloomCombineShader);
    glDeleteShader(blurShader);
    glDeleteShader(hueShiftShader);

    core::Mesh quad = core::Mesh::generateQuad();
    core::Model quadModel({quad});
    quadModel.translate(glm::vec3(0,0,-2.5));
    quadModel.scale(glm::vec3(5, 5, 1));

    core::Mesh screenQuadMesh = core::Mesh::generateScreenQuad();

    core::Model suzanne = core::AssimpLoader::loadModel("models/nonormalmonkey.obj");
    core::Model suzanne2 = core::AssimpLoader::loadModel("models/nonormalmonkey.obj");
    core::Model superleggera = core::AssimpLoader::loadModel("models/superleggera.gltf");
    superleggera.translate(glm::vec3(-3, 2, 0));
    superleggera.scale(glm::vec3(3, 3, 3));
    core::Texture cmgtGatoTexture("textures/CMGaTo_crop.png");
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

    Scene* scene2 = new Scene("Motorcycle Scene");
    scene2->AddObject(&suzanne2);
    scene2->AddObject(&superleggera);
    sceneList.push_back(scene2);

    int sceneIndex = 0;
    Scene* currentScene = sceneList[sceneIndex];
    Camera* camera = currentScene->GetCamera();

    // IMGui:
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(3.0f, 4.0f, 2.0f);
    glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
    float lightStrength = 5.0f;
    float ambientStrength = 0.1f;
    float specularStrength = 0.5f;
    float cameraSpeed = 2.0f;
    bool invertEffect = false;
    bool pixelateEffect = false;
    int pixelSize = 10;
    bool bloom = false;
    float bloomThreshold = 1.0f;
    float bloomStrength = 0.8f;
    float blurRadius = 1.0f;
    float hueShift = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        DestroyFBOs();
        CreateFBOs();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Raw Engine v2");
        //ImGui::Text("Hello :)");
        ImGui::SliderFloat3("Light color", &lightColor[0],0,1);
        ImGui::SliderFloat("Light Strength", &lightStrength, 0, 10);
        ImGui::SliderFloat3("Light Position", &lightPos[0],-10, 10);
        ImGui::SliderFloat3("Ambient Color", &ambientColor[0],0,1);
        ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0, 1);
        ImGui::SliderFloat("Specular Strength", &specularStrength, 0, 1);
        ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0, 10);
        ImGui::Checkbox("Invert Colors", &invertEffect);
        ImGui::Checkbox("Pixelate", &pixelateEffect);
        ImGui::SliderInt("PixelSize", &pixelSize, 1, 100);
        ImGui::Checkbox("Bloom", &bloom);
        ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 2.0f);
        ImGui::SliderFloat("Bloom Strength", &bloomStrength, 0.0f, 2.0f);
        ImGui::SliderFloat("Blur Radius", &blurRadius, 1.0f, 100.0f);
        ImGui::SliderFloat("Hue Shift", &hueShift, 0.0f, 360.0f);
        ImGui::End();

        processInput(window);

        int shouldSwitch = SceneInput(window);
        if (shouldSwitch == 1) {
            sceneIndex++;
            if (sceneIndex >= 2) sceneIndex = 0;

            currentScene = sceneList[sceneIndex];
            camera = currentScene->GetCamera();
            view = camera->GetViewMatrix();
            projection = camera->GetProjectionMatrix(g_width, g_height);
        }

        glUseProgram(lightShaderProgram);

        glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

        if (currentScene->GetSceneName() == "Basic Scene") suzanne.rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rotationStrength) * static_cast<float>(deltaTime));

        //glBindTexture(GL_TEXTURE_2D, texture.getId());

        {
            /*
            glUseProgram(textureShaderProgram);
            glUniformMatrix4fv(textureModelUniform, 1, GL_FALSE, glm::value_ptr(projection * view * quadModel.getModelMatrix()));
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texture0Uniform, 0);
            glBindTexture(GL_TEXTURE_2D, cmgtGatoTexture.getId());

            quadModel.render();
            glBindVertexArray(0);
            */
        }

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

            if (model == &superleggera)
            {
                glUseProgram(textureShaderProgram);
                currentShader = textureShaderProgram;

                glm::mat4 mvp = projection * view * model->getModelMatrix();
                glUniformMatrix4fv(tex_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture.getId());
                glUniform1i(tex_sampler_loc, 0);

                model->render();
                continue;
            }
            else if (model == &suzanne || model == &suzanne2) {
                glUseProgram(lightShaderProgram);
                currentShader = lightShaderProgram;
                //glm::vec3 lightPos = glm::vec3(3.0f, 4.0f, 2.0f);   // example
                //glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // white
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
            }
            else {
                glUseProgram(modelShaderProgram);
                currentShader = modelShaderProgram;
            }

            glm::mat4 modelMatrix = model->getModelMatrix();

            if (currentShader == lightShaderProgram)
            {
                glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
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

        // Bloom
        if (bloom) {
            // Bright Extraction
            // Bright-pass extraction
            glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO1);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(bloomExtractProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneColor);
            glUniform1i(glGetUniformLocation(bloomExtractProgram, "scene"), 0);
            glUniform1f(glGetUniformLocation(bloomExtractProgram, "threshold"), bloomThreshold);
            screenQuadMesh.render();

            // Blur ping-pong
            bool horizontal = true;
            bool first = true;
            int blurPasses = 10;

            GLint horizontalLoc = glGetUniformLocation(blurProgram, "horizontal");
            GLint blurRadiusLoc = glGetUniformLocation(blurProgram, "blurRadius");

            for (int i = 0; i < blurPasses; i++) {
                glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
                glUseProgram(blurProgram);
                glUniform1i(horizontalLoc, horizontal);
                glUniform1f(blurRadiusLoc, 1.0f); // adjust for stronger blur

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, first ? bloomColor : blurColor[!horizontal]);
                glUniform1i(glGetUniformLocation(blurProgram, "image"), 0);

                screenQuadMesh.render();

                horizontal = !horizontal;
                if (first) first = false;
            }

            // Combine scene + bloom
            glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(bloomCombineProgram);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneColor);
            glUniform1i(glGetUniformLocation(bloomCombineProgram, "scene"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, blurColor[!horizontal]); // last blurred result
            glUniform1i(glGetUniformLocation(bloomCombineProgram, "bloomBlur"), 1);
            glUniform1f(glGetUniformLocation(bloomCombineProgram, "bloomStrength"), bloomStrength);

            screenQuadMesh.render();
        }
        else {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postFBO);
            glBlitFramebuffer(0,0,g_width,g_height,0,0,g_width,g_height,GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // Invert Colors
        {
            glBindFramebuffer(GL_FRAMEBUFFER, postFBO2); // write to new FBO
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(invertProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, postColor); // read old postColor
            glUniform1i(glGetUniformLocation(invertProgram, "screenTexture"), 0);
            glUniform1i(glGetUniformLocation(invertProgram, "invertColors"), invertEffect ? 1 : 0);

            screenQuadMesh.render();
        }

        // Pixelate
        {
            glBindFramebuffer(GL_FRAMEBUFFER, postFBO3); // write to new FBO
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(pixelateProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, postColor2); // read from invert result
            glUniform1i(glGetUniformLocation(pixelateProgram, "screenTexture"), 0);
            glUniform1i(glGetUniformLocation(pixelateProgram, "pixelate"), pixelateEffect ? 1 : 0);
            glUniform1i(glGetUniformLocation(pixelateProgram, "pixelSize"), pixelSize);
            glUniform2f(glGetUniformLocation(pixelateProgram, "screenSize"), g_width, g_height);

            screenQuadMesh.render();
        }

        // Hue Shift
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // screen
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(hueShiftProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, postColor3); // read from Pixelate result
            glUniform1i(glGetUniformLocation(hueShiftProgram, "text"), 0);
            glUniform1f(glGetUniformLocation(hueShiftProgram, "shift"), hueShift * glm::pi<float>() /  180.0f);

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
    glDeleteProgram(lightShaderProgram);
    glDeleteProgram(textureShaderProgram);
    glDeleteProgram(modelShaderProgram);
    glDeleteProgram(invertProgram);
    glDeleteProgram(pixelateProgram);
    glDeleteProgram(bloomExtractProgram);
    glDeleteProgram(blurProgram);
    glDeleteProgram(bloomCombineProgram);

    // cleanup framebuffer resources
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneColor);
    glDeleteRenderbuffers(1, &sceneRBO);

    glDeleteFramebuffers(1, &bloomFBO1);
    glDeleteTextures(1, &bloomColor);

    glDeleteFramebuffers(2, blurFBO);
    glDeleteTextures(2, blurColor);

    glDeleteFramebuffers(1, &postFBO);
    glDeleteTextures(1, &postColor);

    glDeleteFramebuffers(1, &postFBO2);
    glDeleteTextures(1, &postColor2);

    glDeleteFramebuffers(1, &postFBO3);
    glDeleteTextures(1, &postColor3);

    // cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // terminate
    glfwTerminate();
    return 0;
}