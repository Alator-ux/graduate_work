#pragma once
#include "Texture.h"
#include "Tools.h"
#include "CImg.h"
#include <GLM/exponential.hpp>
#include "PMSettingsUpdater.h"
class PMDrawer
{
public:
    /// <summary>
    /// di - direct illumination;
    /// gi - global illumination;
    /// sp - specular;
    /// tr - transmission;
    /// ca - caustic;
    /// em - emission
    /// </summary>
    enum Layer { di = 0, sp, tr, em, gi, ca };
private:
    const size_t layers_number;
    std::vector<Matrix<glm::vec3>> layers;
    CImgTexture final_layer;
    cimg_library::CImgDisplay main_disp;
    PMDrawerSettings settings;
    void update() {
        int width = final_layer.get_width();
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
        }
    }
    void hdr(glm::vec3& dest) {
        dest = glm::vec3(1.f) - glm::exp(-dest * settings.exposure);
        dest = glm::pow(dest, glm::vec3(1.f / settings.gamma));
    }
public:
    PMDrawer(size_t width, size_t height) : layers_number(6), layers(layers_number),
        final_layer(CImgTexture(width, height, (unsigned char)0)) {
        std::generate(layers.begin(), layers.end(), 
            [&width, &height]() { return Matrix<glm::vec3>(width, height, glm::vec3(0.f)); });

        settings.active = std::vector<bool>(layers_number);
        std::generate(settings.active.begin(), settings.active.end(), []() { return false; });
        main_disp = cimg_library::CImgDisplay(final_layer.image, "Canvas");
    }
    void set_rgb(int i, int j, const glm::vec3& color, Layer layer, int depth) {
        if (depth > 0) {
            return;
        }
        layers[layer](i, j) = color;
    }
    void display() {
        if (settings.redraw) {
            update();
            settings.redraw = false;
        }
        main_disp.display(final_layer.image);
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
    int get_width() {
        return final_layer.image.width();
    }
    int get_height() {
        return final_layer.image.height();
    }
};