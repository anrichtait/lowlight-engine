/**
  \file G3D-app.lib/include/G3D-app/ArticulatedModelSpecificationEditorDialog.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_Articulated_Model_Specification_Editor_Dialog_H
#define G3D_Articulated_Model_Specification_Editor_Dialog_H

#include "G3D-base/platform.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiButton.h"
#include "G3D-app/GuiTextBox.h"
#include "G3D-app/GuiNumberBox.h"
#include "ArticulatedModel.h"

namespace G3D {

class ArticulatedModelSpecificationEditorDialog : public GuiWindow {
protected:
    typedef ParseOBJ::Options::TexCoord1Mode TexCoord1Mode;
    enum Translation {NONE, CENTER, BASE};

    bool                  ok;
    bool                  useAutoScale;
                       
    Translation           translation;
    TexCoord1Mode         texCoord1Mode;
    bool                  stripRefraction;

    GuiButton*            okButton;
    GuiButton*            cancelButton;
    GuiButton*            saveButton;
    GuiTextBox*           fileNameBox;
    GuiNumberBox<float>*  scaleBox;

    ArticulatedModel::Specification  m_spec;

    OSWindow*          m_osWindow;

    ArticulatedModelSpecificationEditorDialog(OSWindow* osWindow, const shared_ptr<GuiTheme>& theme, const String& note);

    /** Generates the final Specification based on stored state */
    void finalizeSpecification();
    
    /** Save a .ArticulatedModel.Any with the same name as the model in the directory of the model */
    void save();
    void close();

public:

    static shared_ptr<ArticulatedModelSpecificationEditorDialog> create(OSWindow* osWindow, const shared_ptr<GuiTheme>& theme, const String& note = "") {
        return shared_ptr<ArticulatedModelSpecificationEditorDialog>(new ArticulatedModelSpecificationEditorDialog(osWindow, theme, note));
    }

    static shared_ptr<ArticulatedModelSpecificationEditorDialog> create(const shared_ptr<GuiWindow>& parent, const String& note = "") {
        return create(parent->window(), parent->theme(), note);
    }

    /**
       @param spec  This is the initial specification shown, and unless cancelled, receives the final specification as well.
       @return True unless cancelled
    */
    // spec is passed as a reference instead of a pointer because it will not be used after the
    // method ends, so there is no danger of the caller misunderstanding as there is with the GuiPane::addTextBox method.
    virtual bool getSpecification(ArticulatedModel::Specification& spec, const String& caption = "Design Specification");
    virtual bool onEvent(const GEvent& e);

};

} // namespace

#endif
