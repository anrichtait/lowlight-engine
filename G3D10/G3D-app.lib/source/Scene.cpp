/**
  \file G3D-app.lib/source/Scene.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/Scene.h"
#include "G3D-base/units.h"
#include "G3D-base/Table.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/Log.h"
#include "G3D-base/Ray.h"
#include "G3D-base/CubeMap.h"
#include "G3D-app/ArticulatedModel.h"
#include "G3D-app/VisibleEntity.h"
#include "G3D-app/ParticleSystem.h"
#include "G3D-app/Light.h"
#include "G3D-app/Camera.h"
#include "G3D-app/HeightfieldModel.h"
#include "G3D-app/MarkerEntity.h"
#include "G3D-app/Skybox.h"
#include "G3D-app/SkyboxSurface.h"
#include "G3D-app/ParticleSystemModel.h"
#include "G3D-app/GFont.h"
#include "G3D-app/PointModel.h"
#include "G3D-app/FontModel.h"
#include "G3D-app/VoxelModel.h"
#include "G3D-app/SoundEntity.h"

using namespace G3D::units;

namespace G3D {

bool Scene::contains(const shared_ptr<Camera>& c) const {
    for (const shared_ptr<Camera>& it : m_cameraArray) {
        if (it == c) { return true; }
    }
    return false;
}


void Scene::onSimulation(SimTime deltaTime) {
    sortEntitiesByDependency();
    m_time += isNaN(deltaTime) ? 0 : deltaTime;
    for (int i = 0; i < m_entityArray.size(); ++i) {
        const shared_ptr<Entity> entity = m_entityArray[i];
        
        entity->onSimulation(m_time, deltaTime);

        const shared_ptr<Light>& light = dynamic_pointer_cast<Light>(entity);
        if (light) {
            m_lastLightChangeTime = max(m_lastLightChangeTime, entity->lastChangeTime());
            if (light->visible()) {
                m_lastVisibleChangeTime = max(m_lastVisibleChangeTime, entity->lastChangeTime());
            }
        } else if (dynamic_pointer_cast<VisibleEntity>(entity)) {
            m_lastVisibleChangeTime = max(m_lastVisibleChangeTime, entity->lastChangeTime());
        }
        // Intentionally ignoring the case of other Entity subclasses
    }

    if (m_editing) {
        m_lastEditingTime = System::time();
    }

}


void Scene::registerEntitySubclass(const String& name, EntityFactory factory, bool errorIfAlreadyRegistered) {
    alwaysAssertM(! m_entityFactory.containsKey(name) || ! errorIfAlreadyRegistered, name + " has already been registered as an entity subclass.");
    m_entityFactory.set(name, factory);
}


void Scene::registerModelSubclass(const String& name, LazyModelFactory factory, bool errorIfAlreadyRegistered) {
    alwaysAssertM(! m_modelFactory.containsKey(name) || ! errorIfAlreadyRegistered, name + " has already been registered as an entity subclass.");
    m_modelFactory.set(name, factory);
}


static Array<String> searchPaths;
static bool          defaultSearchPathsAppended = false;

void Scene::setSceneSearchPaths(const Array<String>& paths) {
    searchPaths.copyFrom(paths);
}

const shared_ptr<TriTree>& Scene::tritree(TriTreeGPUAPI gpuApi) {
    debugAssertGLOk();
    BEGIN_PROFILER_EVENT("Scene::tritree()"); {
        if (isNull(m_triTree)) {
            // Will attempt to create a GPU tritree by default.
            m_triTree = TriTree::create(true, gpuApi == TriTreeGPUAPI::VULKAN);
        }
    const shared_ptr<Scene>& scene = dynamic_pointer_cast<Scene>(shared_from_this());
	// No-op if no changes (on OptiXTriTree), safe to call repeatedly.
	m_triTree->setContents(scene);
    } END_PROFILER_EVENT();

    return m_triTree;
}

static void appendDefaultSearchPaths() {
    // Add the current directory
    searchPaths.append(".");  

    // Add the directories specified by the environment variable G3D10DATA
    std::vector<String> g3dDataPaths;
    System::getG3DDataPaths(g3dDataPaths);
    for(size_t i = 0; i < g3dDataPaths.size(); i++) {
        searchPaths.append(g3dDataPaths[i]);
    }

    // If not already a subdirectoy of a search path, add the G3D scenes
    // directory, which is detected by the CornellBox-Glossy.scn.any file
    if (! _internal::g3dInitializationSpecification().deployMode) {
        const String& s = System::findDataFile("scene/CornellBox-glossy.Scene.Any", false);
        if (! s.empty()) {
            bool subDirectory = false;
            for (int i = 0; i < searchPaths.size(); ++i) {
                if (s.find(searchPaths[i]) != String::npos) {
                   subDirectory = true;
                }
            }
            if (!subDirectory) {
               searchPaths.append(FilePath::parent(s));
            }
        }
    }
}


/** Returns a table mapping scene names to filenames */
static Table<String, String>& filenameTable() {
    static Table<String, String> filenameTable;

    if (filenameTable.size() == 0) {
        // disable marking files used while building caches of scenes
        FileSystem::setMarkFileUsedEnabled(false);

        if (!defaultSearchPathsAppended) {
            appendDefaultSearchPaths();
            defaultSearchPathsAppended = true;
        }

        // Create a table mapping scene names to filenames
        Array<String> filenameArray;

        FileSystem::ListSettings settings;
        settings.files = true;
        settings.directories = false;
        settings.includeParentPath = true;
        settings.recursive = true;

        for (int i = 0; i < searchPaths.size(); ++i) {
            FileSystem::list(FilePath::concat(searchPaths[i], "*.scn.any"), filenameArray, settings);
            FileSystem::list(FilePath::concat(searchPaths[i], "*.Scene.Any"), filenameArray, settings);
        }

        logLazyPrintf("Found scenes:\n");
        for (int i = 0; i < filenameArray.size(); ++i) {
            if (filenameArray[i].find('$') != String::npos) {
                logPrintf("Scene::filenameTable() skipped \"%s\" because it contained '$' which looked like an environment variable.\n",
                          filenameArray[i].c_str());
                continue;
            }
            Any a;
            String msg;
            try {
                a.load(filenameArray[i]);
                
                const String& name = a["name"].string();
                if (filenameTable.containsKey(name)) {
                    debugPrintf("%s", ("Warning: Duplicate scene names in " + filenameTable[name] +  " and " + filenameArray[i] +". The second was ignored.\n").c_str());
                } else {
                    logLazyPrintf("  \"%s\" (%s)\n", name.c_str(), filenameArray[i].c_str());
                    filenameTable.set(name, filenameArray[i]);
                }
            } catch (const ParseError& e) {
                msg = format("  <Parse error at %s:%d(%d): %s>\n", e.filename.c_str(), e.line, e.character, e.message.c_str());
            } catch (...) {
                msg = format("  <Error while loading %s>\n", filenameArray[i].c_str());
            }

            if (! msg.empty()) {
                logLazyPrintf("%s", msg.c_str());
                debugPrintf("%s", msg.c_str());
            }
        }
        logPrintf("");

        // Re-enable marking files used
        FileSystem::setMarkFileUsedEnabled(true);
    }

    return filenameTable;
}


