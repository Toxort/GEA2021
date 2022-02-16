#include "renderwindow.h"
#include <QTimer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLDebugLogger>
#include <QKeyEvent>
#include <QStatusBar>
#include <QDebug>

#include <iostream>

#include "shader.h"
#include "mainwindow.h"
#include "camera.h"
#include "constants.h"
#include "texture.h"
#include "meshhandler.h"
#include "gameobject.h"
#include "resourcemanager.h"
#include "soundmanager.h"
#include "gameengine.h"
#include "transformsystem.h"
#include "particle.h"
#include "particlesystem.h"

RenderWindow::RenderWindow(const QSurfaceFormat &format, MainWindow *mainWindow)
    : mContext(nullptr), mInitialized(false), mMainWindow(mainWindow)

{
    //This is sent to QWindow:
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
    //Make the OpenGL context
    mContext = new QOpenGLContext(this);
    //Give the context the wanted OpenGL format (v4.1 Core)
    mContext->setFormat(requestedFormat());
    if (!mContext->create()) {
        delete mContext;
        mContext = nullptr;
        qDebug() << "Context could not be made - quitting this application";
    }
}

RenderWindow::~RenderWindow()
{
    SoundManager::getInstance()->cleanUp();
}

// Sets up the general OpenGL stuff and the buffers needed to render a triangle
void RenderWindow::init()
{

    // TODO: Fix textures!!!



    //********************** General OpenGL stuff **********************

    //The OpenGL context has to be set.
    //The context belongs to the instanse of this class!
    if (!mContext->makeCurrent(this)) {
        qDebug() << "makeCurrent() failed";
        return;
    }

    //just to make sure we don't init several times
    //used in exposeEvent()
    if (!mInitialized)
        mInitialized = true;

    //must call this to use OpenGL functions
    initializeOpenGLFunctions();


    //Print render version info (what GPU is used):
    //(Have to use cout to see text- qDebug just writes numbers...)
    //Nice to see if you use the Intel GPU or the dedicated GPU on your laptop
    // - can be deleted
    std::cout << "The active GPU and API: \n";
    std::cout << "  Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "  Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "  Version: " << glGetString(GL_VERSION) << std::endl;

    //Get the texture units of your GPU
    int mTextureUnits; //Supported Texture Units (slots) pr shader. - maybe have in header!?
    int textureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textureUnits);
    std::cout << "  This GPU as " << textureUnits << " texture units / slots in total, ";
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &mTextureUnits);
    std::cout << "and supports " << mTextureUnits << " texture units pr shader" << std::endl;

    //**********************  Texture stuff: **********************
    //Returns a pointer to the Texture class. This reads and sets up the texture for OpenGL
    //and returns the Texture ID that OpenGL uses from Texture::id()
    mTextures[0] = new Texture();
    mTextures[1] = new Texture("hund.bmp");
    mTextures[2] = new Texture("cocademon3.bmp");

    //Set the textures loaded to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]->mGLTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextures[1]->mGLTextureID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextures[2]->mGLTextureID);


    //Start the Qt OpenGL debugger
    //Really helpfull when doing OpenGL
    //Supported on most Windows machines
    //reverts to plain glGetError() on Mac and other unsupported PCs
    // - can be deleted
    startOpenGLDebugger();

    //general OpenGL stuff:
    glEnable(GL_DEPTH_TEST);            //enables depth sorting - must then use GL_DEPTH_BUFFER_BIT in glClear
    //glEnable(GL_CULL_FACE);       //draws only front side of models - usually what you want - test it out!
    glClearColor(0.4f, 0.4f, 0.4f,1.0f);    //gray color used in glClear GL_COLOR_BUFFER_BIT

    //Compile shaders:
    //NB: hardcoded path to files! You have to change this if you change directories for the project.
    //Qt makes a build-folder besides the project folder. That is why we go down one directory
    // (out of the build-folder) and then up into the project folder.
    mShaderPrograms[0] = new Shader((gsl::ShaderFilePath + "plainvertex.vert").c_str(),
                                (gsl::ShaderFilePath + "plainfragment.frag").c_str());
    qDebug() << "Plain shader program id: " << mShaderPrograms[0]->getProgram();

    mShaderPrograms[1] = new Shader((gsl::ShaderFilePath + "textureshader.vert").c_str(),
                                    (gsl::ShaderFilePath + "textureshader.frag").c_str());
    qDebug() << "Texture shader program id: " << mShaderPrograms[1]->getProgram();

    mShaderPrograms[2] = new Shader((gsl::ShaderFilePath + "phongshader.vert").c_str(),
                                    (gsl::ShaderFilePath + "phongshader.frag").c_str());
    qDebug() << "Texture shader program id: " << mShaderPrograms[2]->getProgram();

    mShaderPrograms[3] = new Shader((gsl::ShaderFilePath + "mousepickingvertex.vert").c_str(),
                                    (gsl::ShaderFilePath + "mousepickingfragment.frag").c_str());
    qDebug() << "Texture shader program id: " << mShaderPrograms[3]->getProgram();


    setupPlainShader(0);
    setupTextureShader(1);
    setupPhongShader(2);
    setupMousPickingShader(3);

    GameEngine::getInstance()->SetUpScene();


}

