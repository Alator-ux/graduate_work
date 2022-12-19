#pragma once
#include "ObjModel.h"
#include "Camera.h"
#include <vector>
#include "MoveHelper.h"

class TanksGame {
    Shader shader;
    Model tank;
    PlayerMH camera;
    std::vector<Bullet> bullets;
    std::vector<Model> scene;
    GLfloat speed;
    DirectionLight sun;
    glm::vec3 cameraOffset;
    glm::vec3 lighOffset;

    bool moveCamera(bool* keys) {
        bool moved = false;
        if (keys[GLFW_KEY_W]) {
            camera.ProcessKeyboard(FORWARD);
            moved = true;
        }
        if (keys[GLFW_KEY_S]) {
            camera.ProcessKeyboard(BACKWARD);
            moved = true;
        }
        if (keys[GLFW_KEY_Q]) {
            camera.ProcessKeyboard(LEFT_ROTATE);
            moved = true;
        }
        if (keys[GLFW_KEY_E]) {
            camera.ProcessKeyboard(RIGHT_ROTATE);
            moved = true;
        }
        if (keys[GLFW_KEY_R]) {
            camera.ProcessKeyboard(UP_ROTATE);
            moved = true;
        }
        if (keys[GLFW_KEY_T]) {
            camera.ProcessKeyboard(DOWN_ROTATE);
            moved = true;
        }
        if (keys[GLFW_KEY_SPACE]) {
            
        }
        return moved;
    }
    void buildBullet() {
        Model b = Model("models/cube/Cube.obj");
        
    }
    void buildCamera() {
        this->camera = PlayerMH({0.f,0.f,0.f}, { 0.f, 3.f, 5.f });
        this->camera.MovementSpeed = speed;
    }
    void buildTank() {
        ObjTexture tankTexture("images/WOT/Tank.png", 'n');
        Material m = Material(tankTexture);
        tank = Model("models/WOT/Tanks.obj", m);
        tank.mm = glm::rotate(tank.mm, glm::radians(camera.Yaw), glm::vec3(0.f, 1.f, 0.f));
    }
    void buildScene() {
        ObjTexture fieldTexture("images/WOT/Field.png", 'n');
        auto m = Material(fieldTexture);
        auto field = Model("models/WOT/Field.obj", m);

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

        sun.direction = { 0.f, -1.f, 0.f };
    }

    void buildShader() {
        this->shader.init_shader("phong.vert", "phong.frag");
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
        shader.uniformMatrix4fv("View", glm::value_ptr(camera.GetViewMatrix()));
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
        tank.mm = glm::translate(glm::mat4(1.f), camera.model_pos);
        tank.mm = glm::rotate(tank.mm, -glm::radians(camera.Yaw + 180), glm::vec3(0.f, 1.f, 0.f));
    }

};
