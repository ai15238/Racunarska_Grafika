#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <rg/Shader.h>
#include <rg/Camera.h>
#include <rg/model.h>
#include "rg/Texture2D.h"

#include <iostream>
#include <random>

void update(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int loadTexture(char const * path);
void renderQuad();

//sirina i visina prozora za renderovanje u pixelima
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const float BOUND = 100;
glm::vec3 position;

// kamera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool firstMouse = true;

float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0; // definisemo kao centar ekrana ovu poslednju poz kursa

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// hdr i bloom
bool hdr = true;
bool hdrKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

//lightning
glm::vec3 lightPos(0.0f, 0.7f, -4.4f);

struct DirLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
int main() {
    //inicijalizacija glfw biblioteke i konfiguracija
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw kreiranje prozora
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello window", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //zelimo da opengl crta u ovom prozoru:
    glfwMakeContextCurrent(window);

    //funkcija koja se poziva svaki put kada se velicina prozora promeni
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    //bolja alternativa funkciji update; jednom se izvrsava
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Pozivamo glad biblioteku da ucita sve nase opengl funkcije
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // configure global opengl state
    // -----------------------------
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    //enable frag blending and setup blending function:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("resources/shaders/cube.vs", "resources/shaders/cube.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader modelShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader bloomShader("resources/shaders/bloom.vs", "resources/shaders/bloom.fs");
    Shader hdrShader("resources/shaders/hdr.vs", "resources/shaders/hdr.fs");
    //Shader lightCubeShader("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");
    Shader lightBallShader("resources/shaders/lightBallShader.vs", "resources/shaders/lightBallShader.fs");
    Shader objectShader("resources/shaders/model_loading.vs","resources/shaders/model_loading.fs");

    Model island(FileSystem::getPath("resources/objects/island/island1.obj"), true);
    island.SetShaderTextureNamePrefix("material.");

    Model svetlo(FileSystem::getPath("resources/objects/svetlo/streetlight.obj"), true);
    svetlo.SetShaderTextureNamePrefix("material.");

    Model snesko(FileSystem::getPath("resources/objects/snowman/snowman.obj"), true);
    snesko.SetShaderTextureNamePrefix("material.");

    Model bench(FileSystem::getPath("resources/objects/bench/bench.obj"), true);
    bench.SetShaderTextureNamePrefix("material.");

    Model modelLightBall(FileSystem::getPath("resources/objects/lightBall/lightBall.obj"),true);
    modelLightBall.SetShaderTextureNamePrefix("material.");

    Model drvo(FileSystem::getPath("resources/objects/drvo/t1.obj"), true);
    drvo.SetShaderTextureNamePrefix("material.");

    Model backpack(FileSystem::getPath("resources/objects/backpack/backpack.obj"), true);
    backpack.SetShaderTextureNamePrefix("material.");


    DirLight dirLight;
    dirLight.direction = glm::vec3(0.0, -0.5, 0.0);
    dirLight.ambient = glm::vec3(0.05, 0.05, 0.05);
    dirLight.diffuse = glm::vec3(0.1, 0.1, 0.1);
    dirLight.specular = glm::vec3(1.0, 1.0, 1.0);

    PointLight pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0f, 0.0);
    pointLight.ambient = glm::vec3(0.15, 0.15, 0.15);
    pointLight.diffuse = glm::vec3(0.6f,0.6f,0.6f);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.13f;
    pointLight.quadratic = 0.032f;

    //spotlight lamp
    SpotLight spotLight;
    spotLight.position = glm::vec3(4.0f, 4.0, 0.0);
    spotLight.direction = glm::vec3(0.0f, -3.0f, 0.0f);
    spotLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    spotLight.diffuse = glm::vec3(0.9f, 0.25f, 0.1f);
    spotLight.specular = glm::vec3(0.5f, 1.0f, 0.0f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.05f;
    spotLight.quadratic = 0.001f;
    spotLight.cutOff = glm::cos(glm::radians(30.0f));
    spotLight.outerCutOff = glm::cos(glm::radians(50.0f));
    //Model figure1(FileSystem::getPath("resources/objects/low_obj_15000/low_obj_15000.obj"));

    //crtamo kocku
    float cubeVertices[] = {
            // positions                      // normals
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    float skyboxVertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };

    float lightCubeVertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
    };

    unsigned int VBO, VAO;//, EBO;
    //Vertex buffer object - iz ram-a neke podatke ucitava na graficku karticu

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //aktiviramo objekat
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    /*glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

    glVertexAttribPointer(0, 3,GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //glBindVertexArray(0);

    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) nullptr);


    unsigned int  cubeVBO, cubeVAO;//mozda moze i onaj stari VBO
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightCubeVertices), lightCubeVertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    //light cube
    /*unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    // da imam novi lightCubeVBO onda  bi trebalo za stride da se postavi 3*sizeof(flaot)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) (3*sizeof(float)));
    glEnableVertexAttribArray(0);*/

    /*Texture2D texture("resources/textures/container.jpg", GL_REPEAT, GL_LINEAR, GL_RGB);
    Texture2D texture2("resources/textures/awesomeface.png", GL_REPEAT, GL_LINEAR, GL_RGBA);
*/

   /* shader.use();
    shader.setInt("skybox", 0);*/
    //skybox
    std::vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/space/right.jpg").c_str(),
                    FileSystem::getPath("resources/textures/space/left.jpg").c_str(),
                    FileSystem::getPath("resources/textures/space/top.jpg").c_str(),
                    FileSystem::getPath("resources/textures/space/bottom.jpg").c_str(),
                    FileSystem::getPath("resources/textures/space/front.jpg").c_str(),
                    FileSystem::getPath("resources/textures/space/back.jpg").c_str()
            };

    unsigned int cubemapTexture = loadCubemap(faces);

    shader.use();
    shader.setInt("skybox", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);



