#ifndef CL_VISUALISER_IMPL_HPP
#define CL_VISUALISER_IMPL_HPP

#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "point_cloud.hpp"
#include "algorithms.hpp"

namespace cl {

    /// @brief Declaration of window resize handler used in Visualiser implementation
    ///
    /// @param w pointer to window
    /// @param width new width of window
    /// @param height new height of window
    void onResize(GLFWwindow *w, int width, int height);

    /// @brief Declaration of mouse move handler used in Visualiser implementation
    ///
    /// @param w pointer to window
    /// @param x position of pointer on x-axis
    /// @param y position of pointer on y-axis
    void onMouseMove(GLFWwindow *w, double x, double y);

    /// @brief Declaration of mouse scroll handler used in Visualiser implementation
    ///
    /// @param w pointer to window
    /// @param x position of pointer on x-axis
    /// @param y position of pointer on y-axis
    void onMouseScroll(GLFWwindow *w, double x, double y);

    /// @brief Declaration of keypress handler used in Visualiser implementation
    ///
    /// @param w pointer to window
    /// @param key key code
    /// @param scancode system scan code
    /// @param action pressed/released/repeat
    /// @param mode modifiers
    void onKeyPressed(GLFWwindow *w, int key, int scancode, int action, int mode);

    /// @brief Visualiser provides a way to visualise point cloud in 3D space
    class VisualiserImpl {
        // OpenGL object
        struct Object {
            GLuint vbo;
            GLuint vao;
            size_t size;
        };
        using Objects = std::unordered_map<std::string, Object>;

        int width_;
        int height_;
        std::string windowName_;
        GLFWwindow *window_;
        GLuint program_;
        Objects objects_;

        // model parameters
        glm::mat4 modelRotateX;
        glm::mat4 modelRotateY;
        glm::mat4 modelRotateZ;
        glm::mat4 modelTranslate;
        glm::mat4 modelScale;

        // size of rendered points
        GLfloat pointSize;

        // last mouse positions
        double lastX;
        double lastY;

    public:
        VisualiserImpl()
            : windowName_("CL Visualiser")
            , width_(800)
            , height_(600)
        {
            init();
        };

        VisualiserImpl(std::string name, int width = 800, int height = 600)
            : windowName_(name), width_(width), height_(height)
        {
            init();
        };

        ~VisualiserImpl()
        {
            for (auto &o : objects_) {
                glDeleteVertexArrays(1, &o.second.vao);
                glDeleteBuffers(1, &o.second.vbo);
            }
            glDeleteProgram(program_);

            glfwDestroyWindow(window_);
            glfwTerminate();
        }

        /// @brief This function append given point cloud to visualiser
        ///
        /// @param cloudName name of point cloud (must be unique)
        /// @param cloud pointer to point cloud
        void addPointCloud(std::string cloudName, PointCloud::Ptr cloud)
        {
            if (objects_.find(cloudName) != objects_.end())
                return;

            // new object
            Object object;
            object.size = cloud->size();

            glGenVertexArrays(1, &object.vao);
            glBindVertexArray(object.vao);
            glGenBuffers(1, &object.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
            glBufferData(GL_ARRAY_BUFFER, cloud->size() * 3 * sizeof(GLfloat),
                         reinterpret_cast<const void *>(cloud->data()), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            objects_[cloudName] = object;

            // center object on screen
            auto centroid = cl::centroid(*cloud);
            glm::vec2 centroid2D(centroid.x, centroid.y);
            glm::vec2 screenCenter(width_/2.0f, height_/2.0f);
            auto trans = screenCenter - centroid2D;
            modelTranslate = glm::translate(modelTranslate, glm::vec3(trans.x, trans.y, 0));
        }

        /// @brief This function should be called when user wants to display uploaded point cloud in created window
        void spin()
        {
            double lastFrame = 0.0;
            while (!glfwWindowShouldClose(window_)) {

                // get time difference between frames
                double currentFrame = glfwGetTime();
                double timeDiff = currentFrame - lastFrame;
                lastFrame = currentFrame;

                // process keys
                if (!processKeys(static_cast<GLfloat>(timeDiff)))
                    break;

                // Clear screen
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Draw QUAD with color gradient
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                glBegin(GL_QUADS);
                glColor4f(0.3, 0.3, 0.3, 1.0);
                glVertex3i(-1, -1, -1);
                glVertex3i(1, -1, -1);
                glColor4f(0.1, 0.1, 0.1, 1.0);
                glVertex3i(1, 1, -1);
                glVertex3i(-1, 1, -1);
                glEnd();
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();

                // use main shader program
                glUseProgram(program_);

                // Create model matrix
                glm::mat4 model = modelTranslate * modelRotateZ * modelRotateY * modelRotateZ * modelScale;

                // Get camera matrix
                glm::mat4 view(1.0f);

                // Create projection matrix
                //glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width_ / height_, 0.0f, 100000.0f);
                glm::mat4 projection = glm::ortho(
                    0.0f,
                    static_cast<float>(width_),
                    static_cast<float>(height_),
                    0.0f,
                    -1000.0f,
                    1000.0f
                );

                // Create and apply MVP matrix
                glm::mat4 mvp = projection * view * model;
                GLuint mvp_id = glGetUniformLocation(program_, "mvp");
                glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp[0][0]);

                // Set point size
                GLuint pointSize_id = glGetUniformLocation(program_, "pointSize");
                glUniform1f(pointSize_id, pointSize);

                // draw all objects to screen
                for (auto &o : objects_) {
                    glBindVertexArray(o.second.vao);
                    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(o.second.size));
                    glBindVertexArray(0);
                }

                // unset program
                glUseProgram(0);

                // Swap buffers
                glfwSwapBuffers(window_);

                // Poll for and process events
                glfwPollEvents();
            }
        }

