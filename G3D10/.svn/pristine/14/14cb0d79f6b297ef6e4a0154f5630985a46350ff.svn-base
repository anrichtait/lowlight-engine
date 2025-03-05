/**
  \file G3D-gfx.lib/source/BindlessTextureHandle.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-gfx/BindlessTextureHandle.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/GLSamplerObject.h"
#include "G3D-gfx/glcalls.h"
#include "G3D-gfx/GLCaps.h"

namespace G3D {

    BindlessTextureHandle::BindlessTextureHandle() : m_GLHandle(0) {}
    BindlessTextureHandle::~BindlessTextureHandle() {
        if (isValid() && isResident()) {
            makeNonResident();
        }
    }
    BindlessTextureHandle::BindlessTextureHandle(shared_ptr<Texture> tex, const Sampler& sampler) : m_GLHandle(0) {
        set(tex, sampler);
    }

    void BindlessTextureHandle::set(shared_ptr<Texture> tex, const Sampler& sampler) {
        debugAssertM(GLCaps::supports("GL_ARB_bindless_texture"), "GL_ARB_bindless_texture not supported, cannot use BindlessTextureHandle");
        if (isValid() && isResident()) {
            makeNonResident();
        }
        m_texture = tex;
        m_samplerObject = GLSamplerObject::create(sampler);
        m_GLHandle = glGetTextureSamplerHandleARB(tex->openGLID(), m_samplerObject->openGLID());
        debugAssertGLOk();
        alwaysAssertM(m_GLHandle != 0, "BindlessTextureHandle was unable to create a proper handle");
        makeResident();
    }


    bool BindlessTextureHandle::isResident() const {
        return isValid() && glIsTextureHandleResidentARB(m_GLHandle);
    }

    void BindlessTextureHandle::makeResident() {
        alwaysAssertM(isValid(), "Attempted to makeResident BindlessTextureHandle");
        if (!isResident()) {
            glMakeTextureHandleResidentARB(m_GLHandle);
            debugAssertGLOk();
        }
    }

    void BindlessTextureHandle::makeNonResident() {
        if (isValid() && isResident()) {
            glMakeTextureHandleNonResidentARB(m_GLHandle);
            debugAssertGLOk();
        }
    }

}