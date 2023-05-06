#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/CinderImGui.h>
#include <cinder/Log.h>

#include <Windows.h>

#include <numbers>

struct spokes_app : public ci::app::App
{
    auto setup() -> void override;
    auto update() -> void override;
    auto draw() -> void override;

    auto keyDown(ci::app::KeyEvent event) -> void override;
    auto mouseDown(ci::app::MouseEvent event) -> void override;
    auto mouseUp(ci::app::MouseEvent event) -> void override;

    bool try_closing;
    bool record_mouse;
    // bool do_clear;

    int spokes;

    // glm::ivec2 current_mouse_position;
    // glm::ivec2 previous_mouse_position;

    std::vector<glm::vec2> mouse_positions;
};

auto spokes_app::setup() -> void
{
    setFullScreen(true);
    //setWindowSize(800, 800);

    try_closing = false;
    record_mouse = false;
    spokes = 3;

    ImGui::Initialize();
};

auto spokes_app::update() -> void
{
    if (try_closing)
    {
        ImGui::OpenPopup("exit_modal");
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Settings"))
            {
                ImGui::SliderInt("Spokes", &spokes, 1, 50);

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Quit", "Ctrl + W"))
            {
                quit();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::SetNextWindowPos(getWindowCenter(), ImGuiCond_Appearing, { 0.5, 0.5 });
    if (ImGui::BeginPopupModal("exit_modal", &try_closing, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Do you wish to exit?");
        ImGui::Separator();
        if (ImGui::Button("Yes"))
        {
            quit();
        }
        ImGui::SameLine();
        if (ImGui::Button("No"))
        {
            // do_clear = true;
            try_closing = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::ShowDemoWindow();

    if (record_mouse)
    {
        // previous_mouse_position = current_mouse_position;
        // current_mouse_position = getMousePos() - (getWindowPos() + glm::ivec2{ getWindowCenter() });
        glm::vec2 mouse_position = getMousePos() - (getWindowPos() + glm::ivec2{ getWindowCenter() });
        if (mouse_positions.empty())
        {
            mouse_positions.push_back(mouse_position);
        }
        else if (mouse_position != mouse_positions.back())
        {
            mouse_positions.push_back(mouse_position);
        }
    }
};

auto spokes_app::draw() -> void
{
    // if (do_clear)
    // {
    //     do_clear = false;
    //     ci::gl::clear();
    // }
    ci::gl::clear();
    ci::gl::color(ci::Color::white());

    if (mouse_positions.size() > 1)
    {
        ci::gl::pushModelMatrix();

        ci::gl::translate(getWindowCenter());
        for (std::size_t i = 0; i < spokes; ++i)
        {
            ci::gl::rotate(2 * 3.1415926535897 / spokes);
            ci::gl::draw(mouse_positions);
        }

        ci::gl::popModelMatrix();
    }

    // if (previous_mouse_position != glm::ivec2{}&& previous_mouse_position != current_mouse_position)
    // {
    //     ci::gl::pushModelMatrix();
    //     ci::gl::translate(getWindowCenter());
    //     for (std::size_t i = 0; i < spokes; ++i)
    //     {
    //         ci::gl::rotate(2 * 3.1415926535897 / spokes);
    //         ci::gl::drawLine(previous_mouse_position, current_mouse_position);
    //     }
    //     ci::gl::popModelMatrix();
    // }

    ci::gl::drawSolidCircle({ 50, 50 }, 50);
};

auto spokes_app::keyDown(ci::app::KeyEvent event) -> void
{
    switch (event.getCode())
    {
    case ci::app::KeyEvent::KEY_c:
        // do_clear = true;
        mouse_positions.clear();
        break;
    case ci::app::KeyEvent::KEY_w:
        if (!event.isControlDown())
        {
            break;
        }
        [[fallthrough]];
    case ci::app::KeyEvent::KEY_ESCAPE:
        try_closing = true;
        break;
    }
};

auto spokes_app::mouseDown(ci::app::MouseEvent event) -> void
{
    if (event.isLeft())
    {
        record_mouse = true;
    }
};

auto spokes_app::mouseUp(ci::app::MouseEvent event) -> void
{
    if (event.isLeft())
    {
        record_mouse = false;
    }
};