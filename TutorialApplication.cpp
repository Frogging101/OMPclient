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

std::string str_address;
bool doShutdown = false;
Ogre::Vector3 p1Pos;
Ogre::Real p1yaw;
TutorialApplication app;

enum packetType {
	pSpawn,
	pDespawn,
	pMove,
	pUnknown
};

packetType strToPacketTyp(std::string type){
	if(type == "spawn") return pSpawn;
	else if(type == "despawn") return pDespawn;
	else if(type == "move") return pMove;
	else return pUnknown;
}

std::string realToStr(Ogre::Real num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}
std::string intToStr(int num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}

void packetHandler(std::string data){
	std::stringstream ss;
	ss << data;
	std::string type;
	ss >> type;
	int id;
	float x,y,z,yaw;
	switch(strToPacketTyp(type)){
		case pSpawn:
			ss >> id >> x >> y >> z >> yaw;
			app.connectClient(id,x,y,z,yaw);
			break;
		case pDespawn:
			ss >> id;
			app.disconnectClient(id);
			break;
		case pMove:
			ss >> id >> x >> y >> z >> yaw;
			app.moveClient(id,x,y,z,yaw);
			break;
	}
}

void handleNetwork(std::string ipAddress){
	std::cout << "Setting up connection" << std::endl;
	ENetAddress address;
	//setup host
	enet_address_set_host(&address, ipAddress.c_str());
	address.port = 1255;

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


	while(!doShutdown){
		std::string packetData = "move " + realToStr(p1Pos.x) + " " + realToStr(p1Pos.y) + " " + 
			realToStr(p1Pos.z) + " " + realToStr(p1yaw); 
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

				std::string serverPacket ((char*)event.packet->data,event.packet->dataLength);
				packetHandler(serverPacket);

				//Destroy packet after were done
				enet_packet_destroy(event.packet);
				break;
		}
	}
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

	enet_peer_reset(peer);
	}
}

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void){
}

void TutorialApplication::connectClient(int playerID, float x, float y, float z, float yaw){
	std::cout << playerID << " connected" << std::endl;
	Ogre::Entity *player = mSceneMgr->createEntity("player"+intToStr(playerID),"ninja.mesh");
	Ogre::SceneNode *pnode = mSceneMgr->getRootSceneNode()->createChildSceneNode("player"+intToStr(playerID)); 
	
	pnode->attachObject(player);
}

void TutorialApplication::disconnectClient(int playerID){
	mSceneMgr->destroySceneNode("player"+intToStr(playerID));
	mSceneMgr->destroyEntity("player"+intToStr(playerID));
}

void TutorialApplication::moveClient(int playerID, float x, float y, float z, float yaw){
	std::cout << "moving " << playerID << std::endl;
	mSceneMgr->getSceneNode("player"+intToStr(playerID))->yaw(Ogre::Degree(yaw));
	mSceneMgr->getSceneNode("player"+intToStr(playerID))->setPosition(Ogre::Vector3(x,y,z));
}

//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void){

	//Set scene ambient lighting
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.25,0.25,0.25));

	//Create p1 entity
	Ogre::Entity *player1 = mSceneMgr->createEntity("me","ninja.mesh");
	Ogre::SceneNode *p1node = mSceneMgr->getRootSceneNode()->createChildSceneNode("me");

	//Create room
	Ogre::Entity *cube = mSceneMgr->createEntity("cube","ogrehead.mesh");
	Ogre::SceneNode *cubeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("cube");

	//Attach nodes
	p1node->attachObject(player1);
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
	static Ogre::Real mRotate = 150;
	static Ogre::Real mMove = 250;

	Ogre::Vector3 transVector = Ogre::Vector3::ZERO;

	if(mKeyboard->isKeyDown(OIS::KC_I)){
		transVector.z -= mMove;
	}
	if(mKeyboard->isKeyDown(OIS::KC_K)){
		transVector.z += mMove;
	}
	if(mKeyboard->isKeyDown(OIS::KC_J)){
		//transVector.x -= mMove;
		mSceneMgr->getSceneNode("me")->yaw(Ogre::Degree(mRotate * evt.timeSinceLastFrame));
	}
	if(mKeyboard->isKeyDown(OIS::KC_L)){
		//transVector.x += mMove;
		mSceneMgr->getSceneNode("me")->yaw(Ogre::Degree(-mRotate * evt.timeSinceLastFrame));
	}

	mSceneMgr->getSceneNode("me")->translate(transVector * evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);

	return true;
}

bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt){
	bool ret = BaseApplication::frameRenderingQueued(evt);
	if(!processUnbufferedInput(evt)) return false;
	doShutdown = mShutDown;
	p1Pos = mSceneMgr->getSceneNode("me")->getPosition();
	p1yaw = mSceneMgr->getSceneNode("me")->getOrientation().getYaw().valueDegrees();
	//mSceneMgr->getSceneNode("player2")->setPosition(p2Pos);
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

		//TutorialApplication app;


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
