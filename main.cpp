// sample7.cpp  by Kosei Demura in 2005-2008
// My web site is http://demura.net
// This program uses the Open Dynamics Engine (ODE) by Russell Smith.
// The ODE web site is http://ode.org/

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#ifdef  dDOUBLE
#define dsDrawSphere dsDrawSphereD   // 単精度と倍精度の描画関数に対応するおまじない
#define dsDrawCapsule  dsDrawCapsuleD
#endif

static dWorldID world;
static dSpaceID space;  // add
static dGeomID  ground; // add
static dJointGroupID contactgroup; // add
dsFunctions fn;

typedef struct {
	dBodyID body;
	dGeomID geom;
	dReal   radius;
	dReal   length;
	dReal   mass;
} myLink;
myLink ball, pole;      // add

dJointID joint;


static void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
	const int N = 10;
	dContact contact[N];

	int isGround = ((ground == o1) || (ground == o2));

	int n = dCollide(o1, o2, N, &contact[0].geom, sizeof(dContact));
	if (isGround)  {
		for (int i = 0; i < n; i++) {
			contact[i].surface.mu = dInfinity;
			contact[i].surface.mode = dContactBounce;
			contact[i].surface.bounce = 1.0; // (0.0~1.0)
			contact[i].surface.bounce_vel = 0.01;
			dJointID c = dJointCreateContact(world, contactgroup, &contact[i]);
			dJointAttach(c, dGeomGetBody(contact[i].geom.g1),
				dGeomGetBody(contact[i].geom.g2));
		}
	}
}


static void simLoop(int pause)
{
	const dReal *pos1, *R1, *pos2, *R2;

	dSpaceCollide(space, 0, &nearCallback);  // add, Write this first

	dWorldStep(world, 0.01);

	dJointGroupEmpty(contactgroup); // add

	dsSetColor(1.0, 0.0, 0.0);
	// draw a ball
	pos1 = dBodyGetPosition(ball.body);
	R1 = dBodyGetRotation(ball.body);
	dsDrawSphere(pos1, R1, ball.radius);

	// draw a ccylinder
	pos2 = dBodyGetPosition(pole.body);
	R2 = dBodyGetRotation(pole.body);
	dsDrawCapsule(pos2, R2, pole.length, pole.radius);
}

void start()
{
	static float xyz[3] = { 0.0, -3.0, 1.0 };
	static float hpr[3] = { 90.0, 0.0, 0.0 };
	dsSetViewpoint(xyz, hpr);
}

void  prepDrawStuff() {
	fn.version = DS_VERSION;
	fn.start = &start;
	fn.step = &simLoop;
	fn.command = NULL;
	fn.stop = NULL;
	fn.path_to_textures = "../../drawstuff/textures";
}

// Create a ball and a pole
void createBallandPole() {
	dMass m1;
	dReal x0 = 0.0, y0 = 0.0, z0 = 2.5;

	// ball
	ball.radius = 0.2;
	ball.mass = 1.0;
	ball.body = dBodyCreate(world);
	dMassSetZero(&m1);
	dMassSetSphereTotal(&m1, ball.mass, ball.radius);
	dBodySetMass(ball.body, &m1);
	dBodySetPosition(ball.body, x0, y0, z0);

	ball.geom = dCreateSphere(space, ball.radius);
	dGeomSetBody(ball.geom, ball.body);

	// pole
	pole.radius = 0.025;
	pole.length = 1.0;
	pole.mass = 1.0;
	pole.body = dBodyCreate(world);
	dMassSetZero(&m1);
	dMassSetCapsule(&m1, pole.mass, 3, pole.radius, pole.length);
	dBodySetMass(pole.body, &m1);
	dBodySetPosition(pole.body, x0, y0, z0 - 0.5 * pole.length);

	pole.geom = dCreateCCylinder(space, pole.radius, pole.length);
	dGeomSetBody(pole.geom, pole.body);

	// hinge joint
	joint = dJointCreateHinge(world, 0);
	dJointAttach(joint, ball.body, pole.body);
	dJointSetHingeAnchor(joint, x0, y0, z0 - ball.radius);
	dJointSetHingeAxis(joint, 1, 0, 0);
}

void  setDrawStuff() {
	fn.version = DS_VERSION;
	fn.start = &start;
	fn.step = &simLoop;
	fn.command = NULL;
	fn.stop = NULL;
	fn.path_to_textures = "../drawstuff/textures";
}

int main(int argc, char **argv)
{
	setDrawStuff();
	dInitODE();
	world = dWorldCreate();
	space = dHashSpaceCreate(0);
	contactgroup = dJointGroupCreate(0);

	dWorldSetGravity(world, 0, 0, -9.8);

	// Create a ground
	ground = dCreatePlane(space, 0, 0, 1, 0);

	// create an object
	createBallandPole();

	dsSimulationLoop(argc, argv, 352, 288, &fn);

	dWorldDestroy(world);
	dCloseODE();

	return 0;
}
