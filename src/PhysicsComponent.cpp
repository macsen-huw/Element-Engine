#include "PhysicsComponent.hpp"
#include "CollisionManager.hpp"
#include "Logger.hpp"
#include "core.hpp"

using namespace elmt;

PhysicsComponent::PhysicsComponent() : Component()
{
	typeName = "PhysicsComponent";
}


PhysicsComponent::PhysicsComponent(const char* name, Entity* entity, int shape, Model* model, bool doAddToCollisionManager) : Component(name, entity), model(model)
{
    grav = 0;
	typeName = "PhysicsComponent";

	//Body -> the actual object itself
	//Geom -> the bounding box around the object
	bodyID = dBodyCreate(core::getPhysicsWorld()->getWorldID());

    assert(model);
    assert(model->bvh);
	//Get two opposite corners of the model
	glm::vec3 corner1 = model->bvh->root.b1;
	glm::vec3 corner2 = model->bvh->root.b2;

	//Store dimensions of model
	width = abs(corner1.x - corner2.x);
	height = abs(corner1.y - corner2.y);
	depth = abs(corner1.z - corner2.z);

	//Sync with Entity
	setPosition(entity->pos);
    newPos = glm::vec3(0, 0, 0);
    oldPos = entity->pos;
	this->shape = shape;

	//Add a geom for every mesh in the model
	for (Mesh& mesh : model->meshes) addGeom(mesh);

	//Disable all geoms by default
	disableCollisions();

	
	//TODO fix
	bvhID = 9999;
	if (doAddToCollisionManager) {
		addToCollisionManager();
	}

}

PhysicsComponent::PhysicsComponent(const char* name, Entity* entity, int shape, Model* model) :
	PhysicsComponent(name, entity, shape, model, true) {}

//Destroy body (at the end of the scope)
PhysicsComponent::~PhysicsComponent(){

	if (core::getIsSetup()) {
		core::getCollisionManager()->removePhysicsComponent(this);
		for (dGeomID& geom : geomIDs)
			dGeomDestroy(geom);

		dBodyDestroy(bodyID);
	}
}


void PhysicsComponent::addGeom(Mesh &mesh){
	float localMinWidth = mesh.vertices[0].Position.x, localMaxWidth = mesh.vertices[0].Position.x;
	float localMinHeight = mesh.vertices[0].Position.y, localMaxHeight = mesh.vertices[0].Position.y;
	float localMinDepth = mesh.vertices[0].Position.z, localMaxDepth = mesh.vertices[0].Position.z;

	for (Vertex &v : mesh.vertices){
        if (v.Position.x > localMaxWidth) localMaxWidth = v.Position.x;
		if (v.Position.x < localMinWidth) localMinWidth = v.Position.x;
		if (v.Position.y > localMaxHeight) localMaxHeight = v.Position.y;
		if (v.Position.y < localMinHeight) localMinHeight = v.Position.y;
		if (v.Position.z > localMaxDepth) localMaxDepth = v.Position.z;
		if (v.Position.z < localMinDepth) localMinDepth = v.Position.z;
	}

	float localWidth = localMaxWidth - localMinWidth;
	float localHeight = localMaxHeight - localMinHeight;
	float localDepth = localMaxDepth - localMinDepth;

	dGeomID geomID;

	//Get the shapes of the meshes to assign collision window - width, height, radius etc.
	if (shape == elmt::PRIM_BOX){
		geomID = dCreateBox(core::getPhysicsWorld()->getSpaceID(), localWidth, localHeight, localDepth);
	}

	else if (shape == elmt::PRIM_TRIMESH){

		dTriMeshDataID triMeshData = dGeomTriMeshDataCreate();

		dGeomTriMeshDataBuildSingle(triMeshData, mesh.verts.data(), sizeof(glm::vec3), mesh.verts.size(),
                                    mesh.indices.data(), mesh.indices.size(), sizeof(unsigned int) * 3);
		//dGeomTriMeshDataBuildSimple(triMeshData, vertices.data(), vertices.size() / 4, (dTriIndex*)mesh.indices.data(), mesh.indices.size());

		geomID = dCreateTriMesh(core::getPhysicsWorld()->getSpaceID(), triMeshData, nullptr, nullptr, nullptr);

		// Use this to see if our geometry is being set correctly
		/*
		auto triCount = dGeomTriMeshGetTriangleCount(geomID);
		std::vector<glm::vec4> triRecreate;
		for (int i = 0; i < triCount; i++){
			dVector3 v1, v2, v3;

			dGeomTriMeshGetTriangle(geomID, i, &v1, &v2, &v3);
			triRecreate.push_back(glm::vec4{ v1[0], v1[1], v1[2], v1[3]});
			triRecreate.push_back(glm::vec4{ v2[0], v2[1], v2[2], v2[3] });
			triRecreate.push_back(glm::vec4{ v3[0], v3[1], v3[2], v3[3] });
		}
		std::cout << triCount << std::endl;
		*/
	}

	//Assign geom to body
	dGeomSetBody(geomID, bodyID);

	geomIDs.emplace_back(geomID);
}

