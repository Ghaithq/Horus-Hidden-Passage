#pragma once

#include <application.hpp>
#include <shader/shader.hpp>
#include <texture/texture2d.hpp>
#include <texture/texture-utils.hpp>
#include <material/material.hpp>
#include <mesh/mesh.hpp>

#include <functional>
#include <array>

class Losestate : public our::State {

    our::TexturedMaterial* screenMaterial;
    our::TintedMaterial* highlightMaterial;
    our::Mesh* rectangle;
    float time;
    std::array<Button, 2> buttons;

    void onInitialize() override {
        screenMaterial = new our::TexturedMaterial();
        screenMaterial->shader = new our::ShaderProgram();
        screenMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        screenMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        screenMaterial->shader->link();
        screenMaterial->texture = our::texture_utils::loadImage("assets/textures/lose_screen.jpg");
        screenMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        highlightMaterial = new our::TintedMaterial();
        highlightMaterial->shader = new our::ShaderProgram();
        highlightMaterial->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        highlightMaterial->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        highlightMaterial->shader->link();
        highlightMaterial->tint = glm::vec4(0.937f, 0.569f, 0.008f, 1.0f);
        highlightMaterial->pipelineState.blending.enabled = true;
        highlightMaterial->pipelineState.blending.equation = GL_FUNC_SUBTRACT;
        highlightMaterial->pipelineState.blending.sourceFactor = GL_ONE;
        highlightMaterial->pipelineState.blending.destinationFactor = GL_ONE;

        rectangle = new our::Mesh({
            {{0.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            }, {
                0, 1, 2, 2, 3, 0,
            });

        time = 0;

        buttons[0].position = { 100.0f, 220.0f };
        buttons[0].size = { 330.0f, 60.0f };
        buttons[0].action = [this]() {this->getApp()->changeState("play"); };

        buttons[1].position = { 100.0f, 320.0f };
        buttons[1].size = { 200.0f, 60.0f };
        buttons[1].action = [this]() {this->getApp()->close(); };
    }

    void onDraw(double deltaTime) override {
        auto& keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_SPACE)) {
            getApp()->changeState("play");
        }
        else if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            getApp()->close();
        }

        auto& mouse = getApp()->getMouse();
        glm::vec2 mousePosition = mouse.getMousePosition();

        if (mouse.justPressed(0)) {
            for (auto& button : buttons) {
                if (button.isInside(mousePosition))
                    button.action();
            }
        }

        glm::ivec2 size = getApp()->getFrameBufferSize();
        glViewport(0, 0, size.x, size.y);
        glm::mat4 VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        time += (float)deltaTime;
        screenMaterial->tint = glm::vec4(glm::smoothstep(0.00f, 2.00f, time));
        screenMaterial->setup();
        screenMaterial->shader->set("transform", VP * M);
        rectangle->draw();

        for (auto& button : buttons) {
            if (button.isInside(mousePosition)) {
                highlightMaterial->setup();
                highlightMaterial->shader->set("transform", VP * button.getLocalToWorld());
                rectangle->draw();
            }
        }

    }

    void onDestroy() override {
        delete rectangle;
        delete screenMaterial->texture;
        delete screenMaterial->shader;
        delete screenMaterial;
        delete highlightMaterial->shader;
        delete highlightMaterial;
    }
};