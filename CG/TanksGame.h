#pragma once
#include "ObjModel.h"
#include "Camera.h"
#include <vector>

class TanksGame {
    Shader shader;
    Model tank;
    Camera camera;
    std::vector<Model> scene;
    glm::vec3 pos;
    GLfloat speed;
    DirectionLight sun;
    FlashLight headlightL;
    FlashLight headlightR;
    glm::vec3 cameraOffset;
    glm::vec3 lighOffset;

    bool moveCamera(bool* keys) {
        if (keys[GLFW_KEY_W]) {
            camera.ProcessKeyboard(FORWARD);
        }
        if (keys[GLFW_KEY_S]) {
            camera.ProcessKeyboard(BACKWARD);
        }
        if (keys[GLFW_KEY_A]) {
            camera.ProcessKeyboard(LEFT_ROTATE);
        }
        if (keys[GLFW_KEY_D]) {
            camera.ProcessKeyboard(RIGHT_ROTATE);
        }
        this->pos = camera.Position;
        return false;
    }

   
    void moveHeadlights() {
    }

    void buildCamera() {
        this->camera = Camera({ 0.f, 1.f, 0.f });
        this->camera.MovementSpeed = speed;
    }
    void buildTank() {
        ObjTexture tankTexture("images/WOT/Tank.png", 'n');
        Material m = Material(tankTexture);
        tank = Model("models/WOT/Tanks.obj", m);
        headlightL = FlashLight();
        headlightR = FlashLight({ 0.4, 0, -0.5 });

    }
    void buildScene() {
        ObjTexture fieldTexture("images/WOT/Field.png", 'n');
        auto m = Material(fieldTexture);
        auto field = Model("models/WOT/Tanks.obj", m);
        ObjTexture christTreeTexture("images/WOT/ChristmasTree.png", 'n');
        auto b = Material(christTreeTexture);
        auto christTree = Model("models/WOT/ChristmasTree.obj", b);
        christTree.mm = glm::translate(glm::mat4(1.f), { -10, 0, 4 });

        ObjTexture barrelTexture("images/WOT/Barrel.png", 'n');
        auto c = Material(barrelTexture);
        auto barrel = Model("models/WOT/Barrel.obj", c);
        barrel.mm = glm::translate(glm::mat4(1.f), { 4, 0, -10 });

        ObjTexture stone("images/WOT/Stone-1.png", 'n');
        auto s = Material(stone);
        auto stone1 = Model("models/WOT/Stone-1.obj", s);
        auto stone2 = Model("models/WOT/Stone-2.obj", s);
        stone1.mm = glm::translate(glm::mat4(1.f), { -6, 0, 4 });
        stone2.mm = glm::translate(glm::mat4(1.f), { -6, 0, -4 });

        ObjTexture treeTex("images/WOT/Tree.png", 'n');
        
        auto t = Material(treeTex);
        auto tree = Model("models/WOT/Tree.obj", t);

        tree.mm = glm::translate(glm::mat4(1.f), {10, 0, 4});
        this->scene = std::vector<Model>{ field, christTree, barrel, stone1, stone2, tree };

        sun.direction = { -5.f, -1.f, 0.f };
    }

    void buildShader() {
        this->shader.init_shader("toon_shading.vert", "toon_shading.frag");
        shader.use_program();
        shader.uniformMatrix4fv("Projection",
            glm::value_ptr(glm::perspective(glm::radians(45.f), 1.f, 0.1f, 1000.f)));
        shader.uniformMatrix4fv("View", glm::value_ptr(camera.GetViewMatrix()));
        shader.uniformMatrix4fv("Model", glm::value_ptr(glm::mat4(1.f)));
        shader.uniformDirectionLight(sun, "dirLight.");
        shader.uniformMaterial(tank.material, "material.");
        shader.disable_program();
    }

public:
    TanksGame(GLfloat speed){
        this->speed = speed;
        this->sun = DirectionLight();
        this->tank = Model();
    }
    
    void init() {
        this->buildCamera();
        this->buildTank();
        this->buildScene();
        this->buildShader();
    }

    void render() {
        shader.use_program();
        shader.uniformMatrix4fv("View", glm::value_ptr(camera.Position));
        shader.uniformMatrix4fv("Model", glm::value_ptr(tank.mm));

        tank.render();
        for (auto& elem : scene) {
            shader.uniformMatrix4fv("Model", glm::value_ptr(elem.mm));
            elem.render();
        }
    }
    void move(bool* keys) {
        if (!moveCamera(keys)) {
            return;
        }
        tank.mm = glm::translate(glm::mat4(1.f), camera.Position + this->cameraOffset);
        moveHeadlights();
    }

};
