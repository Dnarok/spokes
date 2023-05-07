#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/CinderImGui.h>
#include <cinder/Log.h>
#include <cinder/Perlin.h>

#include <glm/gtc/random.hpp>

#include <Windows.h>

struct spokes_app : public ci::app::App
{
    auto setup() -> void override;
    auto update() -> void override;
    auto draw() -> void override;

    auto keyDown(ci::app::KeyEvent event) -> void override;
    auto mouseDown(ci::app::MouseEvent event) -> void override;
    auto mouseUp(ci::app::MouseEvent event) -> void override;

    bool try_closing;
    bool record;

    int spokes;
    float line_width;
    float color_cycle;
    float cycle_start;
    float cycle_end;

    ci::Perlin generator;
    std::int32_t perlin_seed;
    std::uint8_t perlin_octaves;
    float perlin_x;
    float perlin_x_change;
    float perlin_y;
    float perlin_y_change;

    std::vector<glm::vec2> positions;
};

auto spokes_app::setup() -> void
{
    //setFullScreen(true);
    setWindowSize(800, 800);

    try_closing = false;
    record = false;
    spokes = 3;
    line_width = 1.f;
    color_cycle = 300.f;
    cycle_start = 0.f;
    cycle_end = 1.f;
    perlin_seed = 12345678;
    perlin_octaves = 4;
    generator = ci::Perlin{ perlin_octaves, perlin_seed };
    perlin_x = glm::linearRand(0.f, 10000.f);
    perlin_x_change = 0.005f;
    perlin_y = glm::linearRand(0.f, 10000.f);
    perlin_y_change = 0.005f;

    ImGui::Initialize();
};

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
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
                ImGui::SameLine(); 
                HelpMarker("The number of copies of the line that is being drawn.\nAll spokes are evenly spaced in a circle.");

                ImGui::SliderFloat("Line Width", &line_width, 1.f, 5.f);
                ImGui::SameLine();
                HelpMarker("The average pixel width of the drawn lines.");

                ImGui::SliderFloat("Color Cycle", &color_cycle, 50.f, 1000.f);
                ImGui::SameLine();
                HelpMarker("How many vertices to process before the color cycle is back to the start.");

                ImGui::SliderFloat("Cycle Start", &cycle_start, 0.f, 1.f);
                ImGui::SameLine();
                HelpMarker("Which hue the color cycle should start at.");

                ImGui::SliderFloat("Cycle End", &cycle_end, 0.f, 1.f);
                ImGui::SameLine();
                HelpMarker("Which hue the color cycle should end at.");

                if (ImGui::InputInt("Perlin Seed", &perlin_seed))
                {
                    generator.setSeed(perlin_seed);
                }
                ImGui::SameLine();
                HelpMarker("The seed that the perlin noise generator should use.");

                if (ImGui::InputScalar("Perlin Octave", ImGuiDataType_U8, &perlin_octaves))
                {
                    generator.setOctaves(perlin_octaves);
                }
                ImGui::SameLine();
                HelpMarker("The octaves that the perlin noise generator should use.");

                ImGui::SliderFloat("Perlin X Change", &perlin_x_change, 0.00001f, 0.1f, "%.5f");
                ImGui::SameLine();
                HelpMarker("The rate of change along the X axis.");

                ImGui::SliderFloat("Perlin Y Change", &perlin_y_change, 0.00001f, 0.1f, "%.5f");
                ImGui::SameLine();
                HelpMarker("The rate of change along the Y ayis.");

                ImGui::EndMenu();
            }

            ImGui::Separator();

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
            try_closing = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // ImGui::ShowDemoWindow();

    if (record)
    {
        perlin_x += perlin_x_change;
        float x = generator.fBm(perlin_x);
        perlin_y += perlin_y_change;
        float y = generator.fBm(perlin_y);
        glm::vec2 position{ getWindowWidth() * x, getWindowHeight() * y };
        if (positions.empty())
        {
            positions.push_back(position);
        }
        else if (position != positions.back())
        {
            positions.push_back(position);
        }
    }
};

auto spokes_app::draw() -> void
{
    ci::gl::clear();
    ci::gl::color(ci::Color::white());
    ci::gl::lineWidth(line_width);

    if (positions.size() > 1)
    {
        ci::gl::pushModelMatrix();

        ci::gl::translate(getWindowCenter());
        for (std::size_t i = 0; i < spokes; ++i)
        {
            ci::gl::rotate(2 * 3.1415926535897 / spokes);
            ci::gl::begin(GL_LINE_STRIP);
            for (std::size_t j = 0; j < positions.size(); ++j)
            {
                float hue = ci::lmap(std::fmod(j / color_cycle, 1.f), 0.f, 1.f, cycle_start, cycle_end);
                ci::gl::color(ci::hsvToRgb({ hue, 1.f, 1.f }));
                ci::gl::vertex(positions[j]);
            }
            ci::gl::end();
        }

        ci::gl::popModelMatrix();
    }

    ci::gl::drawSolidCircle({ 50, 50 }, 50);
};

auto spokes_app::keyDown(ci::app::KeyEvent event) -> void
{
    switch (event.getCode())
    {
    case ci::app::KeyEvent::KEY_MINUS:
        if (event.isShiftDown())
        {
            if (line_width > 0.99)
            {
                line_width -= 1.0;
            }
        }
        else
        {
            if (spokes > 1)
            {
                --spokes;
            }
        }
        break;
    case ci::app::KeyEvent::KEY_EQUALS:
        if (event.isShiftDown())
        {
            if (line_width < 10.01)
            {
                line_width += 1.0;
            }
        }
        else
        {
            if (spokes < 50)
            {
                ++spokes;
            }
        }
        break;
    case ci::app::KeyEvent::KEY_c:
        positions.clear();
        break;
    case ci::app::KeyEvent::KEY_r:
        perlin_x = glm::linearRand(0.f, 10000.f);
        perlin_y = glm::linearRand(0.f, 10000.f);
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
    case ci::app::KeyEvent::KEY_SPACE:
        record = !record;
        break;
    }
};

auto spokes_app::mouseDown(ci::app::MouseEvent event) -> void
{
    // if (event.isLeft())
    // {
    //     record = true;
    // }
};

auto spokes_app::mouseUp(ci::app::MouseEvent event) -> void
{
    // if (event.isLeft())
    // {
    //     record = false;
    // }
};