        friend void onResize(GLFWwindow *w, int width, int height)
        {
            glViewport(0, 0, width, height);
            auto self = static_cast<VisualiserImpl *>(glfwGetWindowUserPointer(w));
            self->width_ = width;
            self->height_ = height;
        }

        friend void onMouseMove(GLFWwindow *w, double x, double y)
        {
            auto self = static_cast<VisualiserImpl *>(glfwGetWindowUserPointer(w));

            if (self->lastY != 0.0 && self->lastX != 0.0) {
                float offsetX = x - self->lastX;
                float offsetY = y - self->lastY;

                int rightState = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT);
                if (rightState == GLFW_PRESS)
                    self->modelTranslate = glm::translate(self->modelTranslate, glm::vec3(offsetX, offsetY, 0.0f));
            }

            self->lastX = x;
            self->lastY = y;
        }

        friend void onKeyPressed(GLFWwindow *w, int key, int scancode, int action, int mode)
        {
            auto self = static_cast<VisualiserImpl *>(glfwGetWindowUserPointer(w));
            if (action != GLFW_PRESS)
                return;

            if (key == GLFW_KEY_KP_ADD) {
                self->pointSize += 1.0f;
            }

            if (key == GLFW_KEY_KP_SUBTRACT) {
                self->pointSize -= 1.0f;
                if (self->pointSize < 1.0f)
                    self->pointSize = 1.0f;
            }
        }

        friend void onMouseScroll(GLFWwindow *w, double x, double y)
        {
            auto self = static_cast<VisualiserImpl *>(glfwGetWindowUserPointer(w));
            if (y < 0)
                self->modelScale = glm::scale(self->modelScale, glm::vec3(0.9f));
            else
                self->modelScale = glm::scale(self->modelScale, glm::vec3(1.1f));
        }

    private:
        void loadShaders()
        {
            std::string v_str("#version 330 core\n"
                              "layout (location = 0) in vec3 position;\n"
                              "uniform mat4 mvp;\n"
                              "uniform float pointSize;\n"
                              "void main()\n"
                              "{\n"
                              "    gl_PointSize = pointSize;\n"
                              "    gl_Position = mvp * vec4(position, 1.0f);\n"
                              "}\n");
            GLuint vs = glCreateShader(GL_VERTEX_SHADER);
            char const *v_str_ptr = v_str.c_str();
            glShaderSource(vs, 1, &v_str_ptr, NULL);
            glCompileShader(vs);

            std::string f_str("#version 330 core\n"
                              "out vec4 color;\n"
                              "void main()\n"
                              "{\n"
                              "    color = vec4(1.0f, 0.8f, 0.2f, 1.0f);\n"
                              "}\n");
            GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
            char const *f_str_ptr = f_str.c_str();
            glShaderSource(fs, 1, &f_str_ptr, NULL);
            glCompileShader(fs);

            // create program
            program_ = glCreateProgram();
            glAttachShader(program_, vs);
            glAttachShader(program_, fs);
            glLinkProgram(program_);
            glDetachShader(program_, vs);
            glDetachShader(program_, fs);
            glDeleteShader(vs);
            glDeleteShader(fs);
        }

        void init()
        {
            modelScale = glm::mat4(1.0f);
            modelTranslate = glm::mat4(1.0f);
            modelRotateX = glm::mat4(1.0f);
            modelRotateY = glm::mat4(1.0f);
            modelRotateZ = glm::mat4(1.0f);
            lastX = 0.0;
            lastY = 0.0;
            pointSize = 1.0f;

            // initialize the GLFW library
            if (!glfwInit())
                throw std::runtime_error("Cannot initialise GLFW.");

            glfwSetErrorCallback([](int error, const char *message) {
                std::cout << "Error: " << error << " Message: " << message << std::endl;
            });

            // initialize GLFW window
            window_ = glfwCreateWindow(width_, height_, windowName_.c_str(), NULL, NULL);
            if (window_ == nullptr) {
                glfwTerminate();
                throw std::runtime_error("Cannot create GLFW window.");
            }
            glfwMakeContextCurrent(window_);

            // set current object as user pointer to GLFW
            glfwSetWindowUserPointer(window_, this);

            // set mouse to disabled state
            // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Initialize GLEW
            glewExperimental = GL_TRUE;
            GLenum err = glewInit();
            if (err != GLEW_OK)
                throw std::runtime_error("Failed to initialize GLEW\n");

            // compile shaders and create program
            loadShaders();

            // set clear color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glEnable(GL_PROGRAM_POINT_SIZE);

            // set callback function on resize
            glfwSetWindowSizeCallback(window_, onResize);

            // set mouse move callback
            glfwSetCursorPosCallback(window_, onMouseMove);

            // set keypress callback
            glfwSetKeyCallback(window_, onKeyPressed);

            // set scroll callback
            glfwSetScrollCallback(window_, onMouseScroll);
        }

        bool processKeys(GLfloat diffTime)
        {
            if (glfwGetKey(window_, GLFW_KEY_ESCAPE)) {
                return false;
            }

            return true;
        }
    };
}

#endif // CL_VISUALISER_IMPL_HPP
