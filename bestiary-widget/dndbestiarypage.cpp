#include "dndbestiarypage.h"
#include "ui_dndbestiarypage.h"

#include <QFile>
#include <QMessageBox>
#include <QJsonArray>


/**
 * Constructs a DndBestiaryPage widget.
 *
 * @param parent The parent QWidget for this widget. Defaults to nullptr if no parent is provided.
 *
 * @return An instance of DndBestiaryPage initialized with the provided parent widget.
 */
DndBestiaryPage::DndBestiaryPage(QWidget *parent) : AbstractCharsheetWidget(parent), ui(new Ui::DndBestiaryPage) {
    ui->setupUi(this);
    m_manager = new QNetworkAccessManager(this);
    connect(ui->infoField, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});
    connect(ui->skillsBrowser, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});
}

DndBestiaryPage::~DndBestiaryPage() {
    delete ui;
}

/**
 * Creates an instance of DndBestiaryPage.
 *
 * @param parent The parent widget for this instance. If no parent is specified, it defaults to nullptr.
 *
 * @return A new instance of DndBestiaryPage configured with the given parent widget.
 */
DndBestiaryPage::DndBestiaryPage(QString filePath, QWidget *parent) : AbstractCharsheetWidget(parent), ui(new Ui::DndBestiaryPage) {
    ui->setupUi(this);
    m_manager = new QNetworkAccessManager(this);
    connect(ui->infoField, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});
    connect(ui->skillsBrowser, &RollTextBrowser::rollRequested, [=](const QString& expr){emit rollRequested(expr);});

    loadFromFile(filePath);
}

/**
 * Loads bestiary data from the specified file path and populates the widget with the parsed data.
 *
 * @param path The file path of the bestiary data file to load.
 */
void DndBestiaryPage::loadFromFile(const QString &path) {
    QFile beastFile(path);
    m_campaignPath = campaignDirFromFile(path);

    if (!beastFile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open bestiary file");
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(beastFile.readAll());
    QJsonObject root = document.object();

    if (!m_parser)
        delete m_parser;

    if (!root["system"].isNull())
        m_parser = new Fvtt11Parser;
    else
        m_parser = new Fvtt10Parser;

    populateWidget(m_parser->parse(root));
}

/**
 * Adds the current character's details to an initiative tracker.
 *
 * @param initiativeTrackerWidget The widget managing the initiative tracker.
 * @param autoRoll Determines whether the initiative roll should be computed automatically.
 *                 If true, the initiative is rolled as a d20 added to the character's Dexterity bonus.
 */
void DndBestiaryPage::addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget, bool autoRoll) {
    int initiative = 0;
    if (autoRoll)
        initiative = rollDice(20) + ui->dexBonusLabel->text().toInt();
    initiativeTrackerWidget->addCharacter(ui->nameLabel->text(), ui->hpLabel->text().toInt(), ui->acLabel->text().toInt(), ui->hpLabel->text().toInt(), initiative);
}

void DndBestiaryPage::updateTranslator() {
    ui->retranslateUi(this);
}

/**
 * Downloads a token from the specified URL and saves it to the campaign's token directory.
 *
 * This method validates the URL, ensures the campaign directory and token subdirectory exist,
 * and downloads the token asynchronously. If the file already exists, the token is immediately
 * loaded without re-downloading.
 *
 * @param link The URL of the token to be downloaded.
 *
 * @return True if the download process was successfully initiated or the token already exists;
 *         false if the URL is invalid, the directory structure cannot be created, or other errors occur.
 */
bool DndBestiaryPage::downloadToken(const QString &link) {
    QUrl qurl(link);
    if (!qurl.isValid()) {
        qWarning() << "Invalid URL:" << link;
        return false;
    }

    QString filename = qurl.fileName();
    if (filename.isEmpty()) {
//        qWarning() << "URL does not contain filename:" << link;
        return false;
    }

    QDir dir(m_campaignPath);
    if (!dir.exists()) {
//        qWarning() << "Campaign dir does not exist:" << m_campaignPath;
        return false;
    }

    // ensure tokens/ folder
    if (!dir.exists("Tokens")) {
        if (!dir.mkdir("Tokens")) {
            qWarning() << "Cannot create tokens dir!";
            return false;
        }
    }

    QString fullPath = dir.filePath("Tokens/" + filename);
    QFileInfo fi(fullPath);
    if (fi.exists()) {
//        qInfo() << "Token already exists:" << fullPath;
        setTokenPixmap(fullPath);
        return false;
    }

    // async download
    QNetworkRequest req(qurl);
    auto reply = m_manager->get(req);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Download failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }
        QByteArray data = reply->readAll();

        QFile f(fullPath);
        if (!f.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot write file:" << fullPath;
            reply->deleteLater();
            return;
        }
        f.write(data);
        f.close();
