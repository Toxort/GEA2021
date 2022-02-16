#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector3D>

class QWidget;
class RenderWindow;
class GameEngine;
class QListWidget;
class QComboBox;
class GameObject;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initList();
    void updateList();
    void refreshList();
    void setID(int ID);
    void clean();

    void initComboboxTexture(std::vector<std::string> &textureNames);
    void initComboboxMeshes(std::vector<std::string> &meshNames);
    void initComboboxLevels(std::vector<std::string> &levelNames);
    void updateComboboxLevels(std::vector<std::string> &levelNames);

    int getObjectListIndex() const;

    const std::string getCurrentLevelName() const;
    std::string levelName{""};
    float randomNumber(int min, int max, bool negative = false);
    
public slots:

    void on_PlayStop_toggled(bool checked);
    void on_actionDelete_Selected_triggered();

    void on_loadScene_clicked();

private slots:


    void on_listWidget_currentRowChanged(int currentRow);

    void on_TranslateXspinBox_valueChanged(double arg1);


    void on_TranslateYspinBox_valueChanged(double arg1);

    void on_TranslateZspinBox_valueChanged(double arg1);

    void on_RotateXspinBox_valueChanged(double arg1);

    void on_actionCube_triggered();

    void on_actionPyramid_triggered();

    void on_actionSphere_triggered();

    void on_actionSuzanne_triggered();

    void on_actionCube_hovered();

    void on_ScaleXspinBox_valueChanged(double arg1);



    void on_actionGetCurrentRow_triggered();

    void on_saveScene_clicked();



    void on_actionLOD_toggle_toggled(bool arg1);

    void on_actionFrustum_Culling_toggle_toggled(bool arg1);

    void on_actionShow_Wireframe_toggle_toggled(bool arg1);

    void on_actionShow_All_Collision_Boxes_toggle_toggled(bool arg1);


    void on_comboBox_currentIndexChanged(int index);

    void on_addObject_released();

    void on_comboBox_2_currentIndexChanged(int index);

    void on_RotateYspinBox_valueChanged(double arg1);

    void on_RotateZspinBox_valueChanged(double arg1);

    void on_actionInfo_triggered();

//    void on_lineEdit_textEdited(const QString &arg1);

    void on_ScaleYspinBox_valueChanged(double arg1);

    void on_ScaleZspinBox_valueChanged(double arg1);

    void on_comboBox_3_currentIndexChanged(int index);

    void on_comboBox_3_currentTextChanged(const QString &arg1);


    void on_actionPlay_toggled(bool arg1);

    void on_actionReset_Ball_triggered();

    void on_usingFrustumCulling_toggled(bool checked);

private:
    void init();
    Ui::MainWindow *ui;

    int ObjectListIndex{0};
    int lastIndex{0};
    int lastXrot{0};
    int xcounter{0};

    float scaleX{1};
    float scaleY{1};
    float scaleZ{1};

        bool getPosOnce{true};

    QVector3D lastBallPos{0,0,0};

    bool bCurrentlyDeleting = false;


    QWidget *mRenderWindowContainer{nullptr};
    RenderWindow *mRenderWindow{nullptr};
    GameEngine *mGameEngine{nullptr};

    QListWidget *listWidget{nullptr};
    QComboBox *textureComboBox{nullptr};
    QComboBox *meshComboBox{nullptr};
    QComboBox *levelComboBox{nullptr};

    std::vector<GameObject*> GameObjects{nullptr};
    std::vector<std::string> allTextures;
    std::vector<std::string> allMeshes;
    std::vector<std::string> allLevels;

    int currentTextureIndex{0};
    int currentMeshIndex{0};
    int currentLevelIndex{0};

    std::string currentLevelName{"default"};

};

#endif // MAINWINDOW_H
