#include "dndcharsheetwidget.h"
#include "ui_dndcharsheetwidget.h"

#include <QMessageBox>


DndCharsheetWidget::DndCharsheetWidget(QWidget* parent) :
        AbstractCharsheetWidget(parent), ui(new Ui::DndCharsheetWidget) {
    ui->setupUi(this);
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
}

void DndCharsheetWidget::populateWidget() {
    ui->nameLabel->setText(m_dataObject["name"].toObject()["value"].toString());

    QJsonObject infoObject = m_dataObject["info"].toObject();
    QString classString = QString("%1 (%2)").arg(infoObject["charClass"].toObject()["value"].toString(), infoObject["charSubclass"].toObject()["value"].toString());
    ui->classLabel->setText(classString);
    ui->levelLabel->setText(infoObject["level"].toObject()["value"].toString());

    QJsonObject vitalityObject = m_dataObject["vitality"].toObject();
    ui->runSpeedLabel->setText(vitalityObject["speed"].toObject()["value"].toString());
    ui->acLabel->setText(vitalityObject["ac"].toObject()["value"].toString());
    ui->hpSpinBox->setValue(vitalityObject["hp-current"].toObject()["value"].toInt());
    ui->maxHpLabel->setText(vitalityObject["hp-max"].toObject()["value"].toString());

    QJsonObject statsObject = m_dataObject["stats"].toObject();
    ui->strValueEdit->setText(statsObject["str"].toObject()["score"].toString());
    ui->dexValueEdit->setText(statsObject["dex"].toObject()["score"].toString());
    ui->conValueEdit->setText(statsObject["con"].toObject()["score"].toString());
    ui->intValueEdit->setText(statsObject["int"].toObject()["score"].toString());
    ui->wisValueEdit->setText(statsObject["wis"].toObject()["score"].toString());
    ui->chaValueEdit->setText(statsObject["cha"].toObject()["score"].toString());


}

void DndCharsheetWidget::saveToFile(QString filePath) {

}

void DndCharsheetWidget::addToInitiative(QInitiativeTrackerWidget *initiativeTrackerWidget) {

}
