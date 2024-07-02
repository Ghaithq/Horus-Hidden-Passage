#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"

namespace our {

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json& config){
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if(config.contains("sky")){
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));
            
            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram* skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();
            
            //TODO: (Req 10) Pick the correct pipeline state to draw the sky
            // Hints: the sky will be drawn after the opaque objects so we would need depth testing but which depth funtion should we pick?
            // We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            skyPipelineState.faceCulling.enabled=true;
            skyPipelineState.faceCulling.culledFace= GL_FRONT;
            skyPipelineState.faceCulling.frontFace = GL_CCW;
            skyPipelineState.depthTesting.enabled=true;
            skyPipelineState.depthTesting.function=GL_LEQUAL;

            
            
            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D* skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky 
            Sampler* skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material 
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if(config.contains("postprocess")){
            //TODO: (Req 11) Create a framebuffer
            GLuint framebuffer;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
            postprocessFrameBuffer = framebuffer; // save the framebuffer in the member variable for the ForwardRenderer class

            //TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            // Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            // The depth format can be (Depth component with 24 bits).
            colorTarget = our::texture_utils::empty(GL_RGBA8, windowSize);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);

            depthTarget = our::texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            //TODO: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler* postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram* postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy(){
        // Delete all objects related to the sky
        if(skyMaterial){
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if(postprocessMaterial){
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::addLight(our::ShaderProgram* program)
    {
        int light_index = 0;
        
        std::string prefix = "lights[" + std::to_string(light_index) + "].";
        for(auto& light:lightSources)
        {
            if(light->enabled){
                program->set(prefix + "diffuse", light->diffuse);
                program->set(prefix + "specular", light->specular);
                program->set(prefix + "ambient", light->ambient);
                program->set(prefix + "type", static_cast<int>(light->lightType));
                program->set(prefix + "color", (light->color));
                program->set(prefix + "direction", glm::normalize(glm::vec3((light->getOwner()->getLocalToWorldMatrix() * glm::vec4(light->direction, 0)))));
                switch (light->lightType) {
                    case LightType::DIRECTIONAL:
                        break;
                    case LightType::POINT:
                        
                        program->set(prefix + "position", light->position);
                        program->set(prefix + "attenuation_constant", light->attenuation.constant);
                        program->set(prefix + "attenuation_linear", light->attenuation.linear);
                        program->set(prefix + "attenuation_quadratic", light->attenuation.quadratic);
                        break;
                    case LightType::SPOT:
                        program->set(prefix + "position", light->position);
                        program->set(prefix + "attenuation_constant", light->attenuation.constant);
                        program->set(prefix + "attenuation_linear", light->attenuation.linear);
                        program->set(prefix + "attenuation_quadratic", light->attenuation.quadratic);
                        program->set(prefix + "inner_angle", light->spot_angle.inner);
                        program->set(prefix + "outer_angle", light->spot_angle.outer);
                        break;
                }
                light_index++;

            }


        }
        program->set("light_count",light_index);
    }

    void ForwardRenderer::render(World* world){
        // First of all, we search for a camera and for all the mesh renderers
        lightSources.clear();
        CameraComponent* camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        for(auto entity : world->getEntities()){
            // If we hadn't found a camera yet, we look for a camera in this entity
            if(!camera) camera = entity->getComponent<CameraComponent>();
            if (const auto light = entity->getComponent<LightComponent>(); light)
            {
                //light->position=glm::vec3(light->getOwner()->getLocalToWorldMatrix() * glm::vec4(light->getOwner()->localTransform.position,1));
                
                if(light->lightType!=LightType::DIRECTIONAL)
                    light->position = glm::vec3(light->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1));
                lightSources.push_back(light);
                
            }
            // If this entity has a mesh renderer component
            if(auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer){
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if(command.material->transparent){
                    transparentCommands.push_back(command);
                } else {
                // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if(camera == nullptr) return;

        //TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        // HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        glm::vec3 cameraForward =(camera->getOwner()->getLocalToWorldMatrix()*glm::vec4(0.0, 0.0, -1.0f,0.0));
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand& first, const RenderCommand& second){
            //TODO: (Req 9) Finish this function
            //HINT: the following return should return true "first" should be drawn before "second". 
            //assuming the camera is at (0,0,0), and the foward direction is (0.0, 0.0, -1.0f)
            // the criterion for sorting is which is closer
            return glm::dot(first.center,cameraForward) > glm::dot(second.center,cameraForward);
        });

        //TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP=camera->getProjectionMatrix(windowSize)*camera->getViewMatrix();
        
        //TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0,0,this->windowSize.x,this->windowSize.y);
        
        //TODO: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0,0.0,0.0,1.0);
        glClearDepth(1);
        
        //TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(true, true, true, true);
        glDepthMask(true);

        // If there is a postprocess material, bind the framebuffer
        if(postprocessMaterial){
            //TODO: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
        }
       
        //TODO: (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //TODO: (Req 9) Draw all the opaque commands
        glm::vec3 cameraPos = camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for(auto opaqueCommand:opaqueCommands)
        {
            opaqueCommand.material->setup();
            if(dynamic_cast<LitMaterial*>(opaqueCommand.material))
            {
                
                addLight(opaqueCommand.material->shader);
                opaqueCommand.material->shader->set("object_to_world",opaqueCommand.localToWorld);
                opaqueCommand.material->shader->set("object_to_world_inv_transpose",glm::transpose(glm::inverse(opaqueCommand.localToWorld)));
                opaqueCommand.material->shader->set("view_projection",VP);
                opaqueCommand.material->shader->set("camera_position", cameraPos);
                // opaqueCommand.material->shader->set("material.diffuse", ((LitMaterial*)opaqueCommand.material)->diffuse);
                opaqueCommand.material->shader->set("material.diffuse", ((LitMaterial*)opaqueCommand.material)->diffuse);
                opaqueCommand.material->shader->set("material.specular", ((LitMaterial*)opaqueCommand.material)->specular);
                opaqueCommand.material->shader->set("material.ambient", ((LitMaterial*)opaqueCommand.material)->ambient);
                opaqueCommand.material->shader->set("material.shininess",((LitMaterial*)opaqueCommand.material)->shininess);
            }

            opaqueCommand.material->shader->set("transform",VP*opaqueCommand.localToWorld);
            opaqueCommand.mesh->draw();
        }
        // If there is a sky material, draw the sky
        if(this->skyMaterial){
            //TODO: (Req 10) setup the sky material
            skyMaterial->setup();

            //TODO: (Req 10) Get the camera position
            glm::vec3 cameraPos = ((camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0f)));
            //TODO: (Req 10) Create a model matrix for the sky such that it always follows the camera (sky sphere center = camera position)
            // skyMaterial->
            glm::mat4 model = glm::translate(glm::mat4(1.0f) , cameraPos);


            //TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            // We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f
            );
            //TODO: (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform",alwaysBehindTransform * VP * model);

            //TODO: (Req 10) draw the sky sphere
            skySphere->draw();
        }


        
        //TODO: (Req 9) Draw all the transparent commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for(auto transparentCommand:transparentCommands)
        {
            // addLight(transparentCommand.material->shader);
            // transparentCommand.material->shader->set("material.diffuse",transparentCommand.material->diffuse);
            // transparentCommand.material->shader->set("material.specular",transparentCommand.material->specular);
            // transparentCommand.material->shader->set("material.ambient",transparentCommand.material->ambient);
            // transparentCommand.material->shader->set("material.shininess",transparentCommand.material->shininess);
            transparentCommand.material->setup();
            if (dynamic_cast<LitMaterial*>(transparentCommand.material))
            {

                addLight(transparentCommand.material->shader);
                transparentCommand.material->shader->set("object_to_world",transparentCommand.localToWorld);
                transparentCommand.material->shader->set("object_to_world_inv_transpose",glm::transpose(glm::inverse(transparentCommand.localToWorld)));
                transparentCommand.material->shader->set("view_projection",VP);
                transparentCommand.material->shader->set("camera_position", cameraPos);
                // opaqueCommand.material->shader->set("material.diffuse", ((LitMaterial*)opaqueCommand.material)->diffuse);
                transparentCommand.material->shader->set("material.diffuse", ((LitMaterial*)transparentCommand.material)->diffuse);
                transparentCommand.material->shader->set("material.specular", ((LitMaterial*)transparentCommand.material)->specular);
                transparentCommand.material->shader->set("material.ambient", ((LitMaterial*)transparentCommand.material)->ambient);
                transparentCommand.material->shader->set("material.shininess",((LitMaterial*)transparentCommand.material)->shininess);
            }
            transparentCommand.material->shader->set("transform", VP*transparentCommand.localToWorld);
            transparentCommand.mesh->draw();
        }
        

        // If there is a postprocess material, apply postprocessing
        if(postprocessMaterial){
            //TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            //TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();
            glBindVertexArray(postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

}