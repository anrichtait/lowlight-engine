/**
  \file G3D-base.lib/source/PhysicsFrame.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Any.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/PhysicsFrame.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/UprightFrame.h"

namespace G3D {

PhysicsFrame::PhysicsFrame() {
    translation = Vector3::zero();
    rotation    = Quat();
}


PhysicsFrame::PhysicsFrame
   (const CoordinateFrame& coordinateFrame) {
    translation = coordinateFrame.translation;
    rotation    = Quat(coordinateFrame.rotation);
}


Any PhysicsFrame::toAny() const {
    // Prefer to serialize as a CFrame, which is easier to read
    if (false) {
        Any a(Any::ARRAY, "PFrame");
        a.append(rotation, translation.toAny("Point3"));
        return a;
    } else {
        return CFrame(*this).toAny();
    }
}


PhysicsFrame::PhysicsFrame(const Any& a) {
    const String& n = toLower(a.name());
    *this = PhysicsFrame();

    if (beginsWith(n, "vector3") || beginsWith(n, "point3")) {
        *this = PhysicsFrame(Point3(a));
    } else if (beginsWith(n, "matrix3")) {        
        *this = PhysicsFrame(Matrix3(a));
    } else if (beginsWith(n, "cframe") || beginsWith(n, "coordinateframe") || beginsWith(n, "matrix4")) {        
        *this = CoordinateFrame(a);
    } else if (beginsWith(n, "uprightframe")) {        
        *this = UprightFrame(a).toCoordinateFrame();
    } else if (beginsWith(n, "pframe") || beginsWith(n, "physicsframe")) {
        if (a.type() == Any::ARRAY) {
            a.verifySize(2);
            rotation    = a[0];
            translation = a[1];
        } else {
            for (Any::AnyTable::Iterator it = a.table().begin(); it.isValid(); ++it) {
                const String& n = toLower(it->key);
                if (n == "translation") {
                    translation = it->value;
                } else if (n == "rotation") {
                    rotation = it->value;
                } else {
                    a.verify(false, "Illegal table key: " + it->key);
                }
            }
        }
    } else {
        a.verify(false, "Unrecognized class name where a PhysicsFrame or equivalent was expected.");
    }
}


PhysicsFrame PhysicsFrame::operator*(const PhysicsFrame& other) const {
    PhysicsFrame result;

    result.rotation = rotation * other.rotation;
    result.translation = translation + rotation.toRotationMatrix() * other.translation;

    return result;
}


PhysicsFrame::operator CoordinateFrame() const {
    CoordinateFrame f;
    
    f.translation = translation;
    f.rotation    = rotation.toRotationMatrix();

    return f;
}


PhysicsFrame PhysicsFrame::lerp(
    const PhysicsFrame&     other,
    float                   alpha) const {

    PhysicsFrame result;

    result.translation = translation.lerp(other.translation, alpha);
    result.rotation    = rotation.slerp(other.rotation, alpha);

    return result;
}


void PhysicsFrame::deserialize(class BinaryInput& b) {
    translation.deserialize(b);
    rotation.deserialize(b);
}


void PhysicsFrame::serialize(class BinaryOutput& b) const {
    translation.serialize(b);
    rotation.serialize(b);
}


}; // namespace