dBodyID PhysicsComponent::getBody()
{
	return bodyID;
}

std::vector<dGeomID> &PhysicsComponent::getGeom()
{
	return geomIDs;
}

//Enable/Disable singular geoms
void PhysicsComponent::enableGeom(dGeomID geom)
{		
	dGeomEnable(geom);
}

void PhysicsComponent::disableGeom(dGeomID geom)
{
	dGeomDisable(geom);
}

bool PhysicsComponent::isGeomEnabled(dGeomID geom)
{
	//All geom IDs will have the same state, so just test the first
	return dGeomIsEnabled(geom);
}

//Enable/Disable every geom in a body
void PhysicsComponent::enableCollisions()
{
  	for (dGeomID& geom : geomIDs)
	{
		dGeomEnable(geom);
	}
}

void PhysicsComponent::disableCollisions(){
	for (dGeomID& geom : geomIDs){
		dGeomDisable(geom);
	}
}

//All geom IDs will have the same state, so just test the first
bool PhysicsComponent::isCollisionEnabled(){
	return dGeomIsEnabled(geomIDs[0]);
}

void PhysicsComponent::addToCollisionManager(){
	if (!inCollisionManager) {
		core::getCollisionManager()->addPhysicsComponent(this);
		glm::mat4x4 identity(1.f);
        bvh = new BVH(model->meshes);
		bvhID = core::getCollisionManager()->addBVH(bvh, entity, this, identity, model->modelID);
	}
	else {
		Logger::Print("Attempted to add Physics Component \"" + name + "\" to Collision Manager, but it was already added (bvhID: " + std::to_string(bvhID) + ")", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_WARNING);
	}
}

void PhysicsComponent::setMass(dReal total_mass){
	//Create mass based on the entire mesh
	dMassSetBoxTotal(&mass, total_mass, width, height, depth);

	dBodySetMass(bodyID, &mass);
}

dReal PhysicsComponent::getMass(){
	return mass.mass;
}

//Set Mass
void PhysicsComponent::setGeomMass(dGeomID geom, dReal total_mass){

	dMass mass;
	dMassSetZero(&mass);

	if (shape == elmt::PRIM_BOX){
		dMassSetBoxTotal(&mass, total_mass, width, height, depth);
	}

	else if (shape == elmt::PRIM_TRIMESH){
		dMassSetTrimeshTotal(&mass, total_mass, geom);
	}
}


//Set Position
void PhysicsComponent::setPosition(glm::vec3 pos){
	// Sync entity position
	entity->pos = pos;
	dBodySetPosition(bodyID, pos.x, pos.y, pos.z);
}

//Get Position
glm::vec3 PhysicsComponent::getPosition(){
	const dReal* pos = dBodyGetPosition(bodyID);

	return glm::vec3{ pos[0], pos[1], pos[2] };
}

//Set Rotation
void PhysicsComponent::setRotationMatrix(glm::mat4x4 rotMatrixGLM){

	//Convert rotation matrix from glm::mat4x4 to dMatrix3
	dMatrix3 rotMatrix;

    glm::mat4x4 rot = glm::transpose(rotMatrixGLM);


	rotMatrix[0] = rot[0][0];
	rotMatrix[1] = rot[0][1];
	rotMatrix[2] = rot[0][2];
	rotMatrix[3] = 0;

	rotMatrix[4] = rot[1][0];
	rotMatrix[5] = rot[1][1];
	rotMatrix[6] = rot[1][2];
	rotMatrix[7] = 0;

	rotMatrix[8] = rot[2][0];
	rotMatrix[9] = rot[2][1];
	rotMatrix[10] = rot[2][2];
	rotMatrix[11] = 0;

	dBodySetRotation(bodyID, rotMatrix);
}


