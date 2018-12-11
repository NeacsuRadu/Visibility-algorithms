#pragma once
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

#include <vector>

class vertex_buffer_layout 
{
public:
    struct element
    {
        int  dimensions = 0;
        unsigned int  type       = 0;
        unsigned char normalized = GL_FALSE;
    };

    inline const std::vector<element>& get_elements() const { return m_elements; }
    inline unsigned int get_stride() const { return m_stride; }

    void push(const element& el, int stride)
    {
        m_elements.push_back(el);
        m_stride += stride;
    }

    static std::size_t gltype_to_size(GLenum type)
    {
        if (type == GL_FLOAT)
            return sizeof(float);

        return 0;
    }
private:
    std::vector<element> m_elements;

    unsigned int         m_stride = 0;
};

template<class T>
inline void push(vertex_buffer_layout& layout, int dimensions)
{
    return;
}

template<>
inline void push<float>(vertex_buffer_layout& layout, int dimensions)
{
    layout.push({dimensions, GL_FLOAT, GL_FALSE}, dimensions * sizeof(float));
}