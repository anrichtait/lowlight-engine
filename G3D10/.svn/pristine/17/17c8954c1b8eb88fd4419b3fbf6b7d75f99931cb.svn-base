/**
  \file App.h
 */
#pragma once
#include <G3D/G3D.h>

/** \brief Application framework. */
class App : public GApp {
protected:

    GuiDropDownList*        m_resolution;
    PathTracer::Options     m_options;
    shared_ptr<PathTracer>  m_pathTracer;   
    
    // Artificially darken images because the scenes are scaled for direct illumination only,
    // and will be over exposed with true path tracing.
    float                   m_radianceScale = 0.5f;

    /** Called from onInit */
    void makeGUI();

    Vector2int32 resolution() const;

    /** Called by onRender and the other batch image processing routines.*/
    shared_ptr<Texture> renderOneImage(const PathTracer::Options& options, float& elapsedTime, shared_ptr<Texture>& HDRImage);

public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;

    void onRender();
    /** Render images of all scenes, for testing */
    void onBatchRender();
    void onRenderConvergence();
};