/*    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);*/


    // bloom - izdvajanje bljestavih fragmenata

    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffer[2]; // colorBuffer[0] = FragColor, [1] = BrightColor
    glGenTextures(2, colorBuffer);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);
    }

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);


    // u color_attachment0 se nalazi cela scena u HDR-u, a u attachment1 - fragmenti intenz vece od neke zadate vrednosti
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer za blurovanje
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // aktiviranje shader-a
    bloomShader.use();
    bloomShader.setInt("image", 0);

    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);
    hdrShader.setInt("bloomBlur", 1);
    //
    camera.Position = glm::vec3(0,1,5);
    //camera.Front = glm::vec3(0,0,-1);
    //camera.Up = glm::vec3(0,1,0);



    // petlja za renderovanje
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame; //za svaki frejm i za svaki prolazak kroz petlju renderovanja
        // se izracuna koliko je vremena proteklo od kad se petlja rend zavrsila
        //pa do trenutnig vremena i onda kada se to izr dobijamo koliko smo vremena proveli
        // u racunanju tog frejma

        update(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float lx = 2.0f * cos(currentFrame/3)-0.3;
        float ly = 1.2f-sin(currentFrame/3)*1.4+cos(currentFrame/3)*0.3;
        float lz = 4.0f * sin(currentFrame/3) - 2.1f;

        //glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
        glm::vec3 lightColor = glm::vec3(0.0, 1.0, 0.0);

        // draw scene as normal
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightBallShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        lightBallShader.setVec3("viewPosition", camera.Position);
        lightBallShader.setFloat("material.shininess", 32.0f);
        lightBallShader.setMat4("projection", projection);
        lightBallShader.setMat4("view", view);

        lightBallShader.setVec3("dirLight.direction", glm::vec3(-20.0, -20.0, 0.0));
        lightBallShader.setVec3("dirLight.ambient", dirLight.ambient);
        lightBallShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        lightBallShader.setVec3("dirLight.specular", dirLight.specular);

        lightBallShader.setVec3("pointLight[0].position", lx, ly, lz);
        lightBallShader.setVec3("pointLight[0].ambient", pointLight.ambient);
        lightBallShader.setVec3("pointLight[0].diffuse", pointLight.diffuse);
        lightBallShader.setVec3("pointLight[0].specular", pointLight.specular);
        lightBallShader.setFloat("pointLight[0].constant", pointLight.constant);
        lightBallShader.setFloat("pointLight[0].linear", pointLight.linear);
        lightBallShader.setFloat("pointLight[0].quadratic", pointLight.quadratic);

        //lightBallShader.setVec3("spotLight[0].position", 0.0f+sin(currentFrame/2)*2.1f*cos(currentFrame/2)*2.3,1.0f-cos(currentFrame/2)*0.2f+sin(currentFrame/2)*1.3*cos(currentFrame/2),-3.6+sin(currentFrame/2)*1.3+cos(currentFrame/2)*1.6f);
        lightBallShader.setVec3("spotLight[0].position", lx, ly, lz);
        lightBallShader.setVec3("spotLight[0].direction", spotLight.direction);
        lightBallShader.setVec3("spotLight[0].ambient", spotLight.ambient);
        lightBallShader.setVec3("spotLight[0].diffuse", spotLight.diffuse);
        lightBallShader.setVec3("spotLight[0].specular", spotLight.specular);
        lightBallShader.setFloat("spotLight[0].constant", spotLight.constant);
        lightBallShader.setFloat("spotLight[0].linear", spotLight.linear);
        lightBallShader.setFloat("spotLight[0].quadratic", spotLight.quadratic);
        lightBallShader.setFloat("spotLight[0].cutOff", spotLight.cutOff);
        lightBallShader.setFloat("spotLight[0].outerCutOff", spotLight.outerCutOff);


        //render lightBallShader
        glm::mat4 lightBallModel = glm::mat4(1.0f);
        lightBallModel = glm::translate(lightBallModel, glm::vec3(lx, ly, lz));
        lightBallModel = glm::scale(lightBallModel, glm::vec3(0.0001f));
        //lightBallModel = glm::rotate(lightBallModel, (float)sin(currentFrame), glm::vec3(0.0f,1.0f,1.0f));
        lightBallModel = glm::translate(lightBallModel, glm::vec3(0.0f));
        lightBallShader.setMat4("model", lightBallModel);
        modelLightBall.Draw(lightBallShader);

        objectShader.use();

        objectShader.setVec3("viewPosition", camera.Position);
        objectShader.setFloat("material.shininess", 32.0f);
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        /*objectShader.setVec3("dirLight.direction", glm::vec3(-20.0, -20.0, 0.0));
        objectShader.setVec3("dirLight.ambient", glm::vec3(0.0f));
        objectShader.setVec3("dirLight.diffuse", glm::vec3(0.0));
        objectShader.setVec3("dirLight.specular", glm::vec3(0.0f));
*/
        objectShader.setVec3("pointLight[0].position", lx, ly, lz);
        objectShader.setVec3("pointLight[0].ambient", glm::vec3(0.01f));
        objectShader.setVec3("pointLight[0].diffuse", glm::vec3(1.6));
        objectShader.setVec3("pointLight[0].specular", pointLight.specular);
        objectShader.setFloat("pointLight[0].constant", pointLight.constant);
        objectShader.setFloat("pointLight[0].linear", pointLight.linear);
        objectShader.setFloat("pointLight[0].quadratic", pointLight.quadratic);


        //lightBallShader.setVec3("spotLight[0].position", 0.0f+sin(currentFrame/2)*2.1f*cos(currentFrame/2)*2.3,1.0f-cos(currentFrame/2)*0.2f+sin(currentFrame/2)*1.3*cos(currentFrame/2),-3.6+sin(currentFrame/2)*1.3+cos(currentFrame/2)*1.6f);
        objectShader.setVec3("spotLight[0].position", glm::vec3(3.6f,(2.0f+sin(glfwGetTime())/6),-4.2f));
        objectShader.setVec3("spotLight[0].direction", spotLight.direction);
        objectShader.setVec3("spotLight[0].ambient", glm::vec3(0.0));
        objectShader.setVec3("spotLight[0].diffuse", glm::vec3(1.0));
        objectShader.setVec3("spotLight[0].diffuse", glm::vec3( 1.0+(float)((sin(glfwGetTime()) / 2 ) * cos(rand()%10) )));
        objectShader.setVec3("spotLight[0].specular", spotLight.specular);
        objectShader.setFloat("spotLight[0].constant", spotLight.constant);
        objectShader.setFloat("spotLight[0].linear", spotLight.linear);
        objectShader.setFloat("spotLight[0].quadratic", spotLight.quadratic);
        objectShader.setFloat("spotLight[0].cutOff", spotLight.cutOff);
        objectShader.setFloat("spotLight[0].outerCutOff", spotLight.outerCutOff);

        //glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //postaviti ostrvo
        glDisable(GL_CULL_FACE);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, (-0.99f+sin(glfwGetTime())/6), -3.6f));
        model = glm::scale(model, glm::vec3(0.6f));
        objectShader.setMat4("model", model);
        island.Draw(objectShader);
        glEnable(GL_CULL_FACE);


        //postaviti sneska
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, (-0.45f+sin(glfwGetTime())/6), -0.3f));
        model = glm::rotate(model, glm::radians((float)20.0f), glm::vec3(0.0f, 0.3f, 0.0f));
        model = glm::scale(model, glm::vec3(0.26f));
        objectShader.setMat4("model", model);
        snesko.Draw(objectShader);


        //klupa
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.1, (-0.74f+sin(glfwGetTime())/6), -1.0));
        model = glm::scale(model, glm::vec3(0.0099f));
        model = glm::rotate(model, glm::radians((float)-20.0), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians((float)-90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        objectShader.setMat4("model", model);
        bench.Draw(objectShader);

        // drvo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.4f, (-0.95f+sin(glfwGetTime())/6), -4.9f));
        model = glm::scale(model, glm::vec3(0.0018f, 0.001f, 0.0018f));
        //model = glm::rotate(model, glm::radians((float)-30.0f), glm::vec3(0.0, 1.0, 0.0));
        objectShader.setMat4("model", model);
        drvo.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.9f, (-0.95f+sin(glfwGetTime())/6), -5.1f));
        model = glm::scale(model, glm::vec3(0.002f, 0.002f, 0.002f));
        model = glm::rotate(model, glm::radians((float)-30.0f), glm::vec3(0.0, 1.0, 0.0));
        objectShader.setMat4("model", model);
        drvo.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.4f, (-0.95f+sin(glfwGetTime())/6), -3.9f));
        model = glm::scale(model, glm::vec3(0.0016f, 0.0016f, 0.0019f));
        //model = glm::rotate(model, glm::radians((float)-30.0f), glm::vec3(0.0, 1.0, 0.0));
        objectShader.setMat4("model", model);
        drvo.Draw(objectShader);

        //svetlo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.2f, (-0.95f+sin(glfwGetTime())/6), -4.4f));
        model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.2f));
        //model = glm::rotate(model, glm::radians((float)-45.0f), glm::vec3(0.0, 1.0, 0.0));
        objectShader.setMat4("model", model);
        svetlo.Draw(objectShader);
