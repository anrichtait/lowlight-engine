/**
  \file G3D-app.lib/include/G3D-app/Entity.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_Entity_h

#include "G3D-base/platform.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/Sphere.h"
#include "G3D-base/AABox.h"
#include "G3D-base/Box.h"
#include "G3D-base/PhysicsFrameSpline.h"
#include "G3D-base/Any.h"
#include "G3D-app/Model.h"

namespace G3D {

class Scene;
class GFont;
class SoundEntity;
class Sound;

/** \brief Base class for objects in a G3D::Scene

    Entity can be subclassed as long as you override the Scene::createEntity method
    to understand the new subclass.

    To make an object controled by its own logic (instead of moving
    along a predetermined spline), subclass VisibleEntity and override
    Entity::onSimulation().  Do not invoke the base class's Entity::onSimulation() in that case.

    \see G3D::VisibleEntity, G3D::Widget
*/
class Entity : public ReferenceCountedObject {
public:
    
    /** Base class for Entity animation controllers.
        This is not intended as a general or extensible simulation mechanism. 
        Override Entity::onSimulation for complex animation needs.
    
        Note that Light%s and Camera%s can be attached to other Entity%s that
        have complex onSimulation without subclassing the Light or Camera
        if desired.

        \sa Manipulator, Entity::onSimulation, Spline
    */
    class Track : public ReferenceCountedObject {
    protected:

        /** For "with" expressions */
        class VariableTable {
        protected:
            Table<String, shared_ptr<Track> > variable;

            /** nullptr in the root environment */
            const VariableTable* parent;
        public:

            VariableTable() : parent(nullptr) {}
            VariableTable(const VariableTable* p) : parent(p) {}

            void set(const String& id, const shared_ptr<Track>& val);

            /** Returns nullptr if not found */
            shared_ptr<Track> operator[](const String& id) const;
        };

        static shared_ptr<Track> create(Entity* entity, Scene* scene, const Any& a, const VariableTable& table);

    public:
    
        /** 
        \brief Creates any of the subclasses described/nested within \a a.

        Grammar:
        \verbatim
        ctrl := 
           "Point3(...)" |
           "CFrame..." |
           "Matrix3..." |
           "PhysicsFrameSpline { ... }" |
           "timeShift( splineTrack or orbitTrack, deltaTime)" |

           // Use the current position of this entity; creates a depends-on relationship.
           // optional cframe is equivalent to the common idiom "transform(entity(...), cframe)"
           "entity(\"entityname\" [, cframe:<CFrame>])" |   

           // use a * b
           "transform( a:<ctrl>, b:<ctrl> )" |

           // attach to <ctrl> by a spring; creates a depends-on relationship...reserved for future use
           "follow( <ctrl>, ? )" |  

           // Rotate to base to look at target's origin
           "lookAt( base:<ctrl>, target:<ctrl> [, up:<Vector3>] )" |

           // Strip the rotation from one track and the translation from another
           "combine(rot:<ctrl>, trans:<ctrl>)" |

           // Rotate around Y axis, facing forward on the track (use combine or transform to alter, use radius 0 to spin in place)
           "orbit(radius, period)" | 

           // name to be bound by a WITH expression
           "<id>" |

           // Bind the ids to controllers (in the previous variable environment), and then evaluate another one
           "with({ [<id> = <ctrl>]* }, <ctrl> )"
        \endverbatim

         Example:

         \htmlonly
         <pre>
            VisibleEntity {
               ...
               controller = 
                  with({
                        target = entity("mothership");
                        spline = PhysicsFrameSpline { .... };
                       },

                       // Lead the target by looking ahead of it in object space
                       lookAt(spline, transform(target, Matrix4::translation(0, 0, -3)), Vector3::unitY())
                   );
            }
         </pre>
         \endhtmlonly

         \param entity Needed to create dependencies in the scene
        */
        static shared_ptr<Track> create(Entity* entity, Scene* scene, const Any& a);

        /**
         \param time Absolute simulation time
         */
        virtual CFrame computeFrame(SimTime time) const = 0;
    };

  

    class SplineTrack : public Entity::Track {
    protected:
        friend class Entity::Track;
        friend class Entity;

        PhysicsFrameSpline          m_spline;
        bool                        m_changed;

        SplineTrack() : m_changed(false) {}
        SplineTrack(const Any& a) : m_spline(a), m_changed(false) {}

