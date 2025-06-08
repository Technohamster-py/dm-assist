#include "dndcharsheetwidget.h"
#include "ui_dndcharsheetwidget.h"

#include <QMessageBox>


DndCharsheetWidget::DndCharsheetWidget(QWidget* parent) :
        AbstractCharsheetWidget(parent), ui(new Ui::DndCharsheetWidget) {
    ui->setupUi(this);

    connectSignals();
}

DndCharsheetWidget::DndCharsheetWidget(const QString& filePath, QWidget *parent): DndCharsheetWidget(parent){
    loadFromFile(filePath);
}

DndCharsheetWidget::~DndCharsheetWidget() {
    delete ui;
}

void DndCharsheetWidget::loadFromFile(QString path) {
    m_originalFilePath = path;
    QFile characterFile(path);
    if (!characterFile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open character file");
        return;
    }
    m_originalDocument = QJsonDocument::fromJson(characterFile.readAll());
    QJsonObject rootObj = m_originalDocument.object();

    QString dataString = rootObj.value("data").toString();
    m_dataObject = QJsonDocument::fromJson(dataString.toUtf8()).object();

    characterFile.close();

    populateWidget();
}

void DndCharsheetWidget::populateWidget() {
    ui->nameLabel->setText(m_dataObject["name"].toObject()["value"].toString());


    QJsonObject infoObject = m_dataObject["info"].toObject();
    QString classString = QString("%1 (%2)").arg(infoObject["charClass"].toObject()["value"].toString(), infoObject["charSubclass"].toObject()["value"].toString());
    ui->classLabel->setText(classString);
    ui->levelBox->setValue(infoObject["level"].toObject()["value"].toInt());
    ui->proficiencyLabel->setText(QString::number(proficiencyByLevel(ui->levelBox->value())));


    QJsonObject vitalityObject = m_dataObject["vitality"].toObject();
    ui->runSpeedLabel->setText(vitalityObject["speed"].toObject()["value"].toString());
    ui->acLabel->setText(QString::number(vitalityObject["ac"].toObject()["value"].toInt()));
    ui->hpSpinBox->setValue(vitalityObject["hp-current"].toObject()["value"].toInt());
    ui->maxHpLabel->setText(QString::number(vitalityObject["hp-max"].toObject()["value"].toInt()));


    QJsonObject statsObject = m_dataObject["stats"].toObject();
    QJsonObject savesObject = m_dataObject["saves"].toObject();
    ui->strValueEdit->setValue(statsObject["str"].toObject()["score"].toInt());
    ui->strBonusLabel->setText(QString::number(bonusFromStat(ui->strValueEdit->value())));
    ui->strSaveCheckBox->setChecked(true);
    ui->strSaveCheckBox->setChecked(savesObject["str"].toObject()["isProf"].toBool());

    ui->dexValueEdit->setValue(statsObject["dex"].toObject()["score"].toInt());
    ui->dexBonusLabel->setText(QString::number(bonusFromStat(ui->dexValueEdit->value())));
    ui->dexSaveCheckBox->setChecked(true);
    ui->dexSaveCheckBox->setChecked(savesObject["dex"].toObject()["isProf"].toBool());

    ui->conValueEdit->setValue(statsObject["con"].toObject()["score"].toInt());
    ui->conBonusLabel->setText(QString::number(bonusFromStat(ui->conValueEdit->value())));
    ui->conSaveCheckBox->setChecked(true);
    ui->conSaveCheckBox->setChecked(savesObject["con"].toObject()["isProf"].toBool());

    ui->intValueEdit->setValue(statsObject["int"].toObject()["score"].toInt());
    ui->intBonusLabel->setText(QString::number(bonusFromStat(ui->intValueEdit->value())));
    ui->intSaveCheckBox->setChecked(true);
    ui->intSaveCheckBox->setChecked(savesObject["int"].toObject()["isProf"].toBool());

    ui->wisValueEdit->setValue(statsObject["wis"].toObject()["score"].toInt());
    ui->wisBonusLabel->setText(QString::number(bonusFromStat(ui->wisValueEdit->value())));
    ui->wisSaveCheckBox->setChecked(true);
    ui->wisSaveCheckBox->setChecked(savesObject["wis"].toObject()["isProf"].toBool());

    ui->chaValueEdit->setValue(statsObject["cha"].toObject()["score"].toInt());
    ui->chaBonusLabel->setText(QString::number(bonusFromStat(ui->chaValueEdit->value())));
    ui->chaSaveCheckBox->setChecked(true);
    ui->chaSaveCheckBox->setChecked(savesObject["cha"].toObject()["isProf"].toBool());


    QJsonObject skillsObject = m_dataObject["skills"].toObject();
    ui->athlecicsCheckBox->setChecked(true);
    ui->athlecicsCheckBox->setChecked(skillsObject["athletics"].toObject()["isProf"].toInt());

    ui->acrobatics->setChecked(true);
    ui->acrobatics->setChecked(skillsObject["acrobatics"].toObject()["isProf"].toInt());

    ui->sleight->setChecked(true);
    ui->sleight->setChecked(skillsObject["sleight of hand"].toObject()["isProf"].toInt());

    ui->stealth->setChecked(true);
    ui->stealth->setChecked(skillsObject["stealth"].toObject()["isProf"].toInt());

    ui->arcana->setChecked(true);
    ui->arcana->setChecked(skillsObject["arcana"].toObject()["isProf"].toInt());

    ui->history->setChecked(true);
    ui->history->setChecked(skillsObject["history"].toObject()["isProf"].toInt());

    ui->investigation->setChecked(true);
    ui->investigation->setChecked(skillsObject["investigation"].toObject()["isProf"].toInt());

    ui->nature->setChecked(true);
    ui->nature->setChecked(skillsObject["nature"].toObject()["isProf"].toInt());

    ui->religion->setChecked(true);
    ui->religion->setChecked(skillsObject["religion"].toObject()["isProf"].toInt());

    ui->handling->setChecked(true);
    ui->handling->setChecked(skillsObject["animal handling"].toObject()["isProf"].toInt());

    ui->insight->setChecked(true);
    ui->insight->setChecked(skillsObject["insight"].toObject()["isProf"].toInt());

    ui->medicine->setChecked(true);
    ui->medicine->setChecked(skillsObject["medicine"].toObject()["isProf"].toInt());

    ui->perception->setChecked(true);
    ui->perception->setChecked(skillsObject["perception"].toObject()["isProf"].toInt());

    ui->survival->setChecked(true);
    ui->survival->setChecked(skillsObject["survival"].toObject()["isProf"].toInt());

    ui->deception->setChecked(true);
    ui->deception->setChecked(skillsObject["deception"].toObject()["isProf"].toInt());

    ui->intimidation->setChecked(true);
    ui->intimidation->setChecked(skillsObject["intimidation"].toObject()["isProf"].toInt());

    ui->performance->setChecked(true);
    ui->performance->setChecked(skillsObject["performance"].toObject()["isProf"].toInt());

    ui->persuasion->setChecked(true);
    ui->persuasion->setChecked(skillsObject["persuasion"].toObject()["isProf"].toInt());
}

