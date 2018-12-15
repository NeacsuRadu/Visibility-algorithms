#pragma once
template<class node_type>
struct node 
{
public:
    ~node<node_type>()
    {
        if (!next)
            return;

        delete next;
        next = prev = nullptr;
    }

    node_type info;
    node<node_type> * next = nullptr;
    node<node_type> * prev = nullptr;
};

template<class type>
class circular_list
{
public:
    ~circular_list<type>()
    {
        if (!first)
            return;

        first->prev->next = nullptr;
        delete first;
        first = nullptr;
    }

    void push_back(type info) // pushes back
    {
        if (!first)
        {
            first = new node<type>();
            first->info = info;
            first->next = first;
            first->prev = first;
            return;
        }

        node<type>* aux = new node<type>();
        aux->info = info;
        aux->next = first;
        aux->prev = first->prev;
        first->prev->next = aux;
        first->prev = aux;
    }

    node<type> * remove(node<type> * node)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        auto ret = node->next;
        if (node == first)
            first = node->next;

        node->prev = node->next = nullptr;
        delete node;

        return ret;
    }

    void destroy()
    {
        if (!first)
            return;

        first->prev->next = nullptr;
        first->destroy();
        delete first;
        first = nullptr;
    }

    inline node<type> * get_first() const { return first; }
private:
    node<type> * first = nullptr;
};