#pragma once

template <typename type>
class singleton
{
public:
    type& get_instance()
    {
        if (!instance)
            instance = new type();
        return *instance;
    }

    void release_instance()
    {
        if (instance)
        {
            delete instance;
            instance = nullptr;
        }
    }
private:
    type * instance = nullptr;
};