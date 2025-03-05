/**
  \file G3D-app.lib/include/G3D-app/G3D-app.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once

#include "G3D-base/G3D-base.h"
#include "G3D-gfx/G3D-gfx.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Surfel.h"
#include "G3D-app/TriTree.h"
#include "G3D-app/TriTreeBase.h"
#include "G3D-app/NativeTriTree.h"
#include "G3D-app/EmbreeTriTree.h"
#include "G3D-app/OptiXTriTree.h"
#include "G3D-app/VulkanTriTree.h"
#include "G3D-app/GFont.h"
#include "G3D-app/UserInput.h"
#include "G3D-app/FirstPersonManipulator.h"
#include "G3D-app/Draw.h"
#include "G3D-app/Light.h"
#include "G3D-app/GApp.h"
#include "G3D-app/Surface.h"
#include "G3D-app/MD2Model.h"
#include "G3D-app/MD3Model.h"
#include "G3D-app/DepthOfFieldSettings.h"
#include "G3D-app/Shape.h"
#include "G3D-app/FilmSettings.h"
#include "G3D-app/Widget.h"
#include "G3D-app/ThirdPersonManipulator.h"
#include "G3D-app/GConsole.h"
#include "G3D-app/BSPMAP.h"
#include "G3D-app/UniversalMaterial.h"
#include "G3D-app/GaussianBlur.h"
#include "G3D-app/UniversalSurface.h"
#include "G3D-app/DirectionHistogram.h"
#include "G3D-app/UniversalBSDF.h"
#include "G3D-app/Component.h"
#include "G3D-app/Film.h"
#include "G3D-app/Tri.h"
#include "G3D-app/TriTree.h"
#include "G3D-app/GuiTheme.h"
#include "G3D-app/GuiButton.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiCheckBox.h"
#include "G3D-app/GuiControl.h"
#include "G3D-app/GuiContainer.h"
#include "G3D-app/GuiLabel.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GuiRadioButton.h"
#include "G3D-app/GuiSlider.h"
#include "G3D-app/GuiTextBox.h"
#include "G3D-app/GuiMenu.h"
#include "G3D-app/GuiDropDownList.h"
#include "G3D-app/GuiNumberBox.h"
#include "G3D-app/GuiFrameBox.h"
#include "G3D-app/GuiFunctionBox.h"
#include "G3D-app/GuiTextureBox.h"
#include "G3D-app/GuiTabPane.h"
#include "G3D-app/FileDialog.h"
#include "G3D-app/IconSet.h"
#include "G3D-app/MotionBlurSettings.h"
#include "G3D-app/LightingEnvironment.h"
#include "G3D-app/UprightSplineManipulator.h"
#include "G3D-app/CameraControlWindow.h"
#include "G3D-app/DeveloperWindow.h"
#include "G3D-app/VideoRecordDialog.h"
#include "G3D-app/SceneEditorWindow.h"
#include "G3D-app/ProfilerWindow.h"
#include "G3D-app/SettingsWindow.h"
#include "G3D-app/VideoInput.h"
#include "G3D-app/VideoOutput.h"
#include "G3D-app/ShadowMap.h"
#include "G3D-app/GBuffer.h"
#include "G3D-app/SlowMesh.h"
#include "G3D-app/Discovery.h"
#include "G3D-app/Entity.h"
#include "G3D-app/FontModel.h"
#include "G3D-app/VoxelModel.h"
#include "G3D-app/PointModel.h"
#include "G3D-app/ArticulatedModel.h"
#include "G3D-app/PhysicsFrameSplineEditor.h"
#include "G3D-app/Scene.h"
#include "G3D-app/SceneVisualizationSettings.h"
#include "G3D-app/UniversalSurfel.h"
#include "G3D-app/MotionBlur.h"
#include "G3D-app/HeightfieldModel.h"
#include "G3D-app/ArticulatedModelSpecificationEditorDialog.h"
#include "G3D-app/DebugTextWidget.h"
#include "G3D-app/Renderer.h"
#include "G3D-app/DefaultRenderer.h"
#include "G3D-app/ParticleSystem.h"
#include "G3D-app/ParticleSurface.h"
#include "G3D-app/PointSurface.h"
#include "G3D-app/VoxelSurface.h"
#include "G3D-app/TextSurface.h"
#include "G3D-app/AmbientOcclusion.h"
#include "G3D-app/DepthOfField.h"
#include "G3D-app/Skybox.h"
#include "G3D-app/SkyboxSurface.h"
#include "G3D-app/VisibleEntity.h"
#include "G3D-app/MarkerEntity.h"
#include "G3D-app/VisualizeCameraSurface.h"
#include "G3D-app/VisualizeLightSurface.h"
#include "G3D-app/SVO.h"
#include "G3D-app/Renderer.h"
#include "G3D-app/TemporalFilter.h"
#include "G3D-app/BilateralFilter.h"
#include "G3D-app/PathTracer.h"
#include "G3D-app/FogVolumeSurface.h"
#include "G3D-app/VRApp.h"
#include "G3D-app/XRWidget.h"
#include "G3D-app/GameController.h"
#include "G3D-app/PythonInterpreter.h"
#include "G3D-app/ScreenCapture.h"
#include "G3D-app/EmulatedGazeTracker.h"
#include "G3D-app/EmulatedXR.h"

// Set up the linker on Windows
#ifdef _MSC_VER
#   pragma comment(lib, "embree.lib")

#ifdef USE_ASSIMP
#   ifdef _DEBUG
#       pragma comment(lib, "assimpd")
#   else
#       pragma comment(lib, "assimp")
#   endif
#endif

#pragma comment(lib, "nfd")
/** \def G3D_NO_FFMPEG If you #define this when building G3D and before including
 GLG3D.h or G3DAll.h, then G3D will not link in FFmpeg DLL's and not provide video
 support or enable the video dialog in the developer window.
*/
//#   define G3D_NO_FFMPEG

#ifndef G3D_NO_FFMPEG
#   pragma comment(lib, "avutil.lib")
#   pragma comment(lib, "avcodec.lib")
#   pragma comment(lib, "avfilter.lib")
#   pragma comment(lib, "avformat.lib")
#   pragma comment(lib, "swscale.lib")
#   pragma comment(lib, "avdevice.lib")
#   pragma comment(lib, "swresample.lib")

#endif // G3D_NO_FFMPEG

#   ifdef _DEBUG
#        pragma comment(lib, "G3D-appd")
#   else
#       pragma comment(lib, "G3D-app")
#   endif

#endif
