#include "vertex_buffer.h"

#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

vertex_buffer::vertex_buffer()
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

vertex_buffer::vertex_buffer(const void * data, unsigned int size)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

vertex_buffer::~vertex_buffer()
{
    glDeleteBuffers(1, &m_id);
}

void vertex_buffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void vertex_buffer::buffer_data(const void * data, unsigned int size) const
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