// Called each frame - doing the rendering
void RenderWindow::render()
{
    bool isPlaying = GameEngine::getInstance()->bIsPlaying;
    Input input = GameEngine::getInstance()->mInput;

    // For makeing the cursor look like a crosshair, stolen from my 3Dprog project
    if(isPlaying)
    {
        setCursor(QCursor(Qt::CrossCursor));
    }
    else
    {
        setCursor(QCursor(Qt::ArrowCursor));
    }

    // For mousepicking to only pick once pr click
    if(input.LMB)
    {
        if(onceLeftClicked)
        {
            isMousePicking = true;
            onceLeftClicked = false;
        }
        else
        {
            isMousePicking = false;
        }
    }
    if(!input.LMB)
    {
        onceLeftClicked = true;
    }

    mVerticesDrawn = 0;
    mObjectsDrawn = 0;

    mTimeStart.restart(); //restart FPS clock
    mContext->makeCurrent(this); //must be called every frame (every time mContext->swapBuffers is called)

    initializeOpenGLFunctions();    //must call this every frame it seems...

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    for(unsigned long long i{0}; i < mGameObjects.size(); i++)
    {
        mCurrentCamera->mProjectionMatrix.perspective(FOV, mAspectratio, mNearPlane , mFarPlane);

        gsl::Vector3D objPos = mGameObjects[i]->mTransformComp->mMatrix.getPosition();

        gsl::Vector3D rightPlaneNormal = (mCurrentCamera->getRightVector());
        //right is minus rotation
        rightPlaneNormal.rotateY(-FOV);

        gsl::Vector3D leftPlaneNormal = -(mCurrentCamera->getRightVector());
        leftPlaneNormal.rotateY(FOV);

        gsl::Vector3D vectorToObj = objPos - mCurrentCamera->position();

        float distanceToRightObject = ((vectorToObj * rightPlaneNormal) / rightPlaneNormal.length());
        float distanceToLeftObject = ((vectorToObj * leftPlaneNormal) / leftPlaneNormal.length());

        gsl::Vector3D forwardPlaneNormal = mCurrentCamera->getFowrardVector();
        float distanceToFrontObject = ((vectorToObj * forwardPlaneNormal) / forwardPlaneNormal.length());

        float  distanceCamToObj = (objPos - mCurrentCamera->position()).length();


        if(bUsingFrustumCulling && mGameObjects[i]->mMeshComp->bUsingFrustumCulling)
        {
            if(distanceToRightObject - mGameObjects[i]->mCollisionComp->mRaidus > 0)
            {
                continue;
            }
            if(distanceToLeftObject - mGameObjects[i]->mCollisionComp->mRaidus > 0)
            {
                continue;
            }
            if(distanceToFrontObject > mFarPlane)
            {
                continue;
            }
        }

        // MousePicking  /// source MousePicking: http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
        GLuint pickingColorID = glGetUniformLocation(mShaderPrograms[3]->getProgram(), "PickingColor");

        if(isMousePicking)
        {
            glUseProgram(mShaderPrograms[3]->getProgram() );

            int id = i; // +50 for å se rød nyansen
            int r = (id & 0x000000FF) >>  0;
            int g = (id & 0x0000FF00) >>  8;
            int b = (id & 0x00FF0000) >> 16;
            glUniform4f(pickingColorID, r/255.0f, g/255.0f, b/255.0f, 1.0f);
            glUniformMatrix4fv( vMatrixUniform2, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
            glUniformMatrix4fv( pMatrixUniform2, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
            glUniformMatrix4fv( mMatrixUniform2, 1, GL_TRUE, mGameObjects[i]->mTransformComp->mMatrix.constData());

            // not draw player while playig
            if(i==1 && isPlaying)
            {
                continue;
            }

            glBindVertexArray( mGameObjects[i]->mMeshComp->mVAO[0] );
            glDrawArrays(mGameObjects[i]->mMeshComp->mDrawType, 0, mGameObjects[i]->mMeshComp->mVertices[0].size());

            //Mouspicking Stuff:
            glFlush();
            glFinish();

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


            unsigned char data[4];
            glReadPixels(xMousePos, height() - yMousePos,1,1, GL_RGBA, GL_UNSIGNED_BYTE, data);

            int pickedID =
                    data[0] +
                    data[1] * 256 +
                    data[2] * 256*256;

            if(pickedID < 10000)
            {
                mMainWindow->setID(pickedID);
            }
            mObjectsDrawn++;
            glBindVertexArray(0);
            continue;
        }

        //Plainshader
        else if(mGameObjects[i]->mMaterialComp->mShaderProgram == 0 && !isMousePicking)
        {
            glUseProgram(mShaderPrograms[0]->getProgram() );
            glUniformMatrix4fv( vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
            glUniformMatrix4fv( pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
            glUniformMatrix4fv( mMatrixUniform, 1, GL_TRUE, mGameObjects[i]->mTransformComp->mMatrix.constData());
        }//Textureshader
        else if(mGameObjects[i]->mMaterialComp->mShaderProgram == 1)
        {
            glUseProgram(mShaderPrograms[1]->getProgram() );
            glUniform1i(mTextureUniform, mGameObjects[i]->mMaterialComp->mTextureUnit);
            glUniformMatrix4fv( vMatrixUniform1, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
            glUniformMatrix4fv( pMatrixUniform1, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
            glUniformMatrix4fv( mMatrixUniform1, 1, GL_TRUE, mGameObjects[i]->mTransformComp->mMatrix.constData());
        }
        //phongShader
        else if(mGameObjects[i]->mMaterialComp->mShaderProgram == 2)
        {
            glUseProgram(mShaderPrograms[2]->getProgram() );
            glUniformMatrix4fv( vMatrixUniform3, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
            glUniformMatrix4fv( pMatrixUniform3, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
            glUniformMatrix4fv( mMatrixUniform3, 1, GL_TRUE, mGameObjects[i]->mTransformComp->mMatrix.constData());
            GameObject* light = GameEngine::getInstance()->mLight;
            gsl::Vector3D lightPos = light->mTransformComp->mMatrix.getPosition();
            gsl::Vector3D lightColor{0.9f, 0.9f, 0.9f};

            glUniform1i(mPhongTextureUniform, mGameObjects[i]->mMaterialComp->mTextureUnit);
            glUniform3f(mLightPositionUniform, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(mCameraPositionUniform, mCurrentCamera->position().x, mCurrentCamera->position().y, mCurrentCamera->position().z);
            glUniform3f(mLightColorUniform, lightColor.x, lightColor.y, lightColor.z);
        }

        // to not draw player while playig
        if(i==1 && isPlaying)
        {
            continue;
        }//to not draw xyz
        if(i == 0 && isPlaying)
        {
            continue;
        }

        //draw for LOD
        if(mGameObjects[i]->mMeshComp->bUsingLOD && bRenderingLOD)
        {
            if(distanceCamToObj < 10)
            {
                glBindVertexArray( mGameObjects[i]->mMeshComp->mVAO[0] );
                glDrawArrays(mGameObjects[i]->mMeshComp->mDrawType, 0, mGameObjects[i]->mMeshComp->mVertices[0].size());
                mVerticesDrawn += mGameObjects[i]->mMeshComp->mVertices[0].size();
                mObjectsDrawn++;
            }else if( distanceCamToObj < 20)
            {
                glBindVertexArray( mGameObjects[i]->mMeshComp->mVAO[1] );
                glDrawArrays(mGameObjects[i]->mMeshComp->mDrawType, 0, mGameObjects[i]->mMeshComp->mVertices[1].size());
                mVerticesDrawn += mGameObjects[i]->mMeshComp->mVertices[1].size();
                mObjectsDrawn++;
            }else{
                glBindVertexArray( mGameObjects[i]->mMeshComp->mVAO[2] );
                glDrawArrays(mGameObjects[i]->mMeshComp->mDrawType, 0, mGameObjects[i]->mMeshComp->mVertices[2].size());
                mVerticesDrawn += mGameObjects[i]->mMeshComp->mVertices[2].size();
                mObjectsDrawn++;
            }
        }// Draw for indices
        else if (mGameObjects[i]->mMeshComp->mIndices->size() > 0)
        {
            glBindVertexArray( mGameObjects[i]->mMeshComp->mVAO[0] );
            glDrawElements(mGameObjects[i]->mMeshComp->mDrawType, mGameObjects[i]->mMeshComp->mIndices->size(), GL_UNSIGNED_INT, nullptr);
            mVerticesDrawn += mGameObjects[i]->mMeshComp->mVertices[0].size();
            mObjectsDrawn++;
        }else // Draw for Vertex array
        {
            glBindVertexArray( mGameObjects[i]->mMeshComp->mVAO[0] );
            glDrawArrays(mGameObjects[i]->mMeshComp->mDrawType, 0, mGameObjects[i]->mMeshComp->mVertices[0].size());
            mVerticesDrawn += mGameObjects[i]->mMeshComp->mVertices[0].size();
            mObjectsDrawn++;
        }

        // For collision boxes
        if( bShowAllCollisionBoxes )
        {
            glBindVertexArray( mGameObjects[i]->mCollisionLines->mVAO[0] );
            glDrawElements(mGameObjects[i]->mCollisionLines->mDrawType, mGameObjects[i]->mCollisionLines->mIndices->size(), GL_UNSIGNED_INT, nullptr);
        }
        if( mGameObjects[i]->mCollisionComp->bShowCollisionBox && !isPlaying)
        {
            glBindVertexArray( mGameObjects[i]->mCollisionLines->mVAO[0] );
            glDrawElements(mGameObjects[i]->mCollisionLines->mDrawType, mGameObjects[i]->mCollisionLines->mIndices->size(), GL_UNSIGNED_INT, nullptr);
        }
    }

    //Rendring the particles
    std::vector<Particle*> ptcl = ParticleSystem::particles;
    for(int i = 0; i < ptcl.size(); i++)
    {
        if(ptcl.at(i) == nullptr)
            continue;

        glUseProgram( mShaderPrograms[0]->getProgram() );
        glUniformMatrix4fv( vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform, 1, GL_TRUE, ptcl.at(i)->mMatrix.constData());

        glBindVertexArray( ptcl.at(i)->mVAO );
        glDrawArrays(ptcl.at(i)->mDrawType, 0, ptcl.at(i)->mVertices.size());
    }

    //Calculate framerate before
    // checkForGLerrors() because that takes a long time
    // and before swapBuffers(), else it will show the vsync time
    calculateFramerate();

    //using our expanded OpenGL debugger to check if everything is OK.
    checkForGLerrors();

    //Qt require us to call this swapBuffers() -function.
    // swapInterval is 1 by default which means that swapBuffers() will (hopefully) block
    // and wait for vsync.
    if (!isMousePicking)
    {
        mContext->swapBuffers(this);
    }

    glUseProgram(0);
}

void RenderWindow::setupPlainShader(int shaderIndex)
{
    mMatrixUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "pMatrix" );
}

void RenderWindow::setupTextureShader(int shaderIndex)
{
    mMatrixUniform1 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform1 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform1 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "pMatrix" );
    mTextureUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "textureSampler");
}

void RenderWindow::setupMousPickingShader(int shaderIndex)
{
    mMatrixUniform2 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform2 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform2 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "pMatrix" );
}

void RenderWindow::setupPhongShader(int shaderIndex)
{
    mMatrixUniform3 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform3 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform3 = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "pMatrix" );

    mPhongTextureUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "textureSampler");

    mLightColorUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "lightColor" );
    mObjectColorUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "objectColor" );
    mAmbientLightStrengthUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "ambientStrength" );
    mLightPositionUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "lightPosition" );
    mSpecularStrengthUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "specularStrength" );
    mSpecularExponentUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "specularExponent" );
    mLightPowerUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "lightPower" );
    mCameraPositionUniform = glGetUniformLocation( mShaderPrograms[shaderIndex]->getProgram(), "cameraPosition" );
}

