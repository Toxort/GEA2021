#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSurfaceFormat>
#include <QDebug>
#include <QScreen>  //for resizing the program at start
#include <QListWidget>
#include <QComboBox>
#include <QStyleFactory>
#include <QMessageBox>

#include<time.h>
#include<cstdlib>
#include <random>

#include "renderwindow.h"
#include "gameengine.h"
#include "transformsystem.h"
#include "collisionsystem.h"
#include "particlesystem.h"
#include "aisystem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    //this sets up what's in the mainwindow.ui
    ui->setupUi(this);

    listWidget = ui->listWidget;
    textureComboBox = ui->comboBox;
//    textureComboBox->setEditable(true);
    meshComboBox = ui->comboBox_2;
    levelComboBox = ui->comboBox_3;
    levelComboBox->setEditable(true);

    ui->GameObjectsGroupBox->setTitle("Scene: " + QString::fromStdString(currentLevelName));


    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    init();
}

MainWindow::~MainWindow()
{
    delete mRenderWindow;
    delete ui;
}

void MainWindow::initList()
{
    if(mRenderWindow)
    {
        GameObjects.clear();
        GameObjects = mRenderWindow->getAllGameObject();
        for(auto it : GameObjects)
        {
            new QListWidgetItem(tr((*it).name.c_str()), listWidget);
        }
    }
    listWidget->setCurrentRow(0);

}

void MainWindow::updateList()
{
    if(mRenderWindow)
    {
        GameObjects.clear();
        GameObjects = mRenderWindow->getAllGameObject();
        new QListWidgetItem(tr( GameObjects[GameObjects.size()-1]->name.c_str() ), listWidget);
    }
}

void MainWindow::refreshList()
{
    if(mRenderWindow)
    {
        GameObjects.clear();
        GameObjects = mRenderWindow->getAllGameObject();
        //listWidget->setCurrentRow(-1);
        for (int i = GameObjects.size(); i>0; i--) {
            listWidget->takeItem(i);
        }
        static bool once {true};
        for(auto it : GameObjects)
        {
            if(once)
            {
                once = false;
                continue;
            }
            new QListWidgetItem(tr((*it).name.c_str()), listWidget);
        }
        once = true;
    }
}



void MainWindow::setID(int ID)
{
    listWidget->setCurrentRow(ID);


    // Small "on hit enemy" functunallity
    if(GameObjects[ID]->mAIComponent != nullptr && GameEngine::getInstance()->bIsPlaying)
    {
        if(AISystem::onHit(GameObjects[ID]))
        {
            ObjectListIndex = ID;
            on_actionDelete_Selected_triggered();
        }
    }
}

void MainWindow::clean()
{
    GameObjects.clear();
    listWidget->clear();
}


void MainWindow::initComboboxTexture(std::vector<std::string> &textureNames)
{
    if(mRenderWindow)
    {
        allTextures = textureNames;
        for(auto it : textureNames)
        {
            it.erase(it.end()-4,it.end());
            textureComboBox->addItem(tr(it.c_str()));
        }
    }
}

void MainWindow::initComboboxMeshes(std::vector<std::string> &meshNames)
{
    if(mRenderWindow)
    {
        allMeshes = meshNames;
        for(auto it : allMeshes)
        {
            it.erase(it.end()-4,it.end());
            meshComboBox->addItem(tr(it.c_str()));
        }
    }
}

void MainWindow::initComboboxLevels(std::vector<std::string> &levelNames)
{
    if(mRenderWindow)
    {
        allLevels = levelNames;
        for(auto it : allLevels)
        {
//            std::string tempName = it;
            it.erase(it.end()-5,it.end());
            levelComboBox->addItem(tr(it.c_str()));
        }
    }
}

void MainWindow::updateComboboxLevels(std::vector<std::string> &levelNames)
{
    levelComboBox->clear();
    if(mRenderWindow)
    {
        allLevels = levelNames;
        for(auto it : allLevels)
        {
//            std::string tempName = it;
            it.erase(it.end()-5,it.end());
            levelComboBox->addItem(tr(it.c_str()));
        }
    }
//    if(mRenderWindow)
//    {
//        GameObjects.clear();
//        GameObjects = mRenderWindow->getAllGameObject();
//        //listWidget->setCurrentRow(-1);
//        for (int i = GameObjects.size(); i>0; i--) {
//            listWidget->takeItem(i);
//        }
//        static bool once {true};
//        for(auto it : GameObjects)
//        {
//            if(once)
//            {
//                once = false;
//                continue;
//            }
//            new QListWidgetItem(tr((*it).name.c_str()), listWidget);
//        }
//        once = true;
//    }
}

