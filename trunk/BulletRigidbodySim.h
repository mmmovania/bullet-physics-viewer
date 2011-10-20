#pragma once

#include "Bullet\src\btBulletCollisionCommon.h"
#include "Bullet\src\btBulletDynamicsCommon.h"
#include "GL_ShapeDrawer.h"
#include <string>

class CBulletRigidbodySim
{
public:
	CBulletRigidbodySim(void);
	~CBulletRigidbodySim(void);

	bool Create();
	btDynamicsWorld* GetDynamicsWorld() { return m_pDynamicsWorld; }
	
protected:
	class btBulletWorldImporter* m_BulletFileLoader;
	btDynamicsWorld* m_pDynamicsWorld;
	btCollisionDispatcher* m_pColDispatcher;
	btBroadphaseInterface* m_pBroadphase;
	btConstraintSolver* m_pConstraintSolver;
	btDefaultCollisionConfiguration* m_pColConfiguration;
	btAlignedObjectArray<btCollisionShape*> m_CollisionShapes;

	GL_ShapeDrawer*	m_shapeDrawer;
	std::string m_BulletFilePath;

public:
	bool LoadBulletFile(const char* filePath);
	bool ReloadBulletFile();
	void Update();	
	void Render(int pass) const;
};

