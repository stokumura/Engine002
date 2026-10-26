#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <stb/stb_image.h>

#include <cmath>
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
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

glm::vec3 lightColorAmbient(0.05f, 0.05f, 0.05f);
glm::vec3 lightColorDiffuse(0.8f, 0.8f, 0.8f);
glm::vec3 lightColorSpecular(0.5f, 0.5f, 0.5f);
glm::vec3 lightPosition(2.0f, 3.0f, -5.0f);
glm::vec3 lightDirection(-0.2f, -0.1f, -0.3f);

float shininess = 16.0f;
float constantAttenuation = 1.0f;
float linearAttenuation = 0.09f;
float quadraticAttenuation = 0.032f;
float cutOff = glm::cos(glm::radians(12.5f));
float outerCutOff = glm::cos(glm::radians(17.5f));

RenderTexture render;

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
      render = RenderTexture(currentWidth, currentHeight);
      ShadowMap shadowMap;

      Sphere sphere(Vertex3DUnlit, 0.1f, 16, 16);
      Cube cube(Vertex3DLit, 1.0f, 16, 16, 16);
      Quad quad(Vertex2D);
      Cube skyboxCube({ VertexAttributeType::POSITION3D }, 2.0f, 1, 1, 1, true);
      Billboard planeBillboard(Vertex3DLit, 1, 16, 16, 50.0f);

      const Mesh &mesh = cube.GetMesh();
      const Mesh &lightMesh = sphere.GetMesh();
      const Mesh &screen = quad.GetMesh();
      const Mesh &skybox = skyboxCube.GetMesh();
      const Mesh &plane = planeBillboard.GetMesh();

      std::filesystem::path texturesPath = GetResourcesPath() / "textures";
      std::filesystem::path albedoTexturePath = texturesPath / "container2.png";
      std::filesystem::path specularTexturePath = texturesPath / "container2_specular.png";
      std::filesystem::path woodAlbedoTexturePath = texturesPath / "wood.png";
      Texture albedoTexture(albedoTexturePath.string(), TextureType::DIFFUSE, 0);
      Texture specularTexture(specularTexturePath.string(), TextureType::SPECULAR, 1);
      Texture woodAlbedoTexture(woodAlbedoTexturePath.string(), TextureType::DIFFUSE, 0);

      std::filesystem::path skyboxTexturesParentPath = texturesPath / "skybox";
      std::vector<std::string> cubemapFaces { 
          "right.jpg",
          "left.jpg",
          "top.jpg",
          "bottom.jpg",
          "front.jpg",
          "back.jpg"
      };
      Cubemap cubemapTex(skyboxTexturesParentPath, cubemapFaces);

      std::filesystem::path modelsPath = GetResourcesPath() / "objects";
      std::filesystem::path backpackPath = modelsPath / "backpack" / "backpack.obj";
      Model backpack(Vertex3DLit, backpackPath);

      std::filesystem::path shadersPath = GetResourcesPath() / "shaders";
      std::filesystem::path vertexShaderPath = shadersPath / "objectMaterial.vert";
      std::filesystem::path fragmentShaderPath = shadersPath / "objectMaterialDirectionalShadow.frag";
      std::filesystem::path vertexShaderLightPath = shadersPath / "objectFlatColor.vert";
      std::filesystem::path fragmentShaderLightPath = shadersPath / "objectFlatColor.frag";
      std::filesystem::path vertexShaderScreen = shadersPath / "screenQuad.vert";
      std::filesystem::path fragmentShaderScreen = shadersPath / "postProcGrayscale.frag";
      std::filesystem::path vertexShaderSkybox = shadersPath / "skybox.vert";
      std::filesystem::path fragmentShaderSkybox = shadersPath / "skybox.frag";
      std::filesystem::path fragmentShaderReflection = shadersPath / "objectReflect.frag";
      std::filesystem::path fragmentShaderRefraction = shadersPath / "objectRefract.frag";
      std::filesystem::path vertexTiledShaderPath = shadersPath / "objectTiledMaterial.vert";
      std::filesystem::path vertexShaderDepthMap = shadersPath / "simpleDepthMap.vert";
      std::filesystem::path fragmentShaderEmpty = shadersPath / "empty.frag";

      ShaderProgram program;
      ShaderProgram modelProgram;
      ShaderProgram floorProgram;
      ShaderProgram lightProgram;
      ShaderProgram screenProgram;
      ShaderProgram skyboxProgram;
      ShaderProgram reflectionProgram;
      ShaderProgram refractionProgram;
      ShaderProgram depthMapProgram;
      {
          ShaderVariants vertex(vertexShaderPath.string(), ShaderType::VERTEX);
          ShaderVariants fragment(fragmentShaderPath.string(), ShaderType::FRAGMENT);
          ShaderVariants vertexLight(vertexShaderLightPath.string(), ShaderType::VERTEX);
          ShaderVariants fragmentLight(fragmentShaderLightPath.string(), ShaderType::FRAGMENT);
          ShaderVariants vertexScreen(vertexShaderScreen.string(), ShaderType::VERTEX);
          ShaderVariants fragmentScreen(fragmentShaderScreen.string(), ShaderType::FRAGMENT);
          ShaderVariants vertexSkybox(vertexShaderSkybox.string(), ShaderType::VERTEX);
          ShaderVariants fragmentSkybox(fragmentShaderSkybox.string(), ShaderType::FRAGMENT);
          ShaderVariants fragmentReflection(fragmentShaderReflection.string(), ShaderType::FRAGMENT);
          ShaderVariants fragmentRefraction(fragmentShaderRefraction.string(), ShaderType::FRAGMENT);
          ShaderVariants vertexTiled(vertexTiledShaderPath.string(), ShaderType::VERTEX);
          ShaderVariants vertexDepthMap(vertexShaderDepthMap.string(), ShaderType::VERTEX);
          ShaderVariants fragmentEmpty(fragmentShaderEmpty.string(), ShaderType::FRAGMENT);

          program.AttachShader(vertex.GetShader({ "USE_NORMAL_MATRIX", "USE_LIGHT_SPACE_MATRIX" }));
          program.AttachShader(fragment.GetShader({ "USE_ALBEDO_TEXTURE_MAP", "USE_SPECULAR_TEXTURE_MAP", "USE_ALBEDO_AS_AMBIENT" }));
          program.Compile();

          modelProgram.AttachShader(vertex.GetShader({ "USE_NORMAL_TEXTURE_MAP", "USE_LIGHT_SPACE_MATRIX" }));
          modelProgram.AttachShader(fragment.GetShader({ "USE_ALBEDO_TEXTURE_MAP", "USE_SPECULAR_TEXTURE_MAP", "USE_NORMAL_TEXTURE_MAP" , "USE_ALBEDO_AS_AMBIENT", "SPECULAR_IS_GRAY" }));
          modelProgram.Compile();

          floorProgram.AttachShader(vertexTiled.GetShader({ "USE_NORMAL_MATRIX", "USE_LIGHT_SPACE_MATRIX" }));
          floorProgram.AttachShader(fragment.GetShader({ "USE_ALBEDO_TEXTURE_MAP", "USE_ALBEDO_AS_AMBIENT" }));
          floorProgram.Compile();

          lightProgram.AttachShader(vertexLight.GetBaseShader());
          lightProgram.AttachShader(fragmentLight.GetBaseShader());
          lightProgram.Compile();

          screenProgram.AttachShader(vertexScreen.GetBaseShader());
          screenProgram.AttachShader(fragmentScreen.GetBaseShader());
          screenProgram.Compile();

          skyboxProgram.AttachShader(vertexSkybox.GetBaseShader());
          skyboxProgram.AttachShader(fragmentSkybox.GetBaseShader());
          skyboxProgram.Compile();

          reflectionProgram.AttachShader(vertex.GetShader({ "USE_NORMAL_MATRIX" }));
          reflectionProgram.AttachShader(fragmentReflection.GetBaseShader());
          reflectionProgram.Compile();

          refractionProgram.AttachShader(vertex.GetShader({ "USE_NORMAL_MATRIX" }));
          refractionProgram.AttachShader(fragmentRefraction.GetBaseShader());
          refractionProgram.Compile();

          depthMapProgram.AttachShader(vertexDepthMap.GetBaseShader());
          depthMapProgram.AttachShader(fragmentEmpty.GetBaseShader());
          depthMapProgram.Compile();
      }

      CameraData cameraData;
      UniformBlock cameraMatrices("CameraData", 2, sizeof(CameraData), &cameraData);
      program.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());
      modelProgram.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());
      floorProgram.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());
      lightProgram.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());
      reflectionProgram.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());
      refractionProgram.BindUniformBlock(cameraMatrices.GetBindingPoint(), cameraMatrices.GetName());

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

          float speed = 25.0f;
          float angleC = glm::radians(currentTime * speed);
          float radiusC = 5.0f;
          glm::vec3 translationB(-2.0f, -1.0f, -3.0f);
          glm::vec3 translationC(radiusC * std::cos(angleC), 0.0f, radiusC * std::sin(angleC));
          translationC += translationB;

          // SHADOW MAP PASS
          shadowMap.BindFBO();
          glEnable(GL_DEPTH_TEST);
          glCullFace(GL_FRONT);
          glDisable(GL_FRAMEBUFFER_SRGB);
          glClear(GL_DEPTH_BUFFER_BIT);
          float near_plane = 1.0f, far_plane = 100.0f, side_size = 50.0f;
          glm::mat4 lightProjection = glm::ortho(-side_size, side_size, -side_size, side_size, near_plane, far_plane);
          glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
          glm::mat4 lightSpaceMatrix = lightProjection * lightView;

          depthMapProgram.Bind();
          depthMapProgram.SetMat4("uLightSpaceMatrix", lightSpaceMatrix);
          // CONTAINER - SHADOW
          glm::mat4 model(1.0f);
          model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));
          model = glm::translate(model, translationC);
          depthMapProgram.SetMat4("model", model);
          mesh.Draw(depthMapProgram);
          depthMapProgram.Bind();
          // MODEL - SHADOW
          model = glm::translate(glm::mat4(1.0f), translationB);
          depthMapProgram.SetMat4("model", model);
          backpack.Draw(depthMapProgram);
          depthMapProgram.Bind();
          // FLOOR - SHADOW
          glCullFace(GL_BACK);
          model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
          model = glm::translate(model, glm::vec3(0.0f, -25.0f, 5.0f));
          depthMapProgram.SetMat4("model", model);
          plane.Draw(depthMapProgram);
          depthMapProgram.Bind();
          glCullFace(GL_FRONT);
          // ENVIRONMENT DYNAMIC - SHADOW
          //model = glm::translate(glm::mat4(1.0), glm::vec3(-4.0, 0.0f, -1.0));
          //depthMapProgram.SetMat4("model", model);
          //mesh.Draw(depthMapProgram);
          //depthMapProgram.Bind();
          //model = glm::translate(glm::mat4(1.0), glm::vec3(2.0, 0.0f, -1.0));
          //depthMapProgram.SetMat4("model", model);
          //mesh.Draw(depthMapProgram);

          shadowMap.UnbindFBO(currentWidth, currentHeight);
          glCullFace(GL_BACK);


          // FIRST PASS
          lightDirection = glm::normalize(-lightPosition);
          render.BindFBO();
          glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glEnable(GL_DEPTH_TEST);

          // CONTAINER
          program.Bind();
          program.SetFloat("uObjectMaterial.shininess", shininess);
          program.SetInt("uObjectMaterial.diffuse1", albedoTexture.GetUnit());
          program.SetInt("uObjectMaterial.specular1", specularTexture.GetUnit());
          program.SetInt("uShadowMap", 2);
          program.SetMat4("uLightSpaceMatrix", lightSpaceMatrix);
          program.SetVec3("uDirLight.direction", lightDirection);
          program.SetVec3("uDirLight.ambient", lightColorAmbient);
          program.SetVec3("uDirLight.diffuse", lightColorDiffuse);
          program.SetVec3("uDirLight.specular", lightColorSpecular);
          //program.SetInt("uNumPointLights", 1);
          //program.SetVec3("uPointLights[0].ambient", lightColorAmbient);
          //program.SetVec3("uPointLights[0].diffuse", lightColorDiffuse);
          //program.SetVec3("uPointLights[0].specular", lightColorSpecular);
          //program.SetVec3("uPointLights[0].position", lightPosition);
          //program.SetFloat("uPointLights[0].constant", constantAttenuation);
          //program.SetFloat("uPointLights[0].linear", linearAttenuation);
          //program.SetFloat("uPointLights[0].quadratic", quadraticAttenuation);
          //program.SetInt("uNumSpotLights", 1);
          //program.SetVec3("uSpotLights[0].position", camera.GetPosition());
          //program.SetVec3("uSpotLights[0].direction", camera.GetFront());
          //program.SetVec3("uSpotLights[0].ambient", lightColorAmbient);
          //program.SetVec3("uSpotLights[0].diffuse", lightColorDiffuse);
          //program.SetVec3("uSpotLights[0].specular", lightColorSpecular);
          //program.SetFloat("uSpotLights[0].constant", constantAttenuation);
          //program.SetFloat("uSpotLights[0].linear", linearAttenuation);
          //program.SetFloat("uSpotLights[0].quadratic", quadraticAttenuation);
          //program.SetFloat("uSpotLights[0].cutOff", cutOff);
          //program.SetFloat("uSpotLights[0].outerCutOff", outerCutOff);

          globalSampler.Bind(albedoTexture.GetUnit());
          albedoTexture.Bind();
          globalSampler.Bind(specularTexture.GetUnit());
          specularTexture.Bind();
          glBindSampler(2, 0);
          shadowMap.BindMap(2);

          glm::mat3 normal(1.0);
          model = glm::rotate(glm::mat4(1.0), glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));
          model = glm::translate(model, translationC);
          normal = glm::mat3(glm::transpose(glm::inverse(model)));
          program.SetMat4("model", model);
          program.SetMat3("normal", normal);
          mesh.Draw(program, globalSampler);
          albedoTexture.Unbind();
          specularTexture.Unbind();
          shadowMap.UnbindMap(2);

          // MODEL
          modelProgram.Bind();
          modelProgram.SetFloat("uObjectMaterial.shininess", shininess);
          modelProgram.SetInt("uObjectMaterial.diffuse1", albedoTexture.GetUnit());
          modelProgram.SetInt("uObjectMaterial.specular1", specularTexture.GetUnit());
          modelProgram.SetMat4("uLightSpaceMatrix", lightSpaceMatrix);
          modelProgram.SetInt("uShadowMap", 3);
          modelProgram.SetVec3("uDirLight.direction", lightDirection);
          modelProgram.SetVec3("uDirLight.ambient", lightColorAmbient);
          modelProgram.SetVec3("uDirLight.diffuse", lightColorDiffuse);
          modelProgram.SetVec3("uDirLight.specular", lightColorSpecular);
          glBindSampler(3, 0);
          shadowMap.BindMap(3);

          model = glm::translate(glm::mat4(1.0f), translationB);
          normal = glm::mat3(glm::transpose(glm::inverse(model)));
          modelProgram.SetMat4("model", model);
          backpack.Draw(modelProgram, globalSampler);
          shadowMap.UnbindMap(3);

          // FLOOR
          floorProgram.Bind();
          floorProgram.SetFloat("uObjectMaterial.shininess", shininess);
          floorProgram.SetInt("uObjectMaterial.diffuse1", woodAlbedoTexture.GetUnit());
          floorProgram.SetVec3("uObjectMaterial.specular", glm::vec3(0.8f, 0.8f, 0.8f));
          floorProgram.SetMat4("uLightSpaceMatrix", lightSpaceMatrix);
          floorProgram.SetInt("uShadowMap", 2);
          floorProgram.SetVec3("uDirLight.direction", lightDirection);
          floorProgram.SetVec3("uDirLight.ambient", lightColorAmbient);
          floorProgram.SetVec3("uDirLight.diffuse", lightColorDiffuse);
          floorProgram.SetVec3("uDirLight.specular", lightColorSpecular);

          globalSampler.Bind(woodAlbedoTexture.GetUnit());
          woodAlbedoTexture.Bind();
          glBindSampler(2, 0);
          shadowMap.BindMap(2);

          model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
          model = glm::translate(model, glm::vec3(0.0f, -25.0f, 5.0f));
          normal = glm::mat3(glm::transpose(glm::inverse(model)));
          floorProgram.SetMat4("model", model);
          floorProgram.SetMat3("normal", normal);
          plane.Draw(floorProgram);
          woodAlbedoTexture.Unbind();
          shadowMap.UnbindMap(2);

          // ENVIRONMENT DYNAMIC
          //reflectionProgram.Bind();
          //model = glm::translate(glm::mat4(1.0), glm::vec3(-4.0, 0.0f, -1.0));
          //normal = glm::mat3(glm::transpose(glm::inverse(model)));
          //reflectionProgram.SetMat4("model", model);
          //reflectionProgram.SetMat3("normal", normal);
          //glBindSampler(0, 0);
          //cubemapTex.Bind(0);
          //reflectionProgram.SetInt("uSkybox", 0);
          //mesh.Draw(reflectionProgram);
          //glBindTextureUnit(0, 0);

          //refractionProgram.Bind();
          //model = glm::translate(glm::mat4(1.0), glm::vec3(2.0, 0.0f, -1.0));
          //normal = glm::mat3(glm::transpose(glm::inverse(model)));
          //refractionProgram.SetMat4("model", model);
          //refractionProgram.SetMat3("normal", normal);
          //glBindSampler(0, 0);
          //cubemapTex.Bind(0);
          //refractionProgram.SetInt("uSkybox", 0);
          //refractionProgram.SetFloat("uRefractionRatio", 1.0f / 1.52f);
          //mesh.Draw(refractionProgram);
          //glBindTextureUnit(0, 0);


          // LIGHT MESH
          model = glm::translate(glm::mat4(1.0), lightPosition);
          lightProgram.Bind();
          lightProgram.SetMat4("model", model);
          lightProgram.SetVec3("uAlbedoFlatColor", glm::vec3(1.0,1.0,1.0));
          lightProgram.SetVec3("uLightColor", glm::vec3(1.0,1.0,1.0));
          lightMesh.Draw(lightProgram, globalSampler);

          // SKYBOX
          glDepthFunc(GL_LEQUAL);
          skyboxProgram.Bind();
          glBindSampler(0, 0);
          skyboxProgram.SetInt("uSkybox", 0);
          skyboxProgram.SetMat4("uProjection", cameraData.projection);
          glm::mat4 noTranslationViewMatrix = glm::mat4(glm::mat3(cameraData.view));
          skyboxProgram.SetMat4("uView", noTranslationViewMatrix);
          cubemapTex.Bind(0);
          skybox.Bind();
          skybox.Draw(skyboxProgram);
          skybox.Unbind();
          glBindTextureUnit(0, 0);
          glDepthFunc(GL_LESS);

          // SECOND PASS
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT);
          glDisable(GL_DEPTH_TEST);
          glEnable(GL_FRAMEBUFFER_SRGB);

          screenProgram.Bind();
          glBindSampler(0, 0);
          screenProgram.SetInt("uScreenTexture", 0);
          render.BindTexture(0);
          screen.Bind();
          screen.Draw(screenProgram, globalSampler);
          glBindTextureUnit(0, 0);

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
    render = RenderTexture(width, height);
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
