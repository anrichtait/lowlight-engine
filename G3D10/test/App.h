/**
  \file test/App.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef App_h
#define App_h
#include <G3D/G3D.h>

/** Application framework. */
class App : public GApp {
protected:

    int                 m_frameCount;

    bool                m_success;
    
    int                 m_sceneIndex;

    /** Called from onInit */
    void makeGBuffer();

    bool setupGoldStandardMode() const;

    void saveAndPossiblyCompareTextureToGoldStandard(const String& name, const shared_ptr<Texture>& texture);

public:

    bool success() const { return m_success; }
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;

    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
};

#endif
