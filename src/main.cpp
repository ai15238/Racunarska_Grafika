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
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "rg/Texture2D.h"

#include <iostream>
#include <random>

void update(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int loadCubemap(vector<std::string> faces);
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
glm::vec3 lightPos(9.3f, 4.0f, -40.4f);
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
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
    //glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Pozivamo glad biblioteku da ucita sve nase opengl funkcije
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST); //objekti koji su na sceni treba uvek da budu ispred skybox-a

    Shader shader("resources/shaders/cube.vs", "resources/shaders/cube.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader modelShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader bloomShader("resources/shaders/bloom.vs", "resources/shaders/bloom.fs");
    Shader hdrShader("resources/shaders/hdr.vs", "resources/shaders/hdr.fs");
    Shader blendingShader("resources/shaders/2.model_lighting.vs", "resources/shaders/blending.fs");
    //Shader lightShader("resources/shaders/colors.vs", "resources/shaders/colors.fs");
    Shader lightCubeShader("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");

    //Model snowflakes(FileSystem::getPath("resources/objects/snowflake/snowflake1.obj"));
    Model snesko(FileSystem::getPath("resources/objects/snowman/snowman.obj"), true);
    Model island(FileSystem::getPath("resources/objects/island/island1.obj"), true);
    Model svetlo(FileSystem::getPath("resources/objects/svetlo/streetlight.obj"), true);
    Model island2(FileSystem::getPath("resources/objects/island/island2.obj"), true);
    island.SetShaderTextureNamePrefix("material.");
    snesko.SetShaderTextureNamePrefix("material.");
    svetlo.SetShaderTextureNamePrefix("material.");
    island2.SetShaderTextureNamePrefix("material.");
    //Model sneg1(FileSystem::getPath("resources/objects/snow_model/terrain.obj"));
    //sneg1.SetShaderTextureNamePrefix("material.");

    Model modelSun("resources/objects/sun/13913_Sun_v2_l3.obj");
    modelSun.SetShaderTextureNamePrefix("material.");

    Model modelMoon("resources/objects/sun/13913_Sun_v2_l3.obj");
    modelMoon.SetShaderTextureNamePrefix("material.");

    Model drvo(FileSystem::getPath("resources/objects/island/tree.obj"), true);
    drvo.SetShaderTextureNamePrefix("material.");

    Model island3(FileSystem::getPath("resources/objects/island2/island1_design2_c4d.obj"), true);
    island3.SetShaderTextureNamePrefix("material.");

    Model kuca(FileSystem::getPath("resources/objects/house2/untitled.obj"), true);
    kuca.SetShaderTextureNamePrefix("material.");

    PointLight pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0f, 0.0);
    pointLight.ambient = glm::vec3(0.15, 0.15, 0.15);
    pointLight.diffuse = glm::vec3(1.5f,1.5f,1.1f);
    pointLight.specular = glm::vec3(0.15, 0.15, 0.15);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.13f;
    pointLight.quadratic = 0.032f;
    //Model figure1(FileSystem::getPath("resources/objects/low_obj_15000/low_obj_15000.obj"));

    //crtamo kocku
    float cubeVertices[] = {
            // positions                      // texture Coords
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
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

    // world space positions of our cubes
    /*glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, -2.0f, -4.0f),
            glm::vec3(1.2f, -2.0f, -4.0f),
            glm::vec3(0.4f, -1.0f, -7.5f),
            glm::vec3(-1.2f, -2.0f, -5.3f),
            glm::vec3(3.0f, -1.7f, -5.5f),
            glm::vec3(-2.4f, -2.0f, -4.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)
    };*/
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
    /*glm::vec3 positions[70000];
    for(int i=0; i<70000; i++) {
        const double lower_bound = -BOUND;
        const double upper_bound = BOUND;
        std::uniform_real_distribution<double> unif(lower_bound, upper_bound);

        std::random_device rand_dev;          // Use random_device to get a random seed.

        std::mt19937 rand_engine(rand_dev()); // mt19937 is a good pseudo-random number
        // generator.

        double x = unif(rand_engine);
        std::cout << x << std::endl;
        const double lower_bound1 = -BOUND;
        const double upper_bound1 = BOUND;
        std::uniform_real_distribution<double> unif1(lower_bound1, upper_bound1);

        std::random_device rand_dev1;          // Use random_device to get a random seed.

        std::mt19937 rand_engine1(rand_dev1()); // mt19937 is a good pseudo-random number
        // generator.

        double y = unif(rand_engine1);
        std::cout << y << std::endl;
        const double lower_bound2 = -BOUND;
        const double upper_bound2 = BOUND;
        std::uniform_real_distribution<double> unif2(lower_bound2, upper_bound2);

        std::random_device rand_dev2;          // Use random_device to get a random seed.

        std::mt19937 rand_engine2(rand_dev2()); // mt19937 is a good pseudo-random number
        // generator.

        double z = unif(rand_engine2);
        std::cout << z << std::endl;
        positions[i] = {
                glm::vec3(x, y, z)
        };
    }
    */

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

    glVertexAttribPointer(0, 3,GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    // da imam novi lightCubeVBO onda  bi trebalo za stride da se postavi 3*sizeof(flaot)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    /*Texture2D texture("resources/textures/container.jpg", GL_REPEAT, GL_LINEAR, GL_RGB);
    Texture2D texture2("resources/textures/awesomeface.png", GL_REPEAT, GL_LINEAR, GL_RGBA);
*/
    //unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/container.jpg").c_str());
    vector<std::string> faces1
            {
                    FileSystem::getPath("resources/textures/interstellar_skybox/xpos.png"),
                    FileSystem::getPath("resources/textures/interstellar_skybox/xneg.png"),
                    FileSystem::getPath("resources/textures/interstellar_skybox/ypos.png"),
                    FileSystem::getPath("resources/textures/interstellar_skybox/yneg.png"),
                    FileSystem::getPath("resources/textures/interstellar_skybox/zpos.png"),
                    FileSystem::getPath("resources/textures/interstellar_skybox/zneg.png")
            };
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/galaxy/galaxy+X.tga"),
                    FileSystem::getPath("resources/textures/galaxy/galaxy-X.tga"),
                    FileSystem::getPath("resources/textures/galaxy/galaxy+Y.tga"),
                    FileSystem::getPath("resources/textures/galaxy/galaxy-Y.tga"),
                    FileSystem::getPath("resources/textures/galaxy/galaxy+Z.tga"),
                    FileSystem::getPath("resources/textures/galaxy/galaxy-Z.tga")
            };
    unsigned int cubemapTexture = loadCubemap(faces);
    unsigned int cubemapTexture1 = loadCubemap(faces1);

    shader.use();
   // shader.setInt("texture1", 0);
    //shader.setInt("texture2", 1);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    //skyboxShader.use();
    skyboxShader.setInt("skybox1", 1);


/*    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);*/


    // bloom - izdvajanje bljestavih fragmenata (za ulicno svetlo)

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
    //camera.Position = glm::vec3(0,0,3);
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

        //glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
         glm::vec3 lightColor = glm::vec3(0.0, (sin(glfwGetTime()) + 1.0) / 2.0, 0.0);
        // draw scene as normal

        //draw island
        modelShader.use();

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Directional Lignt
        modelShader.setVec3("dirLight.direction", -20.0f, -20.0f, 0.0f);
        modelShader.setVec3("dirLight.ambient", 0.06, 0.06, 0.06);
        modelShader.setVec3("dirLight.diffuse",  0.6f,0.2f,0.2);
        modelShader.setVec3("dirLight.specular", 0.1, 0.1, 0.1);

        // Pointlight's
        //1
        modelShader.setVec3("pointLight[0].position", glm::vec3(-1.05f,(1.8f+sin(glfwGetTime())/6),1.7f));
        modelShader.setVec3("pointLight[0].ambient", pointLight.ambient);
        modelShader.setVec3("pointLight[0].diffuse", pointLight.diffuse);
        modelShader.setVec3("pointLight[0].specular", pointLight.specular);
        modelShader.setFloat("pointLight[0].constant", pointLight.constant);
        modelShader.setFloat("pointLight[0].linear", pointLight.linear);
        modelShader.setFloat("pointLight[0].quadratic", pointLight.quadratic);
        //2
        modelShader.setVec3("pointLight[1].position", glm::vec3(3.05f,(1.8f+sin(glfwGetTime())/6),-4.7f));
        modelShader.setVec3("pointLight[1].ambient", pointLight.ambient);
        modelShader.setVec3("pointLight[1].diffuse", pointLight.diffuse);
        modelShader.setVec3("pointLight[1].specular", pointLight.specular);
        modelShader.setFloat("pointLight[1].constant", pointLight.constant);
        modelShader.setFloat("pointLight[1].linear", pointLight.linear);
        modelShader.setFloat("pointLight[1].quadratic", pointLight.quadratic);
        //3
        /*
        modelShader.setVec3("pointLight[2].position", glm::vec3(-5.75f,(4.85f + sin(glfwGetTime())/6),8.95f));
        modelShader.setVec3("pointLight[2].ambient", glm::vec3(0.15, 0.15, 0.15));
        modelShader.setVec3("pointLight[2].diffuse", glm::vec3(1.5f,1.5f,1.1f));
        modelShader.setVec3("pointLight[2].specular", glm::vec3(0.15, 0.15, 0.15));
        modelShader.setFloat("pointLight[2].constant", 1.0f);
        modelShader.setFloat("pointLight[2].linear", pointLight.linear);
        modelShader.setFloat("pointLight[2].quadratic", pointLight.quadratic);
        //4
        modelShader.setVec3("pointLight[3].position", glm::vec3(7.7f,(-0.4f + sin(glfwGetTime())/6),8.75f));
        modelShader.setVec3("pointLight[3].ambient", glm::vec3(0.15, 0.15, 0.15));
        modelShader.setVec3("pointLight[3].diffuse", glm::vec3(1.5f,1.5f,1.1f));
        modelShader.setVec3("pointLight[3].specular", glm::vec3(0.15, 0.15, 0.15));
        modelShader.setFloat("pointLight[3].constant", 1.0f);
        modelShader.setFloat("pointLight[3].linear", pointLight.linear);
        modelShader.setFloat("pointLight[3].quadratic", pointLight.quadratic);*/

        modelShader.setVec3("viewPosition", camera.Position);
        modelShader.setFloat("material.shininess", 32.0f);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //postaviti ostrvo sa snegom
        //glDisable(GL_CULL_FACE);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, (-0.99f+sin(glfwGetTime())/6), -3.6f));
        model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
        modelShader.setMat4("model", model);
        island.Draw(modelShader);
        //glEnable(GL_CULL_FACE);
        //Enabling back face culling
   /*     glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);*/

        // postaviti svetlo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.2f, (-0.95f+sin(glfwGetTime())/6), 1.4f));
        model = glm::rotate(model, (float)90.0f, glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.2f));
        modelShader.setMat4("model", model);
        //modelShader.setVec3("lightColor", lightColor);
        svetlo.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.2f, (-0.95f+sin(glfwGetTime())/6), -4.4f));
        model = glm::rotate(model, (float)45.0f, glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.2f));
        modelShader.setMat4("model", model);
        //modelShader.setVec3("lightColor", lightColor);
        svetlo.Draw(modelShader);

        //postaviti sneska
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.2f, (-0.35f+sin(glfwGetTime())/6), 1.2f));
        model = glm::rotate(model, (float)10.0f, glm::vec3(0.0f, 0.3f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        modelShader.setMat4("model", model);
        //modelShader.setVec3("lightColor", lightColor);
        snesko.Draw(modelShader);

        //postaviti kucu
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5, (-0.73f+sin(glfwGetTime())/6), -1.4));
        model = glm::rotate(model, (float)-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1, 0.2, 0.1));
        modelShader.setMat4("model", model);
        kuca.Draw(modelShader);


        modelShader.setVec3("pointLight[2].position", glm::vec3());
        modelShader.setVec3("pointLights[2].ambient", pointLight.ambient);
        modelShader.setVec3("pointLights[2].diffuse", pointLight.diffuse);
        modelShader.setVec3("pointLights[2].specular", pointLight.specular);
        modelShader.setFloat("pointLights[2].constant", pointLight.constant);
        modelShader.setFloat("pointLights[2].linear", pointLight.linear);
        modelShader.setFloat("pointLights[2].quadratic", pointLight.quadratic);

        //postaviti ostrvo sa rekom

        glDisable(GL_CULL_FACE);
        modelShader.use();
        modelShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        modelShader.setVec3("lightColor", 1.0, 0.5, 0.2);
        modelShader.setVec3("lightPos", lightPos);
        modelShader.setVec3("viewPos", camera.Position);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(15.5f, -1.0f, 20.0f));
        model = glm::rotate(model, (float)10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
        modelShader.setMat4("model", model);
        island2.Draw(modelShader);


        // blending
        //blending shader config
        blendingShader.use();
        blendingShader.setVec3("viewPosition", camera.Position);
        blendingShader.setFloat("material.shininess", 32.0f);

        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);

        blendingShader.setVec3("dirLight.direction", glm::vec3(-0.547f, -0.727f, 0.415f));
        blendingShader.setVec3("dirLight.ambient", glm::vec3(0.35f));
        blendingShader.setVec3("dirLight.diffuse", glm::vec3(0.4f));
        blendingShader.setVec3("dirLight.specular", glm::vec3(0.2f));

        blendingShader.setVec3("pointLights[0].position", glm::vec3(15.5f+sin(currentFrame/2)*6.6f,-1.0f+cos(currentFrame/2)*4.2f,20.0f+cos(currentFrame/2)*4.6f));
        blendingShader.setVec3("pointLights[0].ambient", pointLight.ambient);
        blendingShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
        blendingShader.setVec3("pointLights[0].specular", pointLight.specular);
        blendingShader.setFloat("pointLights[0].constant", pointLight.constant);
        blendingShader.setFloat("pointLights[0].linear", pointLight.linear);
        blendingShader.setFloat("pointLights[0].quadratic", pointLight.quadratic);

        blendingShader.setVec3("pointLights[1].position", glm::vec3(15.5f+cos(currentFrame*2)*0.6f,1.6f+cos(currentFrame)*0.2f+cos(currentFrame)*0.4f,12.45f+sin(currentFrame)*0.6f));
        blendingShader.setVec3("pointLights[1].ambient", pointLight.ambient);
        blendingShader.setVec3("pointLights[1].diffuse", pointLight.diffuse);
        blendingShader.setVec3("pointLights[1].specular", pointLight.specular);
        blendingShader.setFloat("pointLights[1].constant", pointLight.constant);
        blendingShader.setFloat("pointLights[1].linear", pointLight.linear);
        blendingShader.setFloat("pointLights[1].quadratic", pointLight.quadratic);
        glEnable(GL_CULL_FACE);

        //render sun
        glm::mat4 sunModel = glm::mat4(1.0f);
        //glm::vec3 earthPos = glm::vec3(15.5f, -1.0f, 20.0f)
         //       +glm::vec3(sin(1.0f)*100.0, 0.0f, cos(12.0f)*100.0f);
        //sunModel = glm::translate(sunModel, glm::vec3(15.5f+sin(currentFrame)*4.6f,1.6f+cos(currentFrame*2)*4.2f,12.45f+cos(currentFrame)*4.6f));
        /*sunModel = glm::scale(sunModel, glm::vec3(0.001f));
        sunModel = glm::rotate(sunModel,glm::radians(90.0f), glm::vec3(1.0f,0.0f , 0.0f));
        sunModel = glm::rotate(sunModel,glm::radians(-currentFrame*20), glm::vec3(0.0f ,1.0f, 0.0f));
        sunModel = glm::rotate(sunModel,glm::radians(currentFrame*20), glm::vec3(1.0f , 0.0f,0.0f));
*/

        sunModel = glm::translate(sunModel, glm::vec3(15.5f+sin(currentFrame/2)*6.6f, -1.0f+cos(currentFrame/2)*4.2f, 20.0f+cos(currentFrame/2)*4.6f));
        sunModel = glm::scale(sunModel, glm::vec3(0.001f));
        sunModel = glm::rotate(sunModel, sin(currentFrame), glm::vec3(0.3f,0.1f,1.0f));
        sunModel = glm::translate(sunModel, glm::vec3(0.0f));
        blendingShader.setMat4("model", sunModel);
        modelSun.Draw(blendingShader);

        // model meseca
        glm::mat4 moonModel = glm::mat4(1.0f);
        //moonModel = glm::translate(moonModel,glm::vec3(6.7f+cos(currentFrame*2)*0.6f,1.6f+cos(currentFrame)*0.2f+cos(currentFrame)*0.4f,-17.45f+sin(currentFrame)*0.6f));
        moonModel = glm::translate(moonModel,glm::vec3(15.5f,1.0f,20.0));
        moonModel = glm::scale(moonModel, glm::vec3(0.001f));
        //moonModel = glm::rotate(moonModel,glm::radians(currentFrame), glm::vec3(1.0f,0.0f , 0.0f));
        //moonModel = glm::rotate(moonModel,glm::radians(-currentFrame*20), glm::vec3(0.0f ,1.0f, 0.0f));
        //moonModel = glm::rotate(moonModel,glm::radians(currentFrame*20), glm::vec3(1.0f , 0.0f,0.0f));
        blendingShader.setMat4("model", moonModel);
        modelMoon.Draw(blendingShader);

        //TODO - kasnije translirati
        // drvo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(7.3f, -1.1f, -45.4f));
        model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
        modelShader.setMat4("model", model);
        drvo.Draw(modelShader);

        //lampion
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(9.3f, -1.1f, -45.4f));
        model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
        modelShader.setMat4("model", model);
        svetlo.Draw(modelShader);
        
        /*projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();

        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);
        modelShader.use();*/


        //postaviti prolecno ostrvo
        /*glm::mat4 modelIsland = glm::mat4(1.0f);
        modelIsland = glm::translate(modelIsland, glm::vec3(6.7, 1.0, 23.9));
        modelIsland = glm::scale(modelIsland, glm::vec3(0.06f));
        //modelIsland = glm::rotate(modelIsland, glm::radians(45));
        modelShader.setMat4("model", modelIsland);
        island3.Draw(modelShader);*/

        
        //draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightCubeShader.setMat4("model", model);
        lightCubeShader.setVec3("lightColor", lightColor);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
       // glBindVertexArray(0);


       // skybox na kraju
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        skyboxShader.setFloat("p", sin(glfwGetTime()/2.0+0.5));
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


        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        //skyboxShader.setFloat("p", sin(glfwGetTime()/2.0+0.5));
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture1);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


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

        /*update(window);
        //cistimo pozadinu prozora
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f + sin(glfwGetTime())));//da se kamera krece + sin...

        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        //dry skybox first
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        // bind Texture
        //glBindTexture(GL_TEXTURE_2D, texture);
        texture.active(GL_TEXTURE0);
        texture.bind();

        texture2.active(GL_TEXTURE1);
        texture2.bind();

        //transformacije - napravimo ih na procesoru, a onda matricu koju smo kreirali posaljemo a graficku karticu
        //scaliranje (da bude 2x manji) pa transliranje
        *//*glm::mat4 m = glm::mat4(1.0f); // I
        //skaliranje -> rotacija -> translacija
        m = glm::translate(m, position); //glm::vec3(0.6, -0.5, 0.0)); // I * T
        m = glm::rotate(m, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); //(I * T) * R
        m = glm::scale(m, glm::vec3(0.2, 0.2, 1.0)); // (I * T * R) * S
*//*        // I*T*R*S*x <=> T(R(S(x)))
        //model matricu koju smo definisali u vertex shader-u posaljemo na graficku
        //int locationId = glGetUniformLocation(shader.m_Id, "model");
        //glUniformMatrix4fv(locationId, 1, GL_FALSE, glm::value_ptr(m)); // &m[0][0]

        //koordinatni sistem - hocemo da napravimo da kvadrat bude zakrivljen
        // objekat da bi se prikazao prolazi kroz 4 transformacije
        // model matrica postavi objekat u svet
        // view matrica transformise koordinate da izgledaju onako kako su iz ugla kamere
        // matrica projekcije ih redukuje do -1,1
        // viewport transformacija transformise u koordinate na ekranu

        unsigned int modelLocation = glGetUniformLocation(shader.m_Id, "model");
        unsigned int viewLocation = glGetUniformLocation(shader.m_Id, "view");

        //da se vrtimo u krug a da uvek gledamo u koord pocetak
        //glm::mat4 view = glm::mat4(1.0f);
        *//*float radius = 7;
        float camX = sin(glfwGetTime()) * radius; // kamera koja kruzi oko scene i uvek gleda u kood poc
        float camZ = cos(glfwGetTime()) * radius;
        view = glm::lookAt(glm::vec3(camX,0,camZ), glm::vec3(0,0,0), glm::vec3(0, 1, 0));
*//*

        // pravimo kameru da moze da se pomera po prostoru sa WASD
        //scena se ne menja, jedino kamera
        //cameraPos dodajemo na Front da ne bi gledalo u istu tacku kada pomeramo levo desno
        //view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);

        //glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        shader.setMat4("projection", projection); //drugi nacin da se postavi
        //draw triangle
        shader.use();
        //shader.setUniform4f("gColor", sin(glfwGetTime())/2.0+0.5, 0.0, 0.0, 1.0);
        //update(window);
        //shader.setFloat("p", sin(glfwGetTime())/2 + 0.5);

        glBindVertexArray(VAO);

        for(int i=0; i<2; i++) {
            glm::mat4 model = glm::mat4(1.0f);

            // model matrica postavlja objekat u svet kako on treba da stoji
            // hocemo da zarotiramo po x-osi
            float angle = 20.0f * i;
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
            shader.setMat4("model", model);
            //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glDrawArrays(GL_TRIANGLES, 0, 36);

        }
        glBindVertexArray(0);*/
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &skyboxVBO);
    shader.deleteProgram();
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
            exposure -= 0.005f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.005f;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
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
unsigned int loadCubemap(vector<std::string> faces) {

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

/*model = glm::mat4(1.0f);
projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
view = camera.GetViewMatrix();
modelShader.setMat4("view", view);
modelShader.setMat4("projection", projection);
model = glm::translate(model, glm::vec3(1.0f, -2.0f, -4.0f));
model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
model = glm::rotate(model, (float)glm::radians(85.0), glm::vec3(-0.1, 0.0, 0.0));
modelShader.setMat4("model", model);
modelShader.setVec3("lightColor", lightColor);
figure1.Draw(modelShader);*/

//cube with texture
/*shader.use();
model = glm::mat4(1.0f);
projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
view = camera.GetViewMatrix();
shader.setMat4("view", view);
shader.setMat4("projection", projection);
model = glm::translate(model, glm::vec3(0.0f, 1.0f, 1.0f));
model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
shader.setMat4("model", model);
// cubes
glBindVertexArray(VAO);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, cubeTexture);
glDrawArrays(GL_TRIANGLES, 0, 36);
glBindVertexArray(0);*/

/*pointLight.position = glm::vec3(4.0f * cos(currentFrame), 4.0f, 4.0*sin(currentFrame));
modelShader.setVec3("pointLight.position", pointLight.position);
modelShader.setVec3("pointLight.ambient", pointLight.ambient);
modelShader.setVec3("pointLight.diffuse", pointLight.diffuse);
modelShader.setVec3("pointLight.specular", pointLight.specular);
modelShader.setFloat("pointLight.constant", pointLight.constant);
modelShader.setFloat("pointLight.linear", pointLight.linear);
modelShader.setFloat("pointLight.quadratic", pointLight.quadratic);*/


// colored cube
/*
        lightShader.use();
        lightShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightShader.setVec3("lightColor", 1.0, 0.5, 0.2);
        lightShader.setVec3("lightPos", lightPos);
        lightShader.setVec3("viewPos", camera.Position);

        projection = glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        //world transformation:
        model = glm::mat4(1.0f);
        lightShader.setMat4("model", model);
        //render the cube
        glBindVertexArray(cubeVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);*/