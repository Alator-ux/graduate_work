#pragma once
#include "Texture.h"
#include "Tools.h"
#include "CImg.h"
#include <GLM/exponential.hpp>
#include "PMSettingsUpdater.h"
#include "OpenGLWrappers.h"
class PMDrawer
{
public:
    /// <summary>
    /// di - direct illumination;
    /// sp - specular;
    /// tr - transmission;
    /// em - emission
    /// gi - global illumination;
    /// ca - caustic;
    /// </summary>
    enum Layer { di = 0, sp, tr, em, gi, ca };
private:
    enum ShaderT {layer = 0, fxaa, ssaa, final };
    struct Canvas {
        unsigned int quadVAO;
        void init() {
            float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f
            };
            // screen quad VAO
            unsigned int quadVBO;
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }
        void draw() {
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    };
    class FrameBuffer {
    protected:
        std::string fb100x100, cb100x100;
        std::string fb300x300, cb300x300;
        std::string fb600x600, cb600x600;
        std::string fb_cur, cb_cur;
        OpenGLManager* manager;
        void gen_fb(const std::string& fbname, const std::string& cbname, size_t width, size_t height) {
            manager->gen_framebuffer(fbname);
            Texture tex(true);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            manager->set_texture(cbname, tex);
            manager->bind_framebuffer(fbname);
            manager->attach_colorbuffer(cbname);
            manager->check_framebuffer_completeness();
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    public:
        FrameBuffer() {}
        FrameBuffer(const FrameBuffer& other) {
            this->fb100x100 = other.fb100x100;
            this->fb300x300 = other.fb300x300;
            this->fb600x600 = other.fb600x600;
            this->fb_cur = other.fb_cur;
            this->cb100x100 = other.cb100x100;
            this->cb300x300 = other.cb300x300;
            this->cb600x600 = other.cb600x600;
            this->cb_cur = other.cb_cur;
            this->manager = other.manager;
        }
        FrameBuffer(const std::string& pref) {
            manager = OpenGLManager::get_instance();
            fb100x100 = pref + "fb100x100";
            cb100x100 = pref + "cb100x100";
            gen_fb(fb100x100, cb100x100, 100, 100);

            fb300x300 = pref + "fb300x300";
            cb300x300 = pref + "cb300x300";
            gen_fb(fb300x300, cb300x300, 300, 300);

            fb600x600 = pref + "fb600x600";
            cb600x600 = pref + "cb600x600";
            gen_fb(fb600x600, cb600x600, 600, 600);
        }
        void set_resolution(size_t width, size_t height) {
            if (width == 100) {
                fb_cur = fb100x100;
                cb_cur = cb100x100;
            }
            else if (width == 300) {
                fb_cur = fb300x300;
                cb_cur = cb300x300;
            }
            else if (width == 600) {
                fb_cur = fb600x600;
                cb_cur = cb600x600;
            }
        }
        void bind_fb() {
            manager->bind_framebuffer(fb_cur);
        }
        void unbind_fb() {
            manager->unbind_framebuffer();
        }
        void bind_cb(GLint dest = 0) {
            manager->get_texture(cb_cur).bind(dest);
        }
        void unbind_cb() {
            manager->get_texture(cb_cur).unbind();
        }
    };
    class TwoOutFrameBuffer : public FrameBuffer {
        std::string cb100x100_other, cb300x300_other, cb600x600_other;
        std::string cb_cur_other;
        void gen_fb(const std::string& fbname, const std::string& cbname, const std::string& cbname_other,
            size_t width, size_t height) {

            manager->gen_framebuffer(fbname);
            manager->bind_framebuffer(fbname);
            std::vector<std::string> names = { cbname, cbname_other };
            for (unsigned int i = 0; i < 2; i++)
            {
                Texture tex(true);
                glBindTexture(GL_TEXTURE_2D, tex.id);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL
                );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // присоединение текстуры к фреймбуферу
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex.id, 0
                );
                manager->set_texture(names[i], tex);
            }
            unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
            glDrawBuffers(2, attachments);
            // Bind the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    public:
        TwoOutFrameBuffer() {}
        TwoOutFrameBuffer(const TwoOutFrameBuffer& other) {
            this->fb100x100 = other.fb100x100;
            this->fb300x300 = other.fb300x300;
            this->fb600x600 = other.fb600x600;
            this->fb_cur = other.fb_cur;
            this->cb100x100 = other.cb100x100;
            this->cb300x300 = other.cb300x300;
            this->cb600x600 = other.cb600x600;
            this->cb_cur = other.cb_cur;
            this->cb100x100_other = other.cb100x100_other;
            this->cb300x300_other = other.cb300x300_other;
            this->cb600x600_other = other.cb600x600_other;
            this->cb_cur_other = other.cb_cur_other;
            this->manager = other.manager;
        }
        TwoOutFrameBuffer(const std::string& pref) {
            manager = OpenGLManager::get_instance();
            fb100x100 = pref + "fb100x100";
            cb100x100 = pref + "cb100x100";
            cb100x100_other = pref + "cb100x100_other";
            gen_fb(fb100x100, cb100x100, cb100x100_other, 100, 100);

            fb300x300 = pref + "fb300x300";
            cb300x300 = pref + "cb300x300";
            cb300x300_other = pref + "cb300x300_other";
            gen_fb(fb300x300, cb300x300, cb300x300_other, 300, 300);

            fb600x600 = pref + "fb600x600";
            cb600x600 = pref + "cb600x600";
            cb600x600_other = pref + "cb600x600_other";
            gen_fb(fb600x600, cb600x600, cb600x600_other, 600, 600);
        }
        void set_resolution(size_t width, size_t height) {
            if (width == 100) {
                fb_cur = fb100x100;
                cb_cur = cb100x100;
                cb_cur_other = cb100x100_other;
            }
            else if (width == 300) {
                fb_cur = fb300x300;
                cb_cur = cb300x300;
                cb_cur_other = cb300x300_other;
            }
            else if (width == 600) {
                fb_cur = fb600x600;
                cb_cur = cb600x600;
                cb_cur_other = cb600x600_other;
            }
        }
        const std::string& get_cb() {
            return cb_cur;
        }
        const std::string& get_ocb() {
            return cb_cur_other;
        }
    };
    class BloomShader {
        Shader shader;
        PMDrawer* dr;
        OpenGLManager* manager;
    public:
        bool horiz;
        BloomShader() : shader() {}
        void init(PMDrawer* dr) {
            this->dr = dr;
            shader.init_shader("pp_default.vert", "bloom.frag");
            auto kernel = one_dim_gkernel(5, 1.0f);
            shader.use_program();
            shader.uniform1i("tex", 0);
            shader.uniform1i("weightsSize", kernel.size());
            shader.uniform1i("weights", 1);
            shader.disable_program();
            Texture tex(true);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_FLOAT, kernel.size(), 1,
                0, GL_FLOAT, GL_FLOAT, (void*)kernel.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            manager = OpenGLManager::get_instance();
            manager->set_texture("bloomWeights", tex);
            update();
            manager->checkOpenGLerror();
        }
        void draw(size_t count, FrameBuffer& first, FrameBuffer& second,
            TwoOutFrameBuffer& orig) {
            shader.use_program();
            manager->get_texture("bloomWeights").bind(1);
            bool horizontal = true;
            for (size_t i = 0; i < count; i++)
            {
                if (horizontal) {
                    second.bind_fb();
                    shader.uniform2f("dir", glm::vec2(1.f, 0.f));
                }
                else {
                    first.bind_fb();
                    shader.uniform2f("dir", glm::vec2(0.f, 1.f));
                }
                if (i == 0) {
                    manager->get_texture(orig.get_ocb()).bind();
                }
                else if (horizontal) {
                    first.bind_cb();
                }
                else {
                    second.bind_cb();
                }
                dr->canvas.draw();
                horizontal = !horizontal;
            }
            shader.disable_program();
            horiz = horizontal;
        }
        void update() {
            shader.use_program();
            shader.uniform2f("texelSize",
                glm::vec2(1.f / (float)dr->settings.width, 1.f / (float)dr->settings.height));
            shader.disable_program();

        }
    };
    const size_t layers_number;
    std::vector<Matrix<glm::vec3>> layers;
    PMDrawerSettings settings;
    std::vector<Texture> textures;
    std::vector<Shader> shaders;
    OpenGLManager* manager;
    Canvas canvas;
    FrameBuffer buffer1, buffer2, buffer3;
    TwoOutFrameBuffer tobuffer1;
    BloomShader bloom_shader;
    void update_textures() {
        for (size_t i = 0; i < layers_number; i++) {
            auto tex = textures[i];
            tex.del();
            tex = Texture(true);
            fill_texture(tex, layers[i].data());
            textures[i] = tex;
        }
    }
    void updateLayerShader() {
        shaders[layer].use_program();
        //shaders[layer].uniform1f("g_mult", settings.layerShader.gl_mult);
        //shaders[layer].uniform1f("ca_mult", settings.layerShader.ca_mult);
        //shaders[layer].uniform1f("brightness", settings.layerShader.brightness);
        shaders[layer].uniform1f("exposure", settings.layerShader.exposure);
        shaders[layer].uniform1f("gamma", settings.layerShader.gamma);
        shaders[layer].uniform1f("HDR_ON", settings.layerShader.hdr);
        shaders[layer].uniform1iv("active", settings.layerShader.active.data(), settings.layerShader.active.size());
        shaders[layer].disable_program();
        manager->checkOpenGLerror();
    }
    void updateFXAAShader() {
        shaders[fxaa].use_program();
        shaders[fxaa].uniform2f("texCoordsOffset", 
            glm::vec2(1.f / (float)settings.width, 1.f / (float)settings.height));
        shaders[fxaa].uniform1f("SpanMax", 8.f);
        shaders[fxaa].uniform1f("ReduceMin", 1.f / 128.f);
        shaders[fxaa].uniform1f("ReduceMul", 1.f / 8.f);
        shaders[fxaa].disable_program();
        manager->checkOpenGLerror();
    }
    void updateSSAAShader() {
        shaders[ssaa].use_program();
        shaders[ssaa].uniform2f("texelSize",
            glm::vec2(1.f / (float)settings.width, 1.f / (float)settings.height));
        shaders[ssaa].disable_program();
        manager->checkOpenGLerror();
    }
    void update_resolution(size_t width, size_t height) {
        std::generate(layers.begin(), layers.end(),
            [&width, &height]() { return Matrix<glm::vec3>(width, height, glm::vec3(0.f)); });
    }
    void update_framebuffer(size_t width, size_t height) {
        buffer1.set_resolution(width, height);
        buffer2.set_resolution(width, height);
        buffer3.set_resolution(width, height);
        tobuffer1.set_resolution(width, height);
        /*manager->bind_framebuffer("frameBuffer100x100");
        auto tex = manager->get_texture("colorBuffer100x100");
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        manager->attach_colorbuffer("colorBuffer100x100");
        manager->check_framebuffer_completeness();*/
    }
    void update() {
        if (settings.changed.layers) {
            update_textures();
            settings.changed.layers = false;
        }
        if (settings.changed.layerShader) {
            updateLayerShader();
            settings.changed.layerShader = false;
        }
        //settings.changed.drawable = false;
        /*int width = final_layer.get_width();
        int height = final_layer.get_height();
        glm::vec3 rgb(0.f);
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                rgb.r = rgb.g = rgb.b = 0.f;
                for (size_t l = 0; l < Layer::gi; l++) {
                    rgb += settings.active[l] ? layers[l](i, j) : glm::vec3(0.f);
                }
                rgb += settings.active[Layer::gi] ? layers[Layer::gi](i, j) * settings.gl_mult 
                    : glm::vec3(0.f);
                rgb += settings.active[Layer::ca] ? layers[Layer::ca](i, j) * settings.ca_div
                    : glm::vec3(0.f);
                if (settings.hdr) {
                    hdr(rgb);
                }
                else {
                    rgb = glm::clamp(rgb, 0.f, 1.f);
                }
                rgb *= settings.brightness;
                final_layer.set_rgb(i, j, rgb * 255.f);
            }
        }*/
    }
    void hdr(glm::vec3& dest) {
        dest = glm::vec3(1.f) - glm::exp(-dest * settings.layerShader.exposure);
        dest = glm::pow(dest, glm::vec3(1.f / settings.layerShader.gamma));
    }
    void fill_texture(Texture& tex, glm::vec3* data) {
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, settings.width, settings.height,
            0, GL_RGB, GL_FLOAT, (void*)data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
public:
    void opengl_init() {
        settings.width = 100;
        settings.height = 100;
        manager = OpenGLManager::get_instance();
        buffer1 = FrameBuffer("first");
        buffer2 = FrameBuffer("second");
        buffer3 = FrameBuffer("third");
        tobuffer1 = TwoOutFrameBuffer("two_first");

        canvas.init();
        manager->checkOpenGLerror();
        for (size_t i = 0; i < layers_number; i++) {
            auto tex = Texture(true);
            fill_texture(tex, layers[i].data());
            tex.initialized = true;
            textures[i] = tex;
        }

        Shader tshader;
        tshader.init_shader("pp_default.vert", "layer.frag");
        shaders.push_back(tshader);

        tshader.init_shader("pp_default.vert", "fxaa.frag");
        tshader.use_program();
        tshader.uniform1i("tex", 0);
        tshader.disable_program();
        shaders.push_back(tshader);

        tshader.init_shader("pp_default.vert", "ssaa.frag");
        tshader.use_program();
        tshader.uniform1i("tex", 0);
        tshader.disable_program();
        shaders.push_back(tshader);

        tshader.init_shader("pp_default.vert", "pp_final.frag");
        tshader.use_program();
        tshader.uniform1i("final", 0);
        tshader.disable_program();
        shaders.push_back(tshader);


       // bloom_shader.init(this);
       // bloom_shader.update();

        updateSSAAShader();
    }
    PMDrawer(size_t width, size_t height) : layers_number(6), layers(layers_number), 
        textures(layers_number) {
        settings.layerShader.active = std::vector<int>(layers_number);
        std::generate(settings.layerShader.active.begin(), settings.layerShader.active.end(), 
            []() { return false; });

        settings.width = width;
        settings.height = height;
        update_resolution(width, height);
    }
    void set_rgb(int i, int j, const glm::vec3& color, Layer layer, int depth) {
        if (depth > 0) {
            return;
        }
        layers[layer](i, j) = color;
    }
    void display() {
        update();
        
        
        {   // Главный шейдер, суммирует слои и приводит из HDR к LDR.
            glViewport(0, 0, settings.width, settings.height);
            tobuffer1.bind_fb();
            shaders[layer].use_program();
            GLint lloc = shaders[layer].get_uniform_location("layers");
            for (int i = 0; i < textures.size(); i++) {
                textures[i].bind(i);
                shaders[layer].uniform1i(lloc, i);
                lloc++;
            }
            canvas.draw();
            shaders[layer].disable_program();
            tobuffer1.unbind_fb();
        }
        {   // FXAA
            /*buffer1.bind_fb();
            shaders[fxaa].use_program();
            tobuffer1.bind_cb();
            canvas.draw();
            shaders[fxaa].disable_program();
            buffer1.unbind_fb();*/
        }
        {   // SSAA
            
           /* buffer1.set_resolution(settings.width / 2.f, settings.height / 2.f);
            buffer1.bind_fb();
            shaders[ssaa].use_program();
            tobuffer1.bind_cb();
            canvas.draw();
            shaders[ssaa].disable_program();
            buffer1.unbind_fb();*/
            
        }
        {   // Bloom
            //bloom_shader.draw(10, buffer2, buffer3, tobuffer1);
        }
        {   // Просто вывод итоговой текстуры в нужное место
            glViewport(500, 0, settings.width, settings.height);
            shaders[final].use_program();
            manager->get_texture(tobuffer1.get_cb()).bind();
            canvas.draw();
            shaders[final].disable_program();
            manager->checkOpenGLerror();
        }



        OpenGLManager::checkOpenGLerror();
    }
    void clear() {
        for (auto& mat : layers) {
            mat.clear();
            mat.fill(glm::vec3(0.f));
        }
    }
    void set_settings_updater(PMSettingsUpdater& pmsu) {
        pmsu.link_drawer(&settings);
    }
    void check_resolution() {
        if (settings.changed.resolution) {
            update_resolution(settings.width, settings.height);
            update_framebuffer(settings.width, settings.height);
            manager->unbind_framebuffer();
            updateFXAAShader();
            updateSSAAShader();
            settings.changed.resolution = false;
        }
    }
    int get_width() {
        return settings.width;
    }
    int get_height() {
        return settings.height;
    }
};