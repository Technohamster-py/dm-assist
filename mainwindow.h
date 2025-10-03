#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include "QAction"
#include <QMainWindow>
#include <QActionGroup>
#include <QSettings>
#include <QShortcut>
#include <QTranslator>
#include <QProgressBar>
#include "campaign-tree-widget/campaigntreewidget.h"
#include "map-widget/maptabwidget.h"
#include "map-widget/fogtool.h"
#include "map-widget/brushtool.h"
#include "map-widget/calibrationtool.h"
#include "map-widget/lighttool.h"
#include "map-widget/rulertool.h"
#include "map-widget/sharedmapwindow.h"
#include "map-widget/spellshapetool.h"
#include "map-widget/heightmaptool.h"
#include "initiative-tracker/initiativetrackerwidget.h"
#include "music-widget/musicwidget.h"
#include "roll-widget/rollwidget.h"
#include "settingsdialog.h"
#include "updatechecker.h"


static QMap<QString, QString> sourcesMap = {
        {"Icons for initiative statuses", "https://ttg.club/"}
};


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
    void translatorChanged();

public slots:
    void stopAll();
    void stopOtherPlayers(int exeptId);
    void setVolumeDivider(int value);

    void saveSettings();
    void loadSettings();

    void openSharedMapWindow(int index);
    void slotExportMap(int index);

    void handleUpdates(bool hasUpdates) const;

    void slotUpdateProgressBar(int percent, const QString& message = "");

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::MainWindow *ui;
    QTranslator translator;
    UpdateChecker* updateChecker;
    bool m_checkForUpdates;
    QString currentLanguage;
    QString currentCampaignDir;
    QString defaultCampaignDir;
    bool openLastMap = false;

    int deviceIndex = -1;
    QVector<MusicPlayerWidget*> players;
    int prevVolume = 100;
    bool isMuted = false;

    CampaignTreeWidget* campaignTreeWidget;

    InitiativeTrackerWidget* initiativeTrackerWidget;
    bool autoRoll = false;

    TabWidget *mapTabWidget;
    SharedMapWindow* sharedMapWindow = nullptr;
    BrushTool* brushTool;
    CalibrationTool* calibrationTool;
    FogTool* fogTool;
    LightTool* lightTool;
    RulerTool* rulerMapTool;
    LineShapeTool* lineShapeTool;
    CircleShapeTool* circleShapeTool;
    SquareShapeTool* squareShapeTool;
    TriangleShapeTool* triangleShapeTool;
    HeightMapTool* heightMapTool;
    QActionGroup *toolGroup;

    QToolButton* rulerButton;
    QToolButton* fogHideButton;
    QToolButton* fogRevealButton;
    QToolButton* lightButton;
    QToolButton* lineButton;
    QToolButton* circleButton;
    QToolButton* squareButton;
    QToolButton* triangleButton;
    QToolButton* brushButton;
    QToolButton* heightButton;

    RollWidget* rollWidget;

    QProgressBar* progressBar;

    void setupCampaign(const QString &campaignRoot);
    void setupPlayers();
    void setupTracker();
    void setupToolbar();
    void setupMaps();
    void setupShortcuts();
    void exportMap(const QString& path, int index);

    SettingsDialog *settingsDialog = nullptr;
    Settings paths;

    int currentTokenTitleMode = 0;
    int currentTokenFontSize = 12;

private slots:
    void newCampaign();
    void loadCampaign();
    void closeCampaign();

    void addCharacter();

    void on_muteButton_clicked();
    void loadMusicConfigFile(QString fileName);
    void saveMusicConfigFile(const QString& fileName);
    void on_actionSettings_triggered();

    void createNewMapTab();
    void openMapFromFile(const QString& path);
    void deleteMapTab(int index);
    void updateVisibility();

    void coverMapWithFog(bool hide);
    void showSourcesMessageBox(const QMap<QString, QString> &sources);
};


#endif //DM_ASSIST_MAINWINDOW_H
