#include "G3D/G3D.h"

#include "main.h"
#include "gtest/gtest.h"

uint16_t expectedTriangleCount = 15704;
G3D::Vector3 expectedTranslation(0.0, 0.0, 0.0);
G3D::Point3 expectedGLTFCenter(5.20073, 0, -39.5033);
G3D::Point3 expectedFBXCenter(5.20073, 39.5033, 0);
G3D::Matrix3 expectedGLTFRotation(1, 0, 0, 0, 0, -1, 0, 1, 0);
G3D::Matrix3 expectedFBXRotation(1, 0, 0, 0, 1, 0, 0, 0, 1);

namespace G3D {
TEST(ArticulatedModelLoadingFromOtherSpecs, loadAssimpGLTF) {

/* Windowless rendering only supported on Linux */
    GApp::Settings settings;
    settings.window.api = OSWindow::Settings::API_OPENGL_EGL;

    OSWindow* window = OSWindow::create(settings.window);

    RenderDevice* renderDevice = new RenderDevice();
    renderDevice->init(window);

    String asset = System::findDataFile("$g3d/data10/common/model/teapot/teapot.gltf");
    shared_ptr<ArticulatedModel> model = ArticulatedModel::fromFile(asset);
    const Array<ArticulatedModel::Part*> partArray = model->rootArray();
    const Array<ArticulatedModel::Mesh*> meshArray = model->meshArray();

    EXPECT_EQ(partArray.size(), 1);
    EXPECT_EQ(meshArray.size(), 1);
    
    const ArticulatedModel::Part* part = partArray[0];
    const ArticulatedModel::Mesh* mesh = meshArray[0];
    Point3 translation = part->cframe.translation;
    Matrix3 rotation = part->cframe.rotation;

    EXPECT_EQ(translation, expectedTranslation);
    for (unsigned int i = 0; i < 3; i++) {
        EXPECT_NEAR(rotation[0][i], expectedGLTFRotation[0][i], 0.000001);
        EXPECT_NEAR(rotation[1][i], expectedGLTFRotation[1][i], 0.000001);
        EXPECT_NEAR(rotation[2][i], expectedGLTFRotation[2][i], 0.000001);
    }

    EXPECT_EQ(mesh->triangleCount(), expectedTriangleCount);
    EXPECT_FALSE(mesh->hasBones());
    EXPECT_NEAR(mesh->sphereBounds.radius, 99.000038, 0.000001);
    EXPECT_NEAR(mesh->sphereBounds.center.x, expectedGLTFCenter.x, 0.0001);
    EXPECT_NEAR(mesh->sphereBounds.center.y, expectedGLTFCenter.y, 0.0001);
    EXPECT_NEAR(mesh->sphereBounds.center.z, expectedGLTFCenter.z, 0.0001);
    EXPECT_EQ(mesh->sphereBounds.center, mesh->boxBounds.center());

    if (renderDevice) {
        renderDevice->cleanup();
        delete renderDevice;
        delete window;
        renderDevice = NULL;
        window = NULL;
    }
}

TEST(ArticulatedModelLoadingFromOtherSpecs, loadAssimpFBX) {

/* Windowless rendering only supported on Linux */
    GApp::Settings settings;
    settings.window.api = OSWindow::Settings::API_OPENGL_EGL;

    OSWindow* window = OSWindow::create(settings.window);

    RenderDevice* renderDevice = new RenderDevice();
    renderDevice->init(window);

    String asset = System::findDataFile("$g3d/data10/common/model/teapot/teapot.fbx");
    shared_ptr<ArticulatedModel> model = ArticulatedModel::fromFile(asset);
    const Array<ArticulatedModel::Part*> partArray = model->rootArray();
    const Array<ArticulatedModel::Mesh*> meshArray = model->meshArray();

    EXPECT_EQ(partArray.size(), 1);
    EXPECT_EQ(meshArray.size(), 1);

    const ArticulatedModel::Part* part = partArray[0];
    const ArticulatedModel::Mesh* mesh = meshArray[0];
    Point3 translation = part->cframe.translation;
    Matrix3 rotation = part->cframe.rotation;

    EXPECT_EQ(translation, expectedTranslation);
    for (unsigned int i = 0; i < 3; i++) {
        EXPECT_NEAR(rotation[0][i], expectedFBXRotation[0][i], 0.000001);
        EXPECT_NEAR(rotation[1][i], expectedFBXRotation[1][i], 0.000001);
        EXPECT_NEAR(rotation[2][i], expectedFBXRotation[2][i], 0.000001);
    }

    EXPECT_EQ(mesh->triangleCount(), expectedTriangleCount);
    EXPECT_FALSE(mesh->hasBones());
    EXPECT_NEAR(mesh->sphereBounds.radius, 99.000038, 0.000001);
    EXPECT_NEAR(mesh->sphereBounds.center.x, expectedFBXCenter.x, 0.0001);
    EXPECT_NEAR(mesh->sphereBounds.center.y, expectedFBXCenter.y, 0.0001);
    EXPECT_NEAR(mesh->sphereBounds.center.z, expectedFBXCenter.z, 0.0001);
    EXPECT_EQ(mesh->sphereBounds.center, mesh->boxBounds.center());

    if (renderDevice) {
        renderDevice->cleanup();
        delete renderDevice;
        delete window;
        renderDevice = NULL;
        window = NULL;
    }
}
} // namespace G3D
