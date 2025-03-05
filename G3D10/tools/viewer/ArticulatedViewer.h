/**
  \file tools/viewer/ArticulatedViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef ArticulatedViewer_h
#define ArticulatedViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class ArticulatedViewer : public Viewer3D {
private:

    shared_ptr<ArticulatedModel>            m_model;
    int                                     m_numFaces;
    int                                     m_numVertices;

    ArticulatedModel::Part*                 m_selectedPart;

    ArticulatedModel::Mesh*                 m_selectedMesh;

    /** In the Mesh's cpuIndexAray index array */
    int                                     m_selectedTriangleIndex;

    static shared_ptr<GFont>                m_font;

    /** Scale applied to the model; stored only for printing the value as an overlay */
    float                                   m_scale;

    /** Translation applied to the model; stored only for printing the value as an overlay */
    Vector3                                 m_offset;

    /** True if the shadow map is out of date. This is true for the first frame and continues
        to be true if the model animates. */
    bool                                    m_shadowMapDirty;

    SimTime                                 m_time;

    ArticulatedModel::Pose                  m_pose;

    // Will be empty if the model does not have skeletal animations
    ArticulatedModel::Animation             m_animation;

    /** Saves the geometry for the first part to a flat file */
    void saveGeometry();

public:

    ArticulatedViewer();
    ~ArticulatedViewer();
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onInit(const String& filename, App* app) override;
    virtual bool onEvent(const GEvent& e, App* app) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray)  override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;

};

#endif 