//        qInfo() << "Saved token:" << fullPath;
        setTokenPixmap(fullPath);
        reply->deleteLater();
    });
    return true;
}

/**
 * Sets the pixmap of the token display based on the provided file path.
 *
 * @param filePath The file path of the image to be used as the token pixmap.
 */
void DndBestiaryPage::setTokenPixmap(const QString &filePath) {
    ui->token->setPixmap(QPixmap(filePath));
}

/**
 * Populates the UI of the DndBestiaryPage widget with the provided BestiaryPageData.
 *
 * This function updates various UI labels and fields with the corresponding values
 * from the `BestiaryPageData` structure. It also includes the logic to calculate
 * and display ability bonuses and saving throws based on the provided data, as well
 * as setting text for attributes such as skills, damage resistances, details, and more.
 *
 * @param data The `BestiaryPageData` object containing all relevant information
 *             to populate the widget.
 */
void DndBestiaryPage::populateWidget(BestiaryPageData data) {
    ui->nameLabel->setText(data.name);

    if (!data.imgLink.isEmpty())
        downloadToken(data.imgLink);

    ui->acLabel->setText(data.ac);
    ui->acCalcLabel->setText(data.acCalc);
    ui->hpLabel->setText(data.hp);
    ui->hpFormula->setText(data.hpFormula);
    ui->profLabel->setText(QString::number(data.prof));
    ui->speedLabel->setText(data.speed);
    ui->fellingsLabel->setText(data.senses);
    ui->langLabel->setText(data.languages);
    ui->diLabel->setText(data.damageImmunity);
    ui->drLabel->setText(data.damageResistance);
    ui->ciLabel->setText(data.conditionImmunity);
    ui->dvLabel->setText(data.damageVulnerability);

    int saveBonus;
    ui->strValueLabel->setText(QString::number(data.abilities["str"]));
    ui->strBonusLabel->setText(QString::number(bonusFromStat(data.abilities["str"])));
    saveBonus = data.abilities["str"] + ((data.abilityProf["str"]) ? data.prof : 0);
    ui->strSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->dexValueLabel->setText(QString::number(data.abilities["dex"]));
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(data.abilities["dex"])));
    saveBonus = data.abilities["dex"] + ((data.abilityProf["dex"]) ? data.prof : 0);
    ui->dexSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->conValueLabel->setText(QString::number(data.abilities["con"]));
    ui->conBonusLabel->setText(QString::number(bonusFromStat(data.abilities["con"])));
    saveBonus = data.abilities["con"] + ((data.abilityProf["con"]) ? data.prof : 0);
    ui->conSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->intValueLabel->setText(QString::number(data.abilities["int"]));
    ui->intBonusLabel->setText(QString::number(bonusFromStat(data.abilities["int"])));
    saveBonus = data.abilities["int"] + ((data.abilityProf["int"]) ? data.prof : 0);
    ui->intSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->wisValueLabel->setText(QString::number(data.abilities["wis"]));
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(data.abilities["wis"])));
    saveBonus = data.abilities["wis"] + ((data.abilityProf["wis"]) ? data.prof : 0);
    ui->wisSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->chaValueLabel->setText(QString::number(data.abilities["cha"]));
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(data.abilities["cha"])));
    saveBonus = data.abilities["cha"] + ((data.abilityProf["cha"]) ? data.prof : 0);
    ui->chaSaveBonus->setText((saveBonus > 0) ? "+" + QString::number(saveBonus) : QString::number(saveBonus));

    ui->skillsBrowser->setCustomHtml(data.skills);
    ui->detailsLabel->setText(data.details);

    ui->dangerLabel->setText(QString("%1 (%2 XP)").arg(data.dangerLevel, data.xp));
    ui->infoField->setCustomHtml(data.description);
}
