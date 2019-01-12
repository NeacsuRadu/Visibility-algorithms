#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include "shader_program.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "index_buffer.h"
#include "algorithms.h"
#include "graph.h"
#include "triangulation.h"
#include "triangulated_polygon_visibility.h"
#include "simple_polygon_visibility.h"
#include "roboto.h"

#include <iostream>
#include <vector>

int WINDOW_WIDTH  = 640;
int WINDOW_HEIGHT = 480;

std::string run_type;

bool visi_enabled = false;
bool show_triangulation = true;
bool show_graph = false;
bool path = false;
bool show_degenerate_polygon = false;

bool last_space_state = false;
bool last_t_state = false;
bool last_v_state = false;
bool last_p_state = false;
bool last_d_state = false;

roboto rob;

std::vector<float>        vertices;
std::vector<unsigned int> indices;
unsigned int poly_starting_index = 0;

std::vector<std::vector<point>> polygons;
unsigned int polygon_index;

std::vector<float>        vertices_visi;

std::vector<float>        vertices_triangualtion;

std::vector<float>        vertices_graph;

std::vector<float>        vertices_path;

std::vector<float>        degenerate_polygon;

std::vector<float>        polygon_triangles;
std::vector<float>        holes_triangles;


graph<point, compare_pt> * g_visi_graph = nullptr;

std::vector<point>  path_points;

bool space_pressed = false;

void cursor_pos_to_xoy(double& x, double& y)
{
    // coordonatele cursorului sunt masurate din coltul stanga sus al ferestrei ( x : 0 - width, y : 0 - height ) 
    // pentru a le folosi trebuie sa le transformam in - 1.0 : 1.0 cu originea in centrul ferestrei
    x; // already ok 
    y = static_cast<double>(WINDOW_HEIGHT) - y;
}

void normalize(double& x, double& y)
{
    x = ((x / static_cast<double>(WINDOW_WIDTH)) - 0.5) * 2;
    y = ((y / static_cast<double>(WINDOW_HEIGHT)) - 0.5) * 2;
}

bool can_add_edge(const point& p1, const point& p2)
{
    // testam daca p2 poate fi legat la ultimul poligon construit
    // p1 face deja parte din poligon
    for (std::size_t idx_p = 0; idx_p < polygons.size(); ++ idx_p)
    {
        auto sz = polygons[idx_p].size();
        for (std::size_t idx_v = 0; idx_v < sz - 1; ++ idx_v)
        {
            if (p1 == polygons[idx_p][idx_v + 1])
                continue;

            auto inters = get_segments_intersection(polygons[idx_p][idx_v], polygons[idx_p][idx_v + 1], p1, p2);
            if (inters != error_point)
            {
                std::cout << "point not ok" << std::endl;
                return false;
            }
        }

        if (idx_p == polygons.size() - 1)
            continue;

        auto inters = get_segments_intersection(polygons[idx_p][0], polygons[idx_p][sz - 1], p1, p2);
        if (inters != error_point)
        {
            std::cout << "point not ok" << std::endl;
            return false;
        }
    }

    std::cout << "point ok" << std::endl;
    return true;
}

bool can_close_polygon()
{
    if (polygons[polygon_index].size() <= 2)
        return false;


    const auto& p1 = polygons[polygon_index][0];
    const auto& p2 = polygons[polygon_index][polygons[polygon_index].size() - 1];
    for (std::size_t idx_p = 0; idx_p < polygons.size() - 1; ++ idx_p)
    {
        auto sz = polygons[idx_p].size();
        for (std::size_t idx_v = 0; idx_v < sz - 1; ++ idx_v)
        {
            auto inters = get_segments_intersection(polygons[idx_p][idx_v], polygons[idx_p][idx_v + 1], p1, p2);
            if (inters != error_point)
            {
                std::cout << "cannot close polygon" << std::endl;
                return false;
            }
        }

        auto inters = get_segments_intersection(polygons[idx_p][0], polygons[idx_p][sz - 1], p1, p2);
        if (inters != error_point)
        {
            std::cout << "cannot close polygon" << std::endl;
            return false;
        }
    }   

    for (std::size_t idx_v = 1; idx_v < polygons[polygon_index].size() - 2; ++ idx_v)
    {
        auto inters = get_segments_intersection(polygons[polygon_index][idx_v], polygons[polygon_index][idx_v + 1], p1, p2);
        if (inters != error_point)
        {
            std::cout << "cannot close polygon" << std::endl;
            return false;
        }
    }

    std::cout << "can close polygon" << std::endl;
    return true;
}