    public:

        static shared_ptr<SplineTrack> create(const PhysicsFrameSpline& s = PhysicsFrameSpline()) {
            const shared_ptr<SplineTrack>& p = createShared<SplineTrack>();
            p->m_changed = true;
            p->m_spline = s;
            return p;
        }

        virtual CFrame computeFrame(SimTime time) const override {
            return m_spline.evaluate(float(time));
        }

        const PhysicsFrameSpline& spline() const {
            return m_spline;
        }

        void setSpline(const PhysicsFrameSpline& spline) {
            m_changed = true;
            m_spline = spline;
        }

        /** True if setSpline was ever invoked */
        bool changed() const {
            return m_changed;
        }
    };
    

    class EntityTrack : public Entity::Track {
    protected:
        const String            m_entityName;
        CFrame                  m_childFrame;
        Scene*                  m_scene;

        EntityTrack(Entity* entity, const String& n, Scene* scene, const CFrame& frame);

    public:

        /** The entity cannot be changed once the track is created, but the relative frame may be changed */ 
        const String& entityName() const;

        const CFrame& childFrame() const;

        void setChildFrame(const CFrame& frame);
        
        static shared_ptr<EntityTrack> create(Entity* entity, const String& nameOfEntityToFollow, Scene* scene, const CFrame& childFrame = CFrame());

        virtual CFrame computeFrame(SimTime time) const override;
    };

protected:
    
    String                          m_name;

    Scene*                          m_scene;

    /** Current position.  Do not directly mutate--invoke setFrame()
        to ensure that times are modified correctly. */
    CoordinateFrame                 m_frame;

    /** Frame before onSimulation().  Used for tracking poses and for velocity estimation. */
    CoordinateFrame                 m_previousFrame;

    /** The Any from which this was originally constructed */
    Any                             m_sourceAny;

    /** Basic simulation behavior for the Entity. If nullptr, the Entity is never moved by the base class' onSimulation method.
        You can subclass Track, but it is usually easier to subclass Entity and override onSimulation directly when
        creating behaviors more complex than those supported by the default Track language. */
    shared_ptr<Track>               m_track;

    float                           m_mass = 1;
    bool                            m_physicalSimulation = false;
    bool                            m_canCauseCollisions = false;

    /** True if the frame has changed since load.  Used by toAny() to
        decide if m_sourceAny is out of date. */
    bool                            m_movedSinceLoad;

    /** Has this Entity been explicitly moved by setFrame() since it was last simulated?
        If so, onSimulation will not update the m_previousFrame because it assumes some
        external logic is moving the Entity. */
    bool                            m_movedSinceSimulation = false;
    
    //////////////////////////////////////////////

    /** Bounds at the last pose() call, in world space. */ 
    AABox                           m_lastAABoxBounds;

    /** Bounds at the last pose() call, in object space. */ 
    AABox                           m_lastObjectSpaceAABoxBounds;

    /** Bounds at the last pose() call, in world space. */ 
    Box                             m_lastBoxBounds;

    /** Bounds on all of the surfaces from the last pose() call, in world space. */
    Array<Box>                      m_lastBoxBoundArray; 

    /** Bounds at the last pose() call, in world space. */ 
    Sphere                          m_lastSphereBounds;

    /** Time at which the bounds were computed */
    RealTime                        m_lastBoundsTime;

    RealTime                        m_lastChangeTime;

    /** If true, the canChange() method returns true.
        Defaults to true.

        It is illegal to set m_canChange to false if m_frameSpline
        has more than one control point because a spline implies animation.

        Subclasses should set this to false during initialization
        if the object will never move so that other classes can 
        precompute data structures that are affected by the Entity.
      */
    bool                            m_canChange;

    /** \copydoc shouldBeSaved */
    bool                            m_shouldBeSaved;

    /** Construct an entity, m_framSplineChange defaults to false */
    Entity();
    
