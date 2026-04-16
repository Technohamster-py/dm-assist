#ifndef DM_ASSIST_MAINWINDOW_H
#define DM_ASSIST_MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QTranslator>
#include <QProgressBar>
#include <QToolButton>
#include "src/modules/campaign/tree-widget/campaigntreewidget.h"
#include "src/modules/map/widgets/map-tab-widget/maptabwidget.h"
#include "src/modules/map/tools/fog/fogtool.h"
#include "src/modules/map/tools/brush/brushtool.h"
#include "src/modules/map/tools/calibration/calibrationtool.h"
#include "src/modules/map/tools/light/lighttool.h"
#include "src/modules/map/tools/ruler/rulertool.h"
#include "src/modules/map/widgets/shared-map-window/sharedmapwindow.h"
#include "src/modules/map/tools/spell-shape/spellshapetool.h"
#include "src/modules/map/tools/height-map/heightmaptool.h"
#include "src/modules/encounter/widget//initiativetrackerwidget.h"
#include "src/modules/music/widget/musicwidget.h"
#include "src/modules/rolls/roll-widget/rollwidget.h"
#include "src/core/dialogs/settings-dialog/settingsdialog.h"
#include "src/core/update-manager/updatemanager.h"


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


    ~MainWindow() override;

    void changeLanguage(const QString &languageCode);
    QStringList recentCampaigns() const {return m_recentCampaignList;}
    void openCampaign(const QString& campaignRootDir = "");

    static void clearTmp();

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
    UpdateManager* updateManager;

    QString workingDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/dm_assist_files/";
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
    bool m_autoRollCharacter = false;
    bool m_autoRollBeast = false;

    TabWidget *mapTabWidget;
    SharedMapWindow* sharedMapWindow = nullptr;
    bool autoSharedMapMapMode = false;
    BrushTool* brushTool;
    CalibrationTool* calibrationTool;
    FogTool* fogTool;
    LightTool* lightTool;
    RulerTool* rulerMapTool;
    LineShapeTool* lineShapeTool;
    CircleShapeTool* circleShapeTool;
    SquareShapeTool* squareShapeTool;
    TriangleShapeTool* triangleShapeTool;
    LassoTool* lassoTool;
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
    QToolButton* lassoButton;
    QToolButton* brushButton;
    QToolButton* heightButton;

    RollWidget* rollWidget;

    QProgressBar* progressBar;

    void setupCampaign(const QString &campaignRoot);
    void addCampaignToRecentList(const QString& path);
    QStringList m_recentCampaignList;
    void updateRecentMenu();
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
    int m_defaultGridSize = 5;
    qreal m_masterFogOpacity = 0.4;
    qreal m_playerFogOpacity = 1.0;
    QColor m_fogColor = Qt::black;
    qreal m_textureOpacity = 0.5;

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