void Scene::appendSceneSearchPaths(const Array<String>& paths) {
    searchPaths.append(paths);
    // Trigger reloading all filenames
    filenameTable().clear();
}

Array<String> Scene::sceneNames() {
    Array<String> a;
    filenameTable().getKeys(a);
    a.sort(alphabeticalIgnoringCaseG3DFirstLessThan);
    return a;
}


Scene::Scene(const shared_ptr<AmbientOcclusion>& ambientOcclusion) :
    m_needEntitySort(false),
    m_time(0),
    m_lastStructuralChangeTime(0),
    m_lastVisibleChangeTime(0),
    m_lastLightChangeTime(0),
    m_editing(false),
    m_lastEditingTime(0) {

    m_localLightingEnvironment.ambientOcclusion = ambientOcclusion;
    registerEntitySubclass("VisibleEntity",  &VisibleEntity::create);
    registerEntitySubclass("ParticleSystem", &ParticleSystem::create);
    registerEntitySubclass("Light",          &Light::create);
    registerEntitySubclass("Camera",         &Camera::create);
    registerEntitySubclass("MarkerEntity",   &MarkerEntity::create);
    registerEntitySubclass("Skybox",         &Skybox::create);
#ifndef G3D_NO_FMOD
    registerEntitySubclass("SoundEntity",    &SoundEntity::create);
#endif
    registerModelSubclass("ArticulatedModel",&ArticulatedModel::lazyCreate);
    registerModelSubclass("MD2Model",        &MD2Model::lazyCreate);
    registerModelSubclass("MD3Model",        &MD3Model::lazyCreate);
    registerModelSubclass("HeightfieldModel",&HeightfieldModel::lazyCreate);
    registerModelSubclass("ParticleSystemModel",&ParticleSystemModel::lazyCreate);
    registerModelSubclass("PointModel",      &PointModel::lazyCreate);
    registerModelSubclass("VoxelModel",      &VoxelModel::lazyCreate);
    registerModelSubclass("FontModel",       &FontModel::lazyCreate);
}


