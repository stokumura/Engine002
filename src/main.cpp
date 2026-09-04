#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <iostream>

#include "Engine/Engine.h"
#include "Utils/Utils.h"

const unsigned int samples {4};
const unsigned int WIDTH{800};
const unsigned int HEIGHT{600};
unsigned int currentWidth{WIDTH};
unsigned int currentHeight{HEIGHT};

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void CursorCallback(GLFWwindow* window, double xposIn, double yposIn);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessInput(GLFWwindow* window);

Camera camera;

float deltaTime{0.0f};
float lastTime{0.0f};

float lastCursorX{WIDTH / 2.0f};
float lastCursorY{HEIGHT / 2.0f};
bool firstCursorClick{true};
bool cursorInGame{false};

int main(void) {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, samples);

  GLFWwindow* window = glfwCreateWindow(currentWidth, currentHeight, "Hello World", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW Window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
  glfwSetCursorPosCallback(window, CursorCallback);
  glfwSetScrollCallback(window, ScrollCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  {
      Sphere sphere(Vertex3DUnlit, 1.0f, 32, 32);
      Cylinder cylinder(Vertex3DUnlit, 1.0f, 2.5f, 32, 32);
      Cube cube(Vertex3DUnlit, 2.0f, 16, 16, 16);
      const Mesh &mesh = cube.GetMesh();
      const DrawInfo &draw = mesh.GetDrawInfo();

      std::filesystem::path shadersPath = GetResourcesPath() / "shaders";
      std::filesystem::path vertexShaderPath = shadersPath / "test.vs";
      std::filesystem::path fragmentShaderPath = shadersPath / "test.fs";
      ShaderVariants vertex(vertexShaderPath.string(), ShaderType::VERTEX);
      ShaderVariants fragment(fragmentShaderPath.string(), ShaderType::FRAGMENT);
      ShaderProgram program;
      program.AttachShader(vertex.GetBaseShader());
      program.AttachShader(fragment.GetBaseShader());
      program.Compile();

      CameraData cameraData;
      UniformBlock cameraMatrices("CameraData", 0, sizeof(CameraData), &cameraData);
      program.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());

      Sampler globalSampler;
      globalSampler.SetMinFilter(GL_NEAREST);
      globalSampler.SetMagFilter(GL_NEAREST);

      std::filesystem::path texturesPath = GetResourcesPath() / "textures";
      std::filesystem::path imagePath = texturesPath / "box.png";
      Texture image(imagePath.string(), TextureType::DIFFUSE, 0);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);

      glEnable(GL_DEPTH_TEST);

      glEnable(GL_MULTISAMPLE);

      glClearColor(0.8f, 0.0f, 0.8f, 1.0f);

      //Uncomment for drawing as wireframe
      //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      while (!glfwWindowShouldClose(window)) {
          float currentTime = static_cast<float>(glfwGetTime());
          deltaTime = currentTime - lastTime;
          float fps = 1.0f / deltaTime;
          lastTime = currentTime;

          glfwSetWindowTitle(window, ("Basic OpenGL Engine - FPS: " + std::to_string(fps)).c_str());
          ProcessInput(window);

          cameraData.view = camera.GetViewMatrix();
          cameraData.projection = glm::perspective(glm::radians(camera.GetZoom()), static_cast<float>(currentWidth) / currentHeight, 0.1f, 100.0f);
          cameraMatrices.UpdateData(&cameraData, sizeof(CameraData));

          glm::mat4 model(1.0f);
          model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
          //model = glm::scale(model, glm::vec3(1.0f + std::cos(glm::radians(currentTime * 0.5f)), 1.0f + std::sin(glm::radians(currentTime * 2.0f)), 1.0f));
          model = glm::rotate(model, glm::radians(45.0f * currentTime), glm::vec3(1.0, 1.0, 0.0));

          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          program.Bind();
          cameraMatrices.Bind();
          program.SetMat4("model", model);
          program.SetInt("image", image.GetUnit());

          globalSampler.Bind(image.GetUnit());
          image.Bind();

          mesh.Bind();
          glDrawElements(GL_TRIANGLES, draw.indices, GL_UNSIGNED_INT, 0);
          Mesh::Unbind();


          glfwSwapBuffers(window);

          glfwPollEvents();
      }
      GetError();
  }

  glfwTerminate();
  return 0;
}

void ProcessInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && cursorInGame) {
        cursorInGame = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !cursorInGame) {
        cursorInGame = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboardSpeed(true);
    else
        camera.ProcessKeyboardSpeed(false);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::DOWN, deltaTime);
}

void FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    currentWidth = width;
    currentHeight = height;
    glViewport(0, 0, width, height);
}

void CursorCallback(GLFWwindow* window, double xposIn, double yposIn) {
    if(!cursorInGame) {
        firstCursorClick = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstCursorClick)
    {
        lastCursorX = xpos;
        lastCursorY = ypos;
        firstCursorClick = false;
    }

    float xoffset = xpos - lastCursorX;
    float yoffset = lastCursorY - ypos;

    lastCursorX = xpos;
    lastCursorY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
