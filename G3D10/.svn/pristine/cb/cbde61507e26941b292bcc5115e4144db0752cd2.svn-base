#include <G3D/G3D.h>

// OpenGL ES
#define USE_ES 1

// Shader Storage Buffer Object (instead of image2d for output)
#define USE_SSBO 1

G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {

    // Minimal OpenGL initialization (you can use compute shaders in full blown G3D::GApps, too!)
    initGLG3D(G3DSpecification());
    GApp::Settings settings(argc, argv);
    settings.window.caption         = argv[0];
    settings.window.width           = 1280;
    settings.window.height          = 720;

#   if USE_ES
    settings.window.api             = OSWindow::Settings::API_OPENGL_ES;
    settings.window.majorGLVersion  = 3;
    settings.window.minorGLVersion  = 1;
#   endif

    OSWindow* window = OSWindow::create(settings.window);

    RenderDevice* renderDevice = new RenderDevice();
    renderDevice->init(window);

    /////////////////////////////////////////////////////////////////////////////// 
    // Allocate texture inputs and outputs
    shared_ptr<Texture> inputTexture;

    // Put some data in the inputTexture here
    {
        const shared_ptr<PixelTransferBuffer>& src = CPUPixelTransferBuffer::create(512, 512, ImageFormat::RGBA32F());
        Vector4* ptr = (Vector4*)src->mapWrite();
        Random& rng = Random::threadCommon();
        for (int y = 0; y < src->height(); ++y) {
            for (int x = 0; x < src->width(); ++x, ++ptr) {
                *ptr = Vector4(rng.uniform(), rng.uniform(), rng.uniform(), rng.uniform());
            }
        }
        src->unmap();
        inputTexture = Texture::fromPixelTransferBuffer("inputTexture", src);
    }

    const shared_ptr<Texture>& outputTexture = Texture::createEmpty("output", inputTexture->width() * 2, inputTexture->height(), ImageFormat::RGBA32F());
#   if USE_SSBO
        const shared_ptr<GLPixelTransferBuffer>& outputBuffer = GLPixelTransferBuffer::create(inputTexture->width() * 2, inputTexture->height(), ImageFormat::RGBA32F());
#   else 
#       if USE_ES
            // GL ES textures must be immutable-format to work with setImageUniform.
            glBindTexture(outputTexture->openGLTextureTarget(), outputTexture->openGLID());
            debugAssertGLOk();
            glTexStorage2D(outputTexture->openGLTextureTarget(), 1, outputTexture->format()->openGLFormat, outputTexture->width(), outputTexture->height());
            debugAssertGLOk();
            glBindTexture(outputTexture->openGLTextureTarget(), GL_NONE);
#       endif
#   endif

    /////////////////////////////////////////////////////////////////////////////// 
    // Invoke the shader
    Args args;
    args.setUniform("inputTexture", inputTexture, Sampler::buffer());
    args.setMacro("USE_SSBO", USE_SSBO);

#   if USE_SSBO
        args.setUniform("ssboWidth", outputBuffer->width());
        outputBuffer->bindAsShaderStorageBuffer(0);
#   else
        // Example of creating an output using an image2d uniform. This works
        // on GL ES 3.1 and OpenGL 3.3, so it works on Raspberry Pi and macOS.
        // If you don't need those platforms, you can use a Shader Storage Buffer Object
        // for streamlined access.
        args.setImageUniform("outputTexture", outputTexture, Access::WRITE);
#   endif

    const int w = inputTexture->width();
    const int h = inputTexture->height();

    // Choose whatever group dimensions you wish. We're assuming that
    // 8x4 happens to map to 32 wide groups that are efficient on most GPUs,
    // but you can run them as small as 1x1 if you wish and a good driver
    // should still schedule the work efficiently.
    const Vector3int32 groupSize(8, 4, 1);
    alwaysAssertM(w % groupSize.x == 0, "Width must be a multiple of 8");
    alwaysAssertM(h % groupSize.y == 0, "Height must be a multiple of 4");

    // Lanes per workground. Do not set this if using an explicit group size
    // in the shader, which is required by older/less powerful platform
    //
    // args.setComputeGroupSize(groupSize);

    // Number of workgroups
    args.setComputeGridDim(Vector3int32(w / groupSize.x, h / groupSize.y, 1));

    // Run the shader
    LAUNCH_SHADER("quadratic.glc", args);

#   if USE_SSBO
    {
        // Copy back to a texture, if you need to use it for another graphics operation.
        // You can also just memory map the PixelTransferBuffer to read it directly from
        // C++.
        outputTexture->update(outputBuffer);
    }
#   endif

    return 0;
}



