#include "button.h"
#include "stb_image.h"

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

button::button(const std::string& img, const point& position_ul,  const point& position_lr)
    : m_ul(position_ul)
    , m_lr(position_lr)
{
    _load_texture(img);

    double xl = m_ul.x;
    double xr = m_lr.x;
    double yl = m_lr.y;
    double yu = m_ul.y;
    _normalize(xl, yu);
    _normalize(xr, yl);
    float vertices[] = {
        static_cast<float>(xl), static_cast<float>(yu), 0.0f, 1.0f, // upper-left  corner
        static_cast<float>(xl), static_cast<float>(yl), 0.0f, 0.0f, // lower-left  corner
        static_cast<float>(xr), static_cast<float>(yu), 1.0f, 1.0f, // upper-right corner
        static_cast<float>(xl), static_cast<float>(yl), 0.0f, 0.0f, // lower-left  corner
        static_cast<float>(xr), static_cast<float>(yu), 1.0f, 1.0f, // upper-right corner
        static_cast<float>(xr), static_cast<float>(yl), 1.0f, 0.0f  // lower-right corner
    };

    m_va.bind();
    m_vb.bind();
    m_vb.buffer_data(static_cast<const void*>(vertices), sizeof(float) * 24);
    vertex_buffer_layout layout;
    push<float>(layout, 2);
    push<float>(layout, 2);
    m_va.add_buffer(m_vb, layout);
}

void button::render() const 
{
    glBindTexture(GL_TEXTURE_2D, m_texture);
    m_va.bind();
    m_vb.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool button::mouse_clicked_inside(double x, double y) const 
{
    if (x > m_ul.x && x < m_lr.x && y < m_ul.y && y > m_lr.y)
        return true;
    return false;
}

void button::_load_texture(const std::string& img)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char *data = stbi_load(img.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        //std::cout << "ok" << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else 
    {
        //std::cout << "not ok" << std::endl;        
    }

    stbi_image_free(data);
    m_texture = texture;
}

void button::_normalize(double& x, double& y)
{
    x = ((x / static_cast<double>(WINDOW_WIDTH)) - 0.5) * 2;
    y = ((y / static_cast<double>(WINDOW_HEIGHT)) - 0.5) * 2;
}