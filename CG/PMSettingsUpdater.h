#pragma once
#include "PMTools.h"

struct PhotonMapSettings {
    enum FilterType { none = 0, cone, gaussian };
    FilterType ftype = none;
    size_t np_size = 2000;
    // Cone filter coef k
    const float cf_k = 1.1f; // Must be >= 1
    // Gaussian filter coef alpha
    const float gf_alpha = 1.818f;
    // Gaussian filter coef beta
    const float gf_beta = 1.953f;
    float disc_compression = 1.6f;
};

struct PhotonMappingSettings {
    // disable photon mapping for direct illumination
    bool dpmdi = false;
    int max_rt_depth = 2;
    float ca_div = 1.f / 16.f;
    float gl_mult = 15.f;
};

struct PMDrawerSettings {
    bool redraw = false;
    bool hdr = true;
    float ca_div = 1.f / 16.f;
    float gl_mult = 15.f;
    float exposure = 1.f;
    float brightness = 1.f;
    float gamma = 2.2f;
    std::vector<bool> active;
};

class PMSettingsUpdater {
    PhotonMappingSettings* main_settings;
    PhotonMapSettings* gmap_settings;
    PhotonMapSettings* cmap_settings;
    PhotonCollector* pc_settings;
    PMDrawerSettings* d_settings;
public:
    void link_main(PhotonMappingSettings* ms) {
        main_settings = ms;
    }
    void link_gmap(PhotonMapSettings* gms) {
        gmap_settings = gms;
    }
    void link_cmap(PhotonMapSettings* cms) {
        cmap_settings = cms;
    }
    void link_pc(PhotonCollector* pc) {
        pc_settings = pc;
    }
    void link_drawer(PMDrawerSettings* drawer) {
        d_settings = drawer;
    }

    /* ----------Main Settings---------- */
    /// <summary>
    /// Update disabling photon mapping for direct illumination parameter
    /// </summary>
    /// <param name="value"> - if true — disable. if false - enable</param>
    /// <returns></returns>
    void update_dpmdi(bool value) {
        main_settings->dpmdi = value;
    }
    void update_max_rt(size_t depth) {
        main_settings->max_rt_depth = depth;
    }

    /* ----------Photon Maps Settings---------- */
    /// <summary>
    /// Update the number of photons to be emitted
    /// </summary>
    /// <param name="phc"> - photons count</param>
    void update_gphc(size_t gphc) {
        pc_settings->update_gsize(gphc);
    }
    void update_cphc(size_t cphc) {
        pc_settings->update_csize(cphc);
    }
    void update_gnp_count(size_t count) {
        gmap_settings->np_size = count;
    }
    void update_cnp_count(size_t count) {
        cmap_settings->np_size = count;
    }
    void update_disc_compression(float coef) {
        gmap_settings->disc_compression = coef;
        cmap_settings->disc_compression = coef;
    }
    void update_gfilter(int filter) {
        if (filter < 0 || filter > 2) {
            return;
        }
        gmap_settings->ftype = (PhotonMapSettings::FilterType)filter;
    }
    void update_cfilter(int filter) {
        if (filter < 0 || filter > 2) {
            return;
        }
        cmap_settings->ftype = (PhotonMapSettings::FilterType)filter;
    }
    /* ----------Drawer Settings---------- */
    void update_exposure(float exposure) {
        d_settings->exposure = exposure;
        d_settings->redraw = true;
    }
    void update_brightness(float brightness) {
        d_settings->brightness = brightness;
        d_settings->redraw = true;
    }
    void update_rcaustic_divider(float value) {
        main_settings->ca_div = 1.f / value;
    }
    void update_rglobal_multiplier(float value) {
        main_settings->gl_mult = value;
    }
    void update_dcaustic_divider(float value) {
        d_settings->ca_div = 1.f / value;
        d_settings->redraw = true;
    }
    void update_dglobal_multiplier(float value) {
        d_settings->gl_mult = value;
        d_settings->redraw = true;
    }
    void update_active_layers(size_t ind, bool value) {
        d_settings->active[ind] = value;
        d_settings->redraw = true;
    }
    void update_hdr(bool value) {
        d_settings->hdr = value;
        d_settings->redraw = true;
    }
    void redraw(bool value) {
        d_settings->redraw = false;
    }
    ~PMSettingsUpdater() {
        main_settings = nullptr;
        gmap_settings = nullptr;
        cmap_settings = nullptr;
        pc_settings = nullptr;
        d_settings = nullptr;
    }
};