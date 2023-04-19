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

void update(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//sirina i visina prozora za renderovanje u pixelima
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 position;

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

    //bolja alternativa funkciji update; jednom se izvrsava
    //glfwSetKeyCallback(window, key_callback);

    // Pozivamo glad biblioteku da ucita sve nase opengl funkcije
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");

    //triangle
    /*float vertices[] = {
            0.5f, -0.5f, 0.0, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //levo
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,//desno
            0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f //gore
    };*/
    //square
    /*float vertices[] = {
            // pozzicija                        //koordinate tektura
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f, //gore desno
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // dole desno
            -0.5, -0.5f, 0.0f, 0.0f, 0.0f, //dole levo
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f // gore levo
    };
    unsigned int indices[] = {
            0, 1, 3, // prvi trougao
            1, 2, 3 // drugi trougao
    };*/
    //crtamo kocku
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    unsigned int VBO, VAO;//, EBO;
    //Vertex buffer object - iz ram-a neke podatke ucitava na graficku karticu

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //aktiviramo objekat
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /*glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

    glVertexAttribPointer(0, 3,GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    Texture2D texture("resources/textures/tekstura1.jpg", GL_REPEAT, GL_LINEAR, GL_RGB);
    Texture2D texture2("resources/textures/tekstura3.jpg", GL_REPEAT, GL_LINEAR, GL_RGB);
    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // petlja za renderovanje
    while (!glfwWindowShouldClose(window)) {

        update(window);
        //cistimo pozadinu prozora
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind Texture
        //glBindTexture(GL_TEXTURE_2D, texture);
        texture.active(GL_TEXTURE0);
        texture.bind();

        texture2.active(GL_TEXTURE1);
        texture2.bind();

        //transformacije - napravimo ih na procesoru, a onda matricu koju smo kreirali posaljemo a graficku karticu
        //scaliranje (da bude 2x manji) pa transliranje
        /*glm::mat4 m = glm::mat4(1.0f); // I
        //skaliranje -> rotacija -> translacija
        m = glm::translate(m, position); //glm::vec3(0.6, -0.5, 0.0)); // I * T
        m = glm::rotate(m, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); //(I * T) * R
        m = glm::scale(m, glm::vec3(0.2, 0.2, 1.0)); // (I * T * R) * S
*/        // I*T*R*S*x <=> T(R(S(x)))
        //model matricu koju smo definisali u vertex shader-u posaljemo na graficku
        //int locationId = glGetUniformLocation(shader.m_Id, "model");
        //glUniformMatrix4fv(locationId, 1, GL_FALSE, glm::value_ptr(m)); // &m[0][0]

        //koordinatni sistem - hocemo da napravimo da kvadrat bude zakrivljen
        // objekat da bi se prikazao prolazi kroz 4 transformacije
        // model matrica postavi objekat u svet
        // view matrica transformise koordinate da izgledaju onako kako su iz ugla kamere
        // matrica projekcije ih redukuje do -1,1
        // viewport transformacija transformise u koordinate na ekranu

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -6.0f));

        projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        unsigned int modelLocation = glGetUniformLocation(shader.m_Id, "model");
        unsigned int viewLocation = glGetUniformLocation(shader.m_Id, "view");

        //glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        shader.setMat4("projection", projection); //drugi nacin da se postavi
        //draw triangle
        shader.use();
        //shader.setUniform4f("gColor", sin(glfwGetTime())/2.0+0.5, 0.0, 0.0, 1.0);
        //update(window);
        //shader.setFloat("p", sin(glfwGetTime())/2 + 0.5);

        glBindVertexArray(VAO);

        for(int i=0; i<6; i++) {
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
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shader.deleteProgram();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
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
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position.y += 0.04;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position.y -= 0.04;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position.x -= 0.04;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position.x += 0.04;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