//Get Rotation
glm::mat4x4 PhysicsComponent::getRotationMatrix(){
	const dReal* rotMatrix = dBodyGetRotation(bodyID);

	//Convert dReal array to mat4x4
	glm::mat4x4 rot;
	rot[0][0] = rotMatrix[0];
	rot[0][1] = rotMatrix[1];
	rot[0][2] = rotMatrix[2];
	rot[0][3] = 0;

	rot[1][0] = rotMatrix[4];
	rot[1][1] = rotMatrix[5];
	rot[1][2] = rotMatrix[6];
	rot[1][3] = 0;

	rot[2][0] = rotMatrix[8];
	rot[2][1] = rotMatrix[9];
	rot[2][2] = rotMatrix[10];
	rot[2][3] = 0;

	rot[3][0] = 0;
	rot[3][1] = 0;
	rot[3][2] = 0;
	rot[3][3] = 1;


	return glm::transpose(rot);
}

//Set body's linear velocity
void PhysicsComponent::setLinearVelocity(dReal x, dReal y, dReal z){
	if (customCollision) {
		customVelocity = glm::vec3(x, y, z);
	}
	else {
		dBodySetLinearVel(bodyID, x, y, z);
	}
	
}

//Get body's linear velocity
glm::vec3 PhysicsComponent::getLinearVelocity(){
	if (customCollision) {
		const dReal* vel = dBodyGetLinearVel(bodyID);
		return glm::vec3{ vel[0], vel[1], vel[2] };
	}
	else {
		return customVelocity;
	}
	
}

//Set body's angular velocity
void PhysicsComponent::setAngularVelocity(dReal x, dReal y, dReal z){
	dBodySetAngularVel(bodyID, x, y, z);
}

//Get body's angular velocity
glm::vec3 PhysicsComponent::getAngularVelocity(){
	const dReal* vel = dBodyGetAngularVel(bodyID);
	return glm::vec3{ vel[0], vel[1], vel[2] };
}

//Apply force to body
void PhysicsComponent::applyForce(dReal x, dReal y, dReal z){
	dBodyAddForce(bodyID, x, y, z);
}

//Apply torque to body (i.e. applying a force to the rotation)
void PhysicsComponent::applyTorque(dReal x, dReal y, dReal z){
	dBodyAddTorque(bodyID, x, y, z);
}

//Get and Set the linear damping
void PhysicsComponent::setLinearDamping(dReal scale){
	dBodySetLinearDamping(bodyID, scale);
}

dReal PhysicsComponent::getLinearDamping(){
	return dBodyGetLinearDamping(bodyID);
}

//Get and set the angular damping
void PhysicsComponent::setAngularDamping(dReal scale){
	dBodySetAngularDamping(bodyID, scale);
}

dReal PhysicsComponent::getAngularDamping(){
	return dBodyGetAngularDamping(bodyID);
}


//Edit the contact parameters
void PhysicsComponent::setContactMode(dReal mode) { contactParams.mode = mode; }

void PhysicsComponent::setFriction(dReal friction){ contactParams.mu = friction; }

void PhysicsComponent::setRollingFriction(dReal rollFriction){ contactParams.rho = rollFriction; }

void PhysicsComponent::setBounciness(dReal bounce) { contactParams.bounce = bounce; }

void PhysicsComponent::setBouncinessVelocity(dReal bounceVelocity) { contactParams.bounce_vel = bounceVelocity; }

void PhysicsComponent::setSoftERP(dReal soft) { contactParams.soft_erp = soft; }

void PhysicsComponent::setSoftCFM(dReal soft) { contactParams.soft_cfm = soft; }

void PhysicsComponent::setSurfaceVelocity(dReal surfaceVel) { contactParams.motion = surfaceVel; }

void PhysicsComponent::setSlip(dReal slip) { contactParams.slip = slip; }

//Return contact parameters
ContactParams PhysicsComponent::getContactParams() { return contactParams; }

bool elmt::PhysicsComponent::collidedWithBody(PhysicsComponent* comp){
    return std::find(collidedBodies.begin(), collidedBodies.end(), comp) != collidedBodies.end();
}

void PhysicsComponent::setOverrideEntityPos(bool val)
{
	overrideEntityPos = val;
	if (!overrideEntityPos) {
		setPosition(entity->pos);
	}
}


//Set dynamic body i.e. infinite mass (wall, floor)
void PhysicsComponent::setDynamic(){
	isDynamic = true;
	dBodySetDynamic(bodyID);
}

void PhysicsComponent::setKinematic(){
	isDynamic = false;
	dBodySetKinematic(bodyID);
}

BVH* PhysicsComponent::getBVH()
{
	return core::getCollisionManager()->getBVH(this);
}

