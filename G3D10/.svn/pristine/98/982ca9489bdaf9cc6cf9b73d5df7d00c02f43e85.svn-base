/**
  \file G3D-app.lib/source/PointModel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/PointModel.h"
#include "G3D-app/PointSurface.h"
#include "G3D-app/Entity.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/ParseVOX.h"
#include "G3D-base/Ray.h"

namespace G3D {
shared_ptr<PointModel> PointModel::create(const String& name, const Specification& spec) {
    const shared_ptr<PointModel> ps(createShared<PointModel>(name));
    ps->load(spec);
    return ps;
}


Any PointModel::Specification::toAny() const {
    Any a(Any::TABLE, "ArticulatedModel::Specification");
    a["filename"]                  = filename;
    a["scale"]                     = scale;
    a["renderAsDisk"]              = renderAsDisk;
    return a;
}

PointModel::Specification::Specification(const Any& a) {
    *this = Specification();

    if (a.type() == Any::STRING) {
        try {
            filename = a.resolveStringAsFilename();
        } catch (const FileNotFound& e) {
            throw ParseError(a.source().filename, a.source().line, e.message);
        }

    } else {

        AnyTableReader r(a);
        Any f;
        if (! r.getIfPresent("filename", f)) {
            a.verify(false, "Expected a filename field in PointModel::Specification");
        }
        f.verifyType(Any::STRING);
        try {
            filename = f.resolveStringAsFilename();   
        } catch (const FileNotFound& e) {
            throw ParseError(f.source().filename, f.source().line, e.message);
        }

        r.getIfPresent("scale",                     scale);
        r.getIfPresent("renderAsDisk",              renderAsDisk);


        r.verifyDone();
    }
}

lazy_ptr<Model> PointModel::lazyCreate(const String& name, const Any& a) {
    return lazyCreate(a, name);
}


lazy_ptr<Model> PointModel::lazyCreate(const PointModel::Specification& specification, const String& name) {
    return lazy_ptr<Model>([specification, name]{ return PointModel::create(name, specification); }); //the order should be switched to match Articulated Model
}


void PointModel::copyToGPU() {
    for (const shared_ptr<PointArray>& pointArray : m_pointArrayArray) {
        pointArray->copyToGPU();
    }
}


void PointModel::PointArray::copyToGPU() {
    alwaysAssertM(cpuRadiance.size() == cpuPosition.size(), "Attribute arrays must be the same size");
    const int64 N = size();

    // Throw away old values to free GPU memory
    gpuPosition = AttributeArray();
    gpuRadiance = AttributeArray();

    const shared_ptr<VertexBuffer>& buffer = VertexBuffer::create(N * (sizeof(Point3) + sizeof(Color4unorm8)) + 16, VertexBuffer::WRITE_ONCE);
       
    gpuPosition = AttributeArray(cpuPosition, buffer);
    gpuRadiance = AttributeArray(cpuRadiance, buffer);
}


void PointModel::load(const Specification& spec) {

    const String& resolvedFilename = FileSystem::resolve(spec.filename);
    const String& cacheFilename = makeCacheFilename(resolvedFilename);

    bool useCache = FileSystem::exists(cacheFilename) && ! FileSystem::isNewer(resolvedFilename, cacheFilename);

    if (useCache) {
        useCache = loadCache(cacheFilename);
    }

    m_renderAsDisk = spec.renderAsDisk;
    m_numPoints = 0;

    // The cache may have failed, so check again
    if (! useCache) {
        // Load into a single array, and then subdivide below
        m_pointArrayArray.resize(1);
        m_pointArrayArray[0] = shared_ptr<PointArray>(new PointArray());
        m_pointRadius = 0.01f;

        if (endsWith(toLower(spec.filename), ".xyz")) {
            loadXYZ(spec);
        } else if (endsWith(toLower(spec.filename), ".ply")) {
            loadPLY(spec);
        } else if (endsWith(toLower(spec.filename), ".vox")) {
            loadVOX(spec);
        } else if ((endsWith(spec.filename, "/") || endsWith(spec.filename, "\\")) && FileSystem::exists(spec.filename + "000.ply")) {
            for (int i = 0; FileSystem::exists(spec.filename + format("%03d.ply", i)); ++i) {
                debugPrintf("---------------------\nLoading file #%d\n", i);
                Specification individual = spec;
                individual.filename = individual.filename + format("%03d.ply", i);
                loadPLY(individual);
            }
        } else {
            alwaysAssertM(false, "Illegal filename");
        }

        m_pointArrayArray[0]->randomize();

        buildGrid(Vector3(2, 2, 2));

        computeBounds();

        // Write to the cache
        saveCache(cacheFilename);
    }

    // Perform scale as in the specification
    if (spec.scale != 1.0f) {
        m_pointRadius *= spec.scale;
        for (shared_ptr<PointArray> array : m_pointArrayArray) {
            for (Point3& point : array->cpuPosition) {
                point *= spec.scale;
            }
        }
    }


    m_boxBounds = AABox::empty();
    for (shared_ptr<PointArray> array : m_pointArrayArray) {
        m_numPoints += array->size();
        m_boxBounds.merge(array->boxBounds);
    }

    copyToGPU();
}


void PointModel::PointArray::randomize() {
    Random rng(123721321U, false);
    cpuPosition.randomize(cpuRadiance, rng);
}


void PointModel::buildGrid(const Vector3& cellSize) {
    alwaysAssertM(m_pointArrayArray.size() == 1, "There must be exactly one point array when building a grid");
    alwaysAssertM(cellSize.min() > 0, "Cell dimensions must be positive");

    // Divide into cells
    shared_ptr<PointArray> source = m_pointArrayArray[0];
    source->computeBounds();

    const Vector3 vcells = source->boxBounds.extent() / cellSize;
    const Point3int32 cells(iCeil(vcells.x), iCeil(vcells.y), iCeil(vcells.z)); 
    debugPrintf("Cell grid: %d x %d x %d\n", cells.x, cells.y, cells.z);

    alwaysAssertM(cells.x * cells.y * cells.z < 2000, "Too many cells generated");
    const Point3 offset = source->boxBounds.low();

    // Make the grid
    Table<Point3int32, shared_ptr<PointArray>> grid;
    for (Point3int32 p(0,0,0); p.x < cells.x; ++p.x) {
        for (p.y = 0; p.y < cells.y; ++p.y) {
            for (p.z = 0; p.z < cells.z; ++p.z) {
                grid.set(p, shared_ptr<PointArray>(new PointArray()));
            }
        }
    }

    // Redistribute the points
    const Array<Point3>& sourcePos = source->cpuPosition;
    for (int i = 0; i < sourcePos.size(); ++i) {
        const Point3& X = sourcePos[i];
        const Point3int32 p(iFloor((X.x - offset.x) / cellSize.x),
                            iFloor((X.y - offset.y) / cellSize.y),
                            iFloor((X.z - offset.z) / cellSize.z));
        grid[p]->addPoint(X, source->cpuRadiance[i]);
    }
    
    // Accumulate the non-empty cells, including one catch-all for small arrays
    const int minPointsPerSurface = 1000;
    m_pointArrayArray.resize(0);
    m_pointArrayArray.append(PointArray::createShared<PointArray>());
    for (Point3int32 p(0,0,0); p.x < cells.x; ++p.x) {
        for (p.y = 0; p.y < cells.y; ++p.y) {
            for (p.z = 0; p.z < cells.z; ++p.z) {
                const shared_ptr<PointArray>& pointArray = grid[p];
                if (pointArray->size() >= minPointsPerSurface) {
                    debugPrintf("Made a grid cell with %ld points\n", (long)pointArray->size());
                    m_pointArrayArray.append(pointArray);
                } else {
                    // Aggregate into element 0
                    m_pointArrayArray[0]->cpuPosition.append(pointArray->cpuPosition);
                    m_pointArrayArray[0]->cpuRadiance.append(pointArray->cpuRadiance);
                }
            }
        }
    }

    if (m_pointArrayArray[0]->size() == 0) {
        // Remove the empty array
        m_pointArrayArray.remove(0);
    }

    debugPrintf("Made %d grid cells\n", m_pointArrayArray.size());
}


void PointModel::computeBounds() {
    for (const shared_ptr<PointArray>& pointArray : m_pointArrayArray) {
        pointArray->computeBounds();
    }
}


void PointModel::PointArray::computeBounds() {
    if (cpuPosition.size() == 0) {
        boxBounds = AABox();
        sphereBounds = Sphere();
        return;
    }

    Point3 lo = cpuPosition[0];
    Point3 hi = cpuPosition[0];

    for (int i = 1; i < cpuPosition.size(); ++i) {
        const Point3& P = cpuPosition[i];
        lo = lo.min(P);
        hi = hi.max(P);
    }

    boxBounds = AABox(lo, hi);
    boxBounds.getBounds(sphereBounds);
}


void PointModel::PointArray::centerPoints() {
    if (cpuPosition.size() == 0) { return; }

    Point3 center = cpuPosition[0];

    for (int i = 1; i < cpuPosition.size(); ++i) {
        center += cpuPosition[i];
    }

    center /= float(cpuPosition.size());

    for (int i = 0; i < cpuPosition.size(); ++i) {
        cpuPosition[i] -= center;
    }
}


void PointModel::loadXYZ(const Specification& specification) {
    TextInput t(specification.filename);

    alwaysAssertM(specification.sourceColorSpace == ImageFormat::COLOR_SPACE_RGB || 
        specification.sourceColorSpace == ImageFormat::COLOR_SPACE_SRGB, "Only RGB and sRGB color spaces supported");

    bool hasLatLong = specification.xyzOptions.hasLatLong;
    bool hasIR = specification.xyzOptions.hasIR;

    // Skip header/comment row
    while (t.peek().type() == Token::SYMBOL) {
        t.read();
    }

    if (specification.xyzOptions.autodetect) {
        // Autodetection could run on the header row, but don't trust that there is a header
        // and it is right. Go straight to the data. (This is also easier because it is just 
        // a counting task for the way the numbers happen to work out.)

        Array<Token> stack;
        int startLine = t.peekLineNumber();
        while (t.peekLineNumber() == startLine && t.hasMore() && stack.size() <= 9) {
            stack.push(t.read());
        }

        // Format is: [row col] x y z r g b [ir]
        hasLatLong = (stack.size() == 8 || stack.size() == 9);
        hasIR = (stack.size() == 7 || stack.size() == 9);

        // Push the tokens back
        while (stack.size() > 0) {
            t.push(stack.pop());
        }
    }


    // Now read the points
    for (int count = 0; t.hasMore(); ++count) {
        // Coordinates in the lat-lon projection (not all formats have this!)
        int         sourceY = 0;
        int         sourceX = 0;
        
        if (hasLatLong) {
            sourceY = t.readInteger();
            sourceX = t.readInteger();
        }
        
        // World-space position
        const float x       = float(t.readNumber());
        const float y       = float(t.readNumber());
        const float z       = float(t.readNumber());

        // sRGB value
        const int   r       = t.readInteger();
        const int   g       = t.readInteger();
        const int   b       = t.readInteger();

        // IR intensity (not all formats have this!)
        int         i       = 0;
        
        if (hasIR) {
            i = t.readInteger();
        }

        Color4unorm8 color(unorm8::fromBits(r), unorm8::fromBits(g), unorm8::fromBits(b), unorm8::fromBits(255));
        if (specification.sourceColorSpace == ImageFormat::COLOR_SPACE_RGB) {
            color.r = unorm8(pow(float(color.r), 1/2.2f));
            color.g = unorm8(pow(float(color.g), 1/2.2f));
            color.b = unorm8(pow(float(color.b), 1/2.2f));
        }

        addPoint((specification.transform * Vector4(x, y, z, 1.0f)).xyz(), color);

        if ((count & ((1 << 14) - 1)) == 0) {
            debugPrintf("Loaded %d points\n", count);
        }
    }

    if (specification.center) {
        m_pointArrayArray[0]->centerPoints();
    }
}


void PointModel::loadVOX(const Specification& specification){
 
    ParseVOX parseData;
    {
        BinaryInput bi(specification.filename, G3D_LITTLE_ENDIAN);
        parseData.parse(bi);
    }


     // For each voxel
    for (ParseVOX::Voxel voxel : parseData.voxel) {
        const Point3& vox_pos = Point3(voxel.position);

        const Color4& color = parseData.palette[voxel.index];
            // Magic constants for mapping between coordinate systems
        //const float s = 2.0f / 255.0f;

        addPoint(Point3(vox_pos.x, vox_pos.z, -vox_pos.y) * 0.01f, Color4unorm8(color)); //0.01f is hardcoded voxel size, swizzle on Point3 is rotation from MagicaVoxel.
    }
    m_pointArrayArray[0]->centerPoints();
}


void PointModel::addPoint(const Point3& position, const Color4unorm8 radiance) {
    m_pointArrayArray[0]->addPoint(position, radiance);
}


void PointModel::PointArray::addPoint(const Point3& position, const Color4unorm8 radiance) {
    cpuPosition.append(position);
    cpuRadiance.append(radiance);
}


void PointModel::loadPLY(const Specification& spec) {
    TextInput t(spec.filename);
    //bool c = t.hasMore();

    // Assume that we know the format, so skip the header
    while (t.hasMore() && (t.readUntilNewlineAsString().find("end_header") == String::npos));

   

    // Ignore the camera position
    t.readUntilNewlineAsString();
    //bool b_db = t.hasMore();
    // Now read the points
    while (t.hasMore()) {
        const int   sourceY = t.readInteger();
        const int   sourceX = t.readInteger();
        const float x       = float(t.readNumber());
        const float y       = float(t.readNumber());
        const float z       = float(t.readNumber());
        const int   r       = t.readInteger();
        const int   g       = t.readInteger();
        const int   b       = t.readInteger();
        const float nx      = float(t.readNumber());
        const float ny      = float(t.readNumber());
        const float nz      = float(t.readNumber());
        const int   i       = t.readInteger();

        (void)sourceY; (void)sourceX; (void)nx; (void)ny; (void)nz; (void)i;

        // Magic constants for mapping between coordinate systems
        const float s = 2.0f / 255.0f;

        addPoint(Point3(x, z, -y) * 0.0005f, Color4unorm8(Color4(r * s, g * s, b * s, 1.0f)));
    }
    m_pointArrayArray[0]->centerPoints();
}


bool PointModel::intersect
   (const Ray&                      R, 
    const CoordinateFrame&          cframe, 
    float&                          maxDistance, 
    Model::HitInfo&                 info,
    const Entity*                   entity,
    const Model::Pose*              pose) const {
 
    const_cast<PointModel*>(this)->computeBounds();

    const float pRadius = pointRadius();

    float sphereIntersectionTime = finf();
    Point3 intersectedPoint;

    for (const shared_ptr<PointArray>& pointArray : m_pointArrayArray) {

        float t = R.intersectionTime(pointArray->boxBounds);
      
        if (t < maxDistance) {
            for (const Point3& point : pointArray->cpuPosition) {
                float currentIntersectTime = R.intersectionTime(Sphere(point, pRadius), false);
                if (currentIntersectTime < sphereIntersectionTime) {
                    sphereIntersectionTime = currentIntersectTime;
                    intersectedPoint = point;
                }
            }
        }
    }

    if (sphereIntersectionTime < maxDistance) {
        const Point3&  hitPoint = R.direction() * sphereIntersectionTime + R.origin();
        const Vector3& normal   = (hitPoint - intersectedPoint).direction();

        
        info.set(dynamic_pointer_cast<PointModel>(const_cast<PointModel*>(this)->shared_from_this()),
            notNull(entity) ? dynamic_pointer_cast<Entity>(const_cast<Entity*>(entity)->shared_from_this()) : nullptr, 
            nullptr, 
            normal,
            hitPoint);
        return true;
    }
    return false;
}


void PointModel::saveCache(const String& filename) const {
    alwaysAssertM(System::machineEndian() == G3DEndian::G3D_LITTLE_ENDIAN, 
        "Cannot use cache on a big endian machine");

    FileSystem::createDirectory(FilePath::parent(filename));

    FILE* cache = fopen(filename.c_str(), "wb");
    const int64 versionNumber = CURRENT_CACHE_FORMAT;
    fwrite(&versionNumber, sizeof(int64), 1, cache);
    fwrite(&m_pointRadius, sizeof(float), 1, cache);

    const int32 numArrays = m_pointArrayArray.size();
    fwrite(&numArrays, sizeof(numArrays), 1, cache);

    for (const shared_ptr<PointArray>& pointArray : m_pointArrayArray) {
        fwrite(&pointArray->boxBounds.low(), sizeof(Vector3), 1, cache);
        fwrite(&pointArray->boxBounds.high(), sizeof(Vector3), 1, cache);
    
        fwrite(&pointArray->sphereBounds.center, sizeof(Point3), 1, cache);
        fwrite(&pointArray->sphereBounds.radius, sizeof(float), 1, cache);

        const int64 cpuPositionSize = pointArray->cpuPosition.size();
        fwrite(&cpuPositionSize, sizeof(cpuPositionSize), 1, cache);

        fwrite(pointArray->cpuPosition.getCArray(), sizeof(pointArray->cpuPosition[0]), pointArray->cpuPosition.size(), cache);
        fwrite(pointArray->cpuRadiance.getCArray(), sizeof(pointArray->cpuRadiance[0]), pointArray->cpuRadiance.size(), cache);
    }

    fclose(cache);
    cache = nullptr;
}


bool PointModel::loadCache(const String& filename) {
    alwaysAssertM(System::machineEndian() == G3DEndian::G3D_LITTLE_ENDIAN, 
        "Cannot use cache on a big endian machine");

    FILE* cache = fopen(filename.c_str(), "rb");
    if (isNull(cache)) {
        return false;
    }

    // Version
    int64 version = 0;
    fread(&version, sizeof(int64), 1, cache);
    if (CURRENT_CACHE_FORMAT != version) {
        debugPrintf("Cache out of date\n");
        return false;
    }

    fread(&m_pointRadius, sizeof(float), 1, cache);

    int32 numArrays = 0;
    fread(&numArrays, sizeof(int32), 1, cache);

    m_pointArrayArray.resize(numArrays);
    for (int i = 0; i < m_pointArrayArray.size(); ++i) {
        const shared_ptr<PointArray>& pointArray = m_pointArrayArray[i] = shared_ptr<PointArray>(new PointArray());

        Point3 low, high;
        fread(&low, sizeof(Point3), 1, cache);
        fread(&high, sizeof(Point3), 1, cache);
        pointArray->boxBounds = AABox(low, high);

        Point3 center;
        float radius = 0;
        fread(&center, sizeof(Point3), 1, cache);
        fread(&radius, sizeof(float), 1, cache);
        pointArray->sphereBounds = Sphere(center, radius);

        int64 cpuPositionSize = 0;
        fread(&cpuPositionSize, sizeof(int64), 1, cache);
        pointArray->cpuPosition.resize(cpuPositionSize);
        fread(pointArray->cpuPosition.getCArray(), sizeof(Vector3), cpuPositionSize, cache);

        pointArray->cpuRadiance.resize(cpuPositionSize);
        fread(pointArray->cpuRadiance.getCArray(), sizeof(Color4unorm8), cpuPositionSize, cache);
    }
     
    fclose(cache);
    cache = nullptr;

    return true;
}


void PointModel::pose
(Array<shared_ptr<Surface> >&   surfaceArray, 
 const CFrame&                  rootFrame, 
 const CFrame&                  prevFrame,
 const shared_ptr<Entity>&      entity,
 const Model::Pose*             pose,
 const Model::Pose*             prevPose,
 const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties) {
    this->pose(surfaceArray, entity);
}


void PointModel::pose(Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<Entity>& entity) const {

    const shared_ptr<PointModel>& me = dynamic_pointer_cast<PointModel>(const_cast<PointModel*>(this)->shared_from_this());
    
    for (const shared_ptr<PointArray>& pointArray : m_pointArrayArray) {
        surfaceArray.append(PointSurface::create(name(), notNull(entity) ? entity->frame() : CFrame(), notNull(entity) ? entity->previousFrame() : CFrame(),
            pointArray, me, entity, Surface::ExpressiveLightScatteringProperties()));
    }
}


String PointModel::makeCacheFilename(const String& resolvedFilename) {
    return FilePath::concat("cache", manglePathToFilename(resolvedFilename) + ".bin");
}


String PointModel::manglePathToFilename(const String& filename) {
    String outputFilename;

    for (size_t i = 0; i < filename.size(); ++i) {
        switch (filename[i]) {
        case ':':
            outputFilename += "_c";
            break;
        case '/':
            outputFilename += "_s";
            break;
        case '\\':
            outputFilename += "_b";
            break;
        case '.':
            outputFilename += "_p";
            break;
        case '*':
            outputFilename += "_a";
            break;
        case '_':
            outputFilename += "_u";
            break;
        default:
            outputFilename += filename[i];
            break;
        }
    }

    return outputFilename;
}

}