float RenderWindow::getCameraSpeed() const
{
    return mCameraSpeed;
}

//This function is called from Qt when window is exposed (shown)
// and when it is resized
//exposeEvent is a overridden function from QWindow that we inherit from
void RenderWindow::exposeEvent(QExposeEvent *)
{
    //if not already initialized - run init() function
    if (!mInitialized)
        init();

    //This is just to support modern screens with "double" pixels (Macs and some 4k Windows laptops)
    const qreal retinaScale = devicePixelRatio();

    //Set viewport width and height
    glViewport(0, 0, static_cast<GLint>(width() * retinaScale), static_cast<GLint>(height() * retinaScale));


    //calculate aspect ration and set projection matrix
    mAspectratio = static_cast<float>(width()) / height();
    //    qDebug() << mAspectratio;
    mCurrentCamera->mProjectionMatrix.perspective(FOV, mAspectratio, mNearPlane , mFarPlane);
    //    qDebug() << mCamera.mProjectionMatrix;
}

//The way this is set up is that we start the clock before doing the draw call,
// and check the time right after it is finished (done in the render function)
//This will approximate what framerate we COULD have.
//The actual frame rate on your monitor is limited by the vsync and is probably 60Hz
void RenderWindow::calculateFramerate()
{
    long nsecElapsed = mTimeStart.nsecsElapsed();
    static int frameCount{0};                       //counting actual frames for a quick "timer" for the statusbar

    if (mMainWindow)            //if no mainWindow, something is really wrong...
    {
        ++frameCount;
        if (frameCount > 30)    //once pr 30 frames = update the message twice pr second (on a 60Hz monitor)
        {
            //showing some statistics in status bar
            mMainWindow->statusBar()->showMessage(" Time pr FrameDraw: " +
                                                  QString::number(nsecElapsed/1000000.f, 'g', 4) + " ms  |  " +
                                                  "FPS (approximated): " + QString::number(1E9 / nsecElapsed, 'g', 7) + " |  Objects Drawn: " + QString::number(mObjectsDrawn) + " | Vertices Drawn: " + QString::number(mVerticesDrawn)) ;
            frameCount = 0;     //reset to show a new message in 60 frames
        }
    }
}

