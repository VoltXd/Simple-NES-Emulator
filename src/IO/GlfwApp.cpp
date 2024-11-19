#include "IO/GlfwApp.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void resizeCallback(GLFWwindow* window, int windowWidth, int windowHeight);

GlfwApp::GlfwApp(Controller& controllerRef)
    : mControllerRef(controllerRef)
{
    // Window size
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 720;

    // Init GLFW + Version + Core profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window
    mWindow = glfwCreateWindow(windowWidth, windowHeight, "Simple NES Emulator", nullptr, nullptr);
    if (mWindow == nullptr)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(mWindow);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Tells openGL the position & size of the view port
    resizeCallback(mWindow, windowWidth, windowHeight);

    // Enable keyboard & resize callbacks
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetKeyCallback(mWindow, keyCallback);
    glfwSetWindowSizeCallback(mWindow, resizeCallback);

    // Screen VAO, VBO & EBO
    // VAO
    glGenVertexArrays(1, &mScreenVao);
    glBindVertexArray(mScreenVao);

    // VBO
    glGenBuffers(1, &mScreenVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mScreenVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_VERTICES), SCREEN_VERTICES, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // EBO
    glGenBuffers(1, &mScreenEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mScreenEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SCREEN_INDICES), SCREEN_INDICES, GL_STATIC_DRAW);

    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Shaders
    mScreenShader = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");
    mScreenShader->use();
    mScreenShader->setInt("screenTexture", 0);

    // Screen texture (ToDo: PBO & FBO ?)
    glGenTextures(1, &mScreenTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mScreenTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BORDER_COLOR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, PPU_OUTPUT_WIDTH, PPU_OUTPUT_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);

    // OpenGL settings
    glEnable(GL_CULL_FACE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

GlfwApp::~GlfwApp()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &mScreenVao);
    glDeleteBuffers(1, &mScreenVbo);
    glDeleteBuffers(1, &mScreenEbo);
    glDeleteTextures(1, &mScreenTexture);
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void GlfwApp::draw(const picture_t& pictureBuffer)
{
    // Poll events
    glfwPollEvents();

    // Clear frame buffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiID dockSpaceID = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    // TODO: wrap & add options
    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    // Bind screen vao & shader
    mScreenShader->use();
    glBindVertexArray(mScreenVao);  

    // Update screen texture
    // TODO: implement with TexSubImage2D (try FBO & PBO) ?
    // SubImage
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mScreenTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PPU_OUTPUT_WIDTH, PPU_OUTPUT_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pictureBuffer.data()->data()->data());

    // Draw screen texture to ImGUI window
    ImGui::SetNextWindowDockID(dockSpaceID, ImGuiCond_Once);
    drawEmulatorWindow();
    
    // Render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(mWindow);
}

void GlfwApp::updateControllerState(ControllerInput input, bool isPressed)
{
    mControllerRef.updateControllerState(input, isPressed);
}

void GlfwApp::drawEmulatorWindow()
{
    ImGui::Begin("Emulator", nullptr, ImGuiWindowFlags_NoTitleBar);

    // Get top left position window position
    ImVec2 windowTopLeft = ImGui::GetCursorScreenPos();
    ImVec2 windowBotRight;

    // Get current window size
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float windowHeight = ImGui::GetContentRegionAvail().y;

    // Calculate NES window positions 
    if (windowWidth / windowHeight > NES_ASPECT_RATIO)
    {
        // Width is too big
        float windowWidthRatioed = windowHeight * NES_ASPECT_RATIO;
        float windowXOffset = (windowWidth - windowWidthRatioed) / 2; 

        // Top left
        windowTopLeft.x += windowXOffset;
        
        // Bottom right
        windowBotRight.x = windowTopLeft.x + windowWidthRatioed;
        windowBotRight.y = windowTopLeft.y + windowHeight;
    }
    else
    {
        // Height is too big
        float windowHeightRatioed = windowWidth * (1.0f / NES_ASPECT_RATIO);
        float windowYOffset = (windowHeight - windowHeightRatioed) / 2; 

        // Top left
        windowTopLeft.y += windowYOffset;
        
        // Bottom right
        windowBotRight.x = windowTopLeft.x + windowWidth;
        windowBotRight.y = windowTopLeft.y + windowHeightRatioed;
    }

    // Send NES image texture to window
    ImGui::GetWindowDrawList()->AddImage(mScreenTexture, 
                                         windowTopLeft, 
                                         windowBotRight,
                                         ImVec2(0, 0),
                                         ImVec2(1, 1));

    ImGui::End();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GlfwApp* renderer = reinterpret_cast<GlfwApp*>(glfwGetWindowUserPointer(window));
    if (renderer)
    {
        // Handle keyboard press
        bool isPressed;
        ControllerInput input;

        if (mods != 0x0000)
            // No input if ALT, CTRL, Super...
            return;
        scancode; // Useless line to remove warning

        // Window
        if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);

        // Controller 1
        if (action == GLFW_PRESS)
            isPressed = true;
        else if (action == GLFW_RELEASE)
            isPressed = false;
        else 
            return;

        if (key == GLFW_KEY_K)
            input = ControllerInput::A;
        else if  (key == GLFW_KEY_J)
            input = ControllerInput::B;
        else if  (key == GLFW_KEY_G)
            input = ControllerInput::SELECT;
        else if  (key == GLFW_KEY_H)
            input = ControllerInput::START;
        else if  (key == GLFW_KEY_W)
            input = ControllerInput::UP;
        else if  (key == GLFW_KEY_S)
            input = ControllerInput::DOWN;
        else if  (key == GLFW_KEY_A)
            input = ControllerInput::LEFT;
        else if  (key == GLFW_KEY_D)
            input = ControllerInput::RIGHT;
        else
            return;
        
        renderer->updateControllerState(input, isPressed);
    }
}

void resizeCallback(GLFWwindow *window, int windowWidth, int windowHeight)
{
    window = nullptr; // Remove warning
    if ((float)windowWidth / windowHeight > NES_ASPECT_RATIO)
    {
        int windowWidth4_3 = (int)(windowHeight * NES_ASPECT_RATIO);
        int windowXOffset = (windowWidth - windowWidth4_3) / 2; 
        glViewport(windowXOffset, 0, windowWidth4_3, windowHeight);
    }
    else
    {
        int windowHeight4_3 = (int)(windowWidth * (1.0 / NES_ASPECT_RATIO));
        int windowYOffset = (windowHeight - windowHeight4_3) / 2; 
        glViewport(0, windowYOffset, windowWidth, windowHeight4_3);
    }
}
