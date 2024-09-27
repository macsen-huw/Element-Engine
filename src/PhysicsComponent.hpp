#pragma once

#define dSINGLE

#include "Component.hpp"
#include "Entity.hpp"
#include "Model.hpp"
#include "glm.hpp"
#include <ode/ode.h>





namespace elmt {

    class Model;
    class BVH;

    //Primitives that models could be
    enum PhysicsPrimitives {
        PRIM_RAY,
        PRIM_PLANE,
        PRIM_SPHERE,
        PRIM_BOX,
        PRIM_CAPSULE,
        PRIM_CYLINDER,
        PRIM_TRIMESH,
        PRIM_CONVEX,
        PRIM_HEIGHTFIELD
    };

    //Types of joint
    enum JointTypes {
        BALL,
        HINGE,
        SLIDER,
        UNIVERSAL,
        FIX,
        PISTON,
        TRANSMISSION
    };

    //The surface parameters from the ODE Manual
    struct ContactParams{
        int mode = 0;                         //Contact flags

            //Note that these only those required by the flag need to be filled in

            dReal mu = 0.0;                         //Friction coefficients
            dReal rho = 0.0;                        //Rolling friction coefficient
            dReal bounce = 0.0, bounce_vel = 0;     //Restitution (bounciness)
            dReal soft_erp = 0.0, soft_cfm = 0.0;   //Contact normal "softness" parameter
            dReal motion = 0.0;                     //Surface velocity in friction
            dReal slip = 0.0;                       //Coefficients of force-dependent-slip
    };

    // Forward decl
    class PhysicsComponent;
    // Extended wrapper over ODE dContactGeom, used for callbacks
    struct BodyContact {
        glm::vec3 pos;
        glm::vec3 normal;
        float depth;
        dGeomID g1, g2;
        PhysicsComponent* p1;
        PhysicsComponent* p2;
    };



    class PhysicsComponent :
        public Component
    {


    public:
        // Methods
        PhysicsComponent(); //only used for serialisation
        PhysicsComponent(const char* name, Entity* entity, int shape, Model* model, bool doAddToCollisionManager);
        PhysicsComponent(const char* name, Entity* entity, int shape, Model* model);
        virtual ~PhysicsComponent();

        //Set the mass for the total body
        void setMass(dReal total_mass);
        dReal getMass();

        void addGeom(Mesh &mesh);
        void setGeomMass(dGeomID geom, dReal total_mass);

        void setPosition(glm::vec3 pos);
        glm::vec3 getPosition();

        void setRotationMatrix(glm::mat4x4 rot);
        glm::mat4x4 getRotationMatrix();

        void setLinearVelocity(dReal x, dReal y, dReal z);
        void setLinearVelocity(glm::vec3 vel) {setLinearVelocity(vel.x, vel.y, vel.z); }
        glm::vec3 getLinearVelocity();

        void setAngularVelocity(dReal x, dReal y, dReal z);
        void setAngularVelocity(glm::vec3 vel) { setAngularVelocity(vel.x, vel.y, vel.z); }
        glm::vec3 getAngularVelocity();

        void applyForce(dReal x, dReal y, dReal z);
        void applyForce(glm::vec3 force) { applyForce(force.x, force.y, force.z); }
        void applyTorque(dReal x, dReal y, dReal z);

        void setLinearDamping(dReal scale);
        dReal getLinearDamping();

        void setAngularDamping(dReal scale);
        dReal getAngularDamping();

        void setKinematic();
        void setDynamic();

        dBodyID getBody();
        std::vector<dGeomID> &getGeom();

        void enableGeom(dGeomID geom);
        void disableGeom(dGeomID geom);
        bool isGeomEnabled(dGeomID);

        void enableCollisions();
        void disableCollisions();
        bool isCollisionEnabled();

        void addToCollisionManager();

        size_t getBVHID() { return bvhID; }
        BVH* getBVH();

        virtual bool Update();

        virtual void clone(Component*& clonePointer, Entity* entityToAttach);

        bool getIsDynamic() { return isDynamic; }

        void attachToBody(dBodyID attachedBody, int jointType);

        //Edit contact parameters
        void setContactMode(dReal mode);
        void setFriction(dReal friction);
        void setRollingFriction(dReal rollFriction);
        void setBounciness(dReal bounce);
        void setBouncinessVelocity(dReal bounceVelocity);
        void setSoftERP(dReal soft);
        void setSoftCFM(dReal soft);
        void setSurfaceVelocity(dReal surfaceVel);
        void setSlip(dReal slip);

        ContactParams getContactParams();

       

        // Called when collision occurs. Only called once per frame per body collided with
        bool (*callbackCollide)(PhysicsComponent* p1, PhysicsComponent* p2, BodyContact& contactInfo) = nullptr;

        //const std::vector<PhysicsComponent*>& getCollidedBodies() { return collidedBodies; };
        // Check whether a body has collided with this body since last frame
        bool collidedWithBody(PhysicsComponent* comp);
    private:

        // Properties
    private:

        float grav;

        dBodyID bodyID;
        std::vector<dGeomID> geomIDs;

        //TODO replace with dBodyIsKinematic?
        //Whether this body is kinematic or dynamic
        bool isDynamic = false;
        //Whether this component has been added to the collision manager
        bool inCollisionManager = false;

        
        glm::vec3 customVelocity = glm::vec3(0, 0, 0);

        //Dimensions of the body
        float width = 0;
        float height = 0;
        float depth = 0;

        int shape;

        //Mass of object
        dMass mass;

        //Parameters when colliding other objects
        ContactParams contactParams;

        //Info used for cloning
        Model* model;
        BVH *bvh;

        friend class CollisionManager;
        size_t bvhID;

        glm::vec3 angularMomentum;
        glm::vec3 linearMomentum;
        glm::vec3 newPos;
        glm::vec3 oldPos;

        bool overrideEntityPos = false;

        bool timeFlag = false;
        std::chrono::milliseconds prevTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        );

    public:
        //What bodies (PhysicsComponents) this physics component has collided with this frame
        glm::vec3 calculateLinearVelocity();
        glm::vec3 calculateAngularVelocity();

        void setNewPos(glm::vec3 newPos);
        glm::vec3 getNewPos();
        size_t numContacts = 0;

        std::vector<PhysicsComponent*> collidedBodies{};

        bool customCollision = false;

        void setOverrideEntityPos(bool val);
        bool getOverrideEntityPos() { return overrideEntityPos; }
    };

}

