#pragma once

template <typename type>
class singleton
{
public:
    static type& get_instance()
    {
        if (!instance)
            instance = new type();
        return *instance;
    }

    static void release_instance()
    {
        if (instance)
        {
            delete instance;
            instance = nullptr;
        }
    }
private:
    static type * instance;
};