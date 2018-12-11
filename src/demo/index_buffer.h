#pragma once

class index_buffer 
{
public:
    index_buffer();
    index_buffer(const unsigned int * data, unsigned int count);
    ~index_buffer();

    void bind() const;
    void buffer_data(const unsigned int * data, unsigned int count) const;
private:
    unsigned int m_id;
    unsigned int m_count;
};