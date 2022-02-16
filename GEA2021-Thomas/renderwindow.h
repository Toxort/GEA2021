#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions_4_1_Core>
#include <QTimer>
#include <QElapsedTimer>

#include "input.h"
#include "constants.h"

class QOpenGLContext;
class Shader;
class MainWindow;
class VisualObject;
class Camera;
class Texture;
class GameObject;
class ResourceManager;


/** @brief The main render system

    This inherits from QWindow to get access to the Qt functionality and
    OpenGL surface.
    We also inherit from QOpenGLFunctions, to get access to the OpenGL f
    This is the same as using glad and glw from general OpenGL tutorials
 */
class RenderWindow : public QWindow, protected QOpenGLFunctions_4_1_Core
{
    Q_OBJECT
public:
    RenderWindow(const QSurfaceFormat &format, MainWindow *mainWindow);
    ~RenderWindow() override;

    QOpenGLContext *context() { return mContext; }

    void exposeEvent(QExposeEvent *) override;

    // UI
    void toggleWireframe(bool buttonState);
    void renderLOD(bool bIsToggleOn);
    void toggleFrustumCulling(bool bIsToggleOn);
    void toggleShowCollsionBox(bool bIsToggleOn);
    std::vector<GameObject*> getAllGameObject();
    void deleteGameObjectAt(int index);


    std::vector<GameObject*> mGameObjects;
    std::vector<GameObject*> mParticles;

    void render();

    Camera *mCurrentCamera{nullptr};
    float getCameraSpeed() const;


private slots:

private:
    void init();

    void checkForGLerrors();

    void calculateFramerate();

    void startOpenGLDebugger();

    void setCameraSpeed(float value);

    void handleInput();

    void setupPlainShader(int shaderIndex);
    GLint mMatrixUniform{-1};
    GLint vMatrixUniform{-1};
    GLint pMatrixUniform{-1};

    void setupTextureShader(int shaderIndex);
    GLint mMatrixUniform1{-1};
    GLint vMatrixUniform1{-1};
    GLint pMatrixUniform1{-1};
    GLint mTextureUniform{-1};

    void setupMousPickingShader(int shaderIndex);
    GLint mMatrixUniform2{-1};
    GLint vMatrixUniform2{-1};
    GLint pMatrixUniform2{-1};
    GLint mPickingMatrixUniform{-1};

    void setupPhongShader(int shaderIndex);
    GLint mMatrixUniform3{-1};
    GLint vMatrixUniform3{-1};
    GLint pMatrixUniform3{-1};

    GLint mLightColorUniform{-1};
    GLint mObjectColorUniform{-1};
    GLint mAmbientLightStrengthUniform{-1};
    GLint mLightPositionUniform{-1};
    GLint mCameraPositionUniform{-1};
    GLint mSpecularStrengthUniform{-1};
    GLint mSpecularExponentUniform{-1};
    GLint mLightPowerUniform{-1};
    GLint mPhongTextureUniform{-1};

    class ResourceManager *mResourceManager{nullptr};

    class Texture *mTextures[gsl::NumberOfTextures]{nullptr}; //We can hold some textures

    class Shader *mShaderPrograms[gsl::NumberOfShaders]{nullptr};    //holds pointer the GLSL shader programs

    std::vector<VisualObject*> mVisualObjects;

    float mCameraSpeed{0.05f};
    float mCameraRotateSpeed{0.1f};
    int mMouseXlast{0};
    int mMouseYlast{0};
    bool onceLeftClicked{true};
    bool isMousePicking{false};

    QOpenGLContext *mContext{nullptr};
    bool mInitialized;

    QElapsedTimer mTimeStart;       //time variable that reads the calculated FPS

    MainWindow *mMainWindow{nullptr};        //points back to MainWindow to be able to put info in StatusBar

    class QOpenGLDebugLogger *mOpenGLDebugLogger{nullptr};

    float FOV{45.f};
    float mAspectratio{1.f};
    float mNearPlane{0.1f};
    float mFarPlane{1000.f};

    int mVerticesDrawn{0};
    int mObjectsDrawn{0};


    bool bRenderingLOD{true};
    bool bUsingFrustumCulling{true};
    bool bShowAllCollisionBoxes{false};

    bool wasLeftMouseButtonPressedLastFrame{false};

    float xMousePos{0};
    float yMousePos{0};

protected:
    //The QWindow that we inherit from has these functions to capture mouse and keyboard.
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

#endif // RENDERWINDOW_H
