#include "BulletRigidbodySim.h"
#include "Bullet\src\LinearMath\btSerializer.h"
#include "Bullet\Extras\Serialize\BulletWorldImporter\btBulletWorldImporter.h"

CBulletRigidbodySim::CBulletRigidbodySim(void) : m_BulletFileLoader(NULL)
{
	m_pColConfiguration = new btDefaultCollisionConfiguration();
	m_pColDispatcher = new btCollisionDispatcher(m_pColConfiguration);
	m_pBroadphase = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	m_pConstraintSolver = solver;
	m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pColDispatcher, m_pBroadphase, m_pConstraintSolver, m_pColConfiguration);
	
	m_shapeDrawer = new GL_ShapeDrawer ();
}

CBulletRigidbodySim::~CBulletRigidbodySim(void)
{
	delete m_pDynamicsWorld;
	delete m_shapeDrawer;
	delete m_BulletFileLoader;	
	delete m_pColConfiguration;
	delete m_pColDispatcher;
	delete m_pBroadphase;
	delete m_pConstraintSolver;	
}

bool CBulletRigidbodySim::Create()
{
	//m_pDynamicsWorld->setGravity(btVector3(0, -10, 0));

	//// ground plane
	//{
	//	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(100.),btScalar(5.),btScalar(100.)));
	//	m_CollisionShapes.push_back(groundShape);

	//	btTransform groundTransform;
	//	groundTransform.setIdentity();
	//	groundTransform.setOrigin(btVector3(0,-50,0));

	//	btScalar mass(0);
	//	btVector3 localInertia(0, 0, 0);

	//	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	//	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
	//	btRigidBody* body = new btRigidBody(rbInfo);

	//	m_pDynamicsWorld->addRigidBody(body);
	//}

	//// boxes
	//{
	//	btCollisionShape* boxShape = new btBoxShape(btVector3(10.0, 10.0, 10.0));
	//	m_CollisionShapes.push_back(boxShape);

	//	btTransform boxTransform;
	//	boxTransform.setIdentity();
	//	boxTransform.setRotation(btQuaternion(btVector3(1.0, 1.0, 1.0), 45.0));

	//	btScalar mass(1.0f);
	//	btVector3 localInertia(0, 0, 0);

	//	boxShape->calculateLocalInertia(mass, localInertia);
	//	boxTransform.setOrigin(btVector3(0, 50, 0));

	//	btDefaultMotionState* myMotionState = new btDefaultMotionState(boxTransform);
	//	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, boxShape, localInertia);
	//	btRigidBody* body = new btRigidBody(rbInfo);

	//	m_pDynamicsWorld->addRigidBody(body);
	//}

	return true;
}

bool CBulletRigidbodySim::LoadBulletFile(const char* filePath)
{
	m_BulletFilePath = filePath;

	if ( m_BulletFileLoader )
		delete m_BulletFileLoader;

	if ( m_pDynamicsWorld )
		delete m_pDynamicsWorld;

	m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pColDispatcher, m_pBroadphase, m_pConstraintSolver, m_pColConfiguration);
	m_BulletFileLoader = new btBulletWorldImporter(m_pDynamicsWorld);

	return m_BulletFileLoader->loadFile(m_BulletFilePath.c_str());
}

bool CBulletRigidbodySim::ReloadBulletFile()
{
	return LoadBulletFile(m_BulletFilePath.c_str());
}

void CBulletRigidbodySim::Update()
{
	if ( m_pDynamicsWorld )
	{
		m_pDynamicsWorld->stepSimulation(0.0167f);
	}
}

void CBulletRigidbodySim::Render(int pass) const
{
	btScalar m[16];
	btMatrix3x3	rot;rot.setIdentity();
	const int numObjects = m_pDynamicsWorld->getNumCollisionObjects();
	btVector3 wireColor(1,0,0);

	for ( int i=0; i < numObjects; i++ )
	{
		btCollisionObject*	colObj=m_pDynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody*		body=btRigidBody::upcast(colObj);
		if(body&&body->getMotionState())
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
			myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			rot=myMotionState->m_graphicsWorldTrans.getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix(m);
			rot=colObj->getWorldTransform().getBasis();
		}
		btVector3 wireColor(1.f,1.0f,0.5f); //wants deactivation
		if(i&1) wireColor=btVector3(0.f,0.0f,1.f);
		///color differently for active, sleeping, wantsdeactivation states
		if (colObj->getActivationState() == 1) //active
		{
			if (i & 1)
			{
				wireColor += btVector3 (1.f,0.f,0.f);
			}
			else
			{			
				wireColor += btVector3 (.5f,0.f,0.f);
			}
		}
		if(colObj->getActivationState()==2) //ISLAND_SLEEPING
		{
			if(i&1)
			{
				wireColor += btVector3 (0.f,1.f, 0.f);
			}
			else
			{
				wireColor += btVector3 (0.f,0.5f,0.f);
			}
		}

		btVector3 aabbMin,aabbMax;
		m_pDynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);
		
		aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
		aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
//		m_pDynamicsWorld->getDebugDrawer()->drawAabb(aabbMin,aabbMax,btVector3(1,1,1));

		btVector3 m_sundirection(btVector3(1,-2,1)*1000);

		{
			switch(pass)
			{
			case	0:	m_shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor,0,aabbMin,aabbMax);break;
			case	1:	m_shapeDrawer->drawShadow(m,m_sundirection*rot,colObj->getCollisionShape(),aabbMin,aabbMax);break;
			case	2:	m_shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor*btScalar(0.3),0,aabbMin,aabbMax);break;
			}
		}
	}
}