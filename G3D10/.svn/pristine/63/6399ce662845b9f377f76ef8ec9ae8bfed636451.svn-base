/**
  \file G3D-app.lib/source/GBuffer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/GBuffer.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/UniversalBSDF.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/FileSystem.h"
#include "G3D-app/UniversalSurface.h"
#include "G3D-app/Surface.h"
#include "G3D-gfx/Args.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-app/Camera.h"

namespace G3D {
    
const char* DepthEncoding::toString(int i, Value& v) {
    static const char* str[] = 
       {"HYPERBOLIC",
        "LINEAR",
        "COMPLEMENTARY",
        nullptr};

    static const Value val[] = 
       {HYPERBOLIC,
        LINEAR,
        COMPLEMENTARY};

    const char* s = str[i];
    if (s) {
        v = val[i];
    }
    return s;
}


const char* GBuffer::Field::toString(int i, Value& v) {
    static const char* str[] = 
       {"WS_NORMAL",
        "CS_NORMAL",
        "WS_FACE_NORMAL",
        "CS_FACE_NORMAL",
        "WS_POSITION",
        "CS_POSITION",

        "LAMBERTIAN",
        "GLOSSY",
        "TRANSMISSIVE",
        "EMISSIVE",

        "CS_POSITION_CHANGE",
        "SS_POSITION_CHANGE",

        "CS_Z",

        "DEPTH",
        "TS_NORMAL",

        "SVO_POSITION",

        "FLAGS",

        "SVO_COVARIANCE_MAT1",
        "SVO_COVARIANCE_MAT2",

        "TEXCOORD0",
        
        nullptr};

    static const Value val[] = 
       {WS_NORMAL,
        CS_NORMAL,
        WS_FACE_NORMAL,
        CS_FACE_NORMAL,
        WS_POSITION,
        CS_POSITION,

        LAMBERTIAN,
        GLOSSY,
        TRANSMISSIVE,
        EMISSIVE,

        CS_POSITION_CHANGE,
        SS_POSITION_CHANGE,

        CS_Z,

        DEPTH_AND_STENCIL,
        TS_NORMAL,
    
        SVO_POSITION,

        FLAGS,

        SVO_COVARIANCE_MAT1,
        SVO_COVARIANCE_MAT2,

        TEXCOORD0

        };

    const char* s = str[i];
    if (s) {
        v = val[i];
    }
    return s;
}


shared_ptr<GBuffer> GBuffer::create
  (const Specification& specification,
   const String&        name) {
    
    return createShared<GBuffer>(name, specification);
}


GBuffer::Specification::Specification() {
    depth               = 1;
    depthEncoding       = DepthEncoding::HYPERBOLIC;
    dimension           = Texture::DIM_2D;
    numSamples          = 1;
}


GBuffer::GBuffer(const String& name, const Specification& specification) :
    m_name(name),
    m_specification(specification), 
    m_timeOffset(0),
    m_velocityStartTimeOffset(0),
    m_framebuffer(Framebuffer::create(name)),
    m_readDeclarationString("\n"),
    m_writeDeclarationString("\n"),
    m_allTexturesAllocated(false),
    m_depthOnly(true),
    m_hasFaceNormals(false),
    m_resolution(0,0,0),
    m_useImageStore(false) {

    setSpecification(specification, true);
    m_textureSettings = Sampler::buffer();
}


void GBuffer::setSpecification(const Specification& s) {
    if (s != m_specification) {
        setSpecification(s, false);
    }
}


void GBuffer::setSpecification(const Specification& newSpecification, bool forceAllFieldsToUpdate) {
    Specification oldSpec = m_specification;
    m_specification = newSpecification;

    m_writeDeclarationString    = "\n#extension GL_ARB_separate_shader_objects : require\n";
    m_readDeclarationString     = "\n";
    m_depthOnly                 = true;
    m_hasFaceNormals            = true;
    m_allTexturesAllocated      = false;

    m_framebuffer->uniformTable = UniformTable();

    // Compute the attachment point for each Field and initialize the clear value.
    int a = Framebuffer::COLOR0;
    m_framebuffer->clear();
    for (int f = 0; f < Field::COUNT; ++f) {
        const Texture::Encoding& encoding = m_specification.encoding[f];
        if (isNull(encoding.format)) {
            // Drop the old pointers
            m_fieldToAttachmentPoint[f] = Framebuffer::AttachmentPoint(GL_NONE);

        } else if (f == Field::DEPTH_AND_STENCIL) {

            if ((encoding.format->stencilBits > 0) && (encoding.format->depthBits > 0)) {
                m_fieldToAttachmentPoint[f] = Framebuffer::DEPTH_AND_STENCIL;
            } else if (encoding.format->stencilBits > 0) {
                m_fieldToAttachmentPoint[f] = Framebuffer::STENCIL;
            } else if (encoding.format->depthBits > 0) {
                m_fieldToAttachmentPoint[f] = Framebuffer::DEPTH;
            }
                
            // Make sure that it is safe for a shader to bind both read and write, in which case the write should win regardless of order
            m_writeDeclarationString += "#ifdef DEPTH\n#undef DEPTH\n#endif\n#define DEPTH gl_FragDepth\n";
            m_readDeclarationString  += "#ifndef DEPTH\n#define DEPTH\n#endif\n";

        } else {
            m_depthOnly = false;

            if ((f == Field::CS_FACE_NORMAL) || (f == Field::WS_FACE_NORMAL)) {
                m_hasFaceNormals = true;
            }

            m_fieldToAttachmentPoint[f] = Framebuffer::AttachmentPoint(a);

            const char* fieldName = Field(f).toString();
#           if 1
                m_writeDeclarationString += G3D::format("#ifdef %s\n#undef %s\n#endif\n#define GBUFFER_HAS_%s\nlayout(location = %d) out vec4 %s;\n", fieldName, fieldName, fieldName, a - Framebuffer::COLOR0, fieldName); 
#           else
#               error "SVO path needs to be updated to the latest G3D"
                // SVO path that uses image store instead of output shaders
                m_writeDeclarationString += G3D::format("#ifdef %s\n#undef %s\n#endif\n#define %s %s\nvec4 %s;\n", fieldName, fieldName, fieldName, fieldName, fieldName); 
#           endif

            m_writeDeclarationString += String("uniform vec4 ") + fieldName + "_writeMultiplyFirst;\n";
            m_writeDeclarationString += String("uniform vec4 ") + fieldName + "_writeAddSecond;\n";
  
            m_readDeclarationString += String("uniform vec4 ") + fieldName + "_readExponentFirst;\n";
            m_readDeclarationString += String("uniform vec4 ") + fieldName + "_readMultiplyFirst;\n";
            m_readDeclarationString += String("uniform vec4 ") + fieldName + "_readAddSecond;\n";

            ++a;
        } // if format != nullptr

    } // for each format

    // debugPrintf("%s\n", m_writeDeclarationString.c_str());

    // Set the framebuffer's args
    m_framebuffer->uniformTable.appendToPreamble(writeDeclarations());
    setShaderArgsWrite(m_framebuffer->uniformTable);
}


shared_ptr<Texture> GBuffer::texture(Field f) const {
    const shared_ptr<Framebuffer::Attachment>& a = m_framebuffer->get(m_fieldToAttachmentPoint[f]);
    if (isNull(a)) {
        return nullptr;
    } else {
        return a->texture();
    }
}


void GBuffer::setShaderArgsWrite(UniformTable& args, const String& prefix) const {
    // Set macro args
    for (int f = 0; f < Field::COUNT; ++f) {
        if (notNull(m_specification.encoding[f].format) && (f != Field::DEPTH_AND_STENCIL)) {
            bindWriteUniform(args, Field(f), m_specification.encoding[f], prefix);
        }
    }
}


void GBuffer::setShaderArgsWritePosition
   (CFrame&                 previousFrame,
    const RenderDevice*     rd, 
    Args&                   args) const {

    if (notNull(m_specification.encoding[GBuffer::Field::CS_POSITION_CHANGE].format) ||
        notNull(m_specification.encoding[GBuffer::Field::SS_POSITION_CHANGE].format)) {
        // Previous object-to-camera projection for velocity buffer
        const CFrame& previousObjectToCameraMatrix = m_camera->previousFrame().inverse() * previousFrame;
        args.setUniform("PreviousObjectToCameraMatrix", previousObjectToCameraMatrix);
    }

    if (notNull(m_specification.encoding[GBuffer::Field::SS_POSITION_CHANGE].format)) {
        // Map (-1, 1) normalized device coordinates to actual pixel positions
        const Matrix4& screenMatrix =
            Matrix4(width() / 2.0f, 0.0f, 0.0f, width() / 2.0f,
                    0.0f, height() / 2.0f, 0.0f, height() / 2.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f) * rd->invertYMatrix();
        const Matrix4& M = screenMatrix * rd->projectionMatrix();
        args.setUniform("ProjectToScreenMatrix", M);

        debugAssertM((rd->viewport().width() == width()) && (rd->viewport().height() == height()), 
            "This code assumes that the GBuffer is currently bound as the write framebuffer on the RenderDevice.");
        Matrix4 P;
        m_camera->previousProjection().getProjectUnitMatrix(rd->viewport(), P);
        const Matrix4& N = screenMatrix * P;
        args.setUniform("PreviousProjectToScreenMatrix", N);
    }
}


void GBuffer::bindWriteUniform(UniformTable& args, Field f, const Texture::Encoding& encoding, const String& prefix) {
    // Set the inverse of the read values

    // A crash happens below if I just use prefix + String(f.toString()) in release mode!
    const String& p = (prefix.empty()) ? String(f.toString()) : prefix + String(f.toString()); 
    args.setUniform(p + "_writeMultiplyFirst", Color4::one() / encoding.readMultiplyFirst, true);
    args.setUniform(p + "_writeAddSecond", -encoding.readAddSecond / encoding.readMultiplyFirst, true);
}


void GBuffer::setShaderArgsRead(UniformTable& args, const String& prefix) const {
    for (int f = 0; f < Field::COUNT; ++f) {
        if (m_specification.encoding[f].format != nullptr) {
            bindReadArgs(args, Field(f), texture(Field(f)), prefix);
        }
    }

    if (notNull(m_camera)) {
        m_camera->setShaderArgs(args, m_framebuffer->vector2Bounds(), prefix + "camera_");
    }
}


void GBuffer::bindReadArgs(UniformTable& args, Field field, const shared_ptr<Texture>& texture, const String& prefix) {
    if (notNull(texture)) {
        texture->setShaderArgs(args, prefix + field.toString() + "_", Sampler::buffer());
    }
}


int GBuffer::width() const {
    return m_framebuffer->width();
}


int GBuffer::height() const {
    return m_framebuffer->height();
}


int GBuffer::depth() const {
    return m_specification.depth;   //CC: Why ?
    //return m_framebuffer->depth();
}


Rect2D GBuffer::rect2DBounds() const {
    return m_framebuffer->rect2DBounds();
}


void GBuffer::prepare
   (RenderDevice*               rd,
    float                       timeOffset, 
    float                       velocityStartTimeOffset,
    Vector2int16                depthGuardBandThickness,
    Vector2int16                colorGuardBandThickness) {

    m_depthGuardBandThickness = depthGuardBandThickness;
    m_colorGuardBandThickness = colorGuardBandThickness;

    debugAssertM(m_framebuffer, "Must invoke GBuffer::resize before GBuffer::prepare");
    debugAssertGLOk();

    if (m_resolution.z <= 1) {
        rd->pushState(m_framebuffer); {
            rd->setColorClearValue(Color4::clear());
            rd->clear();
        } rd->popState();
    } else {
        debugAssert(GLCaps::supports("GL_ARB_clear_texture"));
        for (int f = 0; f < Field::COUNT; ++f) {
            if (m_specification.encoding[f].format != nullptr) {
                Framebuffer::AttachmentPoint ap = m_fieldToAttachmentPoint[f];
                shared_ptr<Framebuffer::Attachment> attachment = m_framebuffer->get(ap);
                 
                if (attachment->texture()) {
                    static const float cdata[4] = {0.0f, 0.0f, 0.0f, 0.0f};
                    glClearTexImage(attachment->texture()->openGLID(), 0, GL_RGBA, GL_FLOAT, cdata);
                }
            }
        }
    }

    setTimeOffsets(timeOffset, velocityStartTimeOffset);
    debugAssertGLOk();
}


void GBuffer::prepare
   (RenderDevice*   rd, 
    const shared_ptr<Camera>& camera, 
    float           timeOffset, 
    float           velocityStartTimeOffset,
    Vector2int16    depthGuardBandThickness,
    Vector2int16    colorGuardBandThickness) {

   prepare(rd, timeOffset, velocityStartTimeOffset, depthGuardBandThickness, colorGuardBandThickness);

   setCamera(camera);
}


void GBuffer::resize(int w, int h, int d) {
    debugAssert(w >= 0 && h >= 0 && d >= 0);

    if ((w == width()) && (h == height()) && (d == depth())) {
        // Already allocated
        return;
    }

    m_specification.depth = d;      //CC

    if (m_allTexturesAllocated) {
        // Resize
        for (int f = 0; f < Field::COUNT; ++f) {
            if (m_specification.encoding[f].format != nullptr) {
                Framebuffer::AttachmentPoint ap = m_fieldToAttachmentPoint[f];
                shared_ptr<Framebuffer::Attachment> attachment = m_framebuffer->get(ap);
                 
                if (GLCaps::supportsTexture(m_specification.encoding[f].format)) {
                    attachment->texture()->resize(w, h, d);
                } else {
                    alwaysAssertM(false, "Invalid Texture format for GBuffer");
                }
            }
        }
    } else {
        // Allocate
        m_allTexturesAllocated = true;
        for (int f = 0; f < Field::COUNT; ++f) {
            const ImageFormat* fmt = m_specification.encoding[f].format;
            const Framebuffer::AttachmentPoint ap = m_fieldToAttachmentPoint[f];
            const String& n = format("%s/%s", m_name.c_str(), Field(f).toString());
            const Texture::Dimension dimension = m_specification.dimension;
            bool genMipMaps = m_specification.genMipMaps;

            if (notNull(fmt)) {
                if (GLCaps::supportsTexture(fmt)) {
                    const shared_ptr<Texture>& texture = Texture::createEmpty(n, w, h, m_specification.encoding[f], dimension, genMipMaps, d, m_specification.numSamples);
                    m_framebuffer->set(ap, texture);

                    if (f == Field::SS_POSITION_CHANGE) {
                        if (m_specification.encoding[f].readMultiplyFirst.r > 1.0f) {
                            // This has been packed to a small scale.
                            texture->visualization.max = 1.0f;
                        } else {
                            texture->visualization.max = 20.0f;
                        }
                        if (m_specification.encoding[f].readAddSecond.r > 0.0f) {
                            // Biased to begin at zero
                            texture->visualization.min = 0.0f;
                        } else {
                            texture->visualization.min = -texture->visualization.max;
                        }
                        texture->visualization.showMotionVectors = true;
                    } else if (f == Field::CS_POSITION_CHANGE) {
                        texture->visualization.max = 1.0f;
                        texture->visualization.min = -texture->visualization.max;
                    } else if (f == Field::DEPTH_AND_STENCIL) {
                        texture->visualization = Texture::Visualization::depthBuffer();
                    } else if (f == Field::TEXCOORD0) {
                        texture->visualization = Texture::Visualization::textureCoordinates();
                    }
                } else {
                    alwaysAssertM(false, "Invalid Texture format for GBuffer");
                }
            }
        }
    }

    m_resolution = Vector3int16(w, h, d);
}

String& GBuffer::getShaderString(const String& gbufferName, Args& args, Access access, bool& needCreation) {
    if (access == Access::READ) {

        if( !m_readShaderStringCache.containsKey(gbufferName) ){
            needCreation = true;
        }

        return m_readShaderStringCache.getCreate(gbufferName);
        
    } else if( access==Access::WRITE ){

        if( !m_writeShaderStringCache.containsKey(gbufferName) ){
            needCreation = true;
        }

        return m_writeShaderStringCache.getCreate(gbufferName);
    } else {

        if( !m_readwriteShaderStringCache.containsKey(gbufferName) ){
            needCreation = true;
        }

        return m_readwriteShaderStringCache.getCreate(gbufferName);
    }
}
 

String GBuffer::getImageString(const Specification &spec, const ImageFormat* format){
    String res;

    if (spec.numSamples == 1) {
        if (spec.dimension == Texture::DIM_2D) {
            res = "image2D";
        }
        else if (spec.dimension == Texture::DIM_3D) {
            res = "image3D";
        }
        else if (spec.dimension == Texture::DIM_2D_RECT) {
            res = "image2DRect";
        }
        else if (spec.dimension == Texture::DIM_CUBE_MAP) {
            res = "imageCube";
        }
        else {
            alwaysAssertM(false, "Unrecognised dimension");
        }
    }
    else {
        if (spec.dimension == Texture::DIM_2D) {
            res = String("image2DMS");
        }
        else {
            alwaysAssertM(false, "Unrecognised dimension");
        }
    }

    if (format->isIntegerFormat()) {
        res = "i" + res;
    }

    return res;
}

String GBuffer::getSamplerStringFromTexDimension(const Specification& spec){

    if (spec.numSamples == 1) {
        if (spec.dimension == Texture::DIM_2D) {
            return "sampler2D";
        }
        else if (spec.dimension == Texture::DIM_3D) {
            return "sampler3D";
        }
        else if (spec.dimension == Texture::DIM_2D_RECT) {
            return "sampler2DRect";
        }
        else if (spec.dimension == Texture::DIM_CUBE_MAP) {
            return "samplerCube";
        }
        else {
            alwaysAssertM(false, "Unrecognised dimension");
            return "unknown";
        }
    }
    else {
        if (spec.dimension == Texture::DIM_2D) {
            return "sampler2DMS";
        }
        else {
            alwaysAssertM(false, "Unrecognised dimension");
            return "unknown";
        }
    }
}

String GBuffer::getSwizzleComponents(int numComponents) {
    String components("xyzw");
    if (numComponents > 0 && numComponents < (int)components.length()) {
        return components.substr(0, numComponents);
    }
    return components;
}

int GBuffer::getTexDimensionInt(Texture::Dimension dim){

    if (dim == Texture::DIM_2D || dim == Texture::DIM_2D_RECT || dim == Texture::DIM_CUBE_MAP) {
        return 2;
    }
    else if (dim == Texture::DIM_3D) {
        return 3;
    }

    alwaysAssertM(false, "Unrecognized dimension");
    return 0;

}

void GBuffer::connectToShader(String gbufferName, Args& args, Access access, const Sampler& textureSettings, int mipLevel) {
    bool needCreation = false;

    m_textureSettings = textureSettings;


    String& gbufferDeclarations = getShaderString(gbufferName, args, access, needCreation);

    if (needCreation) {
    
        gbufferDeclarations = "\n";

        ///////MACROS//////

        gbufferDeclarations += G3D::format("#define GBUFFER_CONNECTED 1\n");


        gbufferDeclarations += G3D::format("#define GBUFFER_%s 1\n", gbufferName.c_str() );

        if (m_useImageStore) {
            //args.setMacro(String("GBUFFER_USE_IMAGE_STORE"), 1);
            gbufferDeclarations += G3D::format("#define GBUFFER_USE_IMAGE_STORE 1\n" );
        }

        gbufferDeclarations += G3D::format("#define GBUFFER_USE_IMAGE_STORE_%s %d\n", gbufferName.c_str(), int(m_useImageStore) );
        gbufferDeclarations += G3D::format("#define GBUFFER_DIMENSION_%s %d\n", gbufferName.c_str(), getTexDimensionInt(m_specification.dimension) );

        // Read/write enables
        if ((access == Access::WRITE) || (access == Access::READ_WRITE)) {
            //args.setMacro(String("GBUFFER_WRITE_ENABLED_")+gbufferName , 1);
            gbufferDeclarations += G3D::format("#define GBUFFER_WRITE_ENABLED_%s 1\n", gbufferName.c_str() );
        }

        if ((access == Access::READ) || (access == Access::READ_WRITE)) {
            //args.setMacro(String("GBUFFER_READ_ENABLED_")+gbufferName , 1);
            gbufferDeclarations += G3D::format("#define GBUFFER_READ_ENABLED_%s 1\n", gbufferName.c_str() );
        }

        int a = Framebuffer::COLOR0;
        for (int f = 0; f < Field::COUNT; ++f) {
            const ImageFormat* format = m_specification.encoding[f].format;

            if (format != nullptr) {
             
                Texture::Encoding encoding = m_specification.encoding[f];

                String unameBase = gbufferName + String("_") + String(Field(f).toString());

                String typeString;
                if (format->numComponents > 1) {
                    typeString = G3D::format("vec%d", format->numComponents);
                } else {
                    typeString = "float";
                }

                //args.setMacro( G3D::format("GBUFFER_TYPE_%s_%s", gbufferName.c_str(), Field(f).toString() ), typeString );
                gbufferDeclarations += G3D::format("#define GBUFFER_TYPE_%s_%s %s\n", gbufferName.c_str(), Field(f).toString(), typeString.c_str() );


                if ((access == Access::WRITE) || (access == Access::READ_WRITE)) {
                    //args.setMacro(String(Field(f).toString()), String(Field(f).toString()));  //Needed ??
                    gbufferDeclarations += G3D::format("#define %s %s\n", Field(f).toString(), Field(f).toString() );

                    if (! m_useImageStore && (f != Field::DEPTH_AND_STENCIL)) {
                        gbufferDeclarations += G3D::format("#define GBUFFER_OUTPUT_SLOT_%s_%s %d\n", gbufferName.c_str(), Field(f).toString(), (a - Framebuffer::COLOR0) );
                        ++a;
                    }
                }

#if 0
                //New : cast bindless image into the right image type (NV bug)
                gbufferDeclarations += G3D::format("#define GBUFFER_IMAGE_REF_%s_%s  (%s( %s_%s_image ))\n", //unpackUint2x32
                                                    gbufferName.c_str(), Field(f).toString(),
                                                    getImageString(m_specification, format).c_str(), gbufferName.c_str(), Field(f).toString());

#endif
                //args.setMacro(String("GBUFFER_CHANNEL_")+gbufferName+ String("_") + String(Field(f).toString()) , 1);
                gbufferDeclarations += G3D::format("#define GBUFFER_CHANNEL_%s_%s 1\n", gbufferName.c_str(), Field(f).toString() );

                //SWIZZLE
                //args.setMacro(String("GBUFFER_COMPONENTS_")+gbufferName+ String("_") + String(Field(f).toString()) , getSwizzleComponents(format->numComponents) );
                gbufferDeclarations += G3D::format("#define GBUFFER_COMPONENTS_%s_%s %s\n", 
                    gbufferName.c_str(), Field(f).toString(), getSwizzleComponents(format->numComponents).c_str() );
            }
        }

        gbufferDeclarations += String("\n");

        gbufferDeclarations += G3D::format("#define GBUFFER_FIELDS_DECLARATIONS_%s ", gbufferName.c_str());
        gbufferDeclarations += G3D::format("const int GBUFFER_WIDTH_%s = %d; ", gbufferName.c_str(), width() );
        gbufferDeclarations += G3D::format("const int GBUFFER_WIDTH_MASK_%s = %d; ", gbufferName.c_str(), width()-1 );
        gbufferDeclarations += G3D::format("const int GBUFFER_WIDTH_SHIFT_%s = %d; ", gbufferName.c_str(), iRound(log2(width())) );
        gbufferDeclarations += G3D::format("const int GBUFFER_HEIGHT_%s = %d; ", gbufferName.c_str(), height() );
        gbufferDeclarations += G3D::format("const int GBUFFER_HEIGHT_MASK_%s = %d; ", gbufferName.c_str(), height()-1);
        gbufferDeclarations += G3D::format("const int GBUFFER_WIDTH_HEIGHT_SHIFT_%s = %d; ", gbufferName.c_str(), iRound(log2(width()*height())) );

        for (int f = 0; f < Field::COUNT; ++f) {
            const ImageFormat* format = m_specification.encoding[f].format;

            if ( (format != nullptr) ) {
             
                Texture::Encoding encoding = m_specification.encoding[f];

                String unameBase = gbufferName + String("_") + String(Field(f).toString());

                //Global variable//
                {
                    String typeString;
                    if(format->numComponents>1)
                        typeString = G3D::format("vec%d", format->numComponents);
                    else
                        typeString = "float";

                    gbufferDeclarations +=  G3D::format( "%s %s; ", typeString.c_str(), unameBase.c_str() );
                
                    //Define type for the field
                    //args.setMacro( G3D::format("GBUFFER_TYPE_%s_%s", gbufferName.c_str(), Field(f).toString() ), typeString );
                }


                //Always define images (read and write)
                //if(m_useImageStore)
                if (m_useImageStore && (access == Access::WRITE || access == Access::READ_WRITE) )
                {
                    //layout (bindless_sampler)
#if 1
                    gbufferDeclarations += G3D::format( "layout(%s) uniform %s %s_image; ", 
                            format->name().c_str(), getImageString(m_specification, format).c_str(), unameBase.c_str() );
#else
                    gbufferDeclarations += G3D::format("uniform uint64_t %s_image; ", unameBase.c_str()); 
                    
#endif

                        //String("layout(") + format->imageLayoutName() + String(")") + String(" uniform ")+
                        //getImageString(m_specification, format) + String(" ") + uname + String("; ");
                }

                if(access==Access::WRITE || access==Access::READ_WRITE){
                    
                    //Scale bias
                    gbufferDeclarations += G3D::format( "uniform vec2 %s_writeScaleBias; ", unameBase.c_str() );
                

                    //Define attribute names
                    //args.setMacro(String(Field(f).toString()), String(Field(f).toString()));  //Needed ??
                }

                if(access==Access::READ || access==Access::READ_WRITE){

                    //Scale bias
                    gbufferDeclarations += G3D::format( "uniform vec2 %s_readScaleBias; ", unameBase.c_str() );
                

                    gbufferDeclarations += G3D::format( "uniform %s %s_tex; ",
                                getSamplerStringFromTexDimension(m_specification).c_str(), unameBase.c_str() );
                
                }
                
            }
        }

        ///WRITE GLOBAL VARS FUNC///
        if(access==Access::WRITE || access==Access::READ_WRITE){

            //WRITE
            gbufferDeclarations += G3D::format("void gbufferWriteGlobalVars_%s(ivec3 coords){ ", gbufferName.c_str() );

            int a = Framebuffer::COLOR0;
            for (int f = 0; f < Field::COUNT; ++f) {
                const ImageFormat* format = m_specification.encoding[f].format;

                if ( (format != nullptr) ) {
                    if(m_useImageStore) {

#if 1
                        gbufferDeclarations += G3D::format(
                            "imageStore( GBUFFER_IMAGE(%s, %s), GBUFFER_COORDS(%s, coords), GBUFFER_VALUE_WRITE( GBUFFER_GLOBAL_VAR(%s, %s) ) ); ", 
                            gbufferName.c_str(), Field(f).toString(), 
                            gbufferName.c_str(), 
                            gbufferName.c_str(), Field(f).toString() );
#else
                        
                        gbufferDeclarations += G3D::format("uint64_t temp%s; uvec2 tempHdl%s = unpackUint2x32( temp%s ); ",
                                                    Field(f).toString(), Field(f).toString(),
                                                    Field(f).toString());

                        gbufferDeclarations += G3D::format("%s tempImage%s = (%s)( tempHdl%s ); ", 
                                                    getImageString(m_specification, format).c_str(), Field(f).toString(),
                                                    getImageString(m_specification, format).c_str(), Field(f).toString()    );

                        /*gbufferDeclarations += G3D::format(
                            "imageStore( tempImage%s, GBUFFER_COORDS(%s, coords), GBUFFER_VALUE_WRITE( GBUFFER_GLOBAL_VAR(%s, %s) ) ); ",
                            Field(f).toString(),
                            gbufferName.c_str(),
                            gbufferName.c_str(), Field(f).toString()); */


                        /*gbufferDeclarations += G3D::format(
                            "imageStore( %s(%s_%s_image), GBUFFER_COORDS(%s, coords), GBUFFER_VALUE_WRITE( GBUFFER_GLOBAL_VAR(%s, %s) ) ); ",
                            getImageString(m_specification, format).c_str(), gbufferName.c_str(), Field(f).toString(),
                            gbufferName.c_str(),
                            gbufferName.c_str(), Field(f).toString());*/


