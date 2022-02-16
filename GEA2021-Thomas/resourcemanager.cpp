#include "resourcemanager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <qvector3d.h>
#include <qvector2d.h>
#include <vertex.h>
#include "gameobject.h"
#include "meshhandler.h"
#include "components.h"
#include "constants.h"
#include "texture.h"
#include "transformsystem.h"
#include "soundmanager.h"

#include <QOpenGLFunctions>

#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QJsonDocument>

ResourceManager::ResourceManager()
{
    mMeshHandler = new MeshHandler();
    mTransformsystem = TransformSystem::getInstance();
}

GameObject* ResourceManager::CreateObject(std::string filepath, bool UsingLOD, std::string textureName)
{
    // Loops through all the objects, if it finds it it will create a new component with the same mesh component.
    // if it does not find it in the map, it will create a new object with a unique meshComp.

    tempGO = new GameObject();

    tempGO->filepath = filepath;

    tempGO->mTransformComp = new TransformComponent();
    tempGO->mTransformComp->mMatrix.setToIdentity();

    tempGO->mBallPhysicsComp = new BallPhysicsComponent();
    tempGO->mMaterialComp = new MaterialComponent();

    tempGO->mCollisionComp = new CollisionComponent();
    tempGO->mCollisionComp->mMatrix.setToIdentity();
    mMeshHandler->createCollisionCorners(filepath, tempGO->mCollisionComp);

    tempGO->mMaterialComp->mTextureName = textureName;
    tempGO->mMaterialComp->mTextureUnit = CreateMaterial(textureName);

    if(textureName == "plain" || textureName == " ")
    {
        tempGO->mMaterialComp->mShaderProgram = 0;
    }else
    {
        tempGO->mMaterialComp->mShaderProgram = 2;
    }

    auto foundAtIndex = mObjectsMeshesMap.find(filepath);

    if(foundAtIndex != mObjectsMeshesMap.end()){
        tempGO->mMeshComp = foundAtIndex->second.mMeshComp;
        tempGO->mMeshComp->bUsingLOD = UsingLOD;

        tempGO->mCollisionLines = foundAtIndex->second.mCollisionLines;
        if (filepath.find("cacodemon") != std::string::npos)
        {
            tempGO->mAIComponent = new AIComponent();
        }
        tempGO->mSoundSourceComp = foundAtIndex->second.mSoundSourceComp;
        mObjectsMeshesMap.insert(std::pair<std::string, GameObject>{filepath + std::to_string(objectIDcounter) ,*tempGO});
    }else{
        tempGO->mMeshComp = new MeshComponent();
        tempGO->mMeshComp->bUsingLOD = UsingLOD;
        tempGO->mCollisionLines = new MeshComponent();

        if(UsingLOD)
        {
            mMeshHandler->readFile(filepath, tempGO->mMeshComp, 0, tempGO->mCollisionComp,tempGO->mCollisionLines );
            mMeshHandler->readFile(filepath, tempGO->mMeshComp, 1, tempGO->mCollisionComp,tempGO->mCollisionLines );
            mMeshHandler->readFile(filepath, tempGO->mMeshComp, 2, tempGO->mCollisionComp,tempGO->mCollisionLines );
        }else{
            if (filepath.find(".obj") != std::string::npos)
            {   //Hardcoded not ideal...
                // Alle componenter som har lyd har sine egne sound sources som ikke er ideelt
                // Skulle egentlig laget det som game objects er laget og loade in alle lydene slik at jeg
                // har en array med sound sources.
                if (filepath.find("cacodemon") != std::string::npos)
                {
                    qDebug() << "Added AI";
                    tempGO->mAIComponent = new AIComponent();
                    tempGO->mSoundSourceComp = new SoundSourceComponent();
                    tempGO->mSoundSourceComp->mSoundSource[0] = SoundManager::getInstance()->createSource("Damage sound", gsl::Vector3D(0.0f, 0.0f, 0.0f),
                                                                                         gsl::SoundFilePath + "enemiesound.wav", false, 2.0f);
                    tempGO->mSoundSourceComp->mSoundSource[1] = SoundManager::getInstance()->createSource("DeadSound", gsl::Vector3D(0.0f, 0.0f, 0.0f),
                                                                                         gsl::SoundFilePath + "enemydead.wav", false, 2.0f);
                    tempGO->mSoundSourceComp->mSoundSource[2] = SoundManager::getInstance()->createSource("Hehe sound", gsl::Vector3D(0.0f, 0.0f, 0.0f),
                                                                                         gsl::SoundFilePath + "hehe.wav", false, 2.0f);
                }
                mMeshHandler->readFile(filepath, tempGO->mMeshComp, 0, tempGO->mCollisionComp,tempGO->mCollisionLines );
            }
            if (filepath.find("xyz") != std::string::npos)
            {
                mMeshHandler->createXYZAxis(tempGO->mMeshComp,tempGO->mCollisionComp, tempGO->mCollisionLines);
            }
            if (filepath.find("test_las.txt") != std::string::npos)
            {
                mMeshHandler->createTerrain(filepath, tempGO->mMeshComp,tempGO->mCollisionComp, tempGO->mCollisionLines);
            }
            if (filepath.find("HeightCurve") != std::string::npos)
            {
                mMeshHandler->createHeightCurves(tempGO->mMeshComp,tempGO->mCollisionComp, tempGO->mCollisionLines);
            }
        }
        mObjectsMeshesMap.insert(std::pair<std::string, GameObject>{filepath ,*tempGO});
    }

    std::string tempName = filepath;

    if(filepath != "xyz" && filepath != gsl::ProjectFolderName + "test_las.txt" && filepath != "HeightCurve")
    {
        tempName.erase(0,25);
        tempName.erase(tempName.end()-4,tempName.end());
    }

    tempGO->name = tempName + " ID: " + std::to_string(objectIDcounter);

    tempGO->id = objectIDcounter;
    tempGO->mTransformComp->mScaleMatrix = tempGO->mTransformComp->mMatrix;

    objectIDcounter++;
    return tempGO;
}

