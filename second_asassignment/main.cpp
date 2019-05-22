//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include <ctime>
#include <chrono>
#include <sstream>
#include <fstream>
#include <string>

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

using namespace std;

static const GLsizei WIDTH = 1000, HEIGHT = 1000; //размеры окна
static constexpr int N_TRIANGLES = 5;
static constexpr int N_SQUARES = 4;
static constexpr int N_OCTAHEDRONS = 1;
static constexpr int N_BODIES = N_TRIANGLES+N_SQUARES+N_OCTAHEDRONS;

int initGL()
{
	int res = 0;
	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	return 0;
}

bool checkShaderCompileStatus(GLuint obj) {
  GLint status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE) {
    GLint length;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log((unsigned long)length);
    glGetShaderInfoLog(obj, length, &length, &log[0]);
    std::cerr << &log[0];
    return true;
  }
  return false;
}

bool checkProgramLinkStatus(GLuint obj) {
  GLint status;
  glGetProgramiv(obj, GL_LINK_STATUS, &status);
  if(status == GL_FALSE) {
    GLint length;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log((unsigned long)length);
    glGetProgramInfoLog(obj, length, &length, &log[0]);
    std::cerr << &log[0];
    return true;
  }
  return false;
}

GLuint prepareProgram(int id) {
  ifstream vsource(string("vertex (") + to_string(id+1) + string(").glsl"));
  stringstream vbuffer;
  vbuffer << vsource.rdbuf();

  std::string vertexShaderSource = vbuffer.str();

  GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
  const GLchar * const vertexShaderSourcePtr = vertexShaderSource.c_str();
  glShaderSource(vertexShaderId, 1, &vertexShaderSourcePtr, nullptr);
  glCompileShader(vertexShaderId);


  ifstream fsource(string("fragment (") + to_string(id+1) + string(").glsl"));
  stringstream fbuffer;
  fbuffer << fsource.rdbuf();

  std::string fragmentShaderSource = fbuffer.str();

  GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar * const fragmentShaderSourcePtr = fragmentShaderSource.c_str();
  glShaderSource(fragmentShaderId, 1, &fragmentShaderSourcePtr, nullptr);
  glCompileShader(fragmentShaderId);


  GLuint programId = glCreateProgram();
  glAttachShader(programId, vertexShaderId);
  glAttachShader(programId, fragmentShaderId);
  glLinkProgram(programId);


  glDeleteShader(vertexShaderId);
  glDeleteShader(fragmentShaderId);

  return programId;
}

int main(int argc, char** argv)
{
	if(!glfwInit())
    return -1;

	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 

  GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "Screensaver", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if(initGL() != 0) 
		return -1;
	
  //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();


  glfwSwapInterval(1); // force 60 frames per second
  
  GLuint programId[N_BODIES];
  for (int i = 0; i < N_BODIES; ++i) {
      programId[i] = prepareProgram(i);
  }
  
  //Создаем и загружаем геометрию поверхности
  //
  GLuint vbo;
  GLuint vao;
  {
    const float SIZE = 0.05;
    GLfloat trianglePos[] =
    {
      // equilateral triangle
      -SIZE, -SIZE/sqrt(3.0), 0.0f,
      0.0f, 2.0*SIZE/sqrt(3.0), 0.0f,
      SIZE, -SIZE/sqrt(3.0), 0.0f,
      
      // square
      -SIZE, -SIZE, 0.0,
      SIZE, -SIZE, 0.0,
      SIZE, SIZE, 0.0,
      
      -SIZE, -SIZE, 0.0,
      SIZE, SIZE, 0.0,
      -SIZE, SIZE, 0.0,
      
      // octahedron
      SIZE, 0.0, 0.0,
      0.0, SIZE, 0.0,
      0.0, 0.0, SIZE,
      
      SIZE, 0.0, 0.0,
      0.0, SIZE, 0.0,
      0.0, 0.0, -SIZE,
      
      SIZE, 0.0, 0.0,
      0.0, -SIZE, 0.0,
      0.0, 0.0, SIZE,
      
      SIZE, 0.0, 0.0,
      0.0, -SIZE, 0.0,
      0.0, 0.0, -SIZE,
      
      -SIZE, 0.0, 0.0,
      0.0, SIZE, 0.0,
      0.0, 0.0, SIZE,
      
      -SIZE, 0.0, 0.0,
      0.0, SIZE, 0.0,
      0.0, 0.0, -SIZE,
      
      -SIZE, 0.0, 0.0,
      0.0, -SIZE, 0.0,
      0.0, 0.0, SIZE,
      
      -SIZE, 0.0, 0.0,
      0.0, -SIZE, 0.0,
      0.0, 0.0, -SIZE,
    };
    vbo = 0;
    GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0

    glGenBuffers(1, &vbo);                                                        
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                                           
    glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePos), trianglePos, GL_STATIC_DRAW);              

    glGenVertexArrays(1, &vao);                                                    
    glBindVertexArray(vao);                                                        

    glBindBuffer(GL_ARRAY_BUFFER, vbo);                                           
    glEnableVertexAttribArray(vertexLocation);                                    
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);           

    glBindVertexArray(0);
  }

    float iTime = 0;
    auto time_start = std::chrono::system_clock::now();
    
	//цикл обработки сообщений и отрисовки сцены каждый кадр
	while (!glfwWindowShouldClose(window))
	{
        GL_CHECK_ERRORS;

        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        auto time_end = std::chrono::system_clock::now();
        iTime = (time_end - time_start).count()/3000000000.;        
        /*for (int i = 0; i < N_BODIES; ++i) {
            GLint loc = glGetUniformLocation(programId[i], "iTime");
            if (loc != -1)
            {
                glUniform1f(loc, iTime);
            }
        }*/
        

        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        
        // draw
        for (int i = 0; i < N_TRIANGLES; ++i) {
            glUseProgram(programId[i]); 
            GLint loc = glGetUniformLocation(programId[i], "iTime");
            glUniform1f(loc, iTime);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        
        for (int i = N_TRIANGLES; i < N_TRIANGLES + N_SQUARES; ++i) {
            glUseProgram(programId[i]); 
            GLint loc = glGetUniformLocation(programId[i], "iTime");
            glUniform1f(loc, iTime);
            glDrawArrays(GL_TRIANGLES, 3, 6);
        }
        
        for (int i = N_TRIANGLES + N_SQUARES; i < N_BODIES; ++i) {
            glUseProgram(programId[i]); 
            GLint loc = glGetUniformLocation(programId[i], "iTime");
            glUniform1f(loc, iTime);
            glDrawArrays(GL_TRIANGLES, 9, 24);
        }
        
        // close context
        glDisableVertexAttribArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
	}

	//очищаем vboи vao перед закрытием программы
  //
	glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1,      &vbo);

	glfwTerminate();
	return 0;
}