#endif

                    }else{

                        //Just assign gbuffer global variable to output variable
                        ///gbufferDeclarations += String(Field(f).toString()) + String(" = GBUFFER_GLOBAL_VAR(") + gbufferName+String(", ") + String(Field(f).toString()) +  String("); ");
                        if (f == Field::DEPTH_AND_STENCIL) {
                            /*gbufferDeclarations += G3D::format("gl_FragDepth = GBUFFER_GLOBAL_VAR(%s, %s) * GBUFFER_WRITE_SCALEBIAS(%s, %s).x + GBUFFER_WRITE_SCALEBIAS(%s, %s).y; ", 
                                                        gbufferName.c_str(), Field(f).toString(),
                                                        gbufferName.c_str(), Field(f).toString(), 
                                                        gbufferName.c_str(), Field(f).toString() 
                                                    );*/
                        } else {
                            gbufferDeclarations += G3D::format("gl_FragData[%d].%s = GBUFFER_GLOBAL_VAR(%s, %s) * GBUFFER_WRITE_SCALEBIAS(%s, %s).x + GBUFFER_WRITE_SCALEBIAS(%s, %s).y; ", 
                                                                    a - Framebuffer::COLOR0,
                                                                    getSwizzleComponents(format->numComponents).c_str(),
                                                                    gbufferName.c_str(), Field(f).toString(),
                                                                    gbufferName.c_str(), Field(f).toString(), 
                                                                    gbufferName.c_str(), Field(f).toString() 
                                                                );

                            ++a;
                        }
                    }
                }
            }
            gbufferDeclarations += String("} ");
        }
    
        ///LOAD GLOBAL VARS///
        if(access==Access::READ || access==Access::READ_WRITE){
            gbufferDeclarations += G3D::format("void gbufferLoadGlobalVars_%s(ivec3 coords, int sampleID=0){ ", gbufferName.c_str() );
            
            for (int f = 0; f < Field::COUNT; ++f) {
                const ImageFormat* format = m_specification.encoding[f].format;

                if ( (format != nullptr) ) {
                    
                    gbufferDeclarations += G3D::format(
                        "GBUFFER_GLOBAL_VAR(%s, %s) = texelFetch( GBUFFER_TEX(%s, %s), GBUFFER_COORDS(%s, coords), sampleID).GBUFFER_COMPONENTS(%s, %s); ", 
                        gbufferName.c_str(), Field(f).toString(),
                        gbufferName.c_str(), Field(f).toString(),
                        gbufferName.c_str(),
                        gbufferName.c_str(), Field(f).toString() );
                    
                }
            }

            
            gbufferDeclarations += String("} ");
        }

        ///////////////////////////
        gbufferDeclarations += String("\n");
        /////////////////////////// 

        //Coordinates//
        gbufferDeclarations += G3D::format("#define GBUFFER_COORDS_%s(coords) ", gbufferName.c_str() );
        if(m_specification.dimension == Texture::DIM_2D){
            gbufferDeclarations += String(" gbufferCoordsHelper(##coords##).xy");
        }else if(m_specification.dimension == Texture::DIM_3D){
            gbufferDeclarations += String(" gbufferCoordsHelper(##coords##).xyz");
        }else{
            alwaysAssertM(false, "Dimension unsuported");
        }
        gbufferDeclarations += String("\n");

        //alwaysAssertM(format->numComponents != 3, "SVO requires that all fields have 1, 2, or 4 components due to restrictions from OpenGL glBindImageTexture");


        ///////////GENERIC DECLARATIONS//////////////

        if(access==Access::WRITE || access==Access::READ_WRITE){
            //STORE SRC_DST
            gbufferDeclarations += String("#define GBUFFER_STORE_VARS_3D_") + gbufferName + String("(srcGbufferName, coords) ");
            //GBUFFER_STORE_VARS_3D_##dstGbufferName(srcGbufferName, coords)

            for (int f = 0; f < Field::COUNT; ++f) {
                const ImageFormat* format = m_specification.encoding[f].format;

                if ( (format != nullptr) ) {
                    
                    gbufferDeclarations += String("imageStore( GBUFFER_IMAGE(")+gbufferName+String(", ") + String(Field(f).toString()) + String("), GBUFFER_COORDS(")+
                            gbufferName + String(", coords), GBUFFER_VALUE_WRITE( GBUFFER_GLOBAL_VAR(")
                            + String(" srcGbufferName ") + String(", ") + String(Field(f).toString()) + String("))); ");
                    
                }
            }
            gbufferDeclarations += String("\n");
        }

    

    }

    /////////////////////////

    args.appendToPreamble(gbufferDeclarations);

    /////////////////////////
    //Set uniforms//
     
    for (int f = 0; f < Field::COUNT; ++f) {
        const ImageFormat* format = m_specification.encoding[f].format;

            
        if ( (format != nullptr) ) {
            Texture::Encoding encoding = m_specification.encoding[f];

            if(m_useImageStore)
            {
                String uname = gbufferName + String("_") + String(Field(f).toString()) + String("_image");

                args.setImageUniform(uname, texture(Field(f)), access, mipLevel, true);
            }

            if(access==Access::WRITE || access==Access::READ_WRITE){
                String uname = gbufferName + String("_") + String(Field(f).toString()) + String("_writeScaleBias");
                args.setUniform(uname, Vector2(
                                            //Color4( 1.0f / encoding.readMultiplyFirst.r, 1.0f / encoding.readMultiplyFirst.g, 1.0f / encoding.readMultiplyFirst.b, 1.0f / encoding.readMultiplyFirst.a), 
                                            //-encoding.readAddSecond / encoding.readMultiplyFirst
                                            1.0f / encoding.readMultiplyFirst.r,
                                            -encoding.readAddSecond.r / encoding.readMultiplyFirst.r
                                            ), true);
            }

            if(access==Access::READ || access==Access::READ_WRITE){
                {
                    String uname = gbufferName + String("_") + String(Field(f).toString()) + String("_readScaleBias");
                    
                    ///args.setUniform(uname, (Vector2)encoding, true);
                    args.setUniform(uname, Vector2(encoding.readMultiplyFirst.r, encoding.readAddSecond.r) , true); //TODO: convert to vec4
                }

                {
                    //Sampler::Settings settings = Sampler::Settings::buffer();
                    //settings.interpolateMode = Sampler::InterpolateMode( (int)texture(Field(f))->settings().interpolateMode );
                    String uname = gbufferName + String("_") + String(Field(f).toString()) + String("_tex");
                    args.setUniform( uname, texture(Field(f)), m_textureSettings );
                }
            }

        }
    }

    //TODO: clean
    const Rect2D& colorRect = this->colorRect();
    args.setUniform("lowerCoord", colorRect.x0y0());
    args.setUniform("upperCoord", colorRect.x1y1());

    //std::cout<<genericDeclarations<<"\n==============================\n";
    //std::cout<<gbufferDeclarations<<"\n==============================\n";
}


} // namespace G3D