//Simple way to turn on/off wireframe mode
//Not totally accurate, but draws the objects with
//lines instead of filled polygons
void RenderWindow::toggleWireframe(bool buttonState)
{
    if (buttonState)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    //turn on wireframe mode
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    //turn off wireframe mode
        glEnable(GL_CULL_FACE);
    }
}

void RenderWindow::renderLOD(bool bIsToggleOn)
{
    if(bIsToggleOn)
        bRenderingLOD = false;
    else
        bRenderingLOD = true;

}

void RenderWindow::toggleFrustumCulling(bool bIsToggleOn)
{
    if(bIsToggleOn)
        bUsingFrustumCulling = false;
    else
        bUsingFrustumCulling = true;
}

void RenderWindow::toggleShowCollsionBox(bool bIsToggleOn)
{
    if(bIsToggleOn)
        bShowAllCollisionBoxes = true;
    else
        bShowAllCollisionBoxes = false;
}

std::vector<GameObject*> RenderWindow::getAllGameObject()
{
    return mGameObjects;
}

void RenderWindow::deleteGameObjectAt(int index)
{
    mGameObjects.erase(mGameObjects.begin() + index);
}


//GameObject* RenderWindow::RenderWindow::addObject(std::string assetName)
//{
//     GameObject *temp = ResourceManager::getInstance();
//}

