//
// Created by arseniy on 11.10.2024.
//

#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include "QAction"
#include <QMainWindow>
#include <QActionGroup>
#include <QSettings>
#include <QShortcut>
#include <QTranslator>
#include "map-widget/maptabwidget.h"
#include "map-widget/fogtool.h"
#include "map-widget/brushtool.h"
#include "map-widget/calibrationtool.h"
#include "map-widget/lighttool.h"
#include "map-widget/rulermaptool.h"
#include "map-widget/sharedmapwindow.h"
#include "map-widget/spellshapetool.h"
#include "qinitiativetrackerwidget.h"
#include "qplayerwidget.h"
#include "settingsdialog.h"


#define ORGANIZATION_NAME "Technohaster"
#define ORGANIZATION_DOMAIN "github.com/Technohamster-py"
#define APPLICATION_NAME "DM-assist"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief The MainWindow class serves as the main application window, managing user interface components and core functionality.
 *
 * MainWindow is the central component of the application, responsible for managing various tools, widgets, and map-related features.
 * It provides functionality for switching languages, saving and loading settings, and interacting with other application modules.
 *
 * The class includes handling of tools such as calibration, fog management, and lighting, along with features to work with map tabs
 * and a shared map window.
 *
 * Signals and slots are used extensively to enable communication between components, making the application dynamic and interactive.
 */
class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    QString workingDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/dm_assist_files/";

    ~MainWindow() override;

    void changeLanguage(const QString &languageCode);

signals:
    void languageChanged(const QString &languageCode);

public slots:
    void stopAll();
    void stopOtherPlayers(int exeptId);
    void saveSettings();
    void setVolumeDivider(int value);

    void loadSettings();

    void openHelp();
    void openDonate();

    void openSharedMapWindow(int index);
    void exportMap(int index);

private:
    Ui::MainWindow *ui;
    QTranslator translator;
    QString currentLanguage;

    int deviceIndex = -1;
    QVector<QPlayer*> players;

    QInitiativeTrackerWidget* initiativeTrackerWidget;

    TabWidget *mapTabWidget;
    SharedMapWindow* sharedMapWindow = nullptr;
    BrushTool* brushTool;
    CalibrationTool* calibrationTool;
    FogTool* fogTool;
    LightTool* lightTool;
    RulerMapTool* rulerMapTool;
    LineShapeTool* lineShapeTool;
    CircleShapeTool* circleShapeTool;
    SquareShapeTool* squareShapeTool;
    TriangleShapeTool* triangleShapeTool;


    void setupPlayers();
    void setupTracker();
    void setupToolbar();
    void setupMaps();
    void setupShortcuts();
    SettingsDialog *settingsDialog = nullptr;
    Settings paths;

    QActionGroup *toolGroup;

private slots:
    void loadConfigFile();
    void saveConfigFile();
    void on_actionSettings_triggered();

    void createNewMapTab();
    void deleteMapTab(int index);
    void updateVisibility();

    void setCalibrationMode();
    void setMeasureMode(bool checked);
    void setFogTool(bool checked, FogTool::Mode mode = FogTool::Hide);
    void setLightTool(bool checked);
    void coverMapWithFog(bool hide);
};


#endif //DM_ASSIST_MAINWINDOW_H