void ResourceManager::saveScene(std::vector<GameObject *> &objects, std::string &levelName)
{
    if(levelName.empty())
    {
        qDebug() << "You need a level name to save!";
        return;
    }

    QFile saveFile(QString((gsl::AssetFilePath + levelName + ".json").c_str()));

    // open file
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    QJsonObject coreJsonObject;

    QJsonArray objectarray;

    for(auto it2 : objects)
    {
        QJsonObject levelObjects;
        levelObjects["filepath"] = it2->filepath.c_str();
        levelObjects["texturename"] = it2->mMaterialComp->mTextureName.c_str();
        // transform
        levelObjects["positionx"] = double(it2->mTransformComp->mMatrix.getPosition().x);
        levelObjects["positiony"] = double(it2->mTransformComp->mMatrix.getPosition().y);
        levelObjects["positionz"] = double(it2->mTransformComp->mMatrix.getPosition().z);

        levelObjects["rotationx"] = double(it2->mTransformComp->mRotation.getX());
        levelObjects["rotationy"] = double(it2->mTransformComp->mRotation.getY());
        levelObjects["rotationz"] = double(it2->mTransformComp->mRotation.getZ());

        levelObjects["scalex"] = double(it2->mTransformComp->mScale.getX());
        levelObjects["scaley"] = double(it2->mTransformComp->mScale.getY());
        levelObjects["scalez"] = double(it2->mTransformComp->mScale.getZ());

        levelObjects["shader"] = int(it2->mMaterialComp->mShaderProgram);
        levelObjects["usingLOD"] = it2->mMeshComp->bUsingLOD;
        levelObjects["usingFrustumCulling"] = it2->mMeshComp->bUsingFrustumCulling;

        objectarray.append(levelObjects);
    }

    coreJsonObject["objects"] = objectarray;
    saveFile.write(QJsonDocument(coreJsonObject).toJson());
    saveFile.close();
}

void ResourceManager::loadScene(std::vector<GameObject *> &objects, GameObject* &player, GameObject* &light, std::string &levelName)
{
    QFile loadFile(QString((gsl::AssetFilePath + levelName + ".json").c_str()));

    // open file
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    // levelarray
    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    // CoreObject
    QJsonObject loadDocJsonObject = loadDoc.object();

    objects.clear();

    if (loadDocJsonObject.contains("objects") && loadDocJsonObject["objects"].isArray())
    {
        QJsonArray objectsArray = loadDocJsonObject["objects"].toArray();

        for(int objectindex = 0; objectindex < objectsArray.size();objectindex++)
        {
            QJsonObject singleObject = objectsArray[objectindex].toObject();
            GameObject *gameObj {nullptr};
            float x = 0,y = 0,z = 0, rotx = 0, roty = 0, rotz = 0,scalx = 0, scaly = 0, scalz = 0;


            if (singleObject.contains("filepath") && singleObject["filepath"].isString())
            {
                if (singleObject.contains("usingLOD") && singleObject["usingLOD"].isBool())
                {
                    if(singleObject.contains("texturename") && singleObject["texturename"].isString())
                    {

                    gameObj = CreateObject(singleObject["filepath"].toString().toStdString(),singleObject["usingLOD"].toBool(),singleObject["texturename"].toString().toStdString());

                    gameObj->mMeshComp->bUsingLOD = singleObject["usingLOD"].toBool();
                    }
                }
                gameObj->filepath = singleObject["filepath"].toString().toStdString();

            }

            if (singleObject.contains("positionx") && singleObject["positionx"].isDouble())
                x = float(singleObject["positionx"].toDouble());

            if (singleObject.contains("positiony") && singleObject["positiony"].isDouble())
                y = float(singleObject["positiony"].toDouble());

            if (singleObject.contains("positionz") && singleObject["positionz"].isDouble())
                z = float(singleObject["positionz"].toDouble());

            if (singleObject.contains("rotationx") && singleObject["rotationx"].isDouble())
            {
                rotx = float(singleObject["rotationx"].toDouble());
            }

            if (singleObject.contains("rotationy") && singleObject["rotationy"].isDouble())
            {
                roty = float(singleObject["rotationy"].toDouble());
            }

            if (singleObject.contains("rotationz") && singleObject["rotationz"].isDouble())
            {
                rotz = float(singleObject["rotationz"].toDouble());
            }

            if (singleObject.contains("scalex") && singleObject["scalex"].isDouble())
            {
                scalx =float(singleObject["scalex"].toDouble());
            }

            if (singleObject.contains("scaley") && singleObject["scaley"].isDouble())
            {
                scaly =float(singleObject["scaley"].toDouble());
            }

            if (singleObject.contains("scalez") && singleObject["scalez"].isDouble())
            {
                scalz =float(singleObject["scalez"].toDouble());
            }

            //setposition
            mTransformsystem->setPosition(gameObj, gsl::Vector3D(x,y,z));
            //rotation
            mTransformsystem->setRotation(gameObj, gsl::Vector3D(rotx,roty,rotz));
            //scale
            mTransformsystem->setScale(gameObj, gsl::Vector3D(scalx,scaly,scalz));

            if (singleObject.contains("usingFrustumCulling") && singleObject["usingFrustumCulling"].isBool())
            {
                gameObj->mMeshComp->bUsingFrustumCulling = singleObject["usingFrustumCulling"].toBool();
            }

            if (singleObject.contains("shader") && singleObject["shader"].isDouble())
            {
                gameObj->mMaterialComp->mShaderProgram = singleObject["shader"].toInt();
            }

            if(objectindex == 1)
            {
                player = gameObj;
            }
            if(objectindex == 2)
                light = gameObj;

            objects.push_back(gameObj);

        }


    }

    loadFile.close();
}