    /** 
       The initialization sequence for Entity and its subclasses is different
       than for typical C++ classes.  That is because they must avoid throwing
       exceptions from a constructor when parsing, need to support both AnyTableReader
       and direct parameter versions, and have to verify that all fields from an
       AnyTableReader are actually consumed.  See samples/entity and G3D::VisibleEntity
       for examples of how to initialize an Entity subclass.
    
       \param name The name of this Entity, e.g., "Player 1"

       \param propertyTable The form is given below.  It is intended that
       subclasses replace the table name and add new fields.
       \verbatim
       <some base class name> {
           model     = <modelname>;
           frame     = <initial CFrame or equivalent; overriden if a controller is present>
           track     = <see Entity::Track>;
           canChange = <boolean>
       }
       \endverbatim
       - The pose field is optional.  The Entity base class reads this
         field.  Other subclasses read their own fields.
       - The original caller (typically, a Scene subclass createEntity
         or Entity subclass create method) should invoke
         AnyTableReader::verifyDone to ensure that all of the fields
         specified were read by some subclass along the inheritance
         chain.
       - See VisibleEntity::init for an example of using this method.
         This method is separate from the constructor solely because
         it will throw an exception on a parse error, which is
         dangerous to do during a constructor because the destructor
         will not run.

       \param scene May be nullptr so long as no entity() controller is used
     */
     void init
       (const String&               name,
        Scene*                      scene,
        AnyTableReader&             propertyTable);

     void init
       (const String&               name,
        Scene*                      scene,
        const CFrame&               frame,
        const shared_ptr<Track>&    controller,
        bool                        canChange,
        bool                        shouldBeSaved);

public:

    /** This sets the position of the Entity for the current simulation step.
        If there is a controller set and the base class Entity::onSimulation
        is invoked, it will override the value assigned here.
     */
    virtual void setFrame(const CFrame& f, bool updatePreviousFrame = true);

    /** Current position, i.e., as of last onSimulation call */
    const CoordinateFrame& frame() const {
        return m_frame;
    }

    /** True if this Entity can change (when not Scene::editing(); all
        objects can change when in editing mode).  

        It is safe to build static data structures over Entity%s that
        cannot change.
      */
    bool canChange() const {
        return m_canChange;
    }

    /** Explicitly override the previous frame value used for computing motion vectors.
        This is very rarely needed because simulation automatically updates this value. */
    virtual void setPreviousFrame(const CFrame& f) {
        m_previousFrame = f;
    }

    const CoordinateFrame& previousFrame() const {
        return m_previousFrame;
    }

    const String& name() const {
        return m_name;
    }

    /**
       True if this Entity should be saved when the scene is converted to Any for saving/serialization.
       Defaults to true.  Set to false for transient objects.  For example, a character's spawn point
       Entity might have shouldBeSaved() = true, while the character itself might have shouldBeSaved() = false.
       This would allow editing of the scene while the simulation loop is running without having the 
       scene at the end of the editing session reflecting the result of the character moving about.
     */    
    bool shouldBeSaved() const {
        return m_shouldBeSaved;
    }

    void setShouldBeSaved(bool b) {
        m_shouldBeSaved = b;
    }

    /** 
        If there is a controller on this object and it is a SplineTrack,
        mutate it to store this value.  Otherwise, create a new SplineTrack
        to overwrite the current controller.

        \sa setTrack

        Used by SceneEditorWindow */
    virtual void setFrameSpline(const PhysicsFrameSpline& spline);

    /** \sa Entity::Track */
    virtual void setTrack(const shared_ptr<Track>& c) {
        m_track = c;
    }       

    /** Set to a negative number to tell the physics system to use the absolute value as a 
        density and infer the mass from the mesh on load. Default is -10 (kg/m^3). */ 
    virtual void setMass(float m) {
        m_mass = m;
    }

    float mass() const {
        return m_mass;
    }

    /** If true, this object should receive physical simulation for its motion.    
       If true, canMove must also be true and there must be no Track on this Entity. */
    virtual void setPhysicalSimulation(bool s) {
        m_physicalSimulation = s;
    }

    bool physicalSimulation() const {
        return m_physicalSimulation;
    }

    /** If true, causes collisions for other objects during physical simulation.
        It may not react to those collisions if physicalSimulation is false. 
        Default is true for VisibleEntity and false for other subclasses. */
    virtual void setCanCauseCollisions(bool c) {
        m_canCauseCollisions = c;
    }

    bool canCauseCollisions() const {
        return m_canCauseCollisions;
    }

    /** \sa Entity::Track */
    virtual shared_ptr<Track> track() const {
        return m_track;
    }       

    /** Converts the current Entity to an Any.  Subclasses should
        modify at least the name of the Table, which will be "Entity"
        if not changed. 
       
       \sa shouldBeSaved
      */
    virtual Any toAny(const bool forceAll = false) const;