/*
 * komentar
 */

        //draw the lamp object
        /* lightCubeShader.use();
         glBindVertexArray(lightCubeVAO);

         lightCubeShader.setMat4("projection", projection);
         lightCubeShader.setMat4("view", view);
         model = glm::mat4(1.0f);
         model = glm::translate(model, lightPos);
         model = glm::scale(model, glm::vec3(0.15f));
         lightCubeShader.setMat4("model", model);
         lightCubeShader.setVec3("lightColor", lightColor);

         glDrawArrays(GL_TRIANGLES, 0, 36);*/
        // glBindVertexArray(0);



        ////ovo je za cubemap environment
        //glEnable(GL_CULL_FACE);
        /*shader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, -1.0));
        model = glm::scale(model, glm::vec3(15.0f));
        view = camera.GetViewMatrix();
        shader.setVec3("cameraPos", camera.Position);
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        //activate cubebox texture
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);*/

        // skybox na kraju

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        //skyboxShader.setFloat("p", sin(glfwGetTime()/4.0+0.5));
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        //this goes after window implementation
        glEnable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //*********************************************
        //load pingpong
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        bloomShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            bloomShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffer[1] : pingpongColorbuffers[!horizontal]);

            renderQuad();

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // **********************************************
        // load hdr and bloom
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffer[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        hdrShader.setBool("hdr", hdr);
        hdrShader.setBool("bloom", bloom);
        hdrShader.setFloat("exposure", exposure);
        renderQuad();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    //glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &skyboxVBO);
    shader.deleteProgram();
    modelShader.deleteProgram();
    lightBallShader.deleteProgram();
    hdrShader.deleteProgram();
    objectShader.deleteProgram();
    bloomShader.deleteProgram();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
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

// f-ja koja postavlja dimenzije unutar prozora za renderovanja
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    //kad god se promeni dimenzija prozora, glViewPort se poziva da se promeni i velicina unutar prozora
    glViewport(0, 0, width, height);
}

