#include "spelleditwidget.h"
#include "ui_spelleditwidget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include "../map-widget/texturepickerdialog.h"
#include "iconpickerdialog.h"
#include "themediconmanager.h"

SpellEditWidget::SpellEditWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::SpellEditWidget) {
    ui->setupUi(this);

    ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/charSheet/shortrest.svg", ui->iconButton, &QAbstractButton::setIcon);

    connect(ui->materialCheckBox, &QCheckBox::toggled, ui->materialList, &QTextEdit::setEnabled);
    connect(ui->textureButton, &QPushButton::clicked, [=](){
        QString textureName = TexturePickerDialog::getTexture(this);
        ui->textureButton->setIcon(QIcon(textureName));
    });
    connect(ui->iconButton, &QPushButton::clicked, [=](){
        QString icon = IconPickerDialog::getSelectedIcon(this);
        ThemedIconManager::instance().addIconTarget<QAbstractButton>(icon, ui->iconButton, &QAbstractButton::setIcon);
    });
}

SpellEditWidget::SpellEditWidget(const QString& path, QWidget *parent) : SpellEditWidget(parent){
    m_file = path;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open spell file");
        return;
    }
    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject root = document.object();
    parseFromJson(root);
}

SpellEditWidget::~SpellEditWidget() {
    delete ui;
}

bool SpellEditWidget::parseFromJson(QJsonObject json) {
    ui->titleEdit->setText(json.value("name").toString());

    QJsonObject systemObj = json["system"].toObject();

    ui->levelBox->setValue(systemObj.value("level").toInt());
    ui->descriptionEdit->setCustomHtml(systemObj["description"].toObject().value("value").toString());

    ui->verbalCheckBox->setChecked(systemObj["components"].toObject().value("vocal").toBool());
    ui->somaticCheckBox->setChecked(systemObj["components"].toObject().value("somatic").toBool());
    ui->materialCheckBox->setChecked(systemObj["components"].toObject().value("material").toBool());
    if (ui->materialCheckBox->isChecked())
        ui->materialList->setText(systemObj["materials"].toObject().value("value").toString());

    ui->castingUnitsBox->setCurrentIndex(m_casting[systemObj["activation"].toObject().value("type").toString()]);
    ui->castingTimeBox->setValue(systemObj["activation"].toObject().value("cost").toInt());
    ui->ritualCheckBox->setChecked(systemObj["components"].toObject().value("ritual").toBool());

    ui->durationBox->setValue(systemObj["duration"].toObject().value("value").toInt());
    ui->durationUnitsBox->setCurrentIndex(m_duration[systemObj["duration"].toObject().value("units").toString()]);

    ui->rangeBox->setValue(systemObj["range"].toObject().value("value").toInt());
    ui->rangeUnitsBox->setCurrentIndex(m_units[systemObj["range"].toObject().value("units").toString()]);

    ui->aoeBox->setValue(systemObj["target"].toObject().value("value").toInt());
    ui->aoeUnitsBox->setCurrentIndex(m_units[systemObj["target"].toObject().value("units").toString()]);
    ui->shapeComboBox->setCurrentIndex(m_shapes[systemObj["target"].toObject().value("type").toString()]);

    return true;
}

bool SpellEditWidget::saveToFile() {
    QFile file(m_file);
    if (!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open spell file");
        return false;
    }
    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject root = document.object();

    return true;
}
