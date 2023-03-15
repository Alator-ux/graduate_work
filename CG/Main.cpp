#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "OpenGLWrappers.h"
#include "Primitives.h"
#include "Midpoint.h"
#include "imgui.h"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "Widgets.h"
#include "Drawer.h"
#include "Camera.h"
#include "Texture.h"
#include "ObjModel.h"
#include "Tools.h"
#include "PMModel.h"
#include "PhotonMapping.h"
const GLuint W_WIDTH = 600;
const GLuint W_HEIGHT = 600;
std::vector<Shader> shaders;
Shader lampShader;
Drawer drawer;
Camera camera;
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
    const char* glsl_version = "#version 330";
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    glewInit();

    auto manager = OpenGLManager::get_instance();
    Init(manager);


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::StyleColorsDark();

    auto button_row = RadioButtonRow({ "Phong", "Toon shading", "Rim"});
    auto fcspeed_slider = FloatSlider("First Caustic Speed", 0.001f, 1.f);
    auto scspeed_slider = FloatSlider("Second Caustic Speed", 0.001f, 1.f);

    bool show_demo_window = false;
    std::string vbo_name = "";
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        
        {
            ImGui::Begin("Window!");
            ImGui::Text("Current mode: " + mode);
            if (button_row.draw()) {
                mode = button_row.get_value();
            }
            fcspeed_slider.draw();
            scspeed_slider.draw();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Do_Movement();
        double time = glfwGetTime();
        Draw(mode, fcspeed_slider.get_value(), scspeed_slider.get_value(), time);
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

size_t pmpointcount;
void Init(OpenGLManager* manager) {
    Random<unsigned>::set_seed();
    drawer = Drawer(W_WIDTH, W_HEIGHT);
    
    Shader shader = Shader();
    shader.init_shader("main.vert", "main.frag");
    shaders.push_back(shader);

    glm::vec3 lspos(0.f);
    auto map = loadOBJ("./models/cornell_box_original", "CornellBox-Original.obj");
    std::vector<PMModel> scene;
    for (auto& kv : map) {
        if (kv.first == "light") {
            std::for_each(kv.second.vertices.begin(), kv.second.vertices.end(), [&lspos](const ObjVertex& vert) {lspos += vert.position;});
            lspos /= kv.second.vertices.size();
        }
        auto m = PMModel(kv.second);
        scene.push_back(m);
    }
    std::vector<LightSource> lssources({ PointLight(lspos) });
    auto pm = PhotonMapping(scene, lssources, 10);
    auto pmap = pm.build_map();
    pmpointcount = pmap->size();
    std::vector<glm::vec3> points;
    std::for_each(pmap->begin(), pmap->end(), [&points](const PhotonMapping::Photon& ph) {points.push_back(ph.pos);});
    manager->init_vbo("pm", &points[0], sizeof(glm::vec3) * points.size(), GL_STATIC_DRAW);

    shaders[0].uniformMatrix4fv("Model", glm::value_ptr(glm::mat4(1.f)));
    shaders[0].uniformMatrix4fv("Projection", glm::value_ptr(glm::perspective(glm::radians(60.f), (float)W_WIDTH / W_HEIGHT, 0.1f, 1000.f)));

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    manager->checkOpenGLerror();
}
void Draw(int n, float fcspeed, float scspeed, double time) {
    auto manager = OpenGLManager::get_instance();
    shaders[n].use_program();
    shaders[n].uniformMatrix4fv("View", glm::value_ptr(camera.GetViewMatrix()));
    auto vpos = shaders[n].get_attrib_location("vPos");
    glEnableVertexAttribArray(vpos);
    glBindBuffer(GL_ARRAY_BUFFER, manager->get_buffer_id("pm"));
    glVertexAttribPointer(vpos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_POINTS, 0, pmpointcount);
    shaders[n].disable_program();
    OpenGLManager::checkOpenGLerror();
    /*lampShader.use_program();
    lampShader.uniformMatrix4fv("View", glm::value_ptr(camera.GetViewMatrix()));
    //cube.render(1, GL_QUAD_STRIP);
    lampShader.disable_program();*/
}