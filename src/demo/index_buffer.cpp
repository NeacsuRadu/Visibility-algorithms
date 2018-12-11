#include "index_buffer.h"

#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

index_buffer::index_buffer()
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

index_buffer::index_buffer(const unsigned int * data, unsigned int count)
    : m_count (count)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_DYNAMIC_DRAW);
}

index_buffer::~index_buffer()
{
    glDeleteBuffers(1, &m_id);
}

void index_buffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void index_buffer::buffer_data(const unsigned int * data, unsigned int count) const
{
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_DYNAMIC_DRAW);
}