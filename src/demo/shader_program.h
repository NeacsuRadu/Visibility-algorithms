#pragma once

#include <string>

class shader_program
{
public:
    shader_program(const std::string& strVSPath, const std::string& strFSPath);
    ~shader_program();

    void use() const;
    /*void SetUniform(const std::string& name, const glm::vec3& vec) const;
    void SetUniform(const std::string& name, const glm::mat4& mat) const;
    void SetUniform(const std::string& name, int val) const;
    void SetUniform(const std::string& name, float val) const;*/

    inline unsigned int get_id() const { return m_id; };
private:
    unsigned int m_id;
};