void MainWindow::init()
{
    //This will contain the setup of the OpenGL surface we will render into
    QSurfaceFormat format;

    //OpenGL v 4.1 - (Ole Flatens Mac does not support higher than this - sorry!)
    //you can try other versions, but then have to update RenderWindow and Shader
    //to inherit from other than QOpenGLFunctions_4_1_Core
    format.setVersion(4, 1);
    //Using the main profile for OpenGL - no legacy code permitted
    format.setProfile(QSurfaceFormat::CoreProfile);
    //A QSurface can be other types than OpenGL
    format.setRenderableType(QSurfaceFormat::OpenGL);

    //This should activate OpenGL debug Context used in RenderWindow::startOpenGLDebugger().
    //This line (and the startOpenGLDebugger() and checkForGLerrors() in RenderWindow class)
    //can be deleted, but it is nice to have some OpenGL debug info!
    format.setOption(QSurfaceFormat::DebugContext);

    // The renderer will need a depth buffer - (not requiered to set in glfw-tutorials)
    format.setDepthBufferSize(24);

    //Set the number of samples used for multisampling
    //Same as glfwWindowHint(GLFW_SAMPLES, 4);
    //https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
    format.setSamples(4);

    //Just prints out what OpenGL format we try to get
    // - this can be deleted
    qDebug() << "Requesting surface format: " << format;

    //We have a format for the OpenGL window, so let's make it:
    mRenderWindow = new RenderWindow(format, this);

    //Check if renderwindow did initialize, else prints error and quit
    if (!mRenderWindow->context()) {
        qDebug() << "Failed to create context. Can not continue. Quits application!";
        delete mRenderWindow;
        return;
    }

    //The OpenGL RenderWindow got made, so continuing the setup:
    //We put the RenderWindow inside a QWidget so we can put in into a
    //layout that is made in the .ui-file
    mRenderWindowContainer = QWidget::createWindowContainer(mRenderWindow);
    //OpenGLLayout is made in the .ui-file!
    ui->OpenGLLayout->addWidget(mRenderWindowContainer);

    //Set the size of the program in % of the actual screen size
    QSize tempSize = QGuiApplication::primaryScreen()->size();

    tempSize.rheight() *= 0.85; //original value 0.85
    tempSize.rwidth() *= 0.90; //original value 0.65
    resize(tempSize);

    GameEngine::getInstance()->setRenderPointer(mRenderWindow , this);

    //sets the keyboard input focus to the RenderWindow when program starts
    // - can be deleted, but then you have to click inside the renderwindow to get the focus
    mRenderWindowContainer->setFocus();

}

const std::string MainWindow::getCurrentLevelName() const
{
    return currentLevelName;
}

int MainWindow::getObjectListIndex() const
{
    return ObjectListIndex;
}

//Example of a slot called from the button on the top of the program.

void MainWindow::on_PlayStop_toggled(bool checked)
{
        mGameEngine->getInstance()->togglePlay(checked);
        if(checked)
            ui->PlayStop->setText("Stop");
        else
        {
            ui->PlayStop->setText("Play");
            ui->PlayStop->setChecked(false);
        }

}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    if(bCurrentlyDeleting)
    {
        lastIndex = currentRow-1;
        return;
    }
    ObjectListIndex = currentRow;
    ui->TranslateXspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mMatrix.getPosition().x);
    ui->TranslateYspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mMatrix.getPosition().y);
    ui->TranslateZspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mMatrix.getPosition().z);
    ui->RotateXspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mRotation.getX());
    ui->RotateYspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mRotation.getY());
    ui->RotateZspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mRotation.getZ());



    ui->ScaleXspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mScale.getX());
    ui->ScaleYspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mScale.getY());
    ui->ScaleZspinBox->setValue(GameObjects[ObjectListIndex]->mTransformComp->mScale.getZ());

    ui->usingFrustumCulling->setChecked(GameObjects[ObjectListIndex]->mMeshComp->bUsingFrustumCulling);


    // --Visible Selection in 3D window--
    // Turn off collision box of the last selected
        bool &isPlaying = GameEngine::getInstance()->bIsPlaying;
    if(GameObjects.size()>2)
    {
        GameObjects[lastIndex]->mCollisionComp->bShowCollisionBox = false;
        if(!isPlaying)
            GameObjects[ObjectListIndex]->mCollisionComp->bShowCollisionBox = true;
    }
    lastIndex = currentRow;
}


