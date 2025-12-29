#include "spelleditwidget.h"
#include "ui_spelleditwidget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include "../map-widget/texturepickerdialog.h"
#include "../charsheet-widget/abstractcharsheetwidget.h"
#include "iconpickerdialog.h"
#include "themediconmanager.h"

SpellEditWidget::SpellEditWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::SpellEditWidget) {
    ui->setupUi(this);

    ThemedIconManager::instance().addIconTarget<QAbstractButton>(":/charSheet/shortrest.svg", ui->iconButton, &QAbstractButton::setIcon);

    ui->shapeComboBox->addItem(tr("Line"), QVariant::fromValue(SpellShapeType::Line));
    ui->shapeComboBox->addItem(tr("Cone"), QVariant::fromValue(SpellShapeType::Triangle));
    ui->shapeComboBox->addItem(tr("Sphere"), QVariant::fromValue(SpellShapeType::Circle));
    ui->shapeComboBox->addItem(tr("Cube"), QVariant::fromValue(SpellShapeType::Square));
    ui->shapeComboBox->addItem(tr("Cylinder"), QVariant::fromValue(SpellShapeType::Circle));

    connect(ui->materialCheckBox, &QCheckBox::toggled, ui->materialList, &QTextEdit::setEnabled);
    connect(ui->textureButton, &QPushButton::clicked, [=](){
        QString textureName = TexturePickerDialog::getTexture(this);
        ui->textureButton->setIcon(QIcon(textureName));
        QFileInfo info(textureName);
        m_textureFileName = info.fileName();
    });
    connect(ui->iconButton, &QPushButton::clicked, [=](){
        QString icon = IconPickerDialog::getSelectedIcon(this);
        ThemedIconManager::instance().addIconTarget<QAbstractButton>(icon, ui->iconButton, &QAbstractButton::setIcon);
    });

    connect(ui->shapeComboBox, &QComboBox::currentIndexChanged, this, &SpellEditWidget::setShape);

    ui->preview->setInteractive(false);
    ui->preview->setDragMode(QGraphicsView::NoDrag);
    ui->preview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->preview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->preview->setAcceptDrops(false);
}

SpellEditWidget::SpellEditWidget(const QString& path, QWidget *parent) : SpellEditWidget(parent){
    parseFromJson(path);
}

SpellEditWidget::~SpellEditWidget() {
    delete ui;
}

bool SpellEditWidget::parseFromJson(const QString& jsonFilePath) {
    m_file = jsonFilePath;
    QFile jsonFile(jsonFilePath);
    if (!jsonFile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "error", "Can't open spell file");
        return false;
    }
    QJsonDocument document = QJsonDocument::fromJson(jsonFile.readAll());
    jsonFile.close();
    QJsonObject root = document.object();

    ui->titleEdit->setText(root.value("name").toString());

    if (root.contains("texture")){
        m_textureFileName = root.value("texture").toString();
        QString texturePath = QCoreApplication::applicationDirPath() + "/textures/" + m_textureFileName;
        if (QFile::exists(texturePath))
            ui->textureButton->setIcon(QIcon(texturePath));
        else
            ui->textureButton->setIcon(QIcon(":/notexture.png"));
    }

    QJsonObject systemObj = root["system"].toObject();

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

    setShape();
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
    QJsonObject systemObj = root["system"].toObject();

    root["name"] = ui->titleEdit->text();

    systemObj["level"] = ui->levelBox->value();
    systemObj["description"].toObject()["value"] =
            ui->descriptionEdit->toHtml();

    QJsonObject componentsObj;
    componentsObj["vocal"] = ui->verbalCheckBox->isChecked();
    componentsObj["somatic"] = ui->somaticCheckBox->isChecked();
    componentsObj["material"] = ui->materialCheckBox->isChecked();
    componentsObj["ritual"] = ui->ritualCheckBox->isChecked();
    systemObj["components"] = componentsObj;

    if (ui->materialCheckBox->isChecked()) {
        QJsonObject materialsObj;
        materialsObj["value"] = ui->materialList->toPlainText();
        systemObj["materials"] = materialsObj;
    }

    QJsonObject activationObj;
    activationObj["type"] = m_casting.key(ui->castingUnitsBox->currentIndex());
    activationObj["cost"] = ui->castingTimeBox->value();
    systemObj["activation"] = activationObj;

    QJsonObject durationObj;
    durationObj["value"] = ui->durationBox->value();
    durationObj["units"] = m_duration.key(ui->durationUnitsBox->currentIndex());
    systemObj["duration"] = durationObj;

    QJsonObject rangeObj;
    rangeObj["value"] = ui->rangeBox->value();
    rangeObj["units"] = m_units.key(ui->rangeUnitsBox->currentIndex());
    systemObj["range"] = rangeObj;

    QJsonObject targetObj;
    targetObj["value"] = ui->aoeBox->value();
    targetObj["units"] = m_units.key(ui->aoeUnitsBox->currentIndex());
    targetObj["type"] = m_shapes.key(ui->shapeComboBox->currentIndex());
    systemObj["target"] = targetObj;

    root["system"] = systemObj;

    if (!m_textureFileName.isEmpty())
        root["texture"] = m_textureFileName;
    else
        root.remove("texture");

    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

void SpellEditWidget::closeEvent(QCloseEvent *event) {
    if (!m_file.isEmpty())
        saveToFile();
    QWidget::closeEvent(event);
}

void SpellEditWidget::setShape() {
    SpellShapeType type = ui->shapeComboBox->currentData().value<SpellShapeType>();
    ui->preview->setShape(type, ui->aoeBox->value(), QColor(Qt::red), "");
}
