/**
 * @file         liblava-demo/generics.cpp
 * @brief        Generics demo
 * @authors      Lava Block OÜ and contributors
 * @copyright    Copyright (c) 2018-present, MIT License
 */

#include <imgui.h>
#include <liblava/lava.hpp>

using namespace lava;

//-----------------------------------------------------------------------------
name _triangle_frag_ = "triangle_frag";
name _lava_triangle_ = "lava_triangle";
name _int_triangle_ = "int_triangle";
name _double_triangle_ = "double_triangle";

//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    engine app("lava generics", { argc, argv });

    app.platform.on_create_param = [](device::create_param& param) {
        param.features.shaderFloat64 = true;
    };

    app.prop.add(_triangle_frag_, "generics/triangle.frag");
    app.prop.add(_lava_triangle_, "generics/lava_triangle.vert");
    app.prop.add(_int_triangle_, "generics/int_triangle.vert");
    app.prop.add(_double_triangle_, "generics/double_triangle.vert");

    if (!app.setup())
        return error::not_ready;

    // Initialize a lava triangle
    mesh::ptr lava_triangle;
    // These template arguments are optional
    lava_triangle = create_mesh<vertex, false, true, false>(app.device, mesh_type::triangle);
    if (!lava_triangle)
        return error::create_failed;

    auto& lava_triangle_data = lava_triangle->get_data();
    lava_triangle_data.vertices.at(0).color = v4(1.f, 0.f, 0.f, 1.f);
    lava_triangle_data.vertices.at(1).color = v4(0.f, 1.f, 0.f, 1.f);
    lava_triangle_data.vertices.at(2).color = v4(0.f, 0.f, 1.f, 1.f);
    lava_triangle_data.scale(0.5f);
    lava_triangle_data.move({ 0.5f, 0, 0 });
    if (!lava_triangle->reload())
        return error::create_failed;

    // Initialize an int triangle
    struct int_vertex {
        std::array<int, 3> position;
        v4 color;
    };
    mesh_template<int_vertex>::ptr int_triangle;
    // Except for the first one, these template arguments are optional
    int_triangle = create_mesh<int_vertex, false, true, false, true, false, false>(app.device, mesh_type::triangle);
    if (!int_triangle)
        return error::create_failed;

    auto& int_triangle_data = int_triangle->get_data();
    int_triangle_data.vertices.at(0).color = v4(1.f, 0.5f, 0.5f, 1.f);
    int_triangle_data.vertices.at(1).color = v4(0.5f, 1.f, 0.5f, 1.f);
    int_triangle_data.vertices.at(2).color = v4(0.5f, 0.5f, 1.f, 1.f);
    int_triangle_data.scale(2);
    if (!int_triangle->reload())
        return error::create_failed;

    // Initialize a double triangle
    struct double_vertex {
        std::array<double, 3> position;
        v4 color;
    };
    mesh_template<double_vertex>::ptr double_triangle;
    // Except for the first one, these template arguments are optional
    double_triangle = create_mesh<double_vertex, false, true, false, true, false, false>(app.device, mesh_type::triangle);
    if (!double_triangle)
        return error::create_failed;

    auto& double_triangle_data = double_triangle->get_data();
    double_triangle_data.vertices.at(0).color = v4(1.f, 0.f, 0.5f, 1.f);
    double_triangle_data.vertices.at(1).color = v4(0.f, 1.f, 0.5f, 1.f);
    double_triangle_data.vertices.at(2).color = v4(0.f, 0.5f, 1.f, 1.f);
    double_triangle_data.scale(0.854);
    if (!double_triangle->reload())
        return error::create_failed;

    graphics_pipeline::ptr lava_pipeline;
    graphics_pipeline::ptr int_pipeline;
    graphics_pipeline::ptr double_pipeline;
    pipeline_layout::ptr layout;

    app.on_create = [&]() {
        render_pass::ptr render_pass = app.shading.get_pass();

        layout = make_pipeline_layout();
        if (!layout->create(app.device))
            return false;

        // Making a lava triangle pipeline
        lava_pipeline = make_graphics_pipeline(app.device);
        lava_pipeline->add_color_blend_attachment();
        lava_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
            lava_triangle->bind_draw(cmd_buf);
        };

        // Making an int triangle pipeline
        int_pipeline = make_graphics_pipeline(app.device);
        int_pipeline->add_color_blend_attachment();
        int_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
            int_triangle->bind_draw(cmd_buf);
        };

        // Making an double triangle pipeline
        double_pipeline = make_graphics_pipeline(app.device);
        double_pipeline->add_color_blend_attachment();
        double_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
            double_triangle->bind_draw(cmd_buf);
        };

        pipeline::shader_stage::ptr shader_stage = create_pipeline_shader_stage(app.device, app.producer.get_shader(_triangle_frag_), VK_SHADER_STAGE_FRAGMENT_BIT);
        if (!shader_stage)
            return false;

        // Describe the lava triangle
        if (!lava_pipeline->add_shader(app.producer.get_shader(_lava_triangle_), VK_SHADER_STAGE_VERTEX_BIT))
            return false;
        lava_pipeline->add(shader_stage);

        lava_pipeline->set_vertex_input_binding({ 0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX });
        lava_pipeline->set_vertex_input_attributes({
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, to_ui32(offsetof(vertex, position)) },
            { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, to_ui32(offsetof(vertex, color)) },
        });
        lava_pipeline->set_layout(layout);
        if (!lava_pipeline->create(render_pass->get()))
            return false;

        // Describe the int triangle
        if (!int_pipeline->add_shader(app.producer.get_shader(_int_triangle_), VK_SHADER_STAGE_VERTEX_BIT))
            return false;
        int_pipeline->add(shader_stage);

        int_pipeline->set_vertex_input_binding({ 0, sizeof(int_vertex), VK_VERTEX_INPUT_RATE_VERTEX });
        int_pipeline->set_vertex_input_attributes({
            { 0, 0, VK_FORMAT_R32G32B32_SINT, to_ui32(offsetof(int_vertex, position)) },
            { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, to_ui32(offsetof(int_vertex, color)) },
        });
        int_pipeline->set_layout(layout);
        if (!int_pipeline->create(render_pass->get()))
            return false;

        // Describe the double triangle
        if (!double_pipeline->add_shader(app.producer.get_shader(_double_triangle_), VK_SHADER_STAGE_VERTEX_BIT))
            return false;
        double_pipeline->add(shader_stage);

        double_pipeline->set_vertex_input_binding({ 0, sizeof(double_vertex), VK_VERTEX_INPUT_RATE_VERTEX });
        double_pipeline->set_vertex_input_attributes({
            { 0, 0, VK_FORMAT_R64G64B64_SFLOAT, to_ui32(offsetof(double_vertex, position)) },
            { 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, to_ui32(offsetof(double_vertex, color)) },
        });
        double_pipeline->set_layout(layout);
        if (!double_pipeline->create(render_pass->get()))
            return false;

        render_pass->add_front(lava_pipeline);
        render_pass->add_front(double_pipeline);
        render_pass->add_front(int_pipeline);

        return true;
    };

    app.on_destroy = [&]() {
        lava_pipeline->destroy();
        int_pipeline->destroy();
        double_pipeline->destroy();
        layout->destroy();
    };

    app.imgui.on_draw = [&]() {
        ImGui::SetNextWindowPos({ 30, 30 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ 220, 200 }, ImGuiCond_FirstUseEver);

        ImGui::Begin(app.get_name());

        bool lava_active = lava_pipeline->activated();
        if (ImGui::Checkbox("lava triangle", &lava_active))
            lava_pipeline->toggle();

        bool int_active = int_pipeline->activated();
        if (ImGui::Checkbox("int triangle", &int_active))
            int_pipeline->toggle();

        bool double_active = double_pipeline->activated();
        if (ImGui::Checkbox("double triangle", &double_active))
            double_pipeline->toggle();

        app.draw_about(draw_without_separator);

        ImGui::End();
    };

    app.add_run_end([&]() {
        lava_triangle->destroy();
        int_triangle->destroy();
        double_triangle->destroy();
    });

    return app.run();
}