int ResourceManager::CreateMaterial(std::string textureName)
{
    initializeOpenGLFunctions();

    int tempInt{-1};

    auto foundMatIndex = mTextureMap.find(textureName);
    if(foundMatIndex != mTextureMap.end()){
        tempInt = foundMatIndex->second;
    }else{
        if (textureName.find(".bmp") != std::string::npos)
        {
            tempInt = textureIDcounter;
            mTextureNames.push_back(textureName);
            mVectorTextures.push_back(new Texture(textureName));


            glActiveTexture(GL_TEXTURE0 + textureIDcounter);
            glBindTexture(GL_TEXTURE_2D, mVectorTextures.at(textureIDcounter)->mGLTextureID);
            mTextureMap.insert(std::pair<std::string, int>{textureName,tempInt});
            textureIDcounter++;
        }
    }
    if(tempInt == 0 || textureName == " ")
    {
        return 0;
    }
    return tempInt-1;
}

void ResourceManager::setUpAllTextures()
{
    //Regular .bmp textures read from file
    QDir tempDir((gsl::TextureFilePath).c_str());
    if(tempDir.exists())
    {
        QStringList filters;
        filters << "*.bmp";
        tempDir.setNameFilters(filters);


        //read all regular textures
        for(QFileInfo &var : tempDir.entryInfoList())
        {
            CreateMaterial(var.fileName().toStdString());
            qDebug () << "Reading texture: " << var.fileName();
        }
    }
    else
    {
        qDebug() << "ERROR reading texure Failed";

    }
}

void ResourceManager::getAllMeshNames()
{
    QDir tempDir((gsl::MeshFilePath).c_str());
    if(tempDir.exists())
    {
        QStringList filters;
        filters << "*.obj";
        tempDir.setNameFilters(filters);

        //getting all the names
        for(QFileInfo &var : tempDir.entryInfoList())
        {
            mMeshNames.push_back(var.fileName().toStdString());
            qDebug () << "Reading Mesh: " << var.fileName();
        }
    }
    else
    {
        qDebug() << "ERROR reading Mesh Failed";
    }
}

void ResourceManager::getAllLevelNames()
{
    QDir tempDir((gsl::AssetFilePath).c_str());
    if(tempDir.exists())
    {
        QStringList filters;
        filters << "*.json";
        tempDir.setNameFilters(filters);
        mLevelNames.clear();
        //getting all the names
        for(QFileInfo &var : tempDir.entryInfoList())
        {
            mLevelNames.push_back(var.fileName().toStdString());
            qDebug () << "Reading Levels: " << var.fileName();
        }
    }
    else
    {
        qDebug() << "ERROR reading Levels Failed";
    }
}



ResourceManager *ResourceManager::getInstance()
{
    if(!mInstance)
    {
        mInstance = new ResourceManager();
    }
    return mInstance;
}


void ResourceManager::createXYZ(MeshComponent *MeshComp)
{
    MeshComp->mVertices[0].push_back(Vertex{0,0,0,1,0,0});
    MeshComp->mVertices[0].push_back(Vertex{3,0,0,1,0,0});
    MeshComp->mVertices[0].push_back(Vertex{0,0,0,0,1,0});
    MeshComp->mVertices[0].push_back(Vertex{0,3,0,0,1,0});
    MeshComp->mVertices[0].push_back(Vertex{0,0,0,0,0,1});
    MeshComp->mVertices[0].push_back(Vertex{0,0,3,0,0,1});
}




int ResourceManager::objectIDcounter = 0;