std::vector<float> normalize_data(const std::vector<point>& data)
{
    std::vector<float> res;
    for (auto& pt: data)
    {
        double x = pt.x;
        double y = pt.y;
        normalize(x, y);
        res.push_back(static_cast<float>(x));
        res.push_back(static_cast<float>(y));
    }
    return res;
}

std::vector<point> triangles_to_points(const std::vector<triangle*>& triangles)
{
    std::vector<point> res;
    for (auto& tri: triangles)
    {
        res.push_back(tri->e1->a);
        res.push_back(tri->e2->a);
        res.push_back(tri->e3->a);
    }
    return res;
}

std::vector<point> compute_render_polygon(const std::vector<point>& polygon)
{
    std::vector<point> res;
    res.push_back(polygon[0]);
    auto sz = polygon.size();
    for (std::size_t idx = 1; idx < sz; ++ idx)
    {
        res.push_back(polygon[idx]);
        res.push_back(polygon[idx]);
    }
    res.push_back(polygon[0]);
    return res;
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action != GLFW_PRESS)
        return;

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    cursor_pos_to_xoy(mouse_x, mouse_y);
    double x = mouse_x;
    double y = mouse_y;
    normalize(mouse_x, mouse_y);
    float xf = static_cast<float>(mouse_x);
    float yf = static_cast<float>(mouse_y);

    std::cout << "mouse clicked at: " << x << " " << y << std::endl;
    if (visi_enabled && !path)
    {
        vertices_visi.clear();
        try
        {
            std::vector<triangle*> tris;
            if (run_type == "simple")
                tris = simple_polygon_visibility::get_instance().get_visibility({x, y});
            else // triangulated
                tris = triangulated_polygon_visibility::get_instance().get_visibility({x, y});
            auto pts = triangles_to_points(tris);
            vertices_visi = normalize_data(pts);
        }
        catch(const std::runtime_error& er)
        {
            std::cout << er.what() << std::endl;
        }
    }
    else if (visi_enabled && path)
    {
        if (run_type == "simple")
            return;

        if (rob.can_render()) // cannot click while the animation is on the screen
            return;

        path_points.push_back({x, y});
        if (path_points.size() > 2)
            path_points.erase(path_points.cbegin());
        if (path_points.size() == 2)
        {
            auto p = get_path_ab(path_points[0], path_points[1], polygons, g_visi_graph);
            rob.set_path(p);
            vertices_path = normalize_data(p);
        }
        else 
            vertices_path = normalize_data(path_points);
    }
    else 
    {
        if (run_type == "simple" && polygon_index > 0)
            return;

        if (polygons[polygon_index].size() > 0 &&
            !can_add_edge(polygons[polygon_index][polygons[polygon_index].size() - 1], {x, y}))
            return;

        polygons[polygon_index].push_back({x, y});

        if (vertices.size() - poly_starting_index > 2) 
        {
            float last_x = vertices[vertices.size() - 2];
            float last_y = vertices[vertices.size() - 1];
            vertices.push_back(last_x);
            vertices.push_back(last_y);
        }
        vertices.push_back(xf);
        vertices.push_back(yf);        
    }
}

