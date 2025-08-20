#include "campaigntreewidget.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopServices>

/**
 * Constructs a CampaignTreeWidget object.
 *
 * This constructor initializes a tree widget with a single column, hides the header,
 * and applies a transparent background style. It also ensures that the widget's frame
 * is hidden and its background is translucent.
 *
 * @param parent The parent widget for the tree widget.
 */
CampaignTreeWidget::CampaignTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    setColumnCount(1);
    setHeaderHidden(true);
    m_rootPath = QString();

    setStyleSheet("QTreeWidget { background: transparent; }"
                  "QTreeWidget::item { background: transparent; }"
                  "QTreeWidget::item::hover { background: palette(Highlight); }");

    setAttribute(Qt::WA_TranslucentBackground);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFrameStyle(QFrame::NoFrame);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTreeWidget::customContextMenuRequested, this, &CampaignTreeWidget::showContextMenu);
}

/**
 * Determines the type of a node in the campaign tree based on its file path.
 *
 * This method analyzes the given file path to determine its node type
 * by checking for specific substrings that correspond to predefined
 * categories in the campaign structure, such as "Characters", "Encounters",
 * "Maps", and "Music". If no match is found, the node type is classified as
 * "Unknown".
 *
 * @param path The file path to analyze for determining the node type.
 * @return The determined NodeType, which can be one of the following:
 *         - NodeType::Character: Indicates the file path belongs to the "Characters" category.
 *         - NodeType::Encounter: Indicates the file path belongs to the "Encounters" category.
 *         - NodeType::Map: Indicates the file path belongs to the "Maps" category.
 *         - NodeType::Music: Indicates the file path belongs to the "Music" category.
 *         - NodeType::Unknown: Indicates the file path does not match any known category.
 */
NodeType CampaignTreeWidget::determieNodeType(const QString &path)
{
    if (path.contains("/Characters/"))
        return NodeType::Character;
    if (path.contains("/Encounters/"))
        return NodeType::Encounter;
    if (path.contains("/Maps/"))
        return NodeType::Map;
    if (path.contains("/Music"))
        return NodeType::Music;
    if (path.contains("/Bestiary"))
        return NodeType::Beast;
    return NodeType::Unknown;
}

/**
 * Determines whether a given file or directory should be ignored.
 *
 * This method evaluates the provided file or directory information against
 * specific conditions to assess whether it should be excluded from processing.
 * The conditions include:
 *
 * - Files or directories located under the "music" directory.
 * - Files or objects named "music".
 * - Files named "playerconfig.xml".
 * - Objects with the name "root".
 *
 * The check is performed using a normalized relative path from the root
 * directory specified by the `m_rootPath` member. File paths are cleaned
 * and converted to lower case for comparison.
 *
 * @param info The information about the file or directory, encapsulated
 *             in a `QFileInfo` object.
 * @return `true` if the specified file or directory matches the ignore
 *         criteria, otherwise `false`.
 */
bool CampaignTreeWidget::ignore(const QFileInfo &info)
{
    QString relativePath = QDir(m_rootPath).relativeFilePath(info.absoluteFilePath());
    QString normalized = QDir::cleanPath(relativePath).toLower();

    if (normalized.startsWith("music/") || normalized == "music")
        return true;
    if (normalized == "playerconfig.xml" || normalized == "root" || normalized == "campaign.json")
        return true;
    return false;
}

/**
 * Populates a campaign tree structure recursively based on the directory content.
 *
 * This function scans the content of the specified directory path, and for each
 * file or subdirectory, it creates and adds a corresponding tree item to the provided
 * parent item. The function categorizes each entry by its type, represented by NodeType,
 * and associates it with a customized HoverWidget. It also establishes connections
 * for specific actions triggered by user interactions with the HoverWidget based on
 * the node type.
 *
 * The function filters out entries that should be ignored according to custom rules
 * defined in the `ignore` method. For directories, it applies the same logic
 * recursively to build the entire tree structure.
 *
 * @param path The absolute file path of the directory to populate the tree from.
 * @param parentItem Pointer to the parent QTreeWidgetItem to which new items will be added.
 *
 * Node Types and Actions:
 * - NodeType::Character:
 *   - `action1Clicked`: Emits `characterOpenRequested` with the item's full path.
 *   - `action2Clicked`: Emits `characterAddRequested` with the item's full path.
 * - NodeType::Encounter:
 *   - `action1Clicked`: Emits `encounterAddRequested` with the item's full path.
 *   - `action2Clicked`: Emits `encounterReplaceRequested` with the item's full path.
 * - NodeType::Map:
 *   - `action1Clicked`: Emits `mapOpenRequested` with the item's full path.
 * - Other Node Types:
 *   - No specific actions are connected.
 *
 * Additional Details:
 * - New QTreeWidgetItem instances are created for each entry and added to the `parentItem`.
 * - HoverWidget instances are attached as item widgets in the tree, displaying the name
 *   and type of the entry.
 * - For directories, the function calls itself recursively to process subdirectories.
 * - Entries are skipped if they match patterns defined in the `ignore` method.
 *
 * Note:
 * This method assumes the valid root path has been previously set and all required
 * connections and configurations are properly initialized.
 */
