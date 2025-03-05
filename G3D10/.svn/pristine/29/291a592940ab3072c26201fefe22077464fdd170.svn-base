/**
  \file G3D-app.lib/source/ArticulatedModel_animation.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/ArticulatedModel.h"

namespace G3D {

void ArticulatedModel::Animation::getCurrentPose(SimTime time, Pose& pose) {   
    poseSpline.get(float(time), pose);
}

}
