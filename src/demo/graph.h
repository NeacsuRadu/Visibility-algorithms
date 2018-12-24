#pragma once
#include <map>
#include <vector>
#include <queue>
#include <functional>

template<typename type, typename comp = std::less<type>>
class graph
{
public:
    using path = std::vector<std::size_t>;

    graph(std::vector<type> nodes)
    {
        auto sz = nodes.size();
        for (std::size_t idx = 0; idx < sz; ++idx)
        {
            m_nodes[nodes[idx]] = idx;
            m_mapping[idx] = nodes[idx];
            m_edges.push_back({});
            m_paths.push_back(std::vector<path>(sz));
        }
    }

    void add_edge(const type& a, const type& b, double distance)
    {
        auto it_a = m_nodes.find(a);
        if (it_a == m_nodes.end())
            return;

        auto it_b = m_nodes.find(b);
        if (it_b == m_nodes.end())
            return;

        m_edges[it_a->second].push_back({it_b->second, distance});
        /*m_edges[it_b->second].push_back({it_a->second, distance});*/
    }

    std::vector<type> get_path(const type& a, const type& b)
    {
        auto it_a = m_nodes.find(a);
        if (it_a == m_nodes.end())
            return {};

        auto it_b = m_nodes.find(b);
        if (it_b == m_nodes.end())
            return {};


        auto path = m_paths[it_a->second][it_b->second];
        
        std::vector<type> res;
        for (auto& node: path)
            res.push_back(m_mapping[node]);
        return res;
    }

    void compute_all_paths()
    {
        for (std::size_t i = 0; i < m_edges.size(); ++i)
        {
            std::cout << i << " : " << std::endl;
            for (std::size_t j = 0; j < m_edges[i].size(); ++ j)
            {
                std::cout << m_edges[i][j].first << " ";
            }
            std::cout << std::endl;
        }

        auto sz = m_edges.size();
        for (std::size_t idx_i = 0; idx_i < sz - 1; ++idx_i)
        {
            auto prevs = _dijkstra(idx_i);
            for (std::size_t idx_j = 1; idx_j < sz; ++idx_j)
            {
                std::cout << "Path from " << idx_i << " to " << idx_j << " : " << std::endl;
                auto path = _get_path(prevs, idx_i, idx_j);
                auto rev_path = std::vector<std::size_t>(path.rbegin(), path.rend());
                for (auto x: path)
                    std::cout << x << " ";
                std::cout << std::endl;
                m_paths[idx_j][idx_i].swap(path);
                for (auto x: rev_path)
                    std::cout << x << " ";
                std::cout << std::endl;
                m_paths[idx_i][idx_j].swap(rev_path);
            }
        }
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
    std::vector<std::size_t> _get_path(const std::vector<std::size_t>& prevs, std::size_t root, std::size_t node)
    {
        std::vector<std::size_t> path;

        std::size_t crr = node;
        while (prevs[crr] != crr)
        {
            path.push_back(crr);
            crr = prevs[crr];
        }
        path.push_back(crr);

        return path;
    }

    std::vector<std::size_t> _dijkstra(std::size_t root)
    {
        // distances vector
        std::vector<double> dists(m_nodes.size(), 300.0);
        dists[root] = 0.0;

        // keep the previous node of b in the path from root to b
        std::vector<std::size_t> prev(m_nodes.size(), 0);
        prev[root] = root;

        // data for priority q, custom compare
        using info = std::pair<std::size_t, double>;
        using container = std::vector<info>;
        auto compare = [](const info& a, const info& b) -> bool {
            return a.second > b.second;
        };

        std::priority_queue<info, container, decltype(compare)> queue(compare);
        queue.push({root, 0});
        while (!queue.empty())
        {
            auto el = queue.top();
            queue.pop();

            for (auto& edge: m_edges[el.first])
            {
                auto dist = dists[el.first] + edge.second;
                if (dist < dists[edge.first]) 
                {
                    dists[edge.first] = dist;
                    prev[edge.first] = el.first;
                    queue.push({edge.first, dist});
                }
            }
        } 

        return prev;
    }

    std::map<type, std::size_t, comp>      m_nodes;
    std::map<std::size_t, type>            m_mapping;
    std::vector<std::vector<
        std::pair<std::size_t, double>>>   m_edges;

    std::vector<std::vector<path>>         m_paths;
};
