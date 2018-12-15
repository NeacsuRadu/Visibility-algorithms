#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include "shader_program.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "index_buffer.h"
#include "algorithms.h"
#include "circular_list.h"
#include "triangulation.h"

#include <iostream>
#include <vector>

unsigned int WINDOW_WIDTH  = 640;
unsigned int WINDOW_HEIGHT = 480;

std::vector<float>        vertices;
std::vector<unsigned int> indices;

std::vector<float>        vertices_visi;
std::vector<unsigned int> indices_visi;

std::vector<float>        vertices_triangualtion;

bool space_pressed = false;

bool triangulation = true;

void cursor_pos_to_xoy(double width, double height, double& x, double& y)
{
    // coordonatele cursorului sunt masurate din coltul stanga sus al ferestrei ( x : 0 - width, y : 0 - height ) 
    // pentru a le folosi trebuie sa le transformam in - 1.0 : 1.0 cu originea in centrul ferestrei 
    x = (x / width - 0.5) * 2;
    y = ((height - y) / height - 0.5) * 2;
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action != GLFW_PRESS)
        return;

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    cursor_pos_to_xoy(WINDOW_WIDTH, WINDOW_HEIGHT, mouse_x, mouse_y);
    if (space_pressed && !triangulation)
    {
        vertices_visi.clear();
        indices_visi.clear();
        try
        {
            vertices_visi = get_simple_polygon_visibility(vertices, mouse_x, mouse_y);
        }
        catch(const std::runtime_error& er)
        {
            std::cout << er.what() << std::endl;
        }
        if (vertices_visi.size() > 1)
        {
            vertices_visi.push_back(mouse_x);
            vertices_visi.push_back(mouse_y);


            for (unsigned int idx = 0; idx < vertices_visi.size()/2 -2; ++idx)
            {
                indices_visi.push_back(idx);
                indices_visi.push_back(idx + 1);
                indices_visi.push_back(vertices_visi.size()/2 - 1);
            }
            indices_visi.push_back(0);
            indices_visi.push_back(vertices_visi.size()/2 - 2);
            indices_visi.push_back(vertices_visi.size()/2 - 1);
        }
    }
    else if (!space_pressed)
    {
        vertices.push_back(mouse_x);
        vertices.push_back(mouse_y);

        if (vertices.size() == 2 || vertices.size() == 4)
            indices.push_back(vertices.size() / 2 - 1);
        else if (vertices.size() > 4)
        {
            indices.push_back(vertices.size() / 2 - 2);
            indices.push_back(vertices.size() / 2 - 1);
        }
    }
}

void handle_input(GLFWwindow * window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !space_pressed)
    {
        space_pressed = true;
        indices.push_back(0);
        indices.push_back(vertices.size() / 2 - 1);
        if (triangulation)
        {
            vertices_triangualtion = get_triangulation(vertices);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        space_pressed = false;
        indices.clear();
        vertices.clear();
        indices_visi.clear();
        vertices_visi.clear();
    }
}

void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    glViewport(0, 0, width, height);
}

int test();
int main(void)
{
    //return test();
    
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo", NULL, NULL);
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

    vertex_array va_visi;
    vertex_buffer vb_visi;
    index_buffer ib_visi;
    va_visi.add_buffer(vb_visi, layout);

    vertex_array va_triangulation;
    vertex_buffer vb_triangulation;
    va_triangulation.add_buffer(vb_triangulation, layout);


    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Handle keyboard input */
        handle_input(window);

        /* Clear the screen */
        glClearColor(160.0f / 255.0f, 253.0f / 255.0f, 255.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Draw xoy axes */
        program.use();
        va_axes.bind();
        vb_axes.bind();
        glDrawArrays(GL_LINES, 0, 4);
        
        /* Draw polygon */
        program.use();
        if (vertices.size() == 2)
        {
            va_points.bind();
            vb_points.bind();
            vb_points.buffer_data(vertices.data(), sizeof(float) * vertices.size());
            glDrawArrays(GL_POINTS, 0, vertices.size() / 2);
        }
        else if (vertices.size() > 2)
        {
            va_points.bind();
            ib_points.bind();
            ib_points.buffer_data(indices.data(), indices.size());
            vb_points.bind();
            vb_points.buffer_data(vertices.data(), sizeof(float) * vertices.size());
            glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
        }

        /* Draw visibility is possible */
        if (space_pressed && vertices_visi.size() > 1)
        {
            if (!triangulation)
            {
                program.use();
                va_visi.bind();
                ib_visi.bind();
                ib_visi.buffer_data(indices_visi.data(), indices_visi.size());
                vb_visi.bind();
                vb_visi.buffer_data(vertices_visi.data(), sizeof(float) * vertices_visi.size());
                glDrawElements(GL_TRIANGLES, indices_visi.size(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
            }
        }

        /* Draw triangles */
        if (triangulation) 
        {
            program.use();
            va_triangulation.bind();
            vb_triangulation.bind();
            vb_triangulation.buffer_data(vertices_triangualtion.data(), sizeof(float) * vertices_triangualtion.size());
            glDrawArrays(GL_TRIANGLES, 0, vertices_triangualtion.size() / 2);
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void afisare(const triangle * tr, bool dual = true)
{
    if (!tr)
    {
        std::cout << "tr is null" << std::endl;
        return;
    }

    std::cout << tr->e1->a.x << " " << tr->e1->a.y << " " << tr->e1->b.x << " " << tr->e1->b.y << " : "
              << tr->e2->a.x << " " << tr->e2->a.y << " " << tr->e2->b.x << " " << tr->e2->b.y << " : "
              << tr->e3->a.x << " " << tr->e3->a.y << " " << tr->e3->b.x << " " << tr->e3->b.y << std::endl; 

    if (!dual)
       return;

    std::cout << "e1: ";
    if (tr->e1->dual)
        afisare(tr->e1->dual->tri, false);
    std::cout << std::endl << "e2: ";
    if (tr->e2->dual)
        afisare(tr->e2->dual->tri, false);
    std::cout << std::endl << "e3: ";
    if (tr->e3->dual)
        afisare(tr->e3->dual->tri, false);
    std::cout << std::endl;
}

int test()
{
    std::vector<point> points = {
        {1.0, 2.0},
        {0.0, 1.0},
        {-1.0, 2.0},
        {-2.0, 2.0},
        {-1.0, 0.0},
        {-1.0, -1.0},
        {1.0, -1.0}
    };

    auto triangles = get_triangulation(points);
    std::cout << triangles.size() << std::endl;

    for (auto& tr: triangles)
    {
        afisare(tr);
        std::cout << std::endl;
    }

    return 0;
}