bool PhysicsComponent::Update() {
	glm::mat4x4 translationMatrix;
	if (overrideEntityPos) {
		entity->pos = getPosition();
		entity->rotation = getRotationMatrix();

		//Get transformation matrix (translation * rotation matrix)
		translationMatrix = glm::mat4x4(1.f);
		translationMatrix[3][0] = entity->pos.x;
		translationMatrix[3][1] = entity->pos.y;
		translationMatrix[3][2] = entity->pos.z;
	}
	else {
		if ( glm::length(entity->posChange) ) {
			setPosition(entity->pos);
		}
		/*if ( entity->rotChange != glm::mat4(1.0) ) {
			setRotationMatrix(entity->rotation);
		}*/

		
		glm::vec3 thisPos = getPosition();
		translationMatrix = glm::mat4x4(1.f);
		translationMatrix[3][0] = thisPos.x;
		translationMatrix[3][1] = thisPos.y;
		translationMatrix[3][2] = thisPos.z;
		
	}
	
	glm::mat4x4 transformationMatrix = translationMatrix * entity->rotation;
	transformationMatrix[0][0] = entity->scale.x;
	transformationMatrix[1][1] = entity->scale.y;
	transformationMatrix[2][2] = entity->scale.z;




    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            assert(!isnan(translationMatrix[i][j]));
            assert(!isnan(getRotationMatrix()[i][j]));
        }
    }

    float time;
    if (!timeFlag){
        timeFlag = true;
        time = 1.f / 60.f;
    }
    else{
        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        );
        auto diff = currentTime.count() - prevTime.count();
        time = (float) diff / 1000.f;
    }

    if (customCollision){
        glm::vec3 shift = core::getCollisionManager()->simulateEntity(this, 1.0/60.0f );//elmt::core::getDeltaTime()
//
////        if (glm::normalize(getLinearVelocity().y) > 0.3 && numContacts > 0){
////            if (fabs(shift.y) < 0.01){
////                setPosition(oldPos);
////            }
////        }
//
        /*if (glm::distance(getPosition() + shift, oldPos) < 0.01 && numContacts > 0){
            setPosition(oldPos);
        }
        else setPosition(getPosition() + shift);*/
		//shift = customVelocity * ;
		setPosition(getPosition() + shift);// (shift);
	}

   /* else{
        entity->rotation = getRotationMatrix();
    }*/

    //TODO improve
	if ( bvhID != 9999) {
		core::getCollisionManager()->updateTransform(transformationMatrix, bvhID);
	}

	if ( inCollisionManager && isCollisionEnabled() != entity->getInTree() ) {
		// Update collisions
		if (entity->getInTree()) {
			enableCollisions();
		} else {
			disableCollisions();
		}
	}

	collidedBodies.clear();

    prevTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );

    oldPos = getPosition();
    numContacts = 0;
    return true;

}

//Attach the physics component to another body
void PhysicsComponent::attachToBody(dBodyID attachedBody, int jointType){
	dJointID joint;
	
	if (jointType == BALL)
		joint = dJointCreateBall(core::getPhysicsWorld()->getWorldID(), core::getPhysicsWorld()->getGeneralGroupID());
	else if (jointType == HINGE)
		joint = dJointCreateHinge(core::getPhysicsWorld()->getWorldID(), core::getPhysicsWorld()->getGeneralGroupID());
	else if (jointType == SLIDER)
		joint = dJointCreateHinge(core::getPhysicsWorld()->getWorldID(), core::getPhysicsWorld()->getGeneralGroupID());
	else if (jointType == UNIVERSAL)
		joint = dJointCreateUniversal(core::getPhysicsWorld()->getWorldID(), core::getPhysicsWorld()->getGeneralGroupID());
	else if (jointType == PISTON)
		joint = dJointCreatePiston(core::getPhysicsWorld()->getWorldID(), core::getPhysicsWorld()->getGeneralGroupID());
	else{
		std::printf("ERROR: Invalid joint type\n");
		return;
	}

	dJointAttach(joint, bodyID, attachedBody);
}

void PhysicsComponent::clone(Component*& clonePointer, Entity* entityToAttach){
	clonePointer = new PhysicsComponent(name.c_str(), entityToAttach, shape, model, inCollisionManager);
	Logger::Print("Cloned PhysicsComponent " + name + ", UUID " + clonePointer->getName(), LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
}

void PhysicsComponent::setNewPos(glm::vec3 newPos) {
    this->newPos += newPos;
    numContacts++;
}


glm::vec3 PhysicsComponent::getNewPos() {
    if (numContacts == 0) return newPos;
    glm::vec3 ans = newPos / float(numContacts);
    newPos = glm::vec3(0, 0, 0);
    return ans;
}











