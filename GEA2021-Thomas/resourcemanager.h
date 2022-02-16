#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QOpenGLFunctions_4_1_Core>

#include "components.h"
#include "meshhandler.h"
#include <typeindex>
#include <unordered_map>


class GameObject;
class MeshHandler;
class MaterialComponent;
class Texture;
class TransformSystem;

/** @brief Creates objects by seting up correct mesh and texture.

  The main goal of this Resours Manager is to have one way of creating an object.
  The resource manager will take care of making sure the mesh is only saved once.

  It also handles saving and loading.
 */
class ResourceManager  : public QOpenGLFunctions_4_1_Core {

public:
    ResourceManager();
    ResourceManager *getInstance();

    /** @brief Creates objects

      Creates a new object sets up all the components. If the new game object wants to use a mesh or texture that
        allready is use, it uses the same mesh component or texture. A map is used to keep track of all the objects.

       @param filepath - Mesh file path
       @param UsingLOD - if it has 2 more meshes with different level of detail.
       @param textureFilepath - Texture file path
       @return the final GameObject(Entity)

       @note Default shader used, is the phong shader. If texture name is plain, then the object will use plain shader.

     */
    GameObject *CreateObject(std::string filepath, bool UsingLOD = false, std::string textureFilepath = " ");

    /** @brief Saves all the objects in the scene and its properties on a JSON file

      Uses JSON objects to store the most important porperties on a JSON file.

       @param objects - Objects to save
       @param levelName - Name of level / name of the JSON file

       @note Creates new file if levelName does not exist
       @note Overwrites the file if the levelName is the same as an existing file
     */
    void saveScene(std::vector<GameObject*> &objects, std::string &levelName);

    /** @brief Loads from the JSON files through the JSON objects and sett all the correct properties

      For every object, it creates an new object through the CreateObject function.
        After object is created, all the other variables can be sett.


       @param objects - Objects to save
       @param player
       @param light
       @param levelName - Name of level / name of the JSON file

       @note Player and Light are required inn all scenes, and are therefore set here directly.
     */
    void loadScene(std::vector<GameObject *> &objects,GameObject* &player, GameObject* &light, std::string &levelName);

    /** @brief Goes through the texture folder and sets up all the textures.

      Takes out all the .bmp files and creates new textures. Keeps track of existing textures in a Map.

      @author Part of the folder reading functionallity was taken and modified from Ole Flate
      https://github.com/Hedmark-University-College-SPIM/GEA2021/tree/ole_experiments

     */
    void setUpAllTextures();
    /** @brief Goes through the Meshes folder and stores the mesh names in a std::vector.

      @attention Part of the folder reading functionallity was taken and modified from Ole Flate
      https://github.com/Hedmark-University-College-SPIM/GEA2021/tree/ole_experiments
     */
    void getAllMeshNames();
    /** @brief Goes through the Assets folder and stores the level names in a std::vector.

      @attention Part of the folder reading functionallity was taken and modified from Ole Flate
      https://github.com/Hedmark-University-College-SPIM/GEA2021/tree/ole_experiments
     */
    void getAllLevelNames();

    std::vector<std::string> mTextureNames;
    std::vector<std::string> mMeshNames;
    std::vector<std::string> mLevelNames;
private:
    int CreateMaterial(std::string textureName);
    GameObject* tempGO{nullptr};
    MeshHandler* mMeshHandler{nullptr};
    TransformSystem* mTransformsystem{nullptr};
    ResourceManager *mInstance{nullptr};

    std::map<std::string, int> mTextureMap;
    std::map<std::string, MaterialComponent*> mMaterialMap;

    std::vector<Texture*> mVectorTextures;
    unsigned int textureIDcounter{0};

    std::vector<GameObject*> mObjects;
    std::map<std::string, GameObject> mObjectsMeshesMap;

    void createXYZ(MeshComponent *MeshComp);

public:
    static int objectIDcounter;
};




#endif // RESOURCEMANAGER_H
