#include "gameengine.h"

#include <QKeyEvent>
#include <QString>

#include <random>

#include "resourcemanager.h"
#include "gameobject.h"
#include "renderwindow.h"
#include "soundmanager.h"
#include "camera.h"
#include "physicsballsystem.h"
#include "mainwindow.h"
#include "collisionsystem.h"
#include "constants.h"
#include "transformsystem.h"
#include "particlesystem.h"
#include "aisystem.h"

#include "particle.h"


// Make Singelton
GameEngine* GameEngine::mInstance = nullptr;

//TODO: endre slik at constructoren blir kalt
GameEngine::GameEngine()
{
}

void GameEngine::GameLoop()
{
    if(bIsPlaying)
        AISystem::updateMovement(mPlayer, mRenderwindow->mGameObjects);

    SoundManager::getInstance()->updateListener(mGameCamera->position(), gsl::Vector3D(0,0,0), mGameCamera->getFowrardVector(), mGameCamera->up());
    ParticleSystem::update();

    mStereoSound->setPosition(gsl::Vector3D(0,0,0));
    HandleInput();

    mEditorCamera->update();
    mGameCamera->update();
    mRenderwindow->render();
}

void GameEngine::SetUpScene()
{
    mResourceManager = new class ResourceManager();
    mPhysicsBallSystem = new PhysicsBallSystem();
    mTransformSystem = new TransformSystem();
    SoundManager::getInstance()->init();
    mInstance = this;
    mGameLoopRenderTimer = new QTimer(this);

    mEditorCamera = new Camera();
    mEditorCamera->setPosition(gsl::Vector3D(5.f, 2.f, 5.f));

        mEditorCamera->yaw(-45);
    mEditorCamera->pitch(5);
    mRenderwindow->mCurrentCamera = mEditorCamera;

    mGameCamera = new Camera();
    mGameCamera->setPosition(gsl::Vector3D(0,2,0));
    mGameCamera->yaw(180);

    mResourceManager->setUpAllTextures();
    mResourceManager->getAllMeshNames();
    mResourceManager->getAllLevelNames();

    SetUpObjects();

    // Sound startup stuff
    mStereoSound = SoundManager::getInstance()->createSource(
                "dog", gsl::Vector3D(0.0f, 0.0f, 0.0f),
                gsl::SoundFilePath + "rock_background.wav", false, 1.0f);

    gunShotSound = SoundManager::getInstance()->createSource(
                "Stereo", gsl::Vector3D(0.0f, 0.0f, 0.0f),
                gsl::SoundFilePath + "gunshot.wav", false, 1.0f);

    //Gameloop
    connect(mGameLoopRenderTimer, SIGNAL(timeout()), this, SLOT(GameLoop()));

    mGameLoopRenderTimer->start(7);// original value 16, 7 is for 144 hz

    mMainWindow->initList();
    mMainWindow->initComboboxTexture(mResourceManager->mTextureNames);
    mMainWindow->initComboboxMeshes(mResourceManager->mMeshNames);
    mMainWindow->initComboboxLevels(mResourceManager->mLevelNames);

}

void GameEngine::SetUpObjects()
{
//    mParticle = mResourceManager->CreateObject("plane.obj", false, "blood.bmp");
//    mRenderwindow->mGameObjects.push_back(mParticle);


    mXYZaxis = mResourceManager->CreateObject("xyz",false,"plain");
    mRenderwindow->mGameObjects.push_back(mXYZaxis);



    mPlayer = mResourceManager->CreateObject(gsl::MeshFilePath + "player.obj",false,"plain");
    mPlayer->mTransformComp->mMatrix.setPosition(0,0,-8);

//    mPlayer->mMaterialComp->mTextureUnit = 1;
    //mPlayer->mMaterialComp->mShaderProgram = gsl::COLORSHADER;
    qDebug() << "PLAYER filepath: " << mPlayer->filepath.c_str();
    mRenderwindow->mGameObjects.push_back(mPlayer);



    mLight = mResourceManager->CreateObject(gsl::MeshFilePath + "light.obj",false);
        mLight->mTransformComp->mMatrix.setPosition(500,100,20);
                //mLight->mTransformComp->mMatrix.setPosition(5,5,5);
//    mLight->mTransformComp->mMatrix.setPosition(-20,3,-10);
//    mLight->mTransformComp->mMatrix.setPosition(500,300,500);
    mLight->mMaterialComp->mShaderProgram = 2;
//    mLight->mMaterialComp->mTextureUnit = 1;
    mRenderwindow->mGameObjects.push_back(mLight);


}


