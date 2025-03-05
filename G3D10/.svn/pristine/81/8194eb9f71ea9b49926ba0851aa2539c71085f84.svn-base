/**
  \file G3D-app.lib/include/G3D-app/BilateralFilterSettings.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_BilateralFilterSettings_h

#include "G3D-base/platform.h"
#include "G3D-app/GBuffer.h"
namespace G3D {

class GuiPane;
class BilateralFilterSettings {
public:
    /** Filter radius in pixels. This will be multiplied by stepSize.
        Each 1D filter will have 2*radius + 1 taps. If set to 0, the filter is turned off.
        */
    int radius;

    /**
        Default is to step in 2-pixel intervals. This constant can be
        increased while radius decreases to improve
        performance at the expense of some dithering artifacts.

        Must be at least 1.
    */
    int stepSize;

    /** If true, ensure that the "bilateral" weights are monotonically decreasing
        moving away from the current pixel. Default is true. */
    bool monotonicallyDecreasingBilateralWeights;

    /**
        How much depth difference is taken into account.
        Default is 1.
    */
    float depthWeight;

    /**
        How much normal difference is taken into account.
        Default is 1.
    */
    float normalWeight;

    /**
        How much plane difference is taken into account.
        Default is 1.
    */
    float planeWeight;

    /**
    How much glossy exponent is taken into account.
    Default is 1.
    */
    float glossyWeight;

    /** If greater than zero, inscribe a disk in the maximum of the width
        and height and framebuffer and only compute the output within that disk.
        Used for VR. */
    float computeFraction = -1.0f;


    BilateralFilterSettings();

    /**
    Ensures the GBuffer specification has all the fields needed to render this effect
    \sa GApp::extendGBufferSpecification
    */
    void extendGBufferSpecification(GBuffer::Specification& spec) const;

    void makeGUI(GuiPane* pane);

};
}