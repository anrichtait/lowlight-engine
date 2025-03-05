/**
  \file G3D-app.lib/source/BilateralFilterSettings.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/BilateralFilterSettings.h"
#include "G3D-app/GuiPane.h"

namespace G3D {
    BilateralFilterSettings::BilateralFilterSettings() :
        radius(4),
        stepSize(2),
        monotonicallyDecreasingBilateralWeights(true),
        depthWeight(1.0f),
        normalWeight(1.0f),
        planeWeight(1.0f),
        glossyWeight(0.0f) {
    }

    void BilateralFilterSettings::extendGBufferSpecification(GBuffer::Specification & spec) const {
        if (radius > 0) {
            if (normalWeight > 0 || planeWeight > 0) {
                if (spec.encoding[GBuffer::Field::CS_NORMAL].format == nullptr) {

                    const ImageFormat* normalFormat = ImageFormat::RGB10A2();
                    spec.encoding[GBuffer::Field::CS_NORMAL] = Texture::Encoding(normalFormat, FrameName::CAMERA, 2.0f, -1.0f);
                }
            }
            if (glossyWeight > 0) {
                if (spec.encoding[GBuffer::Field::GLOSSY].format == nullptr) {

                    const ImageFormat* glossyFormat = ImageFormat::RGBA8();
                    spec.encoding[GBuffer::Field::GLOSSY] = glossyFormat;
                }
            }
        }
    }

    void BilateralFilterSettings::makeGUI(GuiPane * pane) {
        pane->addLabel("Bilateral Filter Settings");
        pane->addNumberBox("Blur Radius", &radius, "", GuiTheme::LINEAR_SLIDER, 0, 24);
        pane->addNumberBox("Blur Step", &stepSize, "", GuiTheme::LINEAR_SLIDER, 1, 4);
        pane->addCheckBox("monotonicallyDecreasingBilateralWeights;", &monotonicallyDecreasingBilateralWeights);
        pane->addNumberBox("Depth Weight", &depthWeight, "", GuiTheme::LINEAR_SLIDER, 0.0f, 5.0f);
        pane->addNumberBox("Normal Weight", &normalWeight, "", GuiTheme::LINEAR_SLIDER, 0.0f, 5.0f);
        pane->addNumberBox("Plane Weight", &planeWeight, "", GuiTheme::LINEAR_SLIDER, 0.0f, 5.0f);
        pane->addNumberBox("Glossy Weight", &glossyWeight, "", GuiTheme::LINEAR_SLIDER, 0.0f, 5.0f);
    }
}