#pragma once
#include "imgui.h"
#include "Widgets.h"
#include <vector>
#include <string>
#include "PMSettingsUpdater.h"
#include "PMDrawer.h"
#include "PhotonMapping.h"
class Window {
public:
    virtual void draw() = 0;
};
class MainWindow : public Window {
    class PhotonTracingWindow : public Window {
        PMSettingsUpdater* pmsu;
        PhotonMapping* pm;
        InputSizeT gp_count;
        InputSizeT cp_count;
    public:
        PhotonTracingWindow(PMSettingsUpdater* pmsu, PhotonMapping* pm) :
            gp_count("for global map", 100000), cp_count("for caustic map", 100000) {
            this->pm = pm;
            this->pmsu = pmsu;
            this->pmsu->update_gphc(gp_count.get_value());
            this->pmsu->update_cphc(cp_count.get_value());
        }
        void draw() override {
            ImGui::Text("Number of photons");
            if (gp_count.draw()) {
                pmsu->update_gphc(gp_count.get_value());
            }
            if (cp_count.draw()) {
                pmsu->update_cphc(cp_count.get_value());
            }
            ImGui::NewLine();
            if (ImGui::Button("Build maps")) {
                pm->build_map();
            }
        }
    };
    class RayTracingWindow : public Window {
        PhotonMapping* pm;
        PMSettingsUpdater* pmsu;
        CheckBox pmii_only;
        InputSizeT gnp_count;
        InputSizeT cnp_count;
        InputFloat disc_comp;
        InputFloat ca_div;
        InputFloat gl_mult;
        DropDownMenu gtype;
        DropDownMenu ctype;
    public:
        RayTracingWindow(PMSettingsUpdater* pmsu, PhotonMapping* pm) : 
            pmii_only("Use the photon map only for indirect illumination"),
            gnp_count("for global photon map", 2000), cnp_count("for caustic photon map", 500),
            disc_comp("Disc compression coef", 1.6f), 
            gtype("for global map", {"None", "Cone", "Gaussian"}, 60),
            ctype("for caustic map", { "None", "Cone", "Gaussian"}, 60),
            ca_div("Caustic divider", 16.f), gl_mult("Global multiplier", 10.f) {
            this->pm = pm;
            this->pmsu = pmsu;
            this->pmsu->update_gnp_count(gnp_count.get_value());
            this->pmsu->update_cnp_count(cnp_count.get_value());
            this->pmsu->update_disc_compression(disc_comp.get_value());
            this->pmsu->update_dpmdi(convert<bool>(pmii_only.get_value()));
            this->pmsu->update_rcaustic_divider(ca_div.get_value());
            this->pmsu->update_rglobal_multiplier(gl_mult.get_value());
        }
        void draw() override {
            if (pmii_only.draw()) {
                pmsu->update_dpmdi(convert<bool>(pmii_only.get_value()));
            }
            ImGui::Text("Number of nearest photons used to estimate radiance");
            if (gnp_count.draw()) {
                pmsu->update_gnp_count(gnp_count.get_value());
            }
            if (cnp_count.draw()) {
                pmsu->update_cnp_count(cnp_count.get_value());
            }
            ImGui::Text("Filter for photon weights");
            if (gtype.draw()) {
                pmsu->update_gfilter(convert<int>(gtype.get_value()));
            }
            if (ctype.draw()) {
                pmsu->update_cfilter(convert<int>(ctype.get_value()));
            }
            if (gl_mult.draw()) {
                pmsu->update_rcaustic_divider(ca_div.get_value());
            }
            if (ca_div.draw()) {
                pmsu->update_rglobal_multiplier(gl_mult.get_value());
            }
            if (disc_comp.draw()) {
                pmsu->update_disc_compression(disc_comp.get_value());
            }
            ImGui::NewLine();
            if (ImGui::Button("Render")) {
                pm->render();
            }
        }
    };
    class FastChangesWindow : public Window {
        PMDrawer* drawer;
        PMSettingsUpdater* pmsu;
        Row layers;
        InputFloat exposure;
        InputFloat brightness;
        InputFloat ca_div;
        InputFloat gl_mult;
        CheckBox hdr;
    public:
        FastChangesWindow(PMSettingsUpdater* pmsu, PMDrawer* drawer) :
            exposure("Exposure", 1.f), brightness("Brightness", 1.f),
            ca_div("Caustic divider", 16.f), gl_mult("Global multiplier", 10.f),
            layers("Active layers"), hdr("HDR on", false)
        {
            layers.set_childs(std::vector<std::shared_ptr <Widget>>{
                std::shared_ptr <Widget>(new CheckBox("di", true)),
                    std::shared_ptr <Widget>(new CheckBox("sp", true)),
                    std::shared_ptr <Widget>(new CheckBox("tr", true)),
                    std::shared_ptr <Widget>(new CheckBox("em", true)),
                    std::shared_ptr <Widget>(new CheckBox("gi", true)),
                    std::shared_ptr <Widget>(new CheckBox("ca", true)),
            });
            this->drawer = drawer;
            this->pmsu = pmsu;
            this->pmsu->update_exposure(exposure.get_value());
            this->pmsu->update_brightness(brightness.get_value());
            this->pmsu->update_dcaustic_divider(ca_div.get_value());
            this->pmsu->update_dglobal_multiplier(gl_mult.get_value());
            this->pmsu->update_hdr(convert<bool>(hdr.get_value()));
            for (size_t i = 0; i < layers.get_childs()->size(); i++) {
                this->pmsu->update_active_layers(i, true);
            }
            this->pmsu->redraw(false);
        }
        void draw() override {
            hdr.draw();
            exposure.draw();
            brightness.draw();
            ca_div.draw();
            gl_mult.draw();
            layers.draw();

            ImGui::NewLine();
            if (ImGui::Button("Apply")) {
                auto layers_p = layers.get_childs();
                for (size_t i = 0; i < layers_p->size(); i++) {
                   auto& ptr = layers_p->at(i);
                   pmsu->update_active_layers(i, ptr.get()->activated());
                }
                pmsu->update_exposure(exposure.get_value());
                pmsu->update_brightness(brightness.get_value());
                pmsu->update_dcaustic_divider(ca_div.get_value());
                pmsu->update_dglobal_multiplier(gl_mult.get_value());
                pmsu->update_hdr(convert<bool>(hdr.get_value()));
            }
        }
    };
    PMSettingsUpdater* pmsu;
    std::vector<std::unique_ptr<Window>> windows;
    
    TabBar window_selector;
public:
    MainWindow(PMSettingsUpdater* pmsu, PhotonMapping* pm, PMDrawer* drawer) :
        window_selector({"Photon Tr", "Ray Tr", "Fast Ch"}) {
        this->pmsu = pmsu;
        windows.push_back(std::unique_ptr<Window>(new PhotonTracingWindow(pmsu, pm)));
        windows.push_back(std::unique_ptr<Window>(new RayTracingWindow(pmsu, pm)));
        windows.push_back(std::unique_ptr<Window>(new FastChangesWindow(pmsu, drawer)));
    }
    void draw() override {
        ImGui::Begin("Window!", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        window_selector.draw();
        windows[window_selector.get_value()].get()->draw();
        ImGui::End();
    }
};