void CampaignTreeWidget::populateTree(const QString &path, QTreeWidgetItem *parentItem)
{
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (ignore(entry))
            continue;

        NodeType type = determieNodeType(entry.absoluteFilePath());
        auto *item = new QTreeWidgetItem();
        parentItem->addChild(item);

        auto *widget = new HoverWidget(entry.fileName(), type);
        setItemWidget(item, 0, widget);

        QString fullPath = entry.absoluteFilePath();

        switch (type) {
            case NodeType::Character:
                connect(widget, &HoverWidget::action1Clicked, this, [=](){ emit characterOpenRequested(fullPath); });
                connect(widget, &HoverWidget::action2Clicked, this, [=](){ emit characterAddRequested(fullPath); });
                break;
            case NodeType::Encounter:
                connect(widget, &HoverWidget::action1Clicked, this, [=](){ emit encounterAddRequested(fullPath); });
                connect(widget, &HoverWidget::action2Clicked, this, [=](){ emit encounterReplaceRequested(fullPath); });
                break;
            case NodeType::Map:
                connect(widget, &HoverWidget::action1Clicked, this, [=](){ emit mapOpenRequested(fullPath); });
                break;
            case NodeType::Beast:
                connect(widget, &HoverWidget::action1Clicked, [=](){emit beastOpenRequested(fullPath);});
                connect(widget, &HoverWidget::action2Clicked, [=](){emit beastAddRequested(fullPath);});
            default:
                break;
        }

        if (entry.isDir()) {
            populateTree(entry.absoluteFilePath(), item);
        }
    }
}

/**
 * @brief Sets the root directory for the campaign and populates the tree widget.
 *
 * This function verifies if the provided directory path is a valid campaign root by checking
 * the existence of a "campaign.json" file. If the path is invalid, a warning message is displayed,
 * and the function returns false. Otherwise, the tree widget is cleared, and the new campaign is loaded
 * into the tree structure. The function creates a root tree widget item with the campaign name
 * and recursively populates the tree with child items representing the campaign's contents.
 * Upon successful loading, the tree is expanded, and a signal is emitted indicating that
 * the campaign has been loaded.
 *
 * @param rootPath The absolute path to the root directory of the campaign.
 * @return True if the campaign was successfully loaded, otherwise false.
 *
 * @note This function assumes the directory contains a "campaign.json" file with a valid structure.
 * @note The root tree widget item will use the campaign name retrieved from the "campaign.json" file.
 * @note Emits the campaignLoaded signal with the campaign name when the campaign is successfully loaded.
 */
bool CampaignTreeWidget::setRootDir(const QString &rootPath)
{
    if (!isValidCampaignRoot(rootPath)) {
        QMessageBox::warning(this, "Error", tr("The selected folder does not contain campaign.json is not a campaign"));
        return false;
    }

    clear();
    m_rootPath = QDir(rootPath).absolutePath();
    m_campaignName = loadCampaignName(m_rootPath);

    auto *rootItem = new QTreeWidgetItem(this);
    auto *widget = new HoverWidget(m_campaignName, NodeType::Unknown, this);
    setItemWidget(rootItem, 0, widget);

    populateTree(rootPath, rootItem);
    expandAll();

    emit campaignLoaded(m_campaignName);
    return true;
}

/**
 * @brief Determines if a given path is a valid campaign root.
 *
 * This function checks if the specified directory contains a file named
 * "campaign.json" to verify whether it qualifies as a valid campaign root.
 *
 * @param rootPath The file system path to be checked.
 * @return True if the path contains a "campaign.json" file, false otherwise.
 */
bool CampaignTreeWidget::isValidCampaignRoot(const QString &rootPath) {
    QFile markerFile(rootPath + "/campaign.json");
    return markerFile.exists();
}

/**
 * @brief Loads the campaign name from a campaign configuration file.
 *
 * This function attempts to read and parse a `campaign.json` file located
 * at the specified root path. It extracts the "name" field from the JSON
 * object and returns it. If the file cannot be opened, is not valid JSON,
 * or does not contain a "name" field, the default string "Unnamed Campaign"
 * is returned.
 *
 * @param rootPath The root directory path where the `campaign.json` file is located.
 * @return A QString representing the campaign name, or "Unnamed Campaign" if
 *         the name cannot be determined.
 */
QString CampaignTreeWidget::loadCampaignName(const QString &rootPath) {
    QFile file(rootPath + "/campaign.json");
    if (!file.open(QIODevice::ReadOnly))
        return "Unnamed Campaign";

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return "Unnamed Campaign";

    QJsonObject obj = doc.object();
    return obj.value("name").toString("Unnamed Campaign");
}


void CampaignTreeWidget::showContextMenu(const QPoint &pos) {
    QMenu menu;

    QAction *openAction = menu.addAction(tr("Show in file system"));
    connect(openAction, &QAction::triggered, [=](){
        QDesktopServices::openUrl(QUrl(QString("file:/%1").arg(m_rootPath)));
    });

    menu.exec(this->mapToGlobal(pos));
}
