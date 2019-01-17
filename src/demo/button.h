#pragma once
#include "geometry.h"

#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include "shader_program.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "index_buffer.h"

class button
{
public:
    button(const std::string& img, const point& position_ul, const point& position_lr);

    void render() const;
    bool mouse_clicked_inside(double x, double y) const;
private:
    void _load_texture(const std::string& img);
    void _normalize(double& x, double& y);

    point m_ul; // upper-left corner
    point m_lr; // lower-right corner

    vertex_buffer m_vb;
    vertex_array m_va;
    unsigned int m_texture;
};