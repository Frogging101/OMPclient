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
#include <boost/thread.hpp>
#include <OgreSceneManager.h>

TutorialApplication app;
std::string str_address;
bool doShutdown = false;
Ogre::Vector3 p1Pos;
Ogre::Vector3 p2Pos;

std::string realToStr(Ogre::Real num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}

Ogre::Vector3 packetToVect(std::string data){
	std::stringstream ss;
	ss << data;
	//Ogre::Real x,y,z;
	float x,y,z;
	//ss.ignore();
	char* bleh;
	ss >> bleh >> x >> y >> z;
	std::cout << x << " " << y << " " << z << std::endl;
	return Ogre::Vector3(x,y,z);
}

void handleNetwork(std::string ipAddress){
	ENetAddress address;
	//setup host
	enet_address_set_host(&address, ipAddress.c_str());
	address.port = 340;

	ENetHost *client = enet_host_create(NULL,1,2,57600/8,14400/8);
	ENetPeer *peer = enet_host_connect(client, &address,2,0);
	ENetEvent event;

	if(peer == NULL){
		std::cout << "No available peers for initiating an Enet Connection." << std::endl;
		return;
	}

	if(enet_host_service(client, &event,5000) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT){

		std::cout << "Connection to " << ipAddress << " was succesful" << std::endl;
	}else {
		enet_peer_reset(peer);
		std::cout << "Connection to " << ipAddress << " failed" << std::endl;
		return;
	}

	//boost::this_thread::sleep(boost::posix_time::seconds(15));

	while(!doShutdown){
		//Ogre::Vector3 pos = app.mSceneMgr->getSceneNode("player1")->getPosition();
		std::string packetData = "packet " + realToStr(p1Pos.x) + " " + realToStr(p1Pos.y) + " " + realToStr(p1Pos.z); 
		ENetPacket *packet = enet_packet_create(packetData.c_str(),
													strlen(packetData.c_str())+1,
													ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer,0,packet);
		while(enet_host_service(client, &event, 100) > 0){
			if(event.type == ENET_EVENT_TYPE_RECEIVE)
				std::cout << "Packet Recieved, length: "
					<< event.packet->dataLength 
					<< " contains: " << event.packet->data
					<< " receieved from: " << event.peer->data
					<< " on channel: " << event.channelID << std::endl;

				std::string player2Packet ((char*)event.packet->data,event.packet->dataLength);
				Ogre::Vector3 pos = packetToVect(player2Packet);
				p2Pos = pos;
				//app.mSceneMgr->getSceneNode("player2")->setPosition(pos);

				//Destroy packet after were done
				enet_packet_destroy(event.packet);
				break;
		}
	}
}

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void){
	ENetEvent event;
	enet_peer_disconnect(peer,0);
	while(enet_host_service(client, &event, 3000) > 0){
		switch(event.type){
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Disconnected" << std::endl;
				break;
		}
	}

	enet_peer_reset(peer);
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

	boost::thread networkThread(handleNetwork,str_address);


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
	doShutdown = mShutDown;
	p1Pos = mSceneMgr->getSceneNode("player1")->getPosition();
	mSceneMgr->getSceneNode("player2")->setPosition(p2Pos);
	return ret;
}



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
//#else
    int main(int argc, char *argv[])
//#endif
    {
		//Some enet stuff
		if(enet_initialize() != 0){
			std::cout << "An error occured while initializing Enet" << std::endl;
		}
		atexit(enet_deinitialize);
		std::cout << "Enter server IP: ";
		std::cin >> str_address;
		//boost::thread networkThread(handleNetwork,str_address);

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
