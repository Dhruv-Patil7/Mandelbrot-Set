#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>

// Setting initial values 
double centerX = -0.75;
double centerY = 0.0;
double zoom = 1.0;
bool dragging = false;

double lastMouseX = 0.0;
double lastMouseY = 0.0;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            dragging = true;

            glfwGetCursorPos(window,
                             &lastMouseX,
                             &lastMouseY);
        }
        else if (action == GLFW_RELEASE)
        {
            dragging = false;
        }
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    double u = mouseX / width;
    double v = mouseY / height;

    // View size BEFORE zoom
    double aspect = (double)width / height;

    double oldHeight = 3.0 / zoom;
    double oldWidth = oldHeight * aspect;

    // Change zoom
    if (yoffset > 0)
        zoom *= 1.1;
    else if (yoffset < 0)
        zoom /= 1.1;

    double newHeight = 3.0 / zoom;
    double newWidth = newHeight * aspect;

    // Shift camera so the point under the mouse stays fixed
    centerX += (u - 0.5) * (oldWidth - newWidth);
    centerY -= (v - 0.5) * (oldHeight - newHeight);
}

std::string ReadFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file)
    {
        throw std::runtime_error("Failed to open " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

GLuint CompileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);

    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);

        std::cerr << log << std::endl;
    }

    return shader;
}

GLuint CreateShaderProgram()
{
    std::string vertexSource = ReadFile("../shaders/mandelbrot.vert");
    std::string fragmentSource = ReadFile("../shaders/mandelbrot.frag");

    GLuint vertexShader =
        CompileShader(GL_VERTEX_SHADER, vertexSource);

    GLuint fragmentShader =
        CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);

        std::cerr << log << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Request OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Creating Window
    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "Mandelbrot GPU",
        nullptr,
        nullptr);

    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Load OpenGL
    int version = gladLoadGL(glfwGetProcAddress);


    if (version == 0)
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint shaderProgram = CreateShaderProgram();

    GLint centerLoc = glGetUniformLocation(shaderProgram, "center");
    GLint zoomLoc = glGetUniformLocation(shaderProgram, "zoom");
    GLint resolutionLoc = glGetUniformLocation(shaderProgram, "resolution");
    GLint iterationLoc = glGetUniformLocation(shaderProgram, "maxIterations");

    float vertices[] = {
    -1.0f,-1.0f,
     1.0f,-1.0f,
    -1.0f, 1.0f,

    -1.0f, 1.0f,
     1.0f,-1.0f,
     1.0f, 1.0f
    };

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                sizeof(vertices),
                vertices,
                GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0);

    glEnableVertexAttribArray(0);

    std::cout << "Renderer : "
              << glGetString(GL_RENDERER)
              << '\n';

    std::cout << "OpenGL : "
              << glGetString(GL_VERSION)
              << '\n';
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //if for panning around
        if (dragging)
        {
            double mouseX, mouseY;

            glfwGetCursorPos(window, &mouseX, &mouseY);

            double dx = mouseX - lastMouseX;
            double dy = mouseY - lastMouseY;

            double aspect = (double)width / height;

            double viewHeight = 3.0 / zoom;
            double viewWidth  = viewHeight * aspect;

            centerX -= dx * viewWidth / width;
            centerY += dy * viewHeight / height;

            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        glUseProgram(shaderProgram);

        int iterations = std::min(
            1000,
            std::max(
                500,
                static_cast<int>(500 + 40 * std::log2(zoom))
            )
        );


        glUniform2d(centerLoc, centerX, centerY);
        glUniform2f(resolutionLoc, (float)width, (float)height);
        glUniform1i(iterationLoc, iterations);
        glUniform1d(zoomLoc, zoom);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}