    /** 
        \brief Physical simulation callback.

        The default implementation animates the model pose (by calling
        simulatePose()) and moves the frame() along m_frameSpline.

        If setFrame() has been called since the last onSimulation call, then
        the previous frame is not updated to the current frame by onSimulation
        because it assumes some external logic is maintaining the Entity's position.

        \param deltaTime The change in time since the previous call to onSimulation. Two values are special:
         0 means that simulation is paused and time should not advance.  As much as possible, all state should
         remain unchanged. For example, anything computed by differentials such as velocity should remain at its
         current value (rather than becoming infinite!).  In particular, this allows freeze-frame rendering of 
         motion blur.  The default implementation leaves m_previousFrame and the previous pose unchanged.
         A value of nan() means that the time has been changed discontinuously, for example, by
         Scene::setTime.  In this case, all state should update to the new absolute time and differentials can
         be approximated as zero (or whatever other result is reasonable for this Entity).

         \sa Scene::setOrder
     */
    virtual void onSimulation(SimTime absoluteTime, SimTime deltaTime);

    /** Pose as of the last simulation time */
    virtual void onPose(Array< shared_ptr<class Surface> >& surfaceArray);

    /** Return a world-space bounding box array for all surfaces produced by this Entity as of the last call to onPose(). */
    virtual const Array<Box>& lastBoxBoundArray() const {
        return m_lastBoxBoundArray;
    }

    /** Return a world-space axis-aligned bounding box as of the last call to onPose(). */
    virtual void getLastBounds(class AABox& box) const;

    /** Return a world-space bounding sphere as of the last call to onPose(). */
    virtual void getLastBounds(class Sphere& sphere) const;
    
    /** Return a world-space bounding box as of the last call to onPose(). 
        This is always at least as tight as the AABox bounds and often tighter.*/
    virtual void getLastBounds(class Box& box) const;

    /** Creates a new G3D::SoundEntity attached to this Entity at \a childFrame by an Entity::EntityTrack and returns it.
        The new SoundEntity is automatically added to the Scene and will remain rigidly attached to the Entity.

        You do not need to retain the pointer to the created sound for it to remain playing and in the scene.
        The SoundEntity will automatically remove itself from the Scene when it stops playing.
        
        If \a attach == false, then the EntityTrack is not created and the sound will remain fixed in space.
      */
    virtual shared_ptr<SoundEntity> playSound(const shared_ptr<Sound>& sound, const CFrame& childFrame = CFrame(), bool attach = true);

    /** Returns true if there is conservatively some intersection
        with the object's bounds closer than \a maxDistance to the
        ray origin.  If so, updates maxDistance with the intersection distance.
        
        The bounds used may be more accurate than any of the given
        getLastBounds() results because the method may recurse into
        individual parts of the scene graph within the Entity. */
    virtual bool intersectBounds(const Ray& R, float& maxDistance, Model::HitInfo& info = Model::HitInfo::ignore) const;

    virtual bool intersect(const Ray& R, float& maxDistance, Model::HitInfo& info = Model::HitInfo::ignore) const;

    /** Wall-clock time at which this entity changed in some way,
        e.g., that might require recomputing a spatial data
        structure. */
    RealTime lastChangeTime() const {
        return m_lastChangeTime;
    }

    /** Sets the lastChangeTime() to the current System::time() */
    void markChanged() {
        m_lastChangeTime = System::time();
    }

    /** Called by Scene::visualize every frame. 
        During this, Entitys may make rendering calls according to the SceneVisualizationSettings to display
        control points and other features. 

        \param isSelected True if this entity is selected by a sceneEditorWindow, which may trigger additional
        visualization for it.
      */
    virtual void visualize(RenderDevice* rd, bool isSelected, const class SceneVisualizationSettings& s, const shared_ptr<GFont>& font, const shared_ptr<Camera>& camera);

    /** \brief Create a user interface for controlling the properties of this Entity. 
        Called by SceneEditorWindow on selection.
        
        \param app May be nullptr.*/
    virtual void makeGUI(class GuiPane* pane, class GApp* app);

    /** Returns true if the world space bounds of these two entities overlap.
    
        \sa lastBoxBoundArray */
    bool lastBoxBoundArraysOverlap(const shared_ptr<Entity>& other) const;
};

}