void handle_input(GLFWwindow * window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !last_space_state)
    {
        last_space_state = true;
        if (!can_close_polygon())
            return;

        triangulation::get_instance().compute_triangulation({polygons[polygon_index]});
        auto tris = triangulation::get_instance().get_triangulation();
        auto data = triangles_to_points(tris);
        auto verts = normalize_data(data);
        if (polygon_index == 0)
        {
            std::cout << "polygon" << std::endl;
            polygon_triangles = verts;
        }
        else
        {
            std::cout << " hole " << std::endl;
            holes_triangles.insert(holes_triangles.end(), verts.begin(), verts.end());
        }

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
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && last_space_state)
    {
        last_space_state = false;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !visi_enabled)
    {
        visi_enabled = true;
        polygons.pop_back();
        if (run_type == "simple")
            simple_polygon_visibility::get_instance().preprocess_polygons(polygons);
        else // run_type == "triangulated"
        {
            triangulation::get_instance().compute_triangulation(polygons);
            auto triangles = triangulation::get_instance().get_triangulation();
            auto triangulation_points = triangles_to_points(triangles);
            vertices_triangualtion = normalize_data(triangulation_points);

            auto deg_p = triangulation::get_instance().get_degenerate_polygon();
            auto deg_v = compute_render_polygon(deg_p);
            degenerate_polygon = normalize_data(deg_v);

            g_visi_graph = get_visibility_graph(polygons);
            std::cout << "Get visibility graph" << std::endl;
            auto verts = g_visi_graph->get_vertices();
            vertices_graph = normalize_data(verts);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !last_t_state)
    {
        last_t_state = true;
        show_triangulation = !show_triangulation;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && last_t_state)
        last_t_state = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !last_v_state)
    {
        last_v_state = true;
        show_graph = !show_graph;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE && last_v_state)
        last_v_state = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !last_p_state)
    {
        if (!rob.can_render())
        {
            last_p_state = true;
            path = !path;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && last_p_state)
    {
        if (!rob.can_render())
        {
            last_p_state = false;
            vertices_path.clear();
            path_points.clear();
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !last_d_state)
    {
        last_d_state = true;
        show_degenerate_polygon = ! show_degenerate_polygon;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && last_d_state)
    {
        last_d_state = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        space_pressed = false;
        indices.clear();
        vertices.clear();
        vertices_visi.clear();
    }
}

void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    glViewport(0, 0, width, height);
}

