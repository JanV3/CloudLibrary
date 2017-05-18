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

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "point_cloud.hpp"

namespace cl {
    class VisualiserImpl {

        // Defines several possible options for camera movement. Used as abstraction to stay away from window-system
        // specific input methods
        enum class CameraMovement { Forward, Backward, Left, Right, Up, Down };

        // An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and
        // Matrices for use in OpenGL
        class Camera {
            // Default camera values
            const GLfloat SPEED = 10000.0f;
            const GLfloat SENSITIVTY = 0.15f;
            const GLfloat ZOOM = 45.0f;

        public:
            // Camera Attributes
            glm::vec3 Position;
            glm::vec3 Front;
            glm::vec3 Up;
            glm::vec3 Right;
            glm::vec3 WorldUp;
            // Eular Angles
            GLfloat Yaw;
            GLfloat Pitch;
            // Camera options
            GLfloat MovementSpeed;
            GLfloat MouseSensitivity;
            GLfloat Zoom;

            // Constructor with vectors
            Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                   GLfloat yaw = -90.0f, GLfloat pitch = 0.0f)
                : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
            {
                this->Position = position;
                this->WorldUp = up;
                this->Yaw = yaw;
                this->Pitch = pitch;
                this->updateCameraVectors();
            }
            // Constructor with scalar values
            Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw,
                   GLfloat pitch)
                : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
            {
                this->Position = glm::vec3(posX, posY, posZ);
                this->WorldUp = glm::vec3(upX, upY, upZ);
                this->Yaw = yaw;
                this->Pitch = pitch;
                this->updateCameraVectors();
            }

            // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
            glm::mat4 getViewMatrix()
            {
                return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
            }

            // Processes input received from any keyboard-like input system. Accepts input parameter in the form of
            // camera defined ENUM (to abstract it from windowing systems)
            void processKeyboard(CameraMovement direction, GLfloat deltaTime)
            {
                GLfloat velocity = this->MovementSpeed * deltaTime;
                if (direction == CameraMovement::Forward)
                    this->Position += this->Front * velocity;
                if (direction == CameraMovement::Backward)
                    this->Position -= this->Front * velocity;
                if (direction == CameraMovement::Left)
                    this->Position -= this->Right * velocity;
                if (direction == CameraMovement::Right)
                    this->Position += this->Right * velocity;
                if (direction == CameraMovement::Up)
                    this->Position += this->Up * velocity;
                if (direction == CameraMovement::Down)
                    this->Position -= this->Up * velocity;
            }

            // Processes input received from a mouse input system. Expects the offset value in both the x and y
            // direction.
            void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
            {
                xoffset *= this->MouseSensitivity;
                yoffset *= this->MouseSensitivity;

                this->Yaw += xoffset;
                this->Pitch += yoffset;

                // Make sure that when pitch is out of bounds, screen doesn't get flipped
                if (constrainPitch) {
                    if (this->Pitch > 89.0f)
                        this->Pitch = 89.0f;
                    if (this->Pitch < -89.0f)
                        this->Pitch = -89.0f;
                }

                // Update Front, Right and Up Vectors using the updated Eular angles
                this->updateCameraVectors();
            }

            // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
            void processMouseScroll(GLfloat yoffset)
            {
                if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
                    this->Zoom -= yoffset;
                if (this->Zoom <= 1.0f)
                    this->Zoom = 1.0f;
                if (this->Zoom >= 45.0f)
                    this->Zoom = 45.0f;
            }

        private:
            // Calculates the front vector from the Camera's (updated) Eular Angles
            void updateCameraVectors()
            {
                // Calculate the new Front vector
                glm::vec3 front;
                front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
                front.y = sin(glm::radians(this->Pitch));
                front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
                this->Front = glm::normalize(front);
                // Also re-calculate the Right and Up vector
                this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp)); // Normalize the vectors, because
                                                                                      // their length gets closer to 0
                                                                                      // the more you look up or down
                                                                                      // which results in slower
                                                                                      // movement.
                this->Up = glm::normalize(glm::cross(this->Right, this->Front));
            }
        };

#pragma pack(push, 1)
        struct Vertex {
            Vertex(GLfloat x, GLfloat y, GLfloat z) : x_(x), y_(y), z_(z){};
            GLfloat x_;
            GLfloat y_;
            GLfloat z_;
        };
        using Vertices = std::vector<Vertex>;
