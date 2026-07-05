#include "star/physics/physics_world.hpp"

#include <algorithm>
#include <thread>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

using namespace JPH;

namespace star::physics {
    namespace object_layers {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer COUNT = 2;
    } // namespace object_layers

    namespace broad_phase_layers {
        static constexpr BroadPhaseLayer NON_MOVING{0};
        static constexpr BroadPhaseLayer MOVING{1};
        static constexpr uint COUNT = 2;
    } // namespace broad_phase_layers

    class BroadPhaseMapping final : public BroadPhaseLayerInterface {
      public:
        uint GetNumBroadPhaseLayers() const override {
            return broad_phase_layers::COUNT;
        }

        BroadPhaseLayer GetBroadPhaseLayer(const ObjectLayer layer) const override {
            return layer == object_layers::NON_MOVING ? broad_phase_layers::NON_MOVING : broad_phase_layers::MOVING;
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(BroadPhaseLayer) const override {
            return "PhysicsLayer";
        }
#endif
    };

    class ObjectVsBroadPhaseFilter final : public ObjectVsBroadPhaseLayerFilter {
      public:
        bool ShouldCollide(const ObjectLayer object, const BroadPhaseLayer broad_phase) const override {
            if (object == object_layers::NON_MOVING)
                return broad_phase == broad_phase_layers::MOVING;
            return true;
        }
    };

    class ObjectLayerFilter final : public ObjectLayerPairFilter {
      public:
        bool ShouldCollide(const ObjectLayer a, const ObjectLayer b) const override {
            if (a == object_layers::NON_MOVING)
                return b == object_layers::MOVING;
            return true;
        }
    };

    class JoltInitGuard {
      public:
        JoltInitGuard() {
            if (s_refcount++ == 0) {
                RegisterDefaultAllocator();
                Factory::sInstance = new Factory();
                RegisterTypes();
            }
        }

        ~JoltInitGuard() {
            if (--s_refcount == 0) {
                UnregisterTypes();
                delete Factory::sInstance;
                Factory::sInstance = nullptr;
            }
        }

      private:
        static inline int s_refcount = 0;
    };

    struct PhysicsWorld::Impl {
        JoltInitGuard jolt_guard;

        BroadPhaseMapping broad_phase_mapping;
        ObjectVsBroadPhaseFilter object_vs_broad_phase_filter;
        ObjectLayerFilter object_layer_filter;

        TempAllocatorImpl temp_allocator{32 * 1024 * 1024};
        JobSystemThreadPool job_system{cMaxPhysicsJobs, cMaxPhysicsBarriers,
                                       std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1)};
        PhysicsSystem physics_system;

        explicit Impl(const math::Vector3& gravity) {
            constexpr uint max_bodies = 65536;
            constexpr uint num_body_mutexes = 0;
            constexpr uint max_body_pairs = 65536;
            constexpr uint max_contact_constraints = 10240;

            physics_system.Init(max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints,
                                broad_phase_mapping, object_vs_broad_phase_filter, object_layer_filter);
            physics_system.SetGravity(Vec3(gravity.x, gravity.y, gravity.z));
        }
    };

    PhysicsWorld::PhysicsWorld(const Vector3& gravity) : m_impl(std::make_unique<Impl>(gravity)) {}

    PhysicsWorld::~PhysicsWorld() = default;

    void PhysicsWorld::step(const f32 dt) {
        constexpr int collision_steps = 1;
        m_impl->physics_system.Update(dt, collision_steps, &m_impl->temp_allocator, &m_impl->job_system);
    }

    BodyHandle PhysicsWorld::create_body(const BodyDesc& desc) const {
        RefConst<Shape> shape;
        if (desc.shape == ColliderShape::Sphere)
            shape = new SphereShape(desc.radius);
        else
            shape = new BoxShape(Vec3(desc.half_extents.x, desc.half_extents.y, desc.half_extents.z));

        EMotionType motion = EMotionType::Dynamic;
        switch (desc.motion) {
            case MotionType::Static:
                motion = EMotionType::Static;
                break;
            case MotionType::Kinematic:
                motion = EMotionType::Kinematic;
                break;
            case MotionType::Dynamic:
                motion = EMotionType::Dynamic;
                break;
        }

        const ObjectLayer layer = desc.motion == MotionType::Static ? object_layers::NON_MOVING : object_layers::MOVING;

        BodyCreationSettings settings(shape, RVec3(desc.position.x, desc.position.y, desc.position.z),
                                      Quat(desc.rotation.x, desc.rotation.y, desc.rotation.z, desc.rotation.w), motion,
                                      layer);
        settings.mFriction = desc.friction;
        settings.mRestitution = desc.restitution;

        BodyInterface& bodies = m_impl->physics_system.GetBodyInterface();
        const BodyID id = bodies.CreateAndAddBody(settings, motion == EMotionType::Static ? EActivation::DontActivate
                                                                                          : EActivation::Activate);
        return id.GetIndexAndSequenceNumber();
    }

    void PhysicsWorld::remove_body(const BodyHandle handle) const {
        if (handle == INVALID_BODY)
            return;
        const BodyID id(handle);
        BodyInterface& bodies = m_impl->physics_system.GetBodyInterface();
        bodies.RemoveBody(id);
        bodies.DestroyBody(id);
    }

    bool PhysicsWorld::body_transform(const BodyHandle handle, Vector3& out_position, Quaternion& out_rotation) const {
        if (handle == INVALID_BODY)
            return false;

        const BodyID id(handle);
        const BodyInterface& bodies = m_impl->physics_system.GetBodyInterface();
        const RVec3 position = bodies.GetPosition(id);
        const Quat rotation = bodies.GetRotation(id);

        out_position = {position.GetX(), position.GetY(), position.GetZ()};
        out_rotation = {rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()};
        return true;
    }

    void PhysicsWorld::set_body_transform(const BodyHandle handle, const Vector3& position,
                                          const Quaternion& rotation) const {
        if (handle == INVALID_BODY)
            return;
        BodyInterface& bodies = m_impl->physics_system.GetBodyInterface();
        bodies.SetPositionAndRotation(BodyID(handle), RVec3(position.x, position.y, position.z),
                                      Quat(rotation.x, rotation.y, rotation.z, rotation.w), EActivation::Activate);
    }
} // namespace star::physics