//Uses QOpenGLDebugLogger if this is present
//Reverts to glGetError() if not
void RenderWindow::checkForGLerrors()
{
    if(mOpenGLDebugLogger)
    {
        const QList<QOpenGLDebugMessage> messages = mOpenGLDebugLogger->loggedMessages();
        for (const QOpenGLDebugMessage &message : messages)
            qDebug() << message;
    }
    else
    {
        GLenum err = GL_NO_ERROR;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            qDebug() << "glGetError returns " << err;
        }
    }
}

//Tries to start the extended OpenGL debugger that comes with Qt
void RenderWindow::startOpenGLDebugger()
{
    QOpenGLContext * temp = this->context();
    if (temp)
    {
        QSurfaceFormat format = temp->format();
        if (! format.testOption(QSurfaceFormat::DebugContext))
            qDebug() << "This system can not use QOpenGLDebugLogger, so we revert to glGetError()";

        if(temp->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
        {
            qDebug() << "System can log OpenGL errors!";
            mOpenGLDebugLogger = new QOpenGLDebugLogger(this);
            if (mOpenGLDebugLogger->initialize()) // initializes in the current context
                qDebug() << "Started OpenGL debug logger!";
        }
    }
}

void RenderWindow::setCameraSpeed(float value)
{
    mCameraSpeed += value;

    //Keep within some min and max values
    if(mCameraSpeed < 0.01f)
        mCameraSpeed = 0.01f;
    if (mCameraSpeed > 3.6f)
        mCameraSpeed = 3.6f;
}

void RenderWindow::handleInput()
{
    //Camera

//    if(/*isPlaying*/!mInput.RMB && bIsPlaying)
//    {
        //mCurrentCamera->setPosition(mGameObjects[0]->mTransformComp->mMatrix.getPosition());
//        if(mInput.W)
//            mGameObjects[0]->mTransformComp->mMatrix.translateZ(0.1f);
//        if(mInput.S)
//            mGameObjects[0]->mTransformComp->mMatrix.translateZ(-0.1f);
//        if(mInput.D)
//            mGameObjects[0]->mTransformComp->mMatrix.translateX(-0.1f);
//        if(mInput.A)
//            mGameObjects[0]->mTransformComp->mMatrix.translateX(0.1f);

//        if(mInput.W)
//            mCurrentCamera->setSpeed(-mCameraSpeed);
//        if(mInput.S)
//            mCurrentCamera->setSpeed(mCameraSpeed);
//        if(mInput.D)
//            mCurrentCamera->moveRight(mCameraSpeed);
//        if(mInput.A)
//            mCurrentCamera->moveRight(-mCameraSpeed);
//        if(mInput.Q)
//            mCurrentCamera->updateHeigth(-mCameraSpeed);
//        if(mInput.E)
//            mCurrentCamera->updateHeigth(mCameraSpeed);

//    }

}

void RenderWindow::keyPressEvent(QKeyEvent *event)
{
    bool &isPlaying = GameEngine::getInstance()->bIsPlaying;
    Input &input = GameEngine::getInstance()->mInput;
    if (event->key() == Qt::Key_Escape) //Shuts down whole program
    {
        if(isPlaying)
        {
            isPlaying = false;
            //GameEngine::getInstance()->togglePlay(bIsPlaying);
            mMainWindow->on_PlayStop_toggled(isPlaying);
        }else
        {
            mMainWindow->close();
        }
    }

    //    You get the keyboard input like this
    if(event->key() == Qt::Key_W)
    {
        input.W = true;
    }
    if(event->key() == Qt::Key_S)
    {
        input.S = true;
    }
    if(event->key() == Qt::Key_D)
    {
        input.D = true;
    }
    if(event->key() == Qt::Key_A)
    {
        input.A = true;
    }
    if(event->key() == Qt::Key_Q)
    {
        input.Q = true;
    }
    if(event->key() == Qt::Key_E)
    {
        input.E = true;
    }
    if(event->key() == Qt::Key_F)
    {
        input.F = true;
    }
    if(event->key() == Qt::Key_Z)
    {
    }
    if(event->key() == Qt::Key_X)
    {
    }
    if(event->key() == Qt::Key_P)
    {

    }
    if(event->key() == Qt::Key_Up)
    {
        input.UP = true;
    }
    if(event->key() == Qt::Key_Down)
    {
        input.DOWN = true;
    }
    if(event->key() == Qt::Key_Left)
    {
        input.LEFT = true;
    }
    if(event->key() == Qt::Key_Right)
    {
        input.RIGHT = true;
    }
    if(event->key() == Qt::Key_U)
    {
    }
    if(event->key() == Qt::Key_O)
    {
    }
    if(event->key() == Qt::Key_Delete)
    {
        input.DEL = true;
        mMainWindow->on_actionDelete_Selected_triggered();
    }
}

void RenderWindow::keyReleaseEvent(QKeyEvent *event)
{
    Input &input = GameEngine::getInstance()->mInput;
    if(event->key() == Qt::Key_W)
    {
        input.W = false;
    }
    if(event->key() == Qt::Key_S)
    {
        input.S = false;
    }
    if(event->key() == Qt::Key_D)
    {
        input.D = false;
    }
    if(event->key() == Qt::Key_A)
    {
        input.A = false;
    }
    if(event->key() == Qt::Key_Q)
    {
        input.Q = false;
    }
    if(event->key() == Qt::Key_E)
    {
        input.E = false;
    }
    if(event->key() == Qt::Key_F)
    {
        input.F = false;
    }
    if(event->key() == Qt::Key_Z)
    {
    }
    if(event->key() == Qt::Key_X)
    {
    }
    if(event->key() == Qt::Key_Up)
    {
        input.UP = false;
    }
    if(event->key() == Qt::Key_Down)
    {
        input.DOWN = false;
    }
    if(event->key() == Qt::Key_Left)
    {
        input.LEFT = false;
    }
    if(event->key() == Qt::Key_Right)
    {
        input.RIGHT = false;
    }
    if(event->key() == Qt::Key_U)
    {
    }
    if(event->key() == Qt::Key_O)
    {
    }
    if(event->key() == Qt::Key_Delete)
    {
        input.DEL = false;
    }
}

void RenderWindow::mousePressEvent(QMouseEvent *event)
{
    bool &isPlaying = GameEngine::getInstance()->bIsPlaying;
    Input &input = GameEngine::getInstance()->mInput;
    if (event->button() == Qt::RightButton)
        input.RMB = true;
    if (event->button() == Qt::LeftButton)
    {
        input.LMB = true;
    }
    if (event->button() == Qt::MiddleButton)
        input.MMB = true;


    if(input.LMB )
    {
        //Using mMouseXYlast as deltaXY so we don't need extra variables
        xMousePos = event->pos().x();
        yMousePos = event->pos().y();
        if(isPlaying)
            GameEngine::getInstance()->gunShotSound->play();
    }
    //qDebug() << "Mouse position" << xMousePos << "," << yMousePos;

    mMouseXlast = event->pos().x();
    mMouseYlast = event->pos().y();

//    if(isPlaying && input)
}

void RenderWindow::mouseReleaseEvent(QMouseEvent *event)
{

    Input &input = GameEngine::getInstance()->mInput;
    if (event->button() == Qt::RightButton)
        input.RMB = false;
    if (event->button() == Qt::LeftButton)
    {
        input.LMB = false;
        //onceLeftClicked = true;
//        if(onceLeftRelesed)
//        {
//            actualOnce = false;
//            onceLeftRelesed = false;
//        }
    }
    if (event->button() == Qt::MiddleButton)
        input.MMB = false;

}

void RenderWindow::wheelEvent(QWheelEvent *event)
{
    Input &input = GameEngine::getInstance()->mInput;
    QPoint numDegrees = event->angleDelta() / 8;

    //if RMB, change the speed of the camera
    if (input.RMB)
    {
        if (numDegrees.y() < 1)
            setCameraSpeed(0.01f);
        if (numDegrees.y() > 1)
            setCameraSpeed(-0.01f);
    }
    event->accept();
}


void RenderWindow::mouseMoveEvent(QMouseEvent *event)
{
    bool &isPlaying = GameEngine::getInstance()->bIsPlaying;
    Input &input = GameEngine::getInstance()->mInput;
    if (input.RMB && !isPlaying)
    {
        //Using mMouseXYlast as deltaXY so we don't need extra variables
        mMouseXlast = event->pos().x() - mMouseXlast;
        mMouseYlast = event->pos().y() - mMouseYlast;

        if (mMouseXlast != 0)
            mCurrentCamera->yaw(mCameraRotateSpeed * mMouseXlast);
        if (mMouseYlast != 0)
            mCurrentCamera->pitch(mCameraRotateSpeed * mMouseYlast);
    }
    if(isPlaying)
    {
        //Using mMouseXYlast as deltaXY so we don't need extra variables
//        mMouseXlast = event->pos().x() - mMouseXlast;
//        mMouseYlast = event->pos().y() - mMouseYlast;

//        if (mMouseXlast != 0)
//            mCurrentCamera->yaw(mCameraRotateSpeed * mMouseXlast);
//        if (mMouseYlast != 0)
//            mCurrentCamera->pitch(mCameraRotateSpeed * mMouseYlast);




        mMouseXlast = width()/2;
        mMouseYlast = height()/2;

        float xoffset = mMouseXlast - event->pos().x();
        float yoffset = mMouseYlast - event->pos().y();

        QPoint glob = mapToGlobal(QPoint(width()/2, height()/2));
        QCursor::setPos(glob);

        mCurrentCamera->handleMouseMovement(xoffset, yoffset);


    }

    mMouseXlast = event->pos().x();
    mMouseYlast = event->pos().y();
}
