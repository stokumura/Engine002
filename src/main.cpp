#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <stb/stb_image.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <glm/matrix.hpp>
#include <iostream>

#include "Engine/Engine.h"
#include "Engine/Shader/Shader.h"
#include "Engine/Texture/Texture.h"
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

glm::vec3 lightColorAmbient(0.05f, 0.05f, 0.05f);
glm::vec3 lightColorDiffuse(1.0f, 1.0f, 1.0f);
glm::vec3 lightColorSpecular(1.0f, 1.0f, 1.0f);
glm::vec3 lightPosition(2.5f, 2.5f, 2.5f);
glm::vec3 lightDirection(-0.2f, -0.1f, -0.3f);

float shininess = 0.6f;
glm::vec3 objectColorAmbient(0.0215f, 0.1745f, 0.0215f);
glm::vec3 objectColorDiffuse(0.07568f, 0.61424f, 0.07568f);
glm::vec3 objectColorSpecular(0.633f, 0.727811f, 0.633f);
float constantAttenuation = 1.0f;
float linearAttenuation = 0.09f;
float quadraticAttenuation = 0.032f;
float cutOff = glm::cos(glm::radians(12.5f));
float outerCutOff = glm::cos(glm::radians(17.5f));

glm::vec3 cubePositions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f),
};

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
      Sphere sphere(Vertex3DUnlit, 0.1f, 16, 16);
      Cylinder cylinder(Vertex3DUnlit, 1.0f, 2.5f, 32, 32);
      Cube cube(Vertex3DLit, 1.0f, 16, 16, 16);

      const Mesh &mesh = cube.GetMesh();
      const DrawInfo &draw = mesh.GetDrawInfo();

      const Mesh &lightMesh = sphere.GetMesh();
      const DrawInfo &lightDraw = mesh.GetDrawInfo();

      std::filesystem::path shadersPath = GetResourcesPath() / "shaders";
      std::filesystem::path vertexShaderPath = shadersPath / "objectMaterial.vert";
      std::filesystem::path fragmentShaderPath = shadersPath / "objectMaterial.frag";
      std::filesystem::path vertexShaderLightPath = shadersPath / "objectFlatColor.vert";
      std::filesystem::path fragmentShaderLightPath = shadersPath / "objectFlatColor.frag";

      ShaderVariants vertex(vertexShaderPath.string(), ShaderType::VERTEX);
      ShaderVariants fragment(fragmentShaderPath.string(), ShaderType::FRAGMENT);
      ShaderVariants vertexLight(vertexShaderLightPath.string(), ShaderType::VERTEX);
      ShaderVariants fragmentLight(fragmentShaderLightPath.string(), ShaderType::FRAGMENT);

      ShaderProgram program;
      program.AttachShader(vertex.GetShader({ "USE_NORMAL_MATRIX" }));
      program.AttachShader(fragment.GetShader({ "USE_ALBEDO_TEXTURE_MAP", "USE_SPECULAR_TEXTURE_MAP", "USE_ALBEDO_AS_AMBIENT" }));
      program.Compile();

      ShaderProgram lightProgram;
      lightProgram.AttachShader(vertexLight.GetBaseShader());
      lightProgram.AttachShader(fragmentLight.GetBaseShader());
      lightProgram.Compile();

      CameraData cameraData;
      UniformBlock cameraMatrices("CameraData", 0, sizeof(CameraData), &cameraData);
      program.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());

      Sampler globalSampler;
      globalSampler.SetMinFilter(GL_LINEAR_MIPMAP_LINEAR);
      globalSampler.SetMagFilter(GL_LINEAR);

      std::filesystem::path texturesPath = GetResourcesPath() / "textures";
      std::filesystem::path albedoTexturePath = texturesPath / "container2.png";
      std::filesystem::path specularTexturePath = texturesPath / "container2_specular.png";
      Texture albedoTexture(albedoTexturePath.string(), TextureType::DIFFUSE, 0);
      Texture specularTexture(specularTexturePath.string(), TextureType::SPECULAR, 1);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);

      glEnable(GL_DEPTH_TEST);

      glEnable(GL_MULTISAMPLE);

      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

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
          cameraData.viewPosition = camera.GetPosition();
          cameraMatrices.UpdateData(&cameraData, sizeof(CameraData));

          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          cameraMatrices.Bind();

          program.Bind();

          program.SetFloat("uObjectMaterial.shininess", shininess);
          program.SetVec3("uObjectMaterial.ambient", glm::vec3(0.2, 0.2, 0.2));
          program.SetInt("uObjectMaterial.diffuse", albedoTexture.GetUnit());
          program.SetInt("uObjectMaterial.specular", specularTexture.GetUnit());

          program.SetInt("uNumDirLights", 0);
          program.SetInt("uNumPointLights", 1);
          program.SetVec3("uPointLights[0].ambient", lightColorAmbient);
          program.SetVec3("uPointLights[0].diffuse", lightColorDiffuse);
          program.SetVec3("uPointLights[0].specular", lightColorSpecular);
          program.SetVec3("uPointLights[0].position", lightPosition);
          program.SetFloat("uPointLights[0].constant", constantAttenuation);
          program.SetFloat("uPointLights[0].linear", linearAttenuation);
          program.SetFloat("uPointLights[0].quadratic", quadraticAttenuation);

          program.SetInt("uNumSpotLights", 1);
          program.SetVec3("uSpotLights[0].position", camera.GetPosition());
          program.SetVec3("uSpotLights[0].direction", camera.GetFront());
          program.SetVec3("uSpotLights[0].ambient", lightColorAmbient);
          program.SetVec3("uSpotLights[0].diffuse", lightColorDiffuse);
          program.SetVec3("uSpotLights[0].specular", lightColorSpecular);
          program.SetFloat("uSpotLights[0].constant", constantAttenuation);
          program.SetFloat("uSpotLights[0].linear", linearAttenuation);
          program.SetFloat("uSpotLights[0].quadratic", quadraticAttenuation);
          program.SetFloat("uSpotLights[0].cutOff", cutOff);
          program.SetFloat("uSpotLights[0].outerCutOff", outerCutOff);


          globalSampler.Bind(albedoTexture.GetUnit());
          albedoTexture.Bind();

          globalSampler.Bind(specularTexture.GetUnit());
          specularTexture.Bind();

          glm::mat4 model(1.0f);
          glm::mat3 normal(1.0);

          for(int i = 0; i < 10; i++) {
              model = glm::translate(glm::mat4(1.0), cubePositions[i]);
              //model = glm::scale(model, glm::vec3(1.0f + std::cos(glm::radians(currentTime * 0.5f)), 1.0f + std::sin(glm::radians(currentTime * 2.0f)), 1.0f));
              model = glm::rotate(model, glm::radians(45.0f * currentTime), glm::vec3(1.0, 1.0, 0.0));
              normal = glm::mat3(glm::transpose(glm::inverse(model)));

              program.SetMat4("model", model);
              program.SetMat3("normal", normal);

              mesh.Bind();
              glDrawElements(GL_TRIANGLES, draw.indices, GL_UNSIGNED_INT, 0);
          }
          Mesh::Unbind();

          model = glm::translate(glm::mat4(1.0), lightPosition);

          lightProgram.Bind();
          lightProgram.SetMat4("model", model);
          lightProgram.SetVec3("uAlbedoFlatColor", glm::vec3(1.0,1.0,1.0));
          lightProgram.SetVec3("uLightColor", glm::vec3(1.0,1.0,1.0));

          lightMesh.Bind();
          glDrawElements(GL_TRIANGLES, lightDraw.indices, GL_UNSIGNED_INT, 0);
          lightMesh.Unbind();

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
