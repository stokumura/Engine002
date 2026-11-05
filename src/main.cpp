#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <stb/stb_image.h>

#include <cmath>
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>
#include <string>

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
void GLAPIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    fprintf(stderr, "[GL %s] type=0x%x id=%u severity=0x%x: %s\n", type == GL_DEBUG_TYPE_ERROR ? "ERROR" : "DEBUG", type, id, severity, message);
}

Camera camera;

float deltaTime{0.0f};
float lastTime{0.0f};

float lastCursorX{WIDTH / 2.0f};
float lastCursorY{HEIGHT / 2.0f};
bool firstCursorClick{true};
bool cursorInGame{false};

GBuffer gBuffer;

int main(void) {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, samples);
#if defined(DEBUG)
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

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

  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if(flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(glDebugCallback, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  }

  {
      gBuffer = GBuffer(currentWidth, currentHeight);

      Sphere sphereShape(Vertex3DUnlit, 0.25f, 16, 16);
      Quad quad(Vertex2D);

      const Mesh &sphere = sphereShape.GetMesh();
      const Mesh &screen = quad.GetMesh();

      std::filesystem::path texturesPath = GetResourcesPath() / "textures";

      std::filesystem::path modelsPath = GetResourcesPath() / "objects";
      std::filesystem::path backpackPath = modelsPath / "backpack" / "backpack.obj";
      Model backpack(Vertex3DLitFull, backpackPath);
      std::vector<glm::vec3> objectPositions {
          glm::vec3(-3.0f, -0.5f, -3.0f),
          glm::vec3(0.0f, -0.5f, -3.0f),
          glm::vec3(3.0f, -0.5f, -3.0f),
          glm::vec3(-3.0f, -0.5f, 0.0f),
          glm::vec3(0.0f, -0.5f, 0.0f),
          glm::vec3(3.0f, -0.5f, 0.0f),
          glm::vec3(-3.0f, -0.5f, 3.0f),
          glm::vec3(0.0f, -0.5f, 3.0f),
          glm::vec3(3.0f, -0.5f, 3.0f),
      };

      const unsigned int NR_LIGHTS = 32;
      std::vector<glm::vec3> lightPositions;
      std::vector<glm::vec3> lightColors;
      std::srand(13);
      for(unsigned int i = 0; i < NR_LIGHTS; i++) {
          float xPos = static_cast<float>(((std::rand() % 100) / 100.0f) * 6.0f - 3.0f);
          float yPos = static_cast<float>(((std::rand() % 100) / 100.0f) * 6.0f - 4.0f);
          float zPos = static_cast<float>(((std::rand() % 100) / 100.0f) * 6.0f - 3.0f);
          lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

          float rColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5f);
          float gColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5f);
          float bColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5f);
          lightColors.push_back(glm::vec3(rColor, bColor, gColor));
      }

      std::filesystem::path shadersPath = GetResourcesPath() / "shaders";
      std::filesystem::path vertexShaderPath = shadersPath / "objectGBuffer.vert";
      std::filesystem::path fragmentShaderPath = shadersPath / "objectGBuffer.frag";
      std::filesystem::path vertexShaderScreen = shadersPath / "screenQuad.vert";
      std::filesystem::path fragmentShaderDeferred = shadersPath / "deferredLighting.frag";
      std::filesystem::path vertexShaderLightPath = shadersPath / "objectFlatColor.vert";
      std::filesystem::path fragmentShaderLightPath = shadersPath / "objectFlatColor.frag";

      ShaderProgram program;
      ShaderProgram deferredProgram;
      ShaderProgram lightProgram;
      {
          ShaderVariants vertexGBuffer(vertexShaderPath.string(), ShaderType::VERTEX);
          ShaderVariants fragmentGBuffer(fragmentShaderPath.string(), ShaderType::FRAGMENT);
          ShaderVariants vertexScreen(vertexShaderScreen.string(), ShaderType::VERTEX);
          ShaderVariants fragmentDeferred(fragmentShaderDeferred.string(), ShaderType::FRAGMENT);
          ShaderVariants vertexLight(vertexShaderLightPath.string(), ShaderType::VERTEX);
          ShaderVariants fragmentLight(fragmentShaderLightPath.string(), ShaderType::FRAGMENT);

          program.AttachShader(vertexGBuffer.GetBaseShader());
          program.AttachShader(fragmentGBuffer.GetBaseShader());
          program.Compile();

          deferredProgram.AttachShader(vertexScreen.GetBaseShader());
          deferredProgram.AttachShader(fragmentDeferred.GetBaseShader());
          deferredProgram.Compile();

          lightProgram.AttachShader(vertexLight.GetBaseShader());
          lightProgram.AttachShader(fragmentLight.GetBaseShader());
          lightProgram.Compile();
      }

      CameraData cameraData;
      UniformBlock cameraMatrices("CameraData", 2, sizeof(CameraData), &cameraData);
      program.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());
      lightProgram.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());

      Sampler globalSampler;
      globalSampler.SetMinFilter(GL_LINEAR_MIPMAP_LINEAR);
      globalSampler.SetMagFilter(GL_LINEAR);
      globalSampler.SetWrapS(GL_CLAMP_TO_EDGE);
      globalSampler.SetWrapT(GL_CLAMP_TO_EDGE);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);

      glEnable(GL_DEPTH_TEST);

      glEnable(GL_MULTISAMPLE);

      glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

      //Uncomment for drawing as wireframe
      //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      deferredProgram.Bind();
      deferredProgram.SetInt("gPosition", 0);
      deferredProgram.SetInt("gNormal", 1);
      deferredProgram.SetInt("gAlbedoSpec", 2);

      while (!glfwWindowShouldClose(window)) {
          float currentTime = static_cast<float>(glfwGetTime());
          deltaTime = currentTime - lastTime;
          float fps = 1.0f / deltaTime;
          lastTime = currentTime;

          glfwSetWindowTitle(window, ("Basic OpenGL Engine - FPS: " + std::to_string(fps)).c_str());
          ProcessInput(window);

          cameraData.view = camera.GetViewMatrix();
          cameraData.projection = glm::perspective(glm::radians(camera.GetZoom()), static_cast<float>(currentWidth) / currentHeight, 0.1f, 100.0f);
          cameraData.viewPosition = camera.GetPosition();
          cameraMatrices.UpdateData(&cameraData, sizeof(CameraData));
          cameraMatrices.Bind();

          // GEOMETRY PASS
          gBuffer.BindFBO();
          glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glEnable(GL_DEPTH_TEST);
          glDisable(GL_FRAMEBUFFER_SRGB);
          program.Bind();
          glm::mat4 model(1.0f);
          for (unsigned int i = 0; i < objectPositions.size(); i++) {
              model = glm::mat4(1.0f);
              model = translate(model, objectPositions[i]);
              model = glm::scale(model, glm::vec3(0.5f));
              program.SetMat4("model", model);
              backpack.Draw(program, globalSampler);
          }

          // LIGHTING PASS
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDisable(GL_DEPTH_TEST);
          glEnable(GL_FRAMEBUFFER_SRGB);

          deferredProgram.Bind();
          deferredProgram.SetVec3("viewPosition", cameraData.viewPosition);
          for (unsigned int i = 0; i < lightPositions.size(); i++) {
              deferredProgram.SetVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
              deferredProgram.SetVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);

              const float constant = 1.0f;
              const float linear = 0.7f;
              const float quadratic = 1.8f;
              deferredProgram.SetFloat("lights[" + std::to_string(i) + "].Linear", linear);
              deferredProgram.SetFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);

              const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
              float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f/5.0f) * maxBrightness))) / (2.0f * quadratic);
              deferredProgram.SetFloat("lights[" + std::to_string(i) + "].Radius", radius);
          }
          glBindSampler(0, 0);
          glBindSampler(1, 0);
          glBindSampler(2, 0);
          gBuffer.BindTexture(0, 0);
          gBuffer.BindTexture(1, 1);
          gBuffer.BindTexture(2, 2);
          screen.Bind();
          screen.Draw(deferredProgram);
          glBindTextureUnit(0, 0);
          glBindTextureUnit(1, 0);
          glBindTextureUnit(2, 0);

          glBlitNamedFramebuffer(gBuffer.GetFBO(), 0, 0, 0, currentWidth, currentHeight, 0, 0, currentWidth, currentHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

          // FORWARD RENDERING
          glEnable(GL_DEPTH_TEST);
          lightProgram.Bind();
          for (unsigned int i = 0; i < lightPositions.size(); i++) {
              model = glm::mat4(1.0f);
              model = glm::translate(glm::mat4(1.0), lightPositions[i]);
              model = glm::scale(model, glm::vec3(0.125f));
              lightProgram.SetMat4("model", model);
              lightProgram.SetVec3("uAlbedoFlatColor", glm::vec3(1.0,1.0,1.0));
              lightProgram.SetVec3("uLightColor", lightColors[i]);
              sphere.Draw(lightProgram);
          }

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
        camera.ProcessKeyboardMovement(CameraMovement::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(CameraMovement::UP, deltaTime);
}

void FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    currentWidth = width;
    currentHeight = height;
    gBuffer = GBuffer(currentWidth, currentHeight);
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
