#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "OpenGLWrappers.h"
#include "imgui.h"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "MainWindow.h"
#include "Camera.h"
#include "Texture.h"
#include "ObjModel.h"
#include "Tools.h"
#include "PMModel.h"
#include "PhotonMapping.h"
#include "Photon.h"
const GLuint W_WIDTH = 600;
const GLuint W_HEIGHT = 600;
PMDrawer drawer(600, 600);
PMSettingsUpdater pmsu;
Camera camera;
PhotonMapping pm;
void Init(OpenGLManager*);
void Draw(int, float, float, double);
void Release();
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void Do_Movement();
int mode = 0;

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetKeyCallback(window, keyboard_callback);
    const char* glsl_version = "#version 410";
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    glewInit();
    srand(static_cast <unsigned> (time(0)));

    auto manager = OpenGLManager::get_instance();
    Init(manager);
    drawer.opengl_init();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::StyleColorsDark();

   // pmsu.update_xstart(300);

    std::unique_ptr<Window> main_window(new MainWindow(&pmsu, &pm, &drawer));
   
    bool show_demo_window = false;
    std::string vbo_name = "";
    pmsu.window_settings.width = pmsu.window_settings.output->width + 500;
    pmsu.window_settings.height = pmsu.window_settings.output->height;
    glfwSetWindowSize(window, pmsu.window_settings.width, pmsu.window_settings.height);
    while (!glfwWindowShouldClose(window))
    {   
        if (pmsu.window_settings.changed.resolution) {
            //glfwSetWindowSize(window, 
            //    pmsu.window_settings.output->width + 500, pmsu.window_settings.output->height);
            glfwSetWindowSize(window,
                   pmsu.window_settings.output->width + 500, pmsu.window_settings.output->height);
        }
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        main_window.get()->draw();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawer.display();
        Do_Movement();
        double time = glfwGetTime();
        Draw(mode, 0,0, time);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool keys[1024];
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1) {
        camera.Position = glm::vec3(0, 0, 3);
        camera.Yaw = -90;
        camera.updateCameraVectors();
        return;
    }
    if (key >= 0 && key < 1024)
    {
        keys[key] = action == GLFW_PRESS || action == GLFW_REPEAT;
    }

}
void Do_Movement() {
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT);
    if (keys[GLFW_KEY_Z])
        camera.ProcessKeyboard(DOWN);
    if (keys[GLFW_KEY_X])
        camera.ProcessKeyboard(UP);
    if (keys[GLFW_KEY_Q])
        camera.ProcessKeyboard(LEFT_ROTATE);
    if (keys[GLFW_KEY_E])
        camera.ProcessKeyboard(RIGHT_ROTATE);
    if (keys[GLFW_KEY_R])
        camera.ProcessKeyboard(DOWN_ROTATE);
    if (keys[GLFW_KEY_T])
        camera.ProcessKeyboard(UP_ROTATE);
}
void Release() {
    OpenGLManager::get_instance()->release();
}

void load_preset(const std::string path, const std::string fname, std::vector<PMPreset>& presets) {
    auto map = loadOBJ(path, fname);
    glm::vec3 lspos(0.f);
    std::vector<PMModel> scene;
    PMPreset preset;
    for (auto& kv : map) {
        LightSource* ls = nullptr;
        if (kv.name == "light") {
            std::for_each(kv.vertices.begin(), kv.vertices.end(), [&lspos](const ObjVertex& vert) {lspos += vert.position;});
            lspos /= kv.vertices.size();
            ls = new LightSource(lspos);
        }
        PMModel m(kv, ls);
        preset.objects.push_back(m);
    }
    presets.push_back(preset);
}

void Init(OpenGLManager* manager) {
    Random<unsigned>::set_seed();

    std::vector<PMPreset> presets;
    load_preset("./models/cornell_box_original", "CornellBox-Original.obj", presets);
    presets.back().pos = glm::vec3(-0.00999999046, 0.795000017, 2.35000001);
    presets.back().dir = glm::vec3(0.f, 0.f, -1.f);

    load_preset("./models/cornell_box_sphere", "CornellBox-Sphere.obj", presets);
    presets.back().pos = glm::vec3(-0.00999999046, 0.795000017, 2.35000001);
    presets.back().dir = glm::vec3(0.f, 0.f, -1.f);

    //load_preset("./models/wine_glass", "glasses.obj", presets);
    load_preset("./models/diamond", "diamond.obj", presets);
    presets.back().pos = glm::vec3(2.f, 1.f, 4.f);
    presets.back().dir = glm::vec3(-0.3f, -0.21f, -0.87f);

    load_preset("./models/flagon", "flagon.obj", presets);
    presets.back().pos = glm::vec3(2.f, 2.f, 4.f);
    presets.back().dir = glm::normalize(glm::vec3(-0.4f, -0.31f, -0.87f));

    load_preset("./models/wine_glass", "shot_glass.obj", presets);
    presets.back().pos = glm::vec3(0.25f, 0.5f, 2.f);
    presets.back().dir = glm::normalize(glm::vec3(-0.4f, -0.31f, -0.87f));
    
    load_preset("./models/ring", "metal_ring.obj", presets);
    presets.back().pos = glm::vec3(0.f, 1.f, 1.f);
    presets.back().dir = glm::normalize(-presets.back().pos);

    load_preset("./models/cornell_box_water", "CornellBox-Water.obj", presets);
    presets.back().pos = glm::vec3(-0.00999999046, 0.795000017, 1.99000001);
    presets.back().dir = glm::vec3(0.f, 0.f, -1.f);

    //auto map = loadOBJ("./models/cornell_box_original", "CornellBox-Original.obj");
    //auto map = loadOBJ("./models/cornell_box_sphere", "CornellBox-Sphere.obj");
    //auto map = loadOBJ("./models/cornell_box_water", "CornellBox-Water.obj");
    //auto map = loadOBJ("./models/cornell_box_high_water", "cornellbox-water2.obj");
    //auto map = loadOBJ("./models/wine_glass", "WineGlasses.obj");
    //auto map = loadOBJ("./models/ring", "metal_ring.obj");
    //auto map = loadOBJ("./models/wine_glass", "glasses.obj");

    pm.init(&drawer, std::move(presets), pmsu);
    //pm.build_map();

   /* auto pm = PhotonMapping(scene, lssources, 1000);
    auto pmap = pm.build_map();
    pmpointcount = pmap->size();
    std::vector<glm::vec3> points;
    std::for_each(pmap->begin(), pmap->end(), [&points](const PhotonMapping::Photon& ph) {points.push_back(ph.pos);});
    manager->init_vbo("pm", &points[0], sizeof(glm::vec3) * points.size(), GL_STATIC_DRAW);*/

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    manager->checkOpenGLerror();
}
void Draw(int n, float fcspeed, float scspeed, double time) {

}