void MainWindow::on_TranslateXspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D pos = GameObjects[ObjectListIndex]->mTransformComp->mMatrix.getPosition();
    TransformSystem::setPosition(GameObjects[ObjectListIndex], gsl::Vector3D(arg1,pos.y,pos.z));
}


void MainWindow::on_TranslateYspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D pos = GameObjects[ObjectListIndex]->mTransformComp->mMatrix.getPosition();
    TransformSystem::getInstance()->setPosition(GameObjects[ObjectListIndex], gsl::Vector3D(pos.x,arg1,pos.z));
}

void MainWindow::on_TranslateZspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D pos = GameObjects[ObjectListIndex]->mTransformComp->mMatrix.getPosition();
    TransformSystem::getInstance()->setPosition(GameObjects[ObjectListIndex], gsl::Vector3D(pos.x,pos.y,arg1));
}


void MainWindow::on_RotateXspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;
    gsl::Vector3D currentRot = GameObjects[ObjectListIndex]->mTransformComp->mRotation;
    TransformSystem::getInstance()->setRotation(GameObjects[ObjectListIndex], gsl::Vector3D(arg1, currentRot.y, currentRot.z));
//    if(GameObjects[ObjectListIndex]->mTransformComp->mRotation.getY());

}

void MainWindow::on_RotateYspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D currentRot = GameObjects[ObjectListIndex]->mTransformComp->mRotation;
    TransformSystem::getInstance()->setRotation(GameObjects[ObjectListIndex], gsl::Vector3D(currentRot.x, arg1, currentRot.z));
}

void MainWindow::on_RotateZspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D currentRot = GameObjects[ObjectListIndex]->mTransformComp->mRotation;
    TransformSystem::getInstance()->setRotation(GameObjects[ObjectListIndex], gsl::Vector3D(currentRot.x, currentRot.y, arg1));
}

void MainWindow::on_ScaleXspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;
    gsl::Vector3D curScale = GameObjects[ObjectListIndex]->mTransformComp->mScale;
    TransformSystem::getInstance()->setScale(GameObjects[ObjectListIndex], gsl::Vector3D(arg1,curScale.y,curScale.z));
}

void MainWindow::on_ScaleYspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D curScale = GameObjects[ObjectListIndex]->mTransformComp->mScale;
    TransformSystem::getInstance()->setScale(GameObjects[ObjectListIndex], gsl::Vector3D(curScale.x,arg1,curScale.z));
}

void MainWindow::on_ScaleZspinBox_valueChanged(double arg1)
{
    if(ObjectListIndex == 0)
        return;

    gsl::Vector3D curScale = GameObjects[ObjectListIndex]->mTransformComp->mScale;
    TransformSystem::getInstance()->setScale(GameObjects[ObjectListIndex], gsl::Vector3D(curScale.x,curScale.y,arg1));
}


void MainWindow::on_actionCube_triggered()
{
    mGameEngine->getInstance()->CreateCube();
    //listWidget->currentRowChanged(GameObjects.size()-1);
    updateList();
    listWidget->setCurrentRow(GameObjects.size()-1);
}


void MainWindow::on_actionPyramid_triggered()
{
    mGameEngine->getInstance()->CreatePyramid();
    updateList();
    listWidget->setCurrentRow(GameObjects.size()-1);
}


void MainWindow::on_actionSphere_triggered()
{
    mGameEngine->getInstance()->CreateSphere();
    updateList();
    listWidget->setCurrentRow(GameObjects.size()-1);
}


void MainWindow::on_actionSuzanne_triggered()
{
    mGameEngine->getInstance()->CreateSuzanne();
    updateList();
    listWidget->setCurrentRow(GameObjects.size()-1);
}


void MainWindow::on_actionCube_hovered()
{
    //ObjectListIndex = 0;
}

void MainWindow::on_actionDelete_Selected_triggered()
{
    if(ObjectListIndex == 0)
    {
        qDebug() << "You can not delete the AXIS!";
        return;
    }
    bCurrentlyDeleting = true;
    mRenderWindow->deleteGameObjectAt(ObjectListIndex);
    GameObjects.erase(GameObjects.begin() + ObjectListIndex);
    listWidget->takeItem(listWidget->currentRow());
    //listWidget->setCurrentRow(0);
    bCurrentlyDeleting = false;
    //listWidget->removeItemWidget(listWidget->currentItem());

    //refreshList();
    ObjectListIndex = ObjectListIndex - 1;
    listWidget->setCurrentRow(GameObjects.size()-1);
}