void GameEngine::HandleInput()
{
    mRenderwindow->mCurrentCamera->setSpeed(0.f);  //cancel last frame movement
    float cameraSpeed = mRenderwindow->getCameraSpeed();
    Camera *currentCamera = mRenderwindow->mCurrentCamera;
    float camSpeedMultiplyer{1.2f};

    UpdateGameCameraFollow();

    if(mInput.F && !bIsPlaying)
    {
        int tempListIndex = mMainWindow->getObjectListIndex();
        gsl::Vector3D movePos {0,
                    mRenderwindow->mGameObjects.at(tempListIndex)->mCollisionComp->mRaidus*3,
                    mRenderwindow->mGameObjects.at(tempListIndex)->mCollisionComp->mRaidus*3};
        mEditorCamera->setPosition(mRenderwindow->mGameObjects.at(tempListIndex)->mTransformComp->mMatrix.getPosition()+movePos);
        mEditorCamera->setYaw(0);
        mEditorCamera->setPitch(-45.f);
        mEditorCamera->update();
        mInput.F = false;
    }

    if(mInput.RMB)
    {
        if(mInput.W)
            currentCamera->setSpeed(-cameraSpeed);
        if(mInput.S)
            currentCamera->setSpeed(cameraSpeed);
        if(mInput.D)
            currentCamera->moveRight(cameraSpeed);
        if(mInput.A)
            currentCamera->moveRight(-cameraSpeed);
        if(mInput.Q)
            currentCamera->updateHeigth(-cameraSpeed);
        if(mInput.E)
            currentCamera->updateHeigth(cameraSpeed);
    }


    if(bIsPlaying)
    {
        //qDebug() << "nr of obj: " << mRenderwindow->mGameObjects.size();
        QVector3D currentPos{mPlayer->mTransformComp->mMatrix.getPosition().x,
                    mPlayer->mTransformComp->mMatrix.getPosition().y,
                    mPlayer->mTransformComp->mMatrix.getPosition().z};

        if(mInput.W)
        {
            QVector3D nextPos{currentPos.x() + currentCamera->getFowrardVector().x*cameraSpeed*camSpeedMultiplyer,
                        0,
                        currentPos.z() + currentCamera->getFowrardVector().z*cameraSpeed*camSpeedMultiplyer};

            QVector3D nextVec = nextPos - currentPos;

            //This will do for now..
            if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, nextPos))
            {
                if(!CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                {
                    mPlayer->mTransformComp->mMatrix.translateX(nextVec.x());
                    if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                    {
                        mPlayer->mTransformComp->mMatrix.translateZ(nextVec.z());
                    }
                }
            }
            else
            {
                mPlayer->mTransformComp->mMatrix.translateX(currentCamera->getFowrardVector().x*cameraSpeed*camSpeedMultiplyer);
                mPlayer->mTransformComp->mMatrix.translateZ(currentCamera->getFowrardVector().z*cameraSpeed*camSpeedMultiplyer);
            }
        }
        if(mInput.S)
        {
            QVector3D nextPos{currentPos.x() - currentCamera->getFowrardVector().x*cameraSpeed*camSpeedMultiplyer,
                        0,
                        currentPos.z() - currentCamera->getFowrardVector().z*cameraSpeed*camSpeedMultiplyer};

            QVector3D nextVec = nextPos - currentPos;

            if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, nextPos))
            {
                if(!CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                {
                    mPlayer->mTransformComp->mMatrix.translateX(nextVec.x());
                    if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                    {
                        mPlayer->mTransformComp->mMatrix.translateZ(nextVec.z());
                    }
                }
            }
            else
            {
                mPlayer->mTransformComp->mMatrix.translateX(-currentCamera->getFowrardVector().x*cameraSpeed*camSpeedMultiplyer);
                mPlayer->mTransformComp->mMatrix.translateZ(-currentCamera->getFowrardVector().z*cameraSpeed*camSpeedMultiplyer);
            }
        }
        if(mInput.D)
        {
            QVector3D nextPos{currentPos.x() + currentCamera->getRightVector().x*cameraSpeed*camSpeedMultiplyer,
                        0,
                        currentPos.z() + currentCamera->getRightVector().z*cameraSpeed*camSpeedMultiplyer};

            QVector3D nextVec = nextPos - currentPos;

            if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, nextPos))
            {
                if(!CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                {
                    mPlayer->mTransformComp->mMatrix.translateX(nextVec.x());
                    if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                    {
                        mPlayer->mTransformComp->mMatrix.translateZ(nextVec.z());
                    }
                }
            }
            else
            {
                mPlayer->mTransformComp->mMatrix.translateX(currentCamera->getRightVector().x*cameraSpeed*camSpeedMultiplyer);
                mPlayer->mTransformComp->mMatrix.translateZ(currentCamera->getRightVector().z*cameraSpeed*camSpeedMultiplyer);
            }
        }
        if(mInput.A)
        {
            QVector3D nextPos{currentPos.x() - currentCamera->getRightVector().x*cameraSpeed*camSpeedMultiplyer,
                        0,
                        currentPos.z() - currentCamera->getRightVector().z*cameraSpeed*camSpeedMultiplyer};

            QVector3D nextVec = nextPos - currentPos;

            if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, nextPos))
            {
                if(!CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                {
                    mPlayer->mTransformComp->mMatrix.translateX(nextVec.x());
                    if(CollisionSystem::isColliding(mPlayer, mRenderwindow->mGameObjects, currentPos))
                    {
                        mPlayer->mTransformComp->mMatrix.translateZ(nextVec.z());
                    }
                }
            }
            else
            {
                mPlayer->mTransformComp->mMatrix.translateX(-currentCamera->getRightVector().x*cameraSpeed*camSpeedMultiplyer);
                mPlayer->mTransformComp->mMatrix.translateZ(-currentCamera->getRightVector().z*cameraSpeed*camSpeedMultiplyer);
            }
        }
    }
}

