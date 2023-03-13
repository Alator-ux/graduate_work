#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "OpenGLWrappers.h"
#include "imgui.h"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "Widgets.h"
#include "Drawer.h"
#include "FigureBuilder.h"
#include "Camera.h"
#include "Texture.h"
#include <corecrt_math_defines.h>
#include "Cube.h"
#include "ObjModel.h"
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

Model statue1;
Model statue2;
Model cube;
std::map<std::string, Model*> models;
void Init(OpenGLManager* manager) {
    drawer = Drawer(W_WIDTH, W_HEIGHT);
    
    Shader shader = Shader();
    shader.init_shader("phong.vert", "phong.frag");
    shaders.push_back(shader);

    lampShader.init_shader("lamp.vert", "lamp.frag");
    
    //models = loadObjModel("./models/Pool", "Pool.obj");
    //models = loadObjModel("./models/cube", "Cube.obj");
    auto tex = ObjTexture::get_raw_sample("./models/background_001.jpg", 'n');
    /*models = loadObjModel("./models/sand_beach", "model.obj");
    for (auto it = models.begin(); it != models.end(); it++) {
        it->second->mm = glm::rotate(it->second->mm, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    }    */

    auto temp_mm = loadObjModel("./models/cornell_box", "cornellbox-water2.obj"); // temp model map
    /* {
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        ObjTexture::unbind();
        tex.initialized = true;
    }
    temp_mm.begin()->second->material.map_Kd = tex;*/
    temp_mm.begin()->second->mm = glm::translate(temp_mm.begin()->second->mm, glm::vec3(10.f, 5.f, -5.f));
    temp_mm.begin()->second->mm = glm::scale(temp_mm.begin()->second->mm, glm::vec3(10.f));
    models.insert(temp_mm.begin(), temp_mm.end());


    tex = ObjTexture::get_raw_sample("./models/caustic_002.jpg", 'n');
    {
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        ObjTexture::unbind();
        tex.initialized = true;
    }
    auto tex2 = ObjTexture::get_raw_sample("./models/caustic_003.jpg", 'n');
    {
        glBindTexture(GL_TEXTURE_2D, tex2.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        ObjTexture::unbind();
        tex2.initialized = true;
    }

    for (auto it = models.begin(); it != models.end(); it++) {
        it->second->material.textures.push_back(tex);
        it->second->material.textures.push_back(tex2);
    }
    PointLight pLight(glm::vec3(5.0f, 10.0f, 15.0f));;
    pLight.position = { 0, 15, 15 };
    pLight.set_atten_zero();

    DirectionLight dirLight(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.2f));

    auto projection = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 10000.f);
    auto view = camera.GetViewMatrix();
    auto model = glm::mat4(1.f);
    {
        for (auto shader : shaders) {
            shader.use_program();
            shader.uniformMatrix4fv("Projection", glm::value_ptr(projection));
            shader.uniformMatrix4fv("View", glm::value_ptr(view));
            //shader.uniformMatrix4fv("Model", glm::value_ptr(model));
            shader.uniformDirectionLight(dirLight, "dirLight.");
            shader.uniformMaterial(models.begin()->second->material, "material.");
            shader.disable_program();
        }
    }
    {
        lampShader.use_program();
        lampShader.uniformMatrix4fv("Projection", glm::value_ptr(projection));
        lampShader.uniformMatrix4fv("View", glm::value_ptr(view));
        auto lampLoc = glm::translate(model, pLight.position);
        lampLoc = glm::scale(lampLoc, glm::vec3(1.f));
        lampShader.uniformMatrix4fv("Model", glm::value_ptr(lampLoc));
        lampShader.disable_program();
    }


    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    manager->checkOpenGLerror();
}
void Draw(int n, float fcspeed, float scspeed, double time) {
    shaders[n].use_program();
    shaders[n].uniformMatrix4fv("View", glm::value_ptr(camera.GetViewMatrix()));
    shaders[n].uniform1f("time", time);
    shaders[n].uniform1f("fcSpeed", fcspeed);
    shaders[n].uniform1f("scSpeed", scspeed);
    OpenGLManager::checkOpenGLerror();
    for (auto it = models.begin(); it != models.end(); it++) {
        shaders[n].uniformMaterial(it->second->material, "material.");
        shaders[n].uniformMatrix4fv("Model", glm::value_ptr(it->second->mm));
        shaders[n].uniform1i("text", 0);
        shaders[n].uniform1i("caustic1", 1);
        shaders[n].uniform1i("caustic2", 2);
        it->second->render(1);
    }
    shaders[n].disable_program();
    OpenGLManager::checkOpenGLerror();
    /*lampShader.use_program();
    lampShader.uniformMatrix4fv("View", glm::value_ptr(camera.GetViewMatrix()));
    //cube.render(1, GL_QUAD_STRIP);
    lampShader.disable_program();*/
}