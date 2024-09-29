#ifndef SPINNERAPP_HPP
#define SPINNERAPP_HPP

#include "Spinner/App.hpp"

class SpinnerApp final : public Spinner::App
{
public:
    SpinnerApp();
    ~SpinnerApp() override = default;

protected:
    Spinner::Scene::Pointer Scene;
    Spinner::ImGuiInstance::Pointer ImGuiInstance;
    Spinner::Lighting::Pointer Lighting;

    Spinner::Image::Pointer DepthImage;
    bool ViewDebugUI = true;

    float MouseSensitivity = 1.0f;
    int BaseMovementSpeed = 0;

    constexpr static float BaseMovementSpeedModifier = 1.0f / 16.0f;

    void AppInit() override;
    void AppRender(Spinner::CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex) override;
    void AppCleanup() override;
    void AppUpdate() override;
    void AppImGui() override;
    void RecreateDepthImage();
};


#endif //SPINNERAPP_HPP