int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        std::cout << "Application must be called with one of the following parameters: simple, triangulated." << std::endl;
        return 0;
    }
    
    std::string argument = argv[1];
    if (argument != "simple" && argument != "triangulated")
    {
        std::cout << "Invalid parameters, try one of the following: simple, triangulated." << std::endl;
        return 0;
    }
    run_type = argument;
    if (run_type == "simple")
    {
        std::cout << "-> use left click to create a polygon;\n"
                     "-> to close the polygon press space;\n" 
                     "-> after you close the polygon press n;\n"
                     "-> press left click inside the polygon to see the region visibile from the point you clicked" << std::endl;
    }
    else 
    {
        std::cout << "-> use left click to create a polygon and holes in it;\n"
                     "-> to close the polygon or a hole press space;\n" 
                     "-> after you create the polygon and the holes press n;\n"
                     "-> to see the degenerate polygon press d;\n"
                     "-> to see the triangulation of the polygon press t;\n"
                     "-> to see the visibility graph press v;\n"
                     "-> press left click inside the polygon to see the region visibile from the point you clicked;\n"
                     "-> press p to switch from visibility to path generation algorithm, using left click place two points inside the polygon and you will get the path between them;\n"
                     "-> to switch between visibility and path generation press p;" << std::endl;
    }

    triangulation::get_instance();
    triangulated_polygon_visibility::get_instance();
    simple_polygon_visibility::get_instance();

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

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    shader_program program("../../../shaders/vertex_shaders/lines.vs", "../../../shaders/fragment_shaders/lines.fs");
    shader_program blue("../../../shaders/vertex_shaders/lines.vs", "../../../shaders/fragment_shaders/blue.fs");
    shader_program yellow("../../../shaders/vertex_shaders/lines.vs", "../../../shaders/fragment_shaders/yellow.fs");
    shader_program gray("../../../shaders/vertex_shaders/lines.vs", "../../../shaders/fragment_shaders/gray.fs");
    shader_program prog_roboto("../../../shaders/vertex_shaders/roboto.vs", "../../../shaders/fragment_shaders/lines.fs");

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

    vertex_array va_graph;
    vertex_buffer vb_graph;
    va_graph.add_buffer(vb_graph, layout);

    vertex_array va_path;
    vertex_buffer vb_path;
    va_path.add_buffer(vb_path, layout);

    vertex_array va_deg;
    vertex_buffer vb_deg;
    va_deg.add_buffer(vb_deg, layout);

    vertex_array va_blue_poly;
    vertex_buffer vb_blue_poly;
    va_blue_poly.add_buffer(vb_blue_poly, layout);

    vertex_array va_gray_holes;
    vertex_buffer vb_gray_holes;
    va_gray_holes.add_buffer(vb_gray_holes, layout);

    vertex_array va_rob;
    vertex_buffer vb_rob;
    va_rob.add_buffer(vb_rob, layout);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Handle keyboard input */
        handle_input(window);

        /* Clear the screen */
        // albastru: 88, 133, 206
        // galben: 209, 199, 62
        glClearColor(174.0f / 255.0f, 182.0f / 255.0f, 198.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if (polygon_triangles.size() > 0)
        {
            blue.use();
            va_blue_poly.bind();
            vb_blue_poly.bind();
            vb_blue_poly.buffer_data(polygon_triangles.data(), sizeof(float) * polygon_triangles.size());
            glDrawArrays(GL_TRIANGLES, 0, polygon_triangles.size() / 2);
        }

        if (holes_triangles.size() > 0)
        {
            gray.use();
            va_gray_holes.bind();
            vb_gray_holes.bind();
            vb_gray_holes.buffer_data(holes_triangles.data(), sizeof(float) * holes_triangles.size());
            glDrawArrays(GL_TRIANGLES, 0, holes_triangles.size() / 2);
        }

        /* Draw visibility if possible */
        if (visi_enabled && vertices_visi.size() > 1 && !path)
        {
            yellow.use();
            va_visi.bind();
            vb_visi.bind();
            vb_visi.buffer_data(vertices_visi.data(), sizeof(float) * vertices_visi.size());
            glDrawArrays(GL_TRIANGLES, 0, vertices_visi.size() / 2);
        }

        /* Draw xoy axes */
        /*program.use();
        va_axes.bind();
        vb_axes.bind();
        glDrawArrays(GL_LINES, 0, 4);*/

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

        /* Draw graph*/
        if (show_graph)
        {
            program.use();
            va_graph.bind();
            vb_graph.bind();
            vb_graph.buffer_data(vertices_graph.data(), sizeof(float) * vertices_graph.size());
            glDrawArrays(GL_LINES, 0, vertices_graph.size() / 2);
        }

        if (path)
        {
            program.use();
            va_path.bind();
            vb_path.bind();
            vb_path.buffer_data(vertices_path.data(), sizeof(float) * vertices_path.size());
            glDrawArrays(GL_POINTS, 0, vertices_path.size() / 2);
        }

        if (rob.can_render())
        {
            rob.update();
            auto pt = rob.get_position();
            auto data = normalize_data({pt});
            prog_roboto.use();
            va_rob.bind();
            vb_rob.bind();
            vb_rob.buffer_data(data.data(), sizeof(float) * data.size());
            glDrawArrays(GL_POINTS, 0, data.size() / 2 );
        }

        if (degenerate_polygon.size() && show_degenerate_polygon)
        {
            program.use();
            va_deg.bind();
            vb_deg.bind();
            vb_deg.buffer_data(degenerate_polygon.data(), sizeof(float) * degenerate_polygon.size());
            glDrawArrays(GL_LINES, 0, degenerate_polygon.size() / 2);
        }
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    simple_polygon_visibility::release_instance();
    triangulated_polygon_visibility::release_instance();
    triangulation::release_instance();
    return 0;
}