#pragma once
#include <G3D/G3D.h>

/** \brief Application framework. */
class App : public GApp {
protected:
    shared_ptr<Texture>           m_source;

    /** We write to the destination on the GPU, so it must be a framebuffer.
        m_destination->texture(0) is the texture that you can use as the input to a subsequent
        pass, or rendering onto the screen for visualization. */
    shared_ptr<Framebuffer>       m_destination;

    /** Standard deviation of the unsharp mask's primary lobe */
    float                         m_sigma         = 3.0f;

    GuiNumberBox<float>*          m_sigmaSlider   = nullptr;
    bool                          m_showSource    = false;

    void makeGUI();

    /** Recomputes m_destination */
    void onParameterChange();

public:

    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;

    virtual bool onEvent(const GEvent& event) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;
};
