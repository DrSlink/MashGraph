//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
#include <ctime>
#include <chrono>

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static GLsizei WIDTH = 720, HEIGHT = 720; //размеры окна

using namespace LiteMath;

float3 g_camPos(0, 0, 5);
float cam_rot[2] = {0, 0};


void windowResize(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
}

GLuint load_texture(const char *path) { // from stackoverflow
    GLuint g_texture = 0;
    int w, h, c;
    unsigned char *image = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);

    if(image == nullptr) {
        std::cerr << "Failed to load texture" << std::endl;
        exit(1);
    }
    
    if (c != STBI_rgb_alpha && c != STBI_rgb) {
        std::cerr << "Bad pixel size" << std::endl;
        exit(1);
    }
    
    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    if (c == STBI_rgb) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image);
    return g_texture;
}

int initGL() {
    int res = 0;
    //грузим функции opengl через glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "task 3", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, windowResize);

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    GLuint glitter_texture = load_texture("../textures/glitter.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glitter_texture);
    GL_CHECK_ERRORS;
        
    //Создаем и загружаем геометрию поверхности
    //
    GLuint g_vertexBufferObject;
    GLuint g_vertexArrayObject;
    {

        float quadPos[] =
        {
            -1.0f, 1.0f,    // v0 - top left corner
            -1.0f, -1.0f,    // v1 - bottom left corner
            1.0f, 1.0f,    // v2 - top right corner
            1.0f, -1.0f      // v3 - bottom right corner
        };

        g_vertexBufferObject = 0;
        GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0

        glGenBuffers(1, &g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), (GLfloat *) quadPos, GL_STATIC_DRAW);
        GL_CHECK_ERRORS;

        glGenVertexArrays(1, &g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;

        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(vertexLocation);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
        
    }

    auto time_start = std::chrono::system_clock::now();
    const float NANOSECS_PER_SEC = 1000000000.;

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();
        GL_CHECK_ERRORS;

        float4x4 camRotMatrix = mul(rotate_Y_4x4(-cam_rot[1]), rotate_X_4x4(+cam_rot[0]));
        float4x4 camTransMatrix = translate4x4(g_camPos);
        float4x4 rayMatrix = mul(camRotMatrix, camTransMatrix);
        program.SetUniform("g_rayMatrix", rayMatrix);

        program.SetUniform("g_screenWidth", WIDTH);
        program.SetUniform("g_screenHeight", HEIGHT);
        glUniform1i(glGetUniformLocation(program.GetProgram(), "texture_sampler"), 0);GL_CHECK_ERRORS;

        // очистка и заполнение экрана цветом
        //
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
        //
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;
        GLint loc = glGetUniformLocation(program.GetProgram(), "time");
        glUniform1f(loc, (std::chrono::system_clock::now() - time_start).count()/NANOSECS_PER_SEC);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations

        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    // очищаем vboи vao перед закрытием программы
    //
    glDeleteTextures(1, &glitter_texture);
    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);
  
    glfwTerminate();
    return 0;
}
