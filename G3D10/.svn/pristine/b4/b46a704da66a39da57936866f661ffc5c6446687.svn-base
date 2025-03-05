/**
  \file G3D-app.lib/source/TextSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/TextSurface.h"

namespace G3D {
    TextSurface::TextSurface
    (const String&                       name,
        const CFrame&                       frame,
        const CFrame&                       previousFrame,
        const shared_ptr<FontModel>&       model,
        const shared_ptr<Entity>&           entity,
        const Surface::ExpressiveLightScatteringProperties& expressive) :
        Surface(expressive),
        m_name(name),
        m_frame(frame),
        m_previousFrame(previousFrame),
        m_fontModel(model),
        m_profilerHint((notNull(entity) ? entity->name() + "/" : "") + (notNull(model) ? model->name() + "/" : "") + m_name) {

        m_entity = entity;
        m_preferLowResolutionTransparency = false;

    }


    shared_ptr<TextSurface> TextSurface::create
    (const String&                       name,
        const CFrame&                       frame,
        const CFrame&                       previousFrame,
        const shared_ptr<FontModel>&       model,
        const shared_ptr<Entity>&           entity,
        const Surface::ExpressiveLightScatteringProperties& expressive) {

        return createShared<TextSurface>(name, frame, previousFrame, model, entity, expressive);

    }


    void TextSurface::render
    (RenderDevice*               rd,
        const LightingEnvironment&  environment,
        RenderPassType              passType) const {

        //Draw the text in the world.
        if (passType == RenderPassType::OPAQUE_SAMPLES)
            m_fontModel->font->draw3DBillboard(rd, m_fontModel->m_pose->text, m_frame.translation, m_fontModel->m_pose->size, m_fontModel->m_pose->color);
    }
}
