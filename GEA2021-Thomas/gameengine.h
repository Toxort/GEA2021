#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <vector>

#include "gameobject.h"
#include "soundmanager.h"
#include "input.h"


class Camera;
class RenderWindow;
class ResourceManager;
class QTimer;
class SoundSourceComponent;
class MainWindow;
class PhysicsBallSystem;
class CollisionSystem;
class TransformSystem;
class Particle;

/// Core Engine class to handles the game loop and is the main hub for most systems
class GameEngine : public QObject
{
    Q_OBJECT

private slots:
    /** @brief Main game loop.

      For calling functions which needs to be called every frame.

     */
    void GameLoop();
public:
    GameEngine();

    /** @brief For initializing

      Not a constructor, but the next best thing...
     */
    void SetUpScene();

    /**
        @brief Function used for seting up objects in the scene by code.
     */
    void SetUpObjects();
    /** @brief Handles what heppens when buttons are pressed.

        @note  Could be sepperated in an entire inputHandler class.
        Aslo currently the collision detection and collision reaction
        is also in this function and could also
        use or be move to own system.
     */
    void HandleInput();

    /** @brief Turn on and off Game mode.

      Used for UI button.
       @param bInIsPlaying - if play button is on or not.
     */
    void togglePlay(bool bInIsPlaying);

    static GameEngine* getInstance();

    /** @brief For setting the renderwindow and mainwindow pointers to the correct classes.

        This is needed for communicating between the classes.
       @param temp - Renderwidnow pointer
       @param mainW - MainWindow pointer
     */
    void setRenderPointer(RenderWindow* temp, MainWindow *mainW);


    void playMusic(bool bIsPlaying);

    Input mInput;


    void CreateCube();
    void CreatePyramid();
    void CreateSphere();
    void CreateSuzanne();

    /**
        @brief Used to make GameObjects.

        This function calls the CreateObject function from the resource manager,
        sets shader and pushes the object in the the GameObjects array
        (which holds all the objects to render).
        This function is mainly used for creating meshes from UI.

       @param filePath - Where the mesh file is located
       @param bUsingLOD - if it has 2 more meshes with different level of detail.
       @param textureName - What the name of the texture is

       @see ResourceManager::CreateObject()
     */
    void CreateObject(std::string filePath, bool bUsingLOD, std::string textureName);

    /// An esscensial pointer to a game object which allways should exists in the scene.
    GameObject *mXYZaxis{nullptr};
    /// An esscensial pointer to a game object which allways should exists in the scene.
    GameObject* mPlayer{nullptr};
    /// An esscensial pointer to a game object which allways should exists in the scene.
    GameObject* mLight{nullptr};
    
    /** @brief Encapsulation of saveScene function in the Resource manager.

       @param levelName - name of the JSON file.
      @see ResourceManager::saveScene()
     */
    void saveScene(std::string &levelName);
    /** @brief Encapsulation of loadScene function in the Resource manager.

       @param levelName - name of the JSON file.
      @see ResourceManager::loadScene()
     */
    void loadScene(std::string &levelName);

    /** @brief gets all the levelnames and updates the level names inn the combobox in the UI

      @see ResourceManager::getAllLevelNames()
      @see MainWindow::updateComboboxLevels()
     */
    void updateAllLevels();


    SoundSource* gunShotSound{nullptr};

    //VisSim stuff

    bool bIsPlaying{false};
    bool bBallPhysicsIsPlaying{false};

    GameObject* mPhysicsBall{nullptr};
    GameObject* mPhysicsBall2{nullptr};
    std::vector<GameObject*> mPhysicsBalls{nullptr};
    void ResetBallVelocity();
private:


    GameObject* tempGameObject{nullptr};

    Camera *mGameCamera{nullptr};
    Camera *mEditorCamera{nullptr};

    SoundSource* mStereoSound{nullptr};

    static GameEngine* mInstance;
    RenderWindow* mRenderwindow{nullptr};
    MainWindow* mMainWindow{nullptr};
    ResourceManager *mResourceManager{nullptr};
    PhysicsBallSystem *mPhysicsBallSystem{nullptr};
    TransformSystem *mTransformSystem{nullptr};

    QTimer *mGameLoopRenderTimer{nullptr};

    void UpdateGameCameraFollow();

    //VisSim stuff
    GameObject* mTerrainObject{nullptr};
    GameObject* mHeightCurveObject{nullptr};
};

#endif // GAMEENGINE_H