//fukcija koja ce se pokretati kada korisnik aplikacije nesto uradi
void update(GLFWwindow* window) {
    //mogucnost zatvaranja prozora pritiskom na escape
    if( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    //sve vrednosti u brzinama, kretanjima, skaliraju sa deltaTime
    // npr ako nam je za preth frejm bilo potrebno dosta vremenna, npr za prethodni frejm nam je
    //bilo potrebno 3s, mi moramo sada da povecamo za odgovarajuce deltaTime kako bi kamera
    // bila prikazana na onoj poziciji na kojoj bi bila za 3s
    //const float cameraSpeed = 1.5 * deltaTime; // dodajemo da bismo se kretali malo sporije
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //position.y += 0.04;
        //cameraPos += cameraFront * cameraSpeed; // unapred, gledam u smeru napred
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //position.y -= 0.04;
        //cameraPos -= cameraFront * cameraSpeed; //unazad
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //position.x -= 0.04;
        //cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; // levo
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //position.x += 0.04;
        //cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; //udesno, normalizujemo vektore
        camera.ProcessKeyboard(RIGHT, deltaTime);
    //kada uzimamo vektore pravaca. Kad god nam treba da uzimamo smerove neke pa da nadovezujemo
    // kada neki vektor treba da predstavlja neki pravac gledanja

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hdrKeyPressed)
    {
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
    {
        hdrKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.5f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.5f;
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    //kada pomeramo mis, moramo ponovo da racunamo vektore x, y
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if(firstMouse) { //ako je bilo prvo pomeranje misa, postavicemo poziciju na kojoj smo prethodno bili, na trenutnu
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);

}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces) {

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID); //CuBE)MAP tekstura je poseban tip teksture koji je ugradjen u OPENGL

    int width, height, nChannels;
    unsigned char* data;
    for(unsigned int i=0; i<faces.size(); i++) {
        data = stbi_load(faces[i].c_str(), &width, &height, &nChannels, 0);
        if(data) {
            //desno - levo ----> positiveX - negativeX
            // gore - dole -----> positiveY - negativeY
            // napred - nazad ----> positiveZ - negativeZ

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                         width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //stbi_image_free(data);
        }else {
            std::cerr << "Failed to load cube map texture face\n";
            return -1;
        }
        stbi_image_free(data);
    }

    //podesavamo parametre cubemap teksture
    // prvi parametar je filtriranje i
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 2. wrapovanje oko svake koordinate. clamp_to_edge
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



// nacrtati pahulje
/*glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
glm::mat4 view = camera.GetViewMatrix();
modelShader.setMat4("view", view);
modelShader.setMat4("projection", projection);

for(int i=0; i<70000; i++) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, positions[i]);//glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.0008f, 0.0008f, 0.0008f));
    float angle = 20.0f*i;
    model = glm::rotate(model, (float) glm::radians(85.0f), glm::vec3(-0.1, 0.0, 0.0));
    if(i % 2 == 0) {
        model = glm::rotate(model, (float) (glfwGetTime()*(-1.0)), glm::vec3(0.0, -0.3, 0.0));
    }
    else {
        model = glm::rotate(model, (float) (glfwGetTime()), glm::vec3(0.0, -0.3, 0.0));
    }
    modelShader.setMat4("model", model);
    snowflakes.Draw(modelShader);
}
*/