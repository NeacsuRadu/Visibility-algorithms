#pragma once

#include "geometry.h"
#include "nlohmann/json.hpp"

#include <fstream>

inline bool parse_input_param(const std::string& arg, std::vector<std::vector<point>>& polygons, std::string& algorithm, std::string& run_type)
{
    std::ifstream in(arg);
    if (!in.is_open())
        return false;

    using namespace nlohmann;
    json js;
    in >> js;

    //std::cout << "config_json: " << js << std::endl;
    in.close();

    if (!js.is_object())
        return false;

    auto& js_alg = js.at("used_algorithm");
    if (!js_alg.is_string())
        return false;
    algorithm = js_alg.get<std::string>();
    if (algorithm != "simple" && algorithm != "triangulated" && algorithm != "intersect")
        return false;

    auto& js_type = js.at("run_type");
    if (!js_type.is_string())
        return false;
    run_type = js_type.get<std::string>();
    if (run_type != "full" && run_type != "step")
        return false;

    auto& js_polygon = js.at("polygon");
    if (!js_polygon.is_array())
        return false;

    auto no = js_polygon.size();
    std::cout << "number of points: " << js_polygon.size() << std::endl;
    if (js_polygon.size() == 0)
    {
        polygons.push_back({});
        return true;
    }

    polygons.push_back({});
    for (auto& js_point: js_polygon)
    {
        if (!js_point.is_object())
            return false;

        polygons[0].push_back({js_point["x"], js_point["y"]});
    }

    if (algorithm == "simple")
    {
        polygons.push_back({});
        return true;
    }

    auto& js_holes = js.at("holes");
    if (!js_holes.is_array())
        return false;

    std::size_t idx = 0;
    for (auto& js_hole: js_holes)
    {
        polygons.push_back({});
        idx ++;
        if (!js_hole.is_array())
            return false;

        no += js_hole.size();
        std::cout << "number of points: " << js_hole.size() << std::endl;
        for (auto& js_point: js_hole)
        {
            if (!js_point.is_object())
                return false;

            polygons[idx].push_back({js_point["x"], js_point["y"]});
        }
    }
    polygons.push_back({});
    std::cout << "pts:  " << no << std::endl;

    return true;
}

inline void write_points(const std::vector<std::vector<point>>& polygons)
{
    using namespace nlohmann;
    json js;
    json js_polygon;
    for (std::size_t idx_p = 0; idx_p < polygons[0].size(); ++ idx_p)
    {
        json point = {
            {"x", polygons[0][idx_p].x},
            {"y", polygons[0][idx_p].y}
        };
        js_polygon.push_back(point);        
    }
    js["polygon"] = js_polygon;

    json js_holes;
    for (std::size_t idx_h = 1; idx_h < polygons.size() - 1; ++ idx_h)
    {
        json js_hole;
        for (std::size_t idx = 0; idx < polygons[idx_h].size(); ++ idx)
        {
            json point = {
                {"x", polygons[idx_h][idx].x},
                {"y", polygons[idx_h][idx].y}
            };
            js_hole.push_back(point);
        }
        js_holes.push_back(js_hole);
    }
    js["holes"] = js_holes;

    std::ofstream out("polygons.json");
    if (out.is_open())
    {
        out << js;
        out.close();
    }
}