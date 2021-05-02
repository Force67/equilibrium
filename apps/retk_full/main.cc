// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <GL/glew.h>
#include <glfw/glfw3.h>

#include <core/SkCanvas.h>
#include <core/SkFont.h>
#include <core/SkGraphics.h>
#include <core/SkSurface.h>
#include <core/SkColorSpace.h>

#include <gpu/gl/GrGLInterface.h>
#include <gpu/GrContextOptions.h>

#include <gpu/GrDirectContext.h>
#include <gpu/GrBackendSurface.h>

#include <effects/SkGradientShader.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


// uncomment the two lines below to enable correct color spaces
//#define GL_FRAMEBUFFER_SRGB 0x8DB9
//#define GL_SRGB8_ALPHA8 0x8C43

GrDirectContext* sContext = nullptr;
SkSurface* sSurface = nullptr;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

void key_callback(GLFWwindow* window,
                  int key,
                  int scancode,
                  int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void init_skia(int w, int h) {
  GrContextOptions options;
  sContext = GrDirectContext::MakeGL(nullptr, options).release();

  GrGLFramebufferInfo framebufferInfo;
  framebufferInfo.fFBOID = 0;  // assume default framebuffer
  // We are always using OpenGL and we use RGBA8 internal format for both RGBA
  // and BGRA configs in OpenGL.
  //(replace line below with this one to enable correct color spaces)
  //framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
  framebufferInfo.fFormat = GL_RGBA8;
  #if 0
  SkColorType colorType;
  if (kRGBA_8888_GrPixelConfig == kSkia8888_GrPixelConfig) {
    colorType = kRGBA_8888_SkColorType;
  } else {
    colorType = kBGRA_8888_SkColorType;
  }
  #endif
  SkColorType colorType = kBGRA_8888_SkColorType;

  GrBackendRenderTarget backendRenderTarget(w, h,
                                            0,  // sample count
                                            0,  // stencil bits
                                            framebufferInfo);

  //(replace line below with this one to enable correct color spaces) sSurface =
  //SkSurface::MakeFromBackendRenderTarget(sContext, backendRenderTarget,
  //kBottomLeft_GrSurfaceOrigin, colorType, SkColorSpace::MakeSRGB(),
  //nullptr).release();
  sSurface = SkSurface::MakeFromBackendRenderTarget(
                 sContext, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                 colorType, nullptr, nullptr)
                 .release();
  if (sSurface == nullptr)
    abort();
}

void cleanup_skia() {
  delete sSurface;
  delete sContext;
}

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

const int kWidth = 960;
const int kHeight = 640;
const char* glsl_version = "#version 130";

int main(void) {
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //(uncomment to enable correct color spaces) glfwWindowHint(GLFW_SRGB_CAPABLE,
  //GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);

  window = glfwCreateWindow(kWidth, kHeight, "ReTK", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  //(uncomment to enable correct color spaces) glEnable(GL_FRAMEBUFFER_SRGB);

  bool err = glewInit() != GLEW_OK;

  init_skia(kWidth, kHeight);

  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);

   // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Draw to the surface via its SkCanvas.
  SkCanvas* canvas =
      sSurface->getCanvas();  // We don't manage this pointer's lifetime.

  while (!glfwWindowShouldClose(window)) {
    glfwWaitEvents();

        // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    canvas->drawPaint(paint);
    paint.setColor(SK_ColorBLUE);
    canvas->drawRect({100, 200, 300, 500}, paint);
    sContext->flush();

        {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                      // and append into it.

      ImGui::Text("This is some useful text.");  // Display some text (you can
                                                 // use a format strings too)

      ImGui::SliderFloat(
          "float", &f, 0.0f,
          1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f

      if (ImGui::Button(
              "Button"))  // Buttons return true when clicked (most widgets
                          // return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

        ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  cleanup_skia();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}