void DndCharsheetWidget::saveToFile(QString filePath) {

}

void DndCharsheetWidget::connectSignals() {
    connect(ui->levelBox, &QSpinBox::valueChanged, this, [=](){
        ui->proficiencyLabel->setText(QString::number(proficiencyByLevel(ui->levelBox->value())));
        updateCheckBoxes();
    });

    connect(ui->strValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->dexValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->conValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->intValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->wisValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});
    connect(ui->chaValueEdit, &QSpinBox::valueChanged, this, [=](){updateCheckBoxes();});

    connect(ui->strSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->strSaveCheckBox, ui->strValueEdit);});
    connect(ui->dexSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->dexSaveCheckBox, ui->dexValueEdit);});
    connect(ui->conSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->conSaveCheckBox, ui->conValueEdit);});
    connect(ui->intSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->intSaveCheckBox, ui->intValueEdit);});
    connect(ui->wisSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->wisSaveCheckBox, ui->wisValueEdit);});
    connect(ui->chaSaveCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->chaSaveCheckBox, ui->chaValueEdit);});

    connect(ui->athlecicsCheckBox, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->athlecicsCheckBox, ui->strValueEdit);});

    connect(ui->acrobatics, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->acrobatics, ui->dexValueEdit);});
    connect(ui->sleight, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->sleight, ui->dexValueEdit);});
    connect(ui->stealth, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->stealth, ui->dexValueEdit);});


    connect(ui->arcana, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->arcana, ui->intValueEdit);});
    connect(ui->history, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->history, ui->intValueEdit);});
    connect(ui->investigation, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->investigation, ui->intValueEdit);});
    connect(ui->nature, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->nature, ui->intValueEdit);});
    connect(ui->religion, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->religion, ui->intValueEdit);});

    connect(ui->handling, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->handling, ui->wisValueEdit);});
    connect(ui->insight, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->insight, ui->wisValueEdit);});
    connect(ui->medicine, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->medicine, ui->wisValueEdit);});
    connect(ui->perception, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->perception, ui->wisValueEdit);});
    connect(ui->survival, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->survival, ui->wisValueEdit);});

    connect(ui->deception, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->deception, ui->chaValueEdit);});
    connect(ui->intimidation, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->intimidation, ui->chaValueEdit);});
    connect(ui->performance, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->performance, ui->chaValueEdit);});
    connect(ui->persuasion, &QCheckBox::toggled, this, [=](){updateCheckBox(ui->persuasion, ui->chaValueEdit);});
}

