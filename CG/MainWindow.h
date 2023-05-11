#pragma once
#include "imgui.h"
#include "Widgets.h"
#include <vector>
#include <string>
#include "PhotonMapping.h"
class Window {
public:
    virtual void draw() = 0;
};
class MainWindow : public Window {
    class PhotonMapWindow : public Window {
        PhotonMapping* pm;
        InputSizeT gp_count;
        InputSizeT cp_count;
        InputSizeT gnp_count;
        InputSizeT cnp_count;
        InputFloat disc_comp;
    public:
        PhotonMapWindow(PhotonMapping* pm) : 
            gp_count("for global map", 100000), cp_count("for caustic map", 100000), 
            gnp_count("for global photon map", 2000), cnp_count("for caustic photon map", 500),
            disc_comp("Disc compression coef", 1.6f) {
            this->pm = pm;
            this->pm->update_gphc(gp_count.get_value());
            this->pm->update_cphc(cp_count.get_value());
            this->pm->update_gnp_count(gnp_count.get_value());
            this->pm->update_cnp_count(cnp_count.get_value());
            this->pm->update_disc_compression(disc_comp.get_value());
        }
        void draw() override {
            //ImGui::NewLine();
            ImGui::Text("Number of photons");
            if (gp_count.draw()) {
                pm->update_gphc(gp_count.get_value());
            }
            if (cp_count.draw()) {
                pm->update_cphc(cp_count.get_value());
            }
            ImGui::Text("Number of nearest photons used to estimate radiance");
            if (gnp_count.draw()) {
                pm->update_gnp_count(gnp_count.get_value());
            }
            if (cnp_count.draw()) {
                pm->update_cnp_count(cnp_count.get_value());
            }
            if (disc_comp.draw()) {
                pm->update_disc_compression(disc_comp.get_value());
            }
            ImGui::NewLine();
            if (ImGui::Button("Build maps")) {
                pm->build_map();
            }
        }
    };
    class PictureWindow : public Window {
        PhotonMapping* pm;
        InputFloat exposure;
        InputFloat brightness;
        Vec3Selector light_intensity;
        CheckBox pmii_only;
    public:
        PictureWindow(PhotonMapping* pm) : exposure("Exposure", 1.f),
            brightness("Brightness", 1.f), light_intensity(glm::vec3(15.f)),
            pmii_only("Use the photon map only for indirect illumination") {
            this->pm = pm;
            this->pm->update_exposure(exposure.get_value());
            this->pm->update_brightness(brightness.get_value());
            this->pm->update_ls_intensity(light_intensity.get_value());
            this->pm->update_dpmdi(pmii_only.get_value());
        }
        void draw() override {
            if (pmii_only.draw()) {
                pm->update_dpmdi(pmii_only.get_value());
            }
            if (exposure.draw()) {
                pm->update_exposure(exposure.get_value());
            }
            if (brightness.draw()) {
                pm->update_brightness(brightness.get_value());
            }
            ImGui::Text("Light intensity");
            if (light_intensity.draw()) {
                pm->update_ls_intensity(light_intensity.get_value());
            }
            ImGui::NewLine();
            if (ImGui::Button("Render")) {
                pm->render();
            }
        }
    };
    PhotonMapping* pm;
    std::vector<std::unique_ptr<Window>> windows;
    
    TabBar window_selector;
public:
    MainWindow(PhotonMapping* pm) : window_selector({"Picture", "Photon Maps"}) {
        this->pm = pm;
        windows.push_back(std::unique_ptr<Window>(new PictureWindow(pm)));
        windows.push_back(std::unique_ptr<Window>(new PhotonMapWindow(pm)));
    }
    void draw() override {
        ImGui::Begin("Window!", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        window_selector.draw();
        windows[window_selector.get_value()].get()->draw();
        ImGui::End();
    }
};

