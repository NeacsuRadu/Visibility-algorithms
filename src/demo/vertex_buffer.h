#pragma once

class vertex_buffer 
{
public:
    vertex_buffer();
    vertex_buffer(const void * data, unsigned int size);
    ~vertex_buffer();
    
    void buffer_data(const void * data, unsigned int size) const;
    void bind() const;
private:
    unsigned int m_id;
};