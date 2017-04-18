//
// Created by janko on 17.4.2017.
//

#ifndef TESTAPP_VISUALISER_H
#define TESTAPP_VISUALISER_H

#include <memory>
#include <unordered_map>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <GLFW/glfw3.h>

#include "camera.h"
#include "cl.hpp"

namespace cl {
    struct GLFWWindowDeleter {
        void operator()(GLFWwindow* ptr){
             glfwDestroyWindow(ptr);
        }
    };

    using GLFWSmartWindow = std::unique_ptr<GLFWwindow, GLFWWindowDeleter>;

#pragma pack(push, 1)
    struct Vertex {
        Vertex(GLfloat x, GLfloat y, GLfloat z) : _x(x), _y(y), _z(z) {};
        GLfloat _x;
        GLfloat _y;
        GLfloat _z;
    };
    using Vertices = std::vector<Vertex>;
    using VerticesPtr = std::unique_ptr<Vertices>;
#pragma pack(pop)

    struct Object {
        GLuint vbo;
        GLuint vao;
        size_t size;
        //VerticesPtr vertices;
    };
    using Objects = std::unordered_map<std::string, Object>;




    class Visualiser {
        size_t _width;
        size_t _height;
        std::string _windowName;
        GLFWSmartWindow _window;
        GLuint _program;
        Objects _objects;
        // single camera object
        Camera _camera;

        // last mouse positions
        double _lastMouseX = 0.0;
        double _lastMouseY = 0.0;

    public:
        Visualiser() : _windowName("CL Visualiser"), _width(800), _height(600)
        {
            init();
        };

        Visualiser(std::string name, size_t width = 800, size_t height = 600)
                : _windowName(name), _width(width), _height(height)
        {
            init();
        };

        ~Visualiser(){
            for(auto& o : _objects){
                glDeleteBuffers(1, &o.second.vbo);
                glDeleteVertexArrays(1, &o.second.vao);
            }
            glDeleteProgram(_program);
            glfwTerminate();
        }


        void addPointCloud(std::string cloudName, PointCloud::Ptr cloud)
        {
            if(_objects.find(cloudName) != _objects.end())
                return;

            // new object
            Object object;
            object.size = cloud->size();

            // copy vertices to local buffer
            Vertices vertices;
            //object.vertices = std::make_unique<Vertices>();
            vertices.reserve(cloud->size());
            for(auto p = cloud->begin(); p != cloud->end(); ++p) {
                vertices.push_back({static_cast<GLfloat>(p->x()),
                                     static_cast<GLfloat>(p->y()),
                                     static_cast<GLfloat>(p->z())});
            }

            glGenVertexArrays(1, &object.vao);
            glBindVertexArray(object.vao);
            glGenBuffers(1, &object.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat), reinterpret_cast<const void *>(vertices.data()), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);


            _objects[cloudName] = std::move(object);
        }