#pragma pack(pop)

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
        Camera camera_;

        glm::vec3 maxPoint;
        glm::vec3 minPoint;

        // last mouse positions
        double lastX = 0.0;
        double lastY = 0.0;

    public:
        VisualiserImpl() : windowName_("CL Visualiser"), width_(800), height_(600)
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

        void addPointCloud(std::string cloudName, PointCloud::Ptr cloud)
        {
            if (objects_.find(cloudName) != objects_.end())
                return;

            // new object
            Object object;
            object.size = cloud->size();

            // copy vertices to local buffer
            Vertices vertices;
            vertices.reserve(cloud->size());
            for (auto p = cloud->begin(); p != cloud->end(); ++p) {
                auto x = static_cast<GLfloat>(p->x);
                auto y = static_cast<GLfloat>(p->y);
                auto z = static_cast<GLfloat>(p->z);

                if (x > maxPoint.x)
                    maxPoint.x = x;
                if (y > maxPoint.y)
                    maxPoint.y = y;
                if (z > maxPoint.z)
                    maxPoint.z = z;

                if (x < minPoint.x)
                    minPoint.x = x;
                if (y < minPoint.y)
                    minPoint.y = y;
                if (z < minPoint.z)
                    minPoint.z = z;

                vertices.push_back({x, y, z});
            }

            glGenVertexArrays(1, &object.vao);
            glBindVertexArray(object.vao);
            glGenBuffers(1, &object.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat),
                         reinterpret_cast<const void *>(vertices.data()), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            objects_[cloudName] = object;

            // calculate speed of movement
            camera_.MovementSpeed = glm::distance(minPoint, maxPoint) / 3.0f;
        }

        void spin()
        {
            glm::mat4 model(1.0f);
            glm::mat4 view(1.0f);
            glm::mat4 projection(1.0f);
            glm::mat4 mvp(1.0f);

            double lastFrame = 0.0;
            while (!glfwWindowShouldClose(window_)) {

                // get time difference between frames
                double currentFrame = glfwGetTime();
                double timeDiff = currentFrame - lastFrame;
                lastFrame = currentFrame;

                // process keys
                if (!processKeys(static_cast<GLfloat>(timeDiff)))
                    break;

                // get window size
                glfwGetWindowSize(window_, &width_, &height_);

                // update view and projection matrix
                view = camera_.getViewMatrix();
                projection = glm::perspective(45.0f, (GLfloat)width_ / height_, 0.0f, 100000.0f);
                glm::mat4 mvp = projection * view * model;

                // Draw objects
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glUseProgram(program_);
                GLuint mvp_id = glGetUniformLocation(program_, "mvp");
                glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp[0][0]);
                for (auto &o : objects_) {
                    glBindVertexArray(o.second.vao);
                    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(o.second.size));
                    glBindVertexArray(0);
                }
                glUseProgram(0);

                // Swap buffers
                glfwSwapBuffers(window_);

                // Poll for and process events
                glfwPollEvents();
            }
        }

    private:
        std::string loadFile(std::string path)
        {
            std::ifstream file(path, std::ios::in);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot load shader file.");
            }
            return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }

        void loadShaders()
        {
            auto vss = loadFile("shaders/point_vertex.shader");
            const char *v_str = vss.c_str();
            GLuint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &v_str, NULL);
            glCompileShader(vs);

            auto fss = loadFile("shaders/point_fragment.shader");
            const char *f_str = fss.c_str();
            GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &f_str, NULL);
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

            // set mouse to disabled state
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
            glfwSetWindowSizeCallback(
                window_, [](GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); });

            lastX = 0.0;
            lastY = 0.0;

            maxPoint.x = std::numeric_limits<GLfloat>::min();
            maxPoint.y = std::numeric_limits<GLfloat>::min();
            maxPoint.z = std::numeric_limits<GLfloat>::min();
            minPoint.x = std::numeric_limits<GLfloat>::max();
            minPoint.y = std::numeric_limits<GLfloat>::max();
            minPoint.z = std::numeric_limits<GLfloat>::max();
        }

        bool processKeys(GLfloat diffTime)
        {
            if (glfwGetKey(window_, GLFW_KEY_W)) {
                camera_.processKeyboard(CameraMovement::Forward, diffTime);
            }

            if (glfwGetKey(window_, GLFW_KEY_S)) {
                camera_.processKeyboard(CameraMovement::Backward, diffTime);
            }

            if (glfwGetKey(window_, GLFW_KEY_A)) {
                camera_.processKeyboard(CameraMovement::Left, diffTime);
            }

            if (glfwGetKey(window_, GLFW_KEY_D)) {
                camera_.processKeyboard(CameraMovement::Right, diffTime);
            }

            if (glfwGetKey(window_, GLFW_KEY_E)) {
                camera_.processKeyboard(CameraMovement::Up, diffTime);
            }

            if (glfwGetKey(window_, GLFW_KEY_Q)) {
                camera_.processKeyboard(CameraMovement::Down, diffTime);
            }

            if (glfwGetKey(window_, GLFW_KEY_ESCAPE)) {
                return false;
            }

            // get mouse position
            double x;
            double y;
            glfwGetCursorPos(window_, &x, &y);

            if (lastY != 0.0 && lastX != 0.0) {
                camera_.processMouseMovement(static_cast<GLfloat>(x - lastX), static_cast<GLfloat>(lastY - y));
            }
            lastX = x;
            lastY = y;

            return true;
        }
    };
}

#endif // CL_VISUALISER_IMPL_HPP
