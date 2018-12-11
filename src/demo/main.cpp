#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include "shader_program.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "index_buffer.h"

#include <iostream>
#include <vector>

unsigned int WINDOW_WIDTH  = 640;
unsigned int WINDOW_HEIGHT = 480;

std::vector<float>        vertices;
std::vector<unsigned int> indices;

bool space_pressed = false;
bool run_algorithm = true;

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    std::cout << "-- Mouse button callback --" << std::endl;
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            std::cout << "-- left button pressed, pos: " << xpos << " " << ypos << " --" << std::endl;
            xpos = (xpos / WINDOW_WIDTH - 0.5) * 2;
            ypos = ((WINDOW_HEIGHT - ypos) / WINDOW_HEIGHT - 0.5) * 2;

            std::cout << "-- normalized coord: " << xpos << " " << ypos << " --" << std::endl;

            vertices.push_back(xpos);
            vertices.push_back(ypos);

            if (vertices.size() == 2 || vertices.size() == 4)
                indices.push_back(vertices.size() / 2 - 1);
            else if (vertices.size() > 4)
            {
                indices.push_back(vertices.size() / 2 - 2);
                indices.push_back(vertices.size() / 2 - 1);
            }
        }
    }
}

void handle_input(GLFWwindow * window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !space_pressed)
    {
        std::cout << "SPACE PRESSED" << std::endl;
        space_pressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && space_pressed)
    {
        std::cout << "SPACE RELEASED" << std::endl;
        space_pressed = false;
    }
}

void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    glViewport(0, 0, width, height);
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Error glew init" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    shader_program program("../../../shaders/vertex_shaders/lines.vs", "../../../shaders/fragment_shaders/lines.fs");

    std::vector<float> vert_axes {
        -1.0,  0.0,
         1.0, -0.0,
         0.0, -1.0,
         0.0,  1.0
    };

    vertex_array va_axes;
    vertex_buffer vb_axes(vert_axes.data(), sizeof(float) * vert_axes.size());
    vertex_buffer_layout layout;
    push<float>(layout, 2);
    va_axes.add_buffer(vb_axes, layout);

    vertex_array va_points;
    vertex_buffer vb_points;
    index_buffer ib_points;
    va_points.add_buffer(vb_points, layout);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Handle keyboard input */
        handle_input(window);

        /* Render here */
        glClearColor(160.0 / 255.0, 253.0 / 255.0, 255.0 / 255.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        program.use();
        vb_axes.bind();
        va_axes.bind();
        glDrawArrays(GL_LINES, 0, 4);
        
        program.use();
        if (vertices.size() == 2)
        {
            vb_points.bind();
            vb_points.buffer_data(vertices.data(), sizeof(float) * vertices.size());
            va_points.bind();
            glDrawArrays(GL_POINTS, 0, vertices.size() / 2);
        }
        else if (vertices.size() > 2)
        {
            ib_points.bind();
            ib_points.buffer_data(indices.data(), indices.size());
            vb_points.bind();
            vb_points.buffer_data(vertices.data(), sizeof(float) * vertices.size());
            va_points.bind();
            glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}