        void spin()
        {

            glm::mat4 model;
            //model = glm::rotate(model, -55.0f, glm::vec3(1.0f, 0.0f, 0.0f));

            glm::mat4 view;
            // Note that we're translating the scene in the reverse direction of where we want to move
            //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

            glm::mat4 projection;
            projection = glm::perspective(45.0f, (GLfloat )800 / 600, 0.1f, 100.0f);

            GLfloat lastFrame = 0.0f;  	// Time of last frame

            /* Loop until the user closes the window */
            while (!glfwWindowShouldClose(_window.get()))
            {
                // process keys
                if(!processKeys())
                    break;

                /* Render here */
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glUseProgram(_program);

                GLuint modelLoc = glGetUniformLocation(_program, "model");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                GLuint viewLoc = glGetUniformLocation(_program, "view");
                glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(_camera.GetViewMatrix()));

                GLuint projectionLoc = glGetUniformLocation(_program, "projection");
                glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

                for(auto& o : _objects) {
                    glBindVertexArray(o.second.vao);
                    glDrawArrays(GL_POINTS, 0, o.second.size);
                    glBindVertexArray(0);
                }

                glUseProgram(0);

                /* Swap front and back buffers */
                glfwSwapBuffers(_window.get());

                /* Poll for and process events */
                glfwPollEvents();
            }
        }

    private:
        void loadShaders()
        {
            std::ifstream vertexStream("shaders/point_vertex.shader", std::ios::in);
            if(!vertexStream.is_open()){
                throw std::runtime_error("Cannot load vertex shader.");
            }
            std::string vertexShaderText ((std::istreambuf_iterator<char>(vertexStream)),
                            std::istreambuf_iterator<char>());
            const char *c_str = vertexShaderText.c_str();

            GLuint _vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(_vertexShader, 1, &c_str, NULL);
            glCompileShader(_vertexShader);


            std::ifstream fragmentStream("shaders/point_fragment.shader", std::ios::in);
            if(!fragmentStream.is_open()){
                throw std::runtime_error("Cannot load fragment shader.");
            }
            std::string fragmentShaderText ((std::istreambuf_iterator<char>(fragmentStream)),
                                          std::istreambuf_iterator<char>());
            c_str = fragmentShaderText.c_str();

            GLuint _fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(_fragmentShader, 1, &c_str, NULL);
            glCompileShader(_fragmentShader);


            // create program
            _program = glCreateProgram();
            glAttachShader(_program, _vertexShader);
            glAttachShader(_program, _fragmentShader);
            glLinkProgram(_program);
            glDeleteShader(_vertexShader);
            glDeleteShader(_fragmentShader);
        }

        void init()
        {
            // initialize the GLFW library
            if (!glfwInit())
                throw std::runtime_error("Cannot initialise GLFW.");

            glfwSetErrorCallback([](int error, const char* message){
                std::cout << "Error: " << error << " Message: " << message << std::endl;
            });

            // initialize GLFW window
            _window.reset(glfwCreateWindow(_width, _height, _windowName.c_str(), NULL, NULL));
            if (_window == nullptr) {
                glfwTerminate();
                throw std::runtime_error("Cannot create GLFW window.");
            }
            glfwMakeContextCurrent(_window.get());

            // set mouse to disabled state
            glfwSetInputMode(_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // compile shaders and create program
            loadShaders();

            // set clear color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glEnable(GL_PROGRAM_POINT_SIZE);

            // set callback function on resize
            glfwSetWindowSizeCallback(_window.get(), [](GLFWwindow* window, int width, int height) {
                glViewport(0, 0, width, height);
            });

            _lastMouseX = 0.0;
            _lastMouseY = 0.0;

            // set mouse button callback
            glfwSetMouseButtonCallback(_window.get(), [](GLFWwindow* window, int button, int state, int mode){
                std::cout << "Mouse button: " << button << " State: " << state << " Mode: " << mode << std::endl;
            });
        }

        bool processKeys(){
            if(glfwGetKey(_window.get(), GLFW_KEY_W)){
                _camera.ProcessKeyboard(CameraMovement::Forward, 0.0015);
            }

            if(glfwGetKey(_window.get(), GLFW_KEY_S)) {
                _camera.ProcessKeyboard(CameraMovement::Backward, 0.0015);
            }

            if(glfwGetKey(_window.get(), GLFW_KEY_A)){
                _camera.ProcessKeyboard(CameraMovement::Left, 0.0015);
            }

            if(glfwGetKey(_window.get(), GLFW_KEY_D)){
                _camera.ProcessKeyboard(CameraMovement::Right, 0.0015);
            }

            if(glfwGetKey(_window.get(), GLFW_KEY_ESCAPE)){
                return false;
            }

            // get mouse position
            double x;
            double y;
            glfwGetCursorPos(_window.get(), &x, &y);

            if(_lastMouseY != 0.0 && _lastMouseX != 0.0) {
                _camera.ProcessMouseMovement(x - _lastMouseX, _lastMouseY - y);
            }
            _lastMouseX = x;
            _lastMouseY = y;

            return true;
        }
    };
}

#endif //TESTAPP_VISUALISER_H