void GameEngine::togglePlay(bool bInIsPlaying)
{
    playMusic(bInIsPlaying);
    bIsPlaying = bInIsPlaying;
    if(bInIsPlaying)
    {
        mRenderwindow->mCurrentCamera = mGameCamera;
    }else{

        mRenderwindow->mCurrentCamera = mEditorCamera;

        mMainWindow->on_loadScene_clicked();
    }
}

void GameEngine::UpdateGameCameraFollow()
{
    mGameCamera->setPosition(mPlayer->mTransformComp->mMatrix.getPosition() + gsl::Vector3D(0,2,0));
}


void GameEngine::saveScene(std::string &levelName)
{
    mResourceManager->saveScene(mRenderwindow->mGameObjects, levelName);
}

void GameEngine::loadScene(std::string &levelName)
{
    mResourceManager->loadScene(mRenderwindow->mGameObjects,mPlayer, mLight,levelName);
}

void GameEngine::updateAllLevels()
{
    mResourceManager->getAllLevelNames();
    mMainWindow->updateComboboxLevels(mResourceManager->mLevelNames);
}

void GameEngine::ResetBallVelocity()
{
    mPhysicsBallSystem->resetBall();
}

void GameEngine::setRenderPointer(RenderWindow *temp, MainWindow *mainW)
{
    mRenderwindow = temp;
    mMainWindow = mainW;
}

void GameEngine::playMusic(bool bIsPlaying)
{
    if(bIsPlaying)
        mStereoSound->play();
    else
        mStereoSound->stop();
}

// For the four funcitons under: temporary set the shader to 2 for mouspicking testing
void GameEngine::CreateCube()
{
    tempGameObject = mResourceManager->CreateObject(gsl::MeshFilePath + "cube.obj", false);
    tempGameObject->mMaterialComp->mShaderProgram = 2;
    mRenderwindow->mGameObjects.push_back(tempGameObject);
}

void GameEngine::CreatePyramid()
{
    tempGameObject = mResourceManager->CreateObject(gsl::MeshFilePath + "pyramid.obj",false);
    tempGameObject->mMaterialComp->mShaderProgram = 2;
    mRenderwindow->mGameObjects.push_back(tempGameObject);
}

void GameEngine::CreateSphere()
{
    tempGameObject = mResourceManager->CreateObject(gsl::MeshFilePath + "sphere.obj", true);
    tempGameObject->mMaterialComp->mShaderProgram = 2;
    mRenderwindow->mGameObjects.push_back(tempGameObject);
}

void GameEngine::CreateSuzanne()
{
    tempGameObject = mResourceManager->CreateObject(gsl::MeshFilePath + "suzanne.obj", true);
    tempGameObject->mMaterialComp->mShaderProgram = 2;
    mRenderwindow->mGameObjects.push_back(tempGameObject);
}

void GameEngine::CreateObject(std::string filePath, bool bUsingLOD, std::string textureName)
{
    tempGameObject = mResourceManager->CreateObject(filePath, bUsingLOD, textureName);
    tempGameObject->mMaterialComp->mShaderProgram = 2;
    mRenderwindow->mGameObjects.push_back(tempGameObject);
}

GameEngine * GameEngine::getInstance()
{
    if(!mInstance)
    {
        mInstance = new GameEngine();
    }
    return mInstance;
}
