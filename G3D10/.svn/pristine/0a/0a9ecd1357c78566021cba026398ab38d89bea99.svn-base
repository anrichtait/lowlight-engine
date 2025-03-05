/**
  \file tools/viewer/EmptyViewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "EmptyViewer.h"

EmptyViewer::EmptyViewer(){}


void EmptyViewer::onInit(const G3D::String& filename, App* app) {}


void EmptyViewer::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) { 
    screenPrintf("G3D Asset Viewer");
    screenPrintf("\n");
    screenPrintf("\n");
    screenPrintf("Built " __TIME__ " " __DATE__);
    screenPrintf("%s", System::version().c_str());
    screenPrintf("https://casual-effects.com/g3d");
    screenPrintf("\n");
    screenPrintf("\n");
    screenPrintf("Drag and drop a media file here to view it.\n");
    screenPrintf("\n");
    screenPrintf("Image Formats:    png, jpg, bmp, tga, pcx, dds, psd, cut, exr, hdr, iff, mng, tiff, xbm,");
    screenPrintf("                  xpm, pfm, pict, jbig, ppm, ico, gif, (+ cube maps...just drop one face)");
    screenPrintf("\n");
    screenPrintf("3D Formats:       obj, fbx, gltf, dae, 3ds, pk3, md2, md3, bsp, off, ply, ply2, ifs,");
    screenPrintf("                  stl, stla, points, xyz, vox, ArticulatedModel.Any, Scene.Any");
    screenPrintf("\n");
    screenPrintf("Material Formats: UniversalMaterial.Any");
    screenPrintf("\n");
    screenPrintf("GUI Formats:      fnt, gtm");
#   ifndef G3D_NO_FFMPEG
    screenPrintf("Video Formats:    mp4, avi, mp4, mov, ogg, asf, wmv");
#   endif/*
    screenPrintf("Scene Formats:    ")*/
    screenPrintf("\n");
    screenPrintf("Press 'V' to view G3D::OSWindow events");
}