void MainWindow::on_actionGetCurrentRow_triggered()
{
        qDebug() << "currentRow" << listWidget->currentRow();
        qDebug() << "objIndex: " << ObjectListIndex;
}


void MainWindow::on_saveScene_clicked()
{
    qDebug() << "Tying to save " << QString::fromStdString(levelName);
    GameEngine::getInstance()->saveScene(levelName);
    currentLevelName = levelName;
    ui->GameObjectsGroupBox->setTitle("Scene: " + QString::fromStdString(currentLevelName));
    mGameEngine->getInstance()->updateAllLevels();
}


void MainWindow::on_loadScene_clicked()
{
    qDebug() << "Tying to load " << QString::fromStdString(levelName);
    listWidget->setCurrentRow(0);
    GameEngine::getInstance()->loadScene(levelName);
    currentLevelName = levelName;
    ui->GameObjectsGroupBox->setTitle("Scene: " + QString::fromStdString(currentLevelName));
    refreshList();
}

void MainWindow::on_actionLOD_toggle_toggled(bool arg1)
{
    mRenderWindow->renderLOD(!arg1);
    if(arg1)
        ui->actionLOD_toggle->setText("LOD ON");
    else
        ui->actionLOD_toggle->setText("LOD OFF");
}


void MainWindow::on_actionFrustum_Culling_toggle_toggled(bool arg1)
{
    mRenderWindow->toggleFrustumCulling(!arg1);
    if(arg1)
        ui->actionFrustum_Culling_toggle->setText("Frustum Culling ON");
    else
        ui->actionFrustum_Culling_toggle->setText("Frustum Culling OFF");
}


void MainWindow::on_actionShow_Wireframe_toggle_toggled(bool arg1)
{
    mRenderWindow->toggleWireframe(arg1);
}


void MainWindow::on_actionShow_All_Collision_Boxes_toggle_toggled(bool arg1)
{
    mRenderWindow->toggleShowCollsionBox(arg1);
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    currentTextureIndex = index;
}


void MainWindow::on_addObject_released()
{
    mGameEngine->getInstance()->CreateObject(gsl::MeshFilePath + allMeshes[currentMeshIndex], false ,allTextures[currentTextureIndex]);
    updateList();
    if(GameObjects.size()>0)
        listWidget->setCurrentRow(GameObjects.size()-1);
    else
        listWidget->setCurrentRow(GameObjects.size());
}


void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    currentMeshIndex = index;
}

void MainWindow::on_actionInfo_triggered()
{
    QMessageBox::about(this, "Basic Controls",
                       "EDITOR: Move and rotate camera by holding down right mouse button and use WASD.\n"
                       "Q = down, E = up. Use ScrollWheel to adjust speed, DEL to delete object.\n\n"
                       "GAME: Click mouse once to activate player WASD controll, and press escape to exit\n\n");
}

//void MainWindow::on_lineEdit_textEdited(const QString &arg1)
//{
//    levelName = arg1.toStdString();
//}

void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    currentLevelIndex = index;
    qDebug() << "levelIndex:" << index;
}


void MainWindow::on_comboBox_3_currentTextChanged(const QString &arg1)
{
    levelName = arg1.toStdString();
    qDebug () << arg1;
}



void MainWindow::on_actionPlay_toggled(bool arg1)
{
    GameEngine::getInstance()->bBallPhysicsIsPlaying = arg1;

//    if(getPosOnce)
//    {
//        lastBallPos.setX(GameEngine::getInstance()->mPhysicsBall->mTransformComp->mMatrix.getPosition().getX());
//        lastBallPos.setY(GameEngine::getInstance()->mPhysicsBall->mTransformComp->mMatrix.getPosition().getY());
//        lastBallPos.setZ(GameEngine::getInstance()->mPhysicsBall->mTransformComp->mMatrix.getPosition().getZ());
//        getPosOnce = false;
//    }

}


void MainWindow::on_actionReset_Ball_triggered()
{
    getPosOnce = true;
    GameEngine::getInstance()->ResetBallVelocity();
    GameEngine::getInstance()->mPhysicsBall->mTransformComp->mMatrix.setPosition(lastBallPos.x(),lastBallPos.y(),lastBallPos.z());

}


void MainWindow::on_usingFrustumCulling_toggled(bool checked)
{
    GameObjects[ObjectListIndex]->mMeshComp->bUsingFrustumCulling = checked;
}