void DndCharsheetWidget::updateCheckBox(QCheckBox *checkBox, QSpinBox *baseSpinBox) {
    int profBonus = 0;
    if (checkBox->isChecked()) profBonus = ui->proficiencyLabel->text().toInt();
    checkBox->setText(QString("%1: %2").arg(checkBox->text().split(":").value(0), QString::number(bonusFromStat(baseSpinBox->value()) + profBonus)));
}

void DndCharsheetWidget::updateCheckBoxes() {
    updateCheckBox(ui->strSaveCheckBox, ui->strValueEdit);
    updateCheckBox(ui->dexSaveCheckBox, ui->dexValueEdit);
    updateCheckBox(ui->conSaveCheckBox, ui->conValueEdit);
    updateCheckBox(ui->intSaveCheckBox, ui->intValueEdit);
    updateCheckBox(ui->wisSaveCheckBox, ui->wisValueEdit);
    updateCheckBox(ui->chaSaveCheckBox, ui->chaValueEdit);

    updateCheckBox(ui->athlecicsCheckBox, ui->strValueEdit);

    updateCheckBox(ui->acrobatics, ui->dexValueEdit);
    updateCheckBox(ui->sleight, ui->dexValueEdit);
    updateCheckBox(ui->stealth, ui->dexValueEdit);

    updateCheckBox(ui->arcana, ui->intValueEdit);
    updateCheckBox(ui->history, ui->intValueEdit);
    updateCheckBox(ui->investigation, ui->intValueEdit);
    updateCheckBox(ui->nature, ui->intValueEdit);
    updateCheckBox(ui->religion, ui->intValueEdit);

    updateCheckBox(ui->handling, ui->wisValueEdit);
    updateCheckBox(ui->insight, ui->wisValueEdit);
    updateCheckBox(ui->medicine, ui->wisValueEdit);
    updateCheckBox(ui->perception, ui->wisValueEdit);
    updateCheckBox(ui->survival, ui->wisValueEdit);

    updateCheckBox(ui->deception, ui->chaValueEdit);
    updateCheckBox(ui->intimidation, ui->chaValueEdit);
    updateCheckBox(ui->performance, ui->chaValueEdit);
    updateCheckBox(ui->persuasion, ui->chaValueEdit);
}

void DndCharsheetWidget::addToInitiative(InitiativeTrackerWidget *initiativeTrackerWidget) {
    initiativeTrackerWidget->addCharacter(ui->nameLabel->text(), ui->maxHpLabel->text().toInt(), ui->acLabel->text().toInt(), ui->hpSpinBox->value());
}