void Scene::clear() {
    shared_ptr<AmbientOcclusion> old = m_localLightingEnvironment.ambientOcclusion;

    // Entitys, cameras, lights, all settings back to intial defauls
    m_ancestorTable.clear();
    m_needEntitySort = false;
    m_entityTable.clear();
    m_entityArray.fastClear();
    m_cameraArray.fastClear();
    m_localLightingEnvironment = LightingEnvironment();
    m_localLightingEnvironment.ambientOcclusion = old;
    m_skybox.reset();
    m_time = 0;
    m_sourceAny = Any();
    m_lastVisibleChangeTime = m_lastLightChangeTime = m_lastStructuralChangeTime = System::time();
}


shared_ptr<Scene> Scene::create(const shared_ptr<AmbientOcclusion>& ambientOcclusion) {
    return createShared<Scene>(ambientOcclusion);
}


void Scene::setEditing(bool b) {
    m_editing = b;
    m_lastEditingTime = System::time();
}


const shared_ptr<Camera> Scene::defaultCamera() const {
    for (int i = 0; i < 3; ++i) {
        const shared_ptr<Camera>& c = typedEntity<Camera>(m_defaultCameraName);
        if (notNull(c)) { return c; }
    }

    return m_cameraArray[0];
}


void Scene::setTime(const SimTime t) {
    m_time = t;
    // Called twice to wipe out the first-order time derivative

    onSimulation(fnan());
    onSimulation(fnan());
}


String Scene::sceneNameToFilename(const String& scene)  {
    const bool isFilename = endsWith(toLower(scene), ".scn.any") || endsWith(toLower(scene), ".scene.any");

    if (isFilename) {
        return scene;
    } else {
        const String* f = filenameTable().getPointer(scene);
        if (isNull(f)) {
            throw "No scene with name '" + scene + "' found in (" +
                stringJoin(filenameTable().getKeys(), ", ") + ")";
        }

        return *f;
    }
}


Any Scene::load(const String& scene, const LoadOptions& loadOptions) {
    shared_ptr<AmbientOcclusion> old = m_localLightingEnvironment.ambientOcclusion;
    const String& filename = sceneNameToFilename(scene);
    
    clear();
    m_modelTable.clear();
    m_name = scene;

    Any any;
    any.load(filename);

    m_description = any.get("description", "").string();

    {
        const String& n = any.get("name", filename);

        // Ensure that this name appears in the filename table if it does not already,
        // so that it can be loaded by name in the future.
        if (! filenameTable().containsKey(n)) {
            filenameTable().set(n, filename);
        }
    }

    m_sourceAny = any;

    // Load the lighting environment (do this before loading entities, since some of them may
    // be lights that will enter this array)
    bool hasEnvironmentMap = false;
    if (any.containsKey("lightingEnvironment")) {
        m_localLightingEnvironment = any["lightingEnvironment"];
        hasEnvironmentMap = any["lightingEnvironment"].containsKey("environmentMap");
    }
        
    const String modelSectionName[]  = {"models", "models2"};
    const String entitySectionName[] = {"entities", "entities2"};

    m_modelsAny = Any(Any::TABLE);
    // Load the models
    for (int i = 0; i < 2; ++i) {
        if (any.containsKey(modelSectionName[i])) {
            Any models = any[modelSectionName[i]];
            if (models.size() > 0) {
                for (Any::AnyTable::Iterator it = models.table().begin(); it.isValid(); ++it) {
                    const String name = it->key;
                    const Any& v = it->value;
                    createModel(v, name);
                }
            }
        }
    }

    // Instantiate the entities
    // Try for both the current and extended format entity group names...intended to support using #include to merge
    // different files with entitys in them
    for (int i = 0; i < 2; ++i) {
        if (any.containsKey(entitySectionName[i])) { 
            const Any& entities = any[entitySectionName[i]];
            if (entities.size() > 0) {
                for (Table<String, Any>::Iterator it = entities.table().begin(); it.isValid(); ++it) {
                    const String& name = it->key;
                    createEntity(name, it->value, loadOptions);
                } // for each entity
            } // if there is anything in this table
        } // if this entity group name exists
    } 

    
    shared_ptr<Texture> skyboxTexture = Texture::whiteCube();

    Any skyAny;

    // Use the environment map as a skybox if there isn't one already, and vice versa
    Array<shared_ptr<Skybox> > skyboxes;
    getTypedEntityArray<Skybox>(skyboxes);
    if (skyboxes.size() == 0) {
        if (any.containsKey("skybox")) {
            // Legacy path
            debugPrintf("Warning: Use the Skybox entity now instead of a top-level skybox field in a Scene.Any file\n");
            createEntity("Skybox", "skybox", any["skybox"]);
            m_skybox = typedEntity<Skybox>("skybox");
        } else if (hasEnvironmentMap) {
            // Create the skybox from the environment map
            m_skybox = Skybox::create("skybox", this, Array<shared_ptr<Texture> >(m_localLightingEnvironment.environmentMapArray[0]), Array<SimTime>(0.0), 0, SplineExtrapolationMode::CLAMP, false, false);
            // GCC misunderstands overloading, necessitating this cast
            // DO NOT CHANGE without being aware of that
            insert(dynamic_pointer_cast<Entity>(m_skybox));
        } else {
            m_skybox = Skybox::create("skybox", this, Array<shared_ptr<Texture> >(Texture::whiteCube()), Array<SimTime>(0.0), 0, SplineExtrapolationMode::CLAMP, false, false);
            // GCC misunderstands overloading, necessitating this cast
            // DO NOT CHANGE without being aware of that
            insert(dynamic_pointer_cast<Entity>(m_skybox));
        }
    }

    if (any.containsKey("environmentMap")) {
        throw String("environmentMap field has been replaced with lightingEnvironment");
    }


    // Default to using the skybox as an environment map if none is specified.
    if (! hasEnvironmentMap) {
        if (m_skybox->keyframeArray()[0]->dimension() == Texture::DIM_CUBE_MAP) {
            m_localLightingEnvironment.environmentMapArray.append(m_skybox->keyframeArray()[0]);
        } else {
            m_localLightingEnvironment.environmentMapArray.append(Texture::whiteCube());
        }
    }
    any.verify(m_localLightingEnvironment.environmentMapArray[0]->dimension() == Texture::DIM_CUBE_MAP);


    //////////////////////////////////////////////////////

    if (m_cameraArray.size() == 0) {
        // Create a default camera, back it up from the origin
        const shared_ptr<Camera>& c = Camera::create("camera");
        c->setFrame(CFrame::fromXYZYPRDegrees(0, 1, -5, 0, -5));
        insert(c);
    }

    setTime(any.get("time", 0.0));
    m_lastVisibleChangeTime = m_lastLightChangeTime = m_lastStructuralChangeTime = System::time();

    m_defaultCameraName = any.get("defaultCamera", "defaultCamera").string();

    m_localLightingEnvironment.ambientOcclusion = old;

    Any vrSettingsAny = any.get("vrSettings", Any());
    if (vrSettingsAny.type() != Any::NIL) {
        m_vrSettings = vrSettingsAny;
    } else {
        m_vrSettings = VRSettings();
    }

    // Set the initial positions, repeating a few times to allow
    // objects defined relative to others to reach a fixed point
    for (int i = 0; i < 3; ++i) {
        for (int e = 0; e < m_entityArray.size(); ++e) {
            m_entityArray[e]->onSimulation(m_time, fnan());
        }
    }

    // Pose objects so that they have bounds.
    {
        Array< shared_ptr<Surface> > ignore;
        onPose(ignore);
    }

    return any;
}


void Scene::getVisibleBounds(AABox& box) const {
    box = AABox();
    for (int e = 0; e < m_entityArray.size(); ++e) {
        const shared_ptr<VisibleEntity>& entity = dynamic_pointer_cast<VisibleEntity>(m_entityArray[e]);
        if (notNull(entity) && entity->visible() && notNull(entity->model())) {
            AABox eBox;
            entity->getLastBounds(eBox);
            box.merge(eBox);
        }
    }
}


