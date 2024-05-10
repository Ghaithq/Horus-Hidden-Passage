#pragma once

#include <application.hpp>
#include <shader/shader.hpp>
#include <texture/texture2d.hpp>
#include <texture/texture-utils.hpp>
#include <material/material.hpp>
#include <mesh/mesh.hpp>

#include <functional>
#include <array>


class Loadingstate : public our::State {

    our::TexturedMaterial* screenMaterial;
    our::TintedMaterial* highlightMaterial;
    our::Mesh* rectangle;
    float time;

    void onInitialize() override {
        screenMaterial = new our::TexturedMaterial();
        screenMaterial->shader = new our::ShaderProgram();
        screenMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        screenMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        screenMaterial->shader->link();
        screenMaterial->texture = our::texture_utils::loadImage("assets/textures/loading_screen.jpg");
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
    }

    void onDraw(double deltaTime) override {
        glm::ivec2 size = getApp()->getFrameBufferSize();
        glViewport(0, 0, size.x, size.y);
        glm::mat4 VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        time += (float)deltaTime;
        screenMaterial->tint = glm::vec4(glm::smoothstep(0.00f, 2.00f, time));
        screenMaterial->setup();
        screenMaterial->shader->set("transform", VP * M);
        rectangle->draw();
        
        // wait for 2 seconds to load the loading screen
        if (time > 2.0f)
            getApp()->changeState("play");
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