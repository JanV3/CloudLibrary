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

		struct CameraVectors
		{
			glm::vec3 front;
			glm::vec3 right;
			glm::vec3 up;
		};

		class CameraFPS {
		protected:
			CameraVectors getInitialVectors()
			{
				CameraVectors cv;
				cv.front = glm::vec3(0.0f, 0.0f, -1.0f);
				cv.up = glm::vec3(0.0f, 1.0f, 0.0f);
				return cv;
			}

			void updateVectors(GLfloat& yaw, GLfloat& pitch, glm::vec3& worldUp, CameraVectors& cv)
			{
				if (pitch > 89.0f)
					pitch = 89.0f;
				if (pitch < -89.0f)
					pitch = -89.0f;

				// Calculate the new Front vector
				glm::vec3 front;
				front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
				front.y = sin(glm::radians(pitch));
				front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

				cv.front = glm::normalize(front);
				// Also re-calculate the Right and Up vector
				cv.right = glm::normalize(glm::cross(cv.front, worldUp));
				cv.up = glm::normalize(glm::cross(cv.right, cv.front));
			}
		};

		class CameraFlight {
		protected:
			CameraVectors getInitialVectors()
			{
				CameraVectors cv;
				cv.front = glm::vec3(0.0f, 0.0f, -1.0f);
				cv.up = glm::vec3(0.0f, 1.0f, 0.0f);
				return cv;
			}

			void updateVectors(GLfloat& yaw, GLfloat& pitch, glm::vec3& worldUp, CameraVectors& cv)
			{
				GLfloat roll = 0.0f;

				// Fuck quaternions
				glm::fquat pitchQuat(cos(glm::radians(pitch / 2.0f)), cv.right * (float)sin(glm::radians(pitch / 2.0f)));
				glm::fquat yawQuat(cos(glm::radians(yaw / 2.0f)), cv.up * (float)sin(glm::radians(yaw / 2.0f)));
				glm::fquat rollQuat(cos(glm::radians(roll / 2.0f)), cv.front * (float)sin(glm::radians(roll / 2.0f)));

				auto rotation = yawQuat * pitchQuat * rollQuat;

				cv.front = rotation * cv.front * glm::conjugate(rotation);
				cv.up = rotation * cv.up * glm::conjugate(rotation);
				cv.right = glm::cross(cv.front, cv.up);
			}
		};

        // An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and
        // Matrices for use in OpenGL
		template<typename CameraPolicy>
        class Camera : public CameraPolicy {

			using CameraPolicy::getInitialVectors;
			using CameraPolicy::updateVectors;

            // Default camera values
            const GLfloat SPEED = 10000.0f;
            const GLfloat SENSITIVTY = 0.15f;

        public:
            // Camera Attributes
            glm::vec3 position;
            glm::vec3 worldUp;
            CameraVectors cameraVectors;

            // Eular Angles
            GLfloat yaw;
            GLfloat pitch;

            // Camera options
            GLfloat movementSpeed;
            GLfloat mouseSensitivity;

            // Constructor with vectors
            Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), GLfloat yaw = -90.0f, GLfloat pitch = 0.0f)
                : movementSpeed(SPEED), mouseSensitivity(SENSITIVTY)
            {
                this->position = position;
                this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
                this->yaw = yaw;
                this->pitch = pitch;

				cameraVectors = getInitialVectors();

				updateVectors(yaw, pitch, worldUp, cameraVectors);
            }

            // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
            glm::mat4 getViewMatrix()
            {
                return glm::lookAt(position, position + this->cameraVectors.front, this->cameraVectors.up);
            }

            // Processes input received from any keyboard-like input system. Accepts input parameter in the form of
            // camera defined ENUM (to abstract it from windowing systems)
            void processKeyboard(CameraMovement direction, GLfloat deltaTime)
            {
                GLfloat velocity = this->movementSpeed * deltaTime;
                if (direction == CameraMovement::Forward)
                    position += cameraVectors.front * velocity;
                if (direction == CameraMovement::Backward)
                    position -= cameraVectors.front * velocity;
                if (direction == CameraMovement::Left)
                    position -= cameraVectors.right * velocity;
                if (direction == CameraMovement::Right)
                    position += cameraVectors.right * velocity;
                if (direction == CameraMovement::Up)
                    position += cameraVectors.up * velocity;
                if (direction == CameraMovement::Down)
                    position -= cameraVectors.up * velocity;
            }

            // Processes input received from a mouse input system. Expects the offset value in both the x and y
            // direction.
            void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = false)
            {
                xoffset *= mouseSensitivity;
                yoffset *= mouseSensitivity;

                yaw += xoffset;
                pitch += yoffset;

                // Update Front, Right and Up Vectors using the updated Eular angles
				updateVectors(yaw, pitch, worldUp, cameraVectors);
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
        Camera<CameraFlight> camera_;

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
            camera_.movementSpeed = glm::distance(minPoint, maxPoint) / 3.0f;
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

		friend void onResize(GLFWwindow *w, int width, int height) {
			glViewport(0, 0, width, height);
			auto self = static_cast<VisualiserImpl*>(glfwGetWindowUserPointer(w));
			self->width_ = width;
			self->height_ = height;
		}

		friend void onMouseMove(GLFWwindow *w, double x, double y) {
			auto self = static_cast<VisualiserImpl*>(glfwGetWindowUserPointer(w));

			if (self->lastY != 0.0 && self->lastX != 0.0) {
				self->camera_.processMouseMovement(static_cast<GLfloat>(x - self->lastX), static_cast<GLfloat>(self->lastY - y));
			}
			self->lastX = x;
			self->lastY = y;
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

			// set current object as user pointer to GLFW
			glfwSetWindowUserPointer(window_, this);

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
			glfwSetWindowSizeCallback(window_, onResize);

			// set mouse move callback
			glfwSetCursorPosCallback(window_, onMouseMove);

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

            return true;
        }
    };
}

#endif // CL_VISUALISER_IMPL_HPP
