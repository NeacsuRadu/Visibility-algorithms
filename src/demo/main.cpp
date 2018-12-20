#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include "shader_program.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "index_buffer.h"
#include "algorithms.h"

#include <iostream>
#include <vector>
;
unsigned int WINDOW_WIDTH  = 640;
unsigned int WINDOW_HEIGHT = 480;

bool visi_enabled = false;
bool show_triangulation = true;

bool last_space_state = false;
bool last_t_state = false;

std::vector<float>        vertices;
std::vector<unsigned int> indices;
unsigned int poly_starting_index = 0;

std::vector<std::vector<point>> polygons;
unsigned int polygon_index;

std::vector<float>        vertices_visi;
std::vector<unsigned int> indices_visi;

std::vector<float>        vertices_triangualtion;


bool space_pressed = false;

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
    if (visi_enabled)
    {
        vertices_visi.clear();
        indices_visi.clear();
        try
        {
            vertices_visi = get_polygon_visibility(polygons, mouse_x, mouse_y);
        }
        catch(const std::runtime_error& er)
        {
            std::cout << er.what() << std::endl;
        }
    }
    else 
    {
        polygons[polygon_index].push_back({mouse_x, mouse_y});

        if (vertices.size() - poly_starting_index > 2) 
        {
            float last_x = vertices[vertices.size() - 2];
            float last_y = vertices[vertices.size() - 1];
            vertices.push_back(last_x);
            vertices.push_back(last_y);
        }
        vertices.push_back(mouse_x);
        vertices.push_back(mouse_y);        
    }
}

void handle_input(GLFWwindow * window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !last_space_state)
    {
        last_space_state = true;
        polygons.push_back({});
        polygon_index ++;

        float first_x = vertices[poly_starting_index];
        float first_y = vertices[poly_starting_index + 1];
        float last_x  = vertices[vertices.size() - 2];
        float last_y  = vertices[vertices.size() - 1];
        vertices.push_back(first_x);
        vertices.push_back(first_y);
        vertices.push_back(last_x);
        vertices.push_back(last_y);
        poly_starting_index = vertices.size();
        /*if (triangulation)
        {
            vertices_triangualtion = get_triangulation(vertices);
        }*/
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && last_space_state)
    {
        last_space_state = false;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !visi_enabled)
    {
        visi_enabled = true;
        polygons.pop_back();
        vertices_triangualtion = get_triangulation(polygons);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !last_t_state)
    {
        last_t_state = true;
        show_triangulation = !show_triangulation;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && last_t_state)
        last_t_state = false;
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
    polygons.push_back({});

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
        std::size_t sz = vertices.size();
        if ((sz / 2) % 2 == 1)
        {
            va_points.bind();
            vb_points.bind();
            vb_points.buffer_data(vertices.data(), sizeof(float) * vertices.size());
            if (poly_starting_index > 0)
                glDrawArrays(GL_LINES, 0, poly_starting_index / 2);
            glDrawArrays(GL_POINTS, poly_starting_index / 2, 1);
        }
        else if (vertices.size() > 2)
        {
            va_points.bind();
            vb_points.bind();
            vb_points.buffer_data(vertices.data(), sizeof(float) * vertices.size());
            glDrawArrays(GL_LINES, 0, vertices.size() / 2);
        }

        /* Draw visibility if possible */
        if (visi_enabled && vertices_visi.size() > 1)
        {
            program.use();
            va_visi.bind();
            vb_visi.bind();
            vb_visi.buffer_data(vertices_visi.data(), sizeof(float) * vertices_visi.size());
            glDrawArrays(GL_TRIANGLES, 0, vertices_visi.size() / 2);
        }

        /* Draw triangles */
        if (show_triangulation)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            program.use();
            va_triangulation.bind();
            vb_triangulation.bind();
            vb_triangulation.buffer_data(vertices_triangualtion.data(), sizeof(float) * vertices_triangualtion.size());
            glDrawArrays(GL_TRIANGLES, 0, vertices_triangualtion.size() / 2);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
    /*std::vector<point> points = {
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
    }*/

    return 0;
}