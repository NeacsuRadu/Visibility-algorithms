#pragma once
#include <map>
#include <vector>
#include <queue>
#include <functional>

template<typename type, typename comp = std::less<type>>
class graph
{
public:
    using path = std::pair<std::vector<std::size_t>, double>;

    graph(std::vector<type> nodes)
    {
		std::cout << "nodes in: " << nodes.size() << std::endl;
        auto sz = nodes.size();
        for (std::size_t idx = 0; idx < sz; ++idx)
        {
            m_nodes[nodes[idx]] = idx;
            m_mapping[idx] = nodes[idx];
            m_edges.push_back({});
            m_paths.push_back(std::vector<path>(sz));
        }
		std::cout << "nodes out: " << m_nodes.size() << std::endl;
    }

    void add_edge(const type& a, const type& b, double distance)
    {
        auto it_a = m_nodes.find(a);
        if (it_a == m_nodes.end())
            return;

        auto it_b = m_nodes.find(b);
        if (it_b == m_nodes.end())
            return;

        for (auto e: m_edges[it_a->second])
            if (e.first == it_b->second)
                return;

        std::cout << "add edge" << it_a->second << " - " << it_b->second << std::endl;
        m_edges[it_a->second].push_back({it_b->second, distance});
        /*m_edges[it_b->second].push_back({it_a->second, distance});*/
    }

    std::vector<type> get_path(const type& a, const type& b, double& distance)
    {
        distance = 0;
        auto it_a = m_nodes.find(a);
        if (it_a == m_nodes.end())
            return {};

        auto it_b = m_nodes.find(b);
        if (it_b == m_nodes.end())
            return {};


        auto path = m_paths[it_a->second][it_b->second].first;
        distance = m_paths[it_a->second][it_b->second].second;
        
        std::vector<type> res;
        for (auto& node: path)
            res.push_back(m_mapping[node]);
        return res;
    }

    void compute_all_paths()
    {
        /*for (std::size_t i = 0; i < m_edges.size(); ++i)
        {
            std::cout << i << " : " << std::endl;
            for (std::size_t j = 0; j < m_edges[i].size(); ++ j)
            {
                std::cout << m_edges[i][j].first << " " << m_edges[i][j].second << " --- ";
            }
            std::cout << std::endl;
        }*/

        auto sz = m_edges.size();
        for (std::size_t idx_i = 0; idx_i < sz - 1; ++idx_i)
        {
            std::cout << "dij " << std::endl;
            auto aux = _dijkstra(idx_i);
            std::cout <<" adij" << std::endl;
            /*std::cout << "Dijkstra from " << idx_i << std::endl;
            for (auto& inf: aux)
            {
                std::cout << inf.first << " " << inf.second << std::endl;
            }*/
            for (std::size_t idx_j = 1; idx_j < sz; ++idx_j)
            {
                //std::cout << "Path from " << idx_i << " to " << idx_j << " : " << std::endl;
                std::cout << "get path" << std::endl;
                auto path = _get_path(aux, idx_i, idx_j);
                std::cout << " a get path" << std::endl;
                auto distance = aux[idx_j].second;
                auto rev_path = std::vector<std::size_t>(path.rbegin(), path.rend());
                /*for (auto x: path)
                    std::cout << x << " ";c
                std::cout << std::endl;*/
                m_paths[idx_j][idx_i] = {path, distance};
                /*for (auto x: rev_path)
                    std::cout << x << " ";
                std::cout << std::endl;*/
                m_paths[idx_i][idx_j] = {rev_path, distance};
            }
        }

        for (std::size_t idx_i = 0; idx_i < sz; ++idx_i)
            m_paths[idx_i][idx_i] = {{idx_i}, 0.0};
    }

    std::vector<type> get_vertices()
    {
        std::vector<type> res;
        for (std::size_t idx_node = 0; idx_node < m_edges.size(); ++ idx_node)
            for (std::size_t idx_edge = 0; idx_edge < m_edges[idx_node].size(); ++ idx_edge)
            {
                res.push_back(m_mapping[idx_node]);
                res.push_back(m_mapping[m_edges[idx_node][idx_edge].first]);
            }
        return res;
    }
private:
    std::vector<std::size_t> _get_path(const std::vector<std::pair<std::size_t, double>>& prevs, std::size_t root, std::size_t node)
    {
        std::vector<std::size_t> path;

        std::size_t crr = node;
        while (prevs[crr].first != crr)
        {
            path.push_back(crr);
            crr = prevs[crr].first;
        }
        path.push_back(crr);

        return path;
    }

    std::vector<std::pair<std::size_t, double>> _dijkstra(std::size_t root)
    {
        // data for priority q, custom compare
        using info = std::pair<std::size_t, double>;
        using container = std::vector<info>;
        auto compare = [](const info& a, const info& b) -> bool {
            return a.second > b.second;
        };

        container aux(m_nodes.size());
        for (auto& inf: aux)
        {
            inf.first = m_nodes.size();
            inf.second = 999999.0;
        }
        aux[root].first = root;
        aux[root].second = 0.0;

        std::priority_queue<info, container, decltype(compare)> queue(compare);
        queue.push({root, 0.0});
        std::cout << "before loop, size: " << aux.size() << std::endl;
        while (!queue.empty())
        {
            auto el = queue.top();
            queue.pop();
            std::cout << "queue popped, el: " << el.first << std::endl;
            for (auto& edge: m_edges[el.first])
            {
                std::cout << "edge: " << edge.first << std::endl;
                auto dist = aux[el.first].second + edge.second;
                if (dist < aux[edge.first].second) 
                {
                    aux[edge.first].second = dist;
                    aux[edge.first].first = el.first;
                    queue.push({edge.first, dist});
                }
            }
        } 

        return aux;
    }

    std::map<type, std::size_t, comp>       m_nodes;
    std::map<std::size_t, type>             m_mapping;
    std::vector<std::vector<
        std::pair<std::size_t, double>>> m_edges;

    std::vector<std::vector<path>>          m_paths;
};
