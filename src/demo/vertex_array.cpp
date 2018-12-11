#include "vertex_array.h"

#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

vertex_array::vertex_array()
{
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
}

vertex_array::~vertex_array()
{
    glDeleteVertexArrays(1, &id);
}

void vertex_array::bind() const
{
    glBindVertexArray(id);
}

void vertex_array::add_buffer(const vertex_buffer& buffer, const vertex_buffer_layout& layout) const
{
    buffer.bind();
    auto& elements = layout.get_elements();
    unsigned int offset = 0;
    for (size_t idx = 0; idx < elements.size(); ++idx)
    {
        glVertexAttribPointer(
            static_cast<unsigned int>(idx),
            elements[idx].dimensions, 
            elements[idx].type, 
            elements[idx].normalized,
            layout.get_stride(),
            reinterpret_cast<const void*>(offset));
        glEnableVertexAttribArray(static_cast<unsigned int>(idx));
        offset += elements[idx].dimensions * vertex_buffer_layout::gltype_to_size(elements[idx].type);
    }
}