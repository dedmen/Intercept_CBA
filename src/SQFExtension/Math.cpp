#include "SQFExtensions.hpp"
#include <intercept.hpp>
#include <pmmintrin.h>
#include <cmath>
using namespace intercept::client;
using namespace SQFExtensions;


game_value caternaryFunc(uintptr_t, game_value_parameter right) {
    #ifndef __linux__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    #endif

    if (right.size() < 3) {
        /* too few arguments */
        return  { {} };
    }
#define MAX_ITERATIONS_BISECTION 20
#define MAX_ITERATIONS_NEWTON 10
#define MAX_DIFF_BISECTION 0.01
#define MAX_DIFF_NEWTON 0.001
#define MAX_SEGMENTS 500
#define MAX_SEGMENT_DIFF 0.0001
    //pos1,pos2,rope_length_factor,segment_length

    vector3 pos1 = right[0];
    vector3 pos2 = right[1];
    float segment_length = 1.0;
    float rope_length_factor = right[2];
    if (right.size() >= 4)
        segment_length = right[3];

    /* calculate the 2d catenary curve */
    float delta_x = sqrt(pow(pos2.x - pos1.y, 2) + pow(pos2.y - pos1.y, 2));
    float delta_y = pos2.z - pos1.z;
    float rope_length = rope_length_factor * sqrt(delta_x*delta_x + delta_y*delta_y);

    if (delta_x == 0) {
        return  { {} };
    };

    float a = 1.0;
    float aMin = 0.1;
    float aMax = 10 * rope_length;
    float diff = 1.0e+7;
    float alpha = sqrt(pow(rope_length, 2) - pow(delta_y, 2));


    for (int i = 1; i <= MAX_ITERATIONS_BISECTION && abs(diff) > MAX_DIFF_BISECTION; i = i + 1) {
        a = (aMin + aMax) / 2;
        diff = 2 * a * sinh(delta_x / (2 * a)) - alpha;

        if (diff < 0)
            aMax = a;
        else
            aMin = a;
    }

    float prev = 1.0e+7;

    for (int i = 1; i <= MAX_ITERATIONS_NEWTON && abs(prev - a) > MAX_DIFF_NEWTON; i = i + 1) {
        a = a -
            (2 * a * sinh(delta_x / (2 * a)) - alpha) /
            (2 * sinh(delta_x / (2 * a)) - delta_x / a * cosh(delta_x / (2 * a)));

        prev = a;
    };

    if (!std::isfinite(a)) {
        /* rope too short */
        return  { {} };
    };

    float x1 = a * atanh(delta_y / rope_length) - delta_x / 2;
    float x2 = a * atanh(delta_y / rope_length) + delta_x / 2;
    float y1 = a * cosh(x1 / a);
    float y2 = a * cosh(x2 / a);

    /* estimate amount of needed segments and increase segment length if necessary */

    if (rope_length / segment_length > MAX_SEGMENTS) {
        segment_length = rope_length / MAX_SEGMENTS;

        if (segment_length > 1)
            segment_length = ceil(segment_length) + 1;
    };

    /* generate a vector of x,y points on catenary with distance of the segment length */
    vector2 last_pos = { x1, y1 };
    vector2  end_pos = { x2, y2 };
    std::vector<vector2> catenary_points;
    catenary_points.reserve(std::min(rope_length / segment_length, 500.f));

    while (sqrt(pow(end_pos.x - last_pos.x, 2) + pow(end_pos.y - last_pos.y, 2)) > segment_length) {
        float u = last_pos.x;
        float y = last_pos.y;

        float uMin = u;
        float uMax = u + segment_length;

        /* find the next "u" */
        while (abs(uMin - uMax) > MAX_SEGMENT_DIFF * segment_length) {
            y = a * cosh(u / a);

            if (sqrt(pow(last_pos.x - u, 2) + pow(last_pos.y - y, 2)) > segment_length)
                uMax = u;
            else
                uMin = u;

            u = (uMin + uMax) / 2;
        };

        y = a * cosh(u / a);

        last_pos = { u, y };

        catenary_points.push_back({ u,y });
    };

    catenary_points.push_back(end_pos);

    /* convert 2d points on catenary to world positions */
    vector2 dir_map{ pos2.x - pos1.x, pos2.y - pos1.y };
    float dir_map_length = dir_map.magnitude();
    dir_map /= dir_map_length;


    std::string result;

    //for (auto& p : catenary_points) {
    //    std::string x = std::to_string(pos1.x + (p.x - x1) * dir_map.x);
    //    std::string y = std::to_string(pos1.y + (p.x - x1) * dir_map.y);
    //    std::string z = std::to_string(pos1.z + p.y - y1);
    //    result = result + "[" + x + "," + y + +"," + z + "],";
    //};

    auto_array<vector3> ret;
    ret.reserve(catenary_points.size());

    for (auto& p : catenary_points) {
        ret.emplace_back(
            pos1.x + (p.x - x1) * dir_map.x,
            pos1.y + (p.x - x1) * dir_map.y,
            pos1.z + p.y - y1
        );
    }

    game_value output(std::move(ret));
    return std::move(output);
}

void Math::preStart() {

    static auto _sinh = host::register_sqf_command("sinh"sv, ""sv, [](uintptr_t, game_value_parameter right) -> game_value {
        return std::sinh(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _cosh = host::register_sqf_command("cosh"sv, ""sv, [](uintptr_t, game_value_parameter right) -> game_value {
        return std::cosh(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _tanh = host::register_sqf_command("tanh"sv, ""sv, [](uintptr_t, game_value_parameter right) -> game_value {
        return std::tanh(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _asinh = host::register_sqf_command("asinh"sv, ""sv, [](uintptr_t, game_value_parameter right) -> game_value {
        return std::asinh(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _acosh = host::register_sqf_command("acosh"sv, ""sv, [](uintptr_t, game_value_parameter right) -> game_value {
        return std::acosh(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _atanh = host::register_sqf_command("atanh"sv, ""sv, [](uintptr_t, game_value_parameter right) -> game_value {
        return std::atanh(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _naturalLog = intercept::client::host::register_sqf_command("ln", "", [](uintptr_t, game_value_parameter right) -> game_value {
        return std::log(static_cast<float>(right));
    }, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _nthRoot = intercept::client::host::register_sqf_command("root", "", [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
        return std::pow(static_cast<float>(right), 1.0f / static_cast<float>(left));
    }, game_data_type::SCALAR, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _logn = intercept::client::host::register_sqf_command("log", "", [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
        return std::log(static_cast<float>(right)) / std::log(static_cast<float>(left));
    }, game_data_type::SCALAR, game_data_type::SCALAR, game_data_type::SCALAR);


    static auto _catenaryConnect = intercept::client::host::register_sqf_command("catenaryConnect"sv, ""sv, caternaryFunc, game_data_type::ARRAY, game_data_type::ARRAY);



}
