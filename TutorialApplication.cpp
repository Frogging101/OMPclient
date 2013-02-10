/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "TutorialApplication.h"
#include <OgreSceneManager.h>

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void){

	//Set scene ambient lighting
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.25,0.25,0.25));

	//Create p1 entity
	Ogre::Entity *player1 = mSceneMgr->createEntity("player1","ninja.mesh");
	Ogre::SceneNode *p1node = mSceneMgr->getRootSceneNode()->createChildSceneNode("player1");

	Ogre::Entity *player2 = mSceneMgr->createEntity("player2","robot.mesh");
	Ogre::SceneNode *p2node = mSceneMgr->getRootSceneNode()->createChildSceneNode("player2");

	//Create room
	Ogre::Entity *cube = mSceneMgr->createEntity("cube","ogrehead.mesh");
	Ogre::SceneNode *cubeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("cube");

	//Attach nodes
	p1node->attachObject(player1);
	p2node->attachObject(player2);
	cubeNode->attachObject(cube);

	//Create light cause light is cool
	Ogre::Light *light = mSceneMgr->createLight("Light1");
	light->setType(Ogre::Light::LT_POINT);
	light->setPosition(Ogre::Vector3(250,150,250));
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue::White);

}

bool TutorialApplication::processUnbufferedInput(const Ogre::FrameEvent& evt){
	static bool mMouseDown = false;
	static Ogre::Real mToggle = 0.0;
	static Ogre::Real mRotate = 0.13;
	static Ogre::Real mMove = 250;

	Ogre::Vector3 transVector = Ogre::Vector3::ZERO;

	if(mKeyboard->isKeyDown(OIS::KC_I)){
		transVector.z -= mMove;
	}
	if(mKeyboard->isKeyDown(OIS::KC_K)){
		transVector.z += mMove;
	}
	if(mKeyboard->isKeyDown(OIS::KC_J)){
		transVector.x -= mMove;
	}
	if(mKeyboard->isKeyDown(OIS::KC_L)){
		transVector.x += mMove;
	}

	mSceneMgr->getSceneNode("player1")->translate(transVector * evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);

	return true;
}

bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt){
	bool ret = BaseApplication::frameRenderingQueued(evt);
	if(!processUnbufferedInput(evt)) return false;
	return ret;
}



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        TutorialApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