lazy_ptr<Model> Scene::createModel(const Any& v, const String& name) {
    v.verify(! m_modelTable.containsKey(name), "A model named '" + name + "' already exists in this scene.");

    lazy_ptr<Model> m;

    if (v.type() == Any::STRING) {
        // Default/fall through
        m = ArticulatedModel::lazyCreate(ArticulatedModel::Specification(v), name);
    } else {
        String modelClassName = v.name();
        // Strip any suffix
        const size_t i = modelClassName.find("::");
        if (i != String::npos) {
            modelClassName = modelClassName.substr(0, i);
        }

        const LazyModelFactory* factory = m_modelFactory.getPointer(modelClassName);
        if (notNull(factory)) {
            m = (**factory)(name, v);
        } else {
            v.verify(false, String("Unrecognized Model subclass: \"") + v.name() + "\"");
        }
    }

    alwaysAssertM(notNull(m), "Null model");

    m_modelTable.set(name, m);
    m_modelsAny[name] = v;
    return m;
}


void Scene::getEntityNames(Array<String>& names) const {
    for (int e = 0; e < m_entityArray.size(); ++e) {
        names.append(m_entityArray[e]->name());
    }
}


void Scene::getCameraNames(Array<String>& names) const {
    for (int e = 0; e < m_cameraArray.size(); ++e) {
        names.append(m_cameraArray[e]->name());
    }
}


const shared_ptr<Entity> Scene::entity(const String& name) const {
    shared_ptr<Entity>* e = m_entityTable.getPointer(name);
    if (notNull(e)) {
        return *e;
    } else {
        return nullptr;
    }
}


shared_ptr<Model> Scene::insert(const shared_ptr<Model>& model) {
    debugAssert(notNull(model));
    debugAssert(! m_modelTable.containsKey(model->name()));
    m_modelTable.set(model->name(), model);
    return model;
}


void Scene::removeModel(const String& modelName) {
    debugAssert(m_modelTable.containsKey(modelName));
    m_modelTable.remove(modelName);
}


void Scene::remove(const shared_ptr<Model>& model) {
    debugAssert(notNull(model));
    removeModel(model->name());
}


void Scene::removeEntity(const String& entityName) {
    remove(entity(entityName));
}


void Scene::remove(const shared_ptr<Entity>& entity) {
    debugAssert(notNull(entity));

    const String& name = entity->name();

    // Remove from dependency tables
    {
        DependencyList* list = m_ancestorTable.getPointer(name);
        if (list) {
            for (int j = 0; j < list->size(); ++j) {
                DependencyList& L = m_descendantTable[(*list)[j]];
                const int i = L.findIndex(name);
                L.fastRemove(i);
            }
            m_ancestorTable.remove(name);
        }
    }

    {
        DependencyList* list = m_descendantTable.getPointer(name);
        if (list) {
            for (int j = 0; j < list->size(); ++j) {
                DependencyList& L = m_ancestorTable[(*list)[j]];
                const int i = L.findIndex(name);
                L.fastRemove(i);
            }
            m_descendantTable.remove(name);
        }
    }
    
    m_entityTable.remove(name);
    m_entityArray.remove(m_entityArray.findIndex(entity));

    const shared_ptr<VisibleEntity>& visible = dynamic_pointer_cast<VisibleEntity>(entity);
    if (notNull(visible)) {
        m_lastVisibleChangeTime = System::time();
    }

    const shared_ptr<Camera>& camera = dynamic_pointer_cast<Camera>(entity);
    if (notNull(camera)) {
        m_cameraArray.remove(m_cameraArray.findIndex(camera));
    }

    const shared_ptr<Light>& light = dynamic_pointer_cast<Light>(entity);
    if (notNull(light)) {
        m_localLightingEnvironment.lightArray.remove(m_localLightingEnvironment.lightArray.findIndex(light));
        m_lastLightChangeTime = System::time();
    }

}


shared_ptr<Entity> Scene::insert(const shared_ptr<Entity>& entity) {
    debugAssert(notNull(entity));

    debugAssertM(! m_entityTable.containsKey(entity->name()), "Two Entitys with the same name, \"" + entity->name() + "\"");
    m_entityTable.set(entity->name(), entity);
    m_entityArray.append(entity);
    m_lastStructuralChangeTime = System::time();
    
    const shared_ptr<VisibleEntity>& visible = dynamic_pointer_cast<VisibleEntity>(entity);
    if (notNull(visible)) {
        m_lastVisibleChangeTime = System::time();
    }

    const shared_ptr<Camera>& camera = dynamic_pointer_cast<Camera>(entity);
    if (notNull(camera)) {
        m_cameraArray.append(camera);
    }

    const shared_ptr<Light>& light = dynamic_pointer_cast<Light>(entity);
    if (notNull(light)) {
        m_localLightingEnvironment.lightArray.append(light);
        m_lastLightChangeTime = System::time();
    }

    const shared_ptr<Skybox>& skybox = dynamic_pointer_cast<Skybox>(entity);
    if (notNull(skybox)) {
        m_skybox = skybox;
    }

    // Simulate and pose the entity so that it has bounds
    entity->onSimulation(m_time, 0);
    Array< shared_ptr<Surface> > ignore;
    entity->onPose(ignore);

    return entity;
}


shared_ptr<Entity> Scene::createEntity(const String& name, const Any& any, const Scene::LoadOptions& options) {
    return createEntity(any.name(), name, any, options);
}


shared_ptr<Entity> Scene::createEntity(const String& entityType, const String& name, const Any& any, const Scene::LoadOptions& options) {
    shared_ptr<Entity> entity;
    AnyTableReader propertyTable(any);

    const EntityFactory* factory = m_entityFactory.getPointer(entityType);
    if (notNull(factory)) {
        entity = (**factory)(name, this, propertyTable, m_modelTable, options);
        if (notNull(entity)) {
            insert(entity);
        }            
    } else {
        any.verify(false, String("Unrecognized Entity type: \"") + entityType + "\"");
    }
        
    return entity;
}


void Scene::onPose(Array<shared_ptr<Surface> >& surfaceArray) {
    for (int e = 0; e < m_entityArray.size(); ++e) {
        m_entityArray[e]->onPose(surfaceArray);
    }
}


shared_ptr<Entity> Scene::intersectBounds(const Ray& ray, float& distance, bool intersectMarkers, const Array<shared_ptr<Entity> >& exclude) const {
    shared_ptr<Entity> closest;
    
    for (int e = 0; e < m_entityArray.size(); ++e) {
        const shared_ptr<Entity>& entity = m_entityArray[e];
        if ((intersectMarkers || isNull(dynamic_pointer_cast<MarkerEntity>(entity))) &&
            ! exclude.contains(entity) && 
            entity->intersectBounds(ray, distance)) {
            closest = entity;
        }
    }

    return closest;
}


shared_ptr<Entity> Scene::intersect
   (const Ray&                          ray, 
    float&                              distance,
    bool                                intersectMarkers, 
    const Array<shared_ptr<Entity> >&   exclude, 
    Model::HitInfo&                     info) const {

    shared_ptr<Entity> closest;    
    for (const shared_ptr<Entity>& entity : m_entityArray) {
        if ((intersectMarkers || isNull(dynamic_pointer_cast<MarkerEntity>(entity))) &&
            ! exclude.contains(entity) &&
            entity->intersect(ray, distance, info)) {
            closest = entity;
        }
    }

    return closest;
}


Any Scene::toAny(const bool forceAll) const {
    Any a = m_sourceAny;

    // Overwrite the entity table
    Any entityTable(Any::TABLE);
    for (int i = 0; i < m_entityArray.size(); ++i) {
        const shared_ptr<Entity>& entity = m_entityArray[i];
        if (entity->shouldBeSaved()) {
            entityTable[entity->name()] = entity->toAny(forceAll);
        }
        //debugPrintf("Saving %s\n",  entity->name().c_str());
    }

    a["entities"] = entityTable;
    a["lightingEnvironment"] = m_localLightingEnvironment;
    a["models"] = m_modelsAny;
    a["description"] = m_description;
    
    return a;
}


void Scene::getEntityArray(const Array<String>& names, Array<shared_ptr<Entity> >& array) const {
    for (const String& n : names) {
        array.append(entity(n));
    }
}


#define DEBUG_SORT 0
void Scene::getDescendants(const Array<String>& root, Array<String>& descendants) const {
    Array<String> stack;
    Set<String> visited;

    for (const String& r : root) { visited.insert(r); }
    stack.append(root);

    while (stack.size() > 0) {
        const String& s = stack.pop();
        const DependencyList* list = m_descendantTable.getPointer(s);
        if (notNull(list)) {
            for (int i = 0; i < list->size(); ++i) {
                const String& c = (*list)[i];
                if (! visited.contains(c)) {
                    descendants.append(c);
                    stack.push(c);
                    visited.insert(c);
                }
            } // for each dependent
        } // if
    } // while
}


void Scene::sortEntitiesByDependency() {
    if (! m_needEntitySort) { return; }

    if (m_ancestorTable.size() > 0) {
        Table<Entity*, VisitorState> visitorStateTable;

        // Push all entities onto a stack. Fill the stack backwards, 
        // so that we don't change the order of
        // unconstrained objects.
        Array< shared_ptr<Entity> > stack;
        stack.reserve(m_entityArray.size());
        for (int e = m_entityArray.size() - 1; e >= 0; --e) {
            const shared_ptr<Entity>& entity = m_entityArray[e];
            stack.push(entity);
            visitorStateTable.set(entity.get(), NOT_VISITED);
        }
        
        m_entityArray.fastClear();

        // For each element of the stack that has not been visited, push all
        // of its dependencies on top of it.
        while (stack.size() > 0) {
#           if DEBUG_SORT
                debugPrintf("______________________________________________\nstack = ["); 
                for (int i = 0; i < stack.size(); ++i) { debugPrintf("\"%s\", ", stack[i]->name().c_str()); }; 
                debugPrintf("]\nm_entityArray = ["); 
                for (int i = 0; i < m_entityArray.size(); ++i) { debugPrintf("\"%s\", ", m_entityArray[i]->name().c_str()); };  
                debugPrintf("]\n\n");
#           endif
            const shared_ptr<Entity>& entity = stack.pop();
            VisitorState& state = visitorStateTable[entity.get()];

#           if DEBUG_SORT
                debugPrintf("entity = \"%s\", state = %s\n", entity->name().c_str(),
                    (state == NOT_VISITED) ? "NOT_VISITED" :
                    (state == VISITING) ? "VISITING" :
                    "ALREADY_VISITED");
#           endif
            switch (state) {
            case NOT_VISITED:
                {
                    state = VISITING;

                    // See if this node has any dependencies
                    const DependencyList* dependencies = m_ancestorTable.getPointer(entity->name());
                    if (notNull(dependencies)) {

                        // Push this node back on the stack
                        stack.push(entity);
                        debugAssertM(dependencies->size() > 0, "Empty dependency list stored");

                        // Process each dependency
                        for (int d = 0; d < dependencies->size(); ++d) {
                            const String& parentName = (*dependencies)[d];
                            const shared_ptr<Entity>& parent = this->entity(parentName);

                            if (notNull(parent)) {
                                debugAssertM(notNull(parent), entity->name() + " depends on " + parentName + ", which does not exist.");
                        
                                VisitorState& parentState = visitorStateTable[parent.get()];

                                debugAssertM(parentState != VISITING,
                                    String("Dependency cycle detected containing ") + entity->name() + " and " + parentName);

                                if (parentState == NOT_VISITED) {
                                    // Push the dependency on the stack so that it will be processed ahead of entity
                                    // that depends on it.  The parent may already be in the stack
                                    stack.push(parent);
                                } else {
                                    // Do nothing, this parent was already processed and is in the
                                    // entity array ahead of the child.
                                    debugAssert(parentState == ALREADY_VISITED);
                                }
                            } else {
#                               ifdef G3D_DEBUG
                                    debugPrintf("%s", ("Warning: " + entity->name() + " depends on " + parentName + ", which does not exist.").c_str());
#                               endif
                            } // parent not null
                        }

                    } else {
                        // There are no dependencies
                        state = ALREADY_VISITED;
                        m_entityArray.push(entity);
                    }
                } break;

            case VISITING:
                // We've come back to this entity after processing its dependencies, and are now ready to retire it
                state = ALREADY_VISITED;
                m_entityArray.push(entity);
                break;

            case ALREADY_VISITED:;
                // Ignore this entity because it was already processed            
            } // switch
        } // while
    } // if there are dependencies

    /*
    // Show result
    debugPrintf("---------------------------\n");
    for (int i = 0; i < m_entityArray.size(); ++i) {
        debugPrintf("'%s'\n", m_entityArray[i]->name().c_str());
    }
    */

    m_needEntitySort = false;
}


void Scene::setOrder(const String& entity1Name, const String& entity2Name) {
    debugAssert(entity1Name != entity2Name);
    bool ignore;
    DependencyList& list = m_ancestorTable.getCreate(entity2Name, ignore);

    debugAssertM(! m_ancestorTable.containsKey(entity1Name) || ! m_ancestorTable[entity1Name].contains(entity2Name), 
        "Tried to specify a cyclic dependency between " + entity1Name + " and " + entity2Name);
    debugAssertM(! list.contains(entity1Name), "Duplicate dependency specified");
    list.append(entity1Name);

    DependencyList& descendantList = m_descendantTable.getCreate(entity1Name);
    descendantList.append(entity2Name);

    m_needEntitySort = true;
}


void Scene::clearOrder(const String& entity1Name, const String& entity2Name) {
    debugAssert(entity1Name != entity2Name);
    bool created = false;
    DependencyList& list = m_ancestorTable.getCreate(entity2Name, created);
    debugAssertM(! created, "Tried to remove a dependency that did not exist");

    int i = list.findIndex(entity1Name);
    debugAssertM(i != -1, "Tried to remove a dependency that did not exist");
    list.fastRemove(i);

    if (list.size() == 0) {
        // The list is empty, don't store it any more
        m_ancestorTable.remove(entity2Name);
    }

    created = false;
    DependencyList& descendantList = m_descendantTable.getCreate(entity1Name, created);
    i = descendantList.findIndex(entity2Name);
    debugAssertM(i != -1, "Tried to remove a dependency that did not exist");
    descendantList.fastRemove(i);

    if (descendantList.size() == 0) {
        // The list is empty, don't store it any more
        m_descendantTable.remove(entity1Name);
    }
    
    m_needEntitySort = true;
}


Ray Scene::eyeRay(const shared_ptr<Camera>& camera, const Vector2& pixel, const Rect2D& viewport, const Vector2int16 guardBandThickness) const {
    return camera->worldRay(pixel.x + guardBandThickness.x, pixel.y + guardBandThickness.y, Rect2D(Vector2(viewport.width() + 2 * guardBandThickness.x, viewport.height() + 2 * guardBandThickness.y)));
}


void Scene::visualize(RenderDevice* rd, const shared_ptr<Entity>& selectedEntity, const Array<shared_ptr<Surface> >& allSurfaces, const SceneVisualizationSettings& settings, const shared_ptr<Camera>& camera) {
    if (settings.showWireframe) {
        Surface::renderWireframe(rd, allSurfaces);
    }
    
    if (isNull(m_font)) {
        m_font = GFont::fromFile(System::findDataFile(System::findDataFile("arial.fnt")));
    }

    // Visualize markers, light source bounds, selected entities, and other features
    for (int i = 0; i < m_entityArray.size(); ++i) {
        m_entityArray[i]->visualize(rd, m_entityArray[i] == selectedEntity, settings, m_font, camera);
    }
}


shared_ptr<CubeMap> Scene::skyboxAsCubeMap() const {
    if (isNull(m_skybox)) {
        return nullptr;
    }
    debugAssertGLOk();

    Array<shared_ptr<Surface>> surfaceArray;
    BEGIN_PROFILER_EVENT("skyboxOnpose");
    m_skybox->onPose(surfaceArray);
    END_PROFILER_EVENT();

    return dynamic_pointer_cast<SkyboxSurface>(surfaceArray[0])->texture0()->toCubeMap();
}


shared_ptr<CubeMap> Scene::environmentMapAsCubeMap() const {
    if (m_localLightingEnvironment.environmentMapArray.size() == 0) {
        return nullptr;
    }

    return m_localLightingEnvironment.environmentMapArray[0]->toCubeMap();
}


Scene::VRSettings::Avatar::Avatar(const Any& any) {
    *this = Avatar();
    AnyTableReader r(any);
    r.getIfPresent("addHandEntity",       addHandEntity);
    r.getIfPresent("addControllerEntity", addControllerEntity);
    r.getIfPresent("addTorsoEntity",      addTorsoEntity);
    r.verifyDone();
}


Scene::VRSettings::VRSettings(const Any& any) {
    AnyTableReader r(any);
    *this = VRSettings();

    r.getIfPresent("avatar", avatar);

    r.verifyDone();
}

} // namespace G3D
