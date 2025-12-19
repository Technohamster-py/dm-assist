#include "spelleditwidget.h"
#include "ui_spelleditwidget.h"

#include <QJsonObject>
#include "../map-widget/texturepickerdialog.h"

SpellEditWidget::SpellEditWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::SpellEditWidget) {
    ui->setupUi(this);

    connect(ui->materialCheckBox, &QCheckBox::toggled, ui->materialList, &QTextEdit::setEnabled);
    connect(ui->textureButton, &QPushButton::clicked, [=](){
        QString textureName = TexturePickerDialog::getTexture(this);
        ui->textureButton->setIcon(QIcon(textureName));
    });
}

SpellEditWidget::~SpellEditWidget() {
    delete ui;
}

bool SpellEditWidget::parseFromJson(QJsonObject json) {
    ui->titleEdit->setText(json.value("name").toString());
    ui->levelBox->setValue(json.value("level").toInt());
    ui->descriptionEdit->setCustomHtml(json["description"].toObject().value("value").toString());

    ui->verbalCheckBox->setChecked(json["components"].toObject().value("vocal").toBool());
    ui->somaticCheckBox->setChecked(json["components"].toObject().value("somatic").toBool());
    ui->materialCheckBox->setChecked(json["components"].toObject().value("material").toBool());
    if (ui->materialCheckBox->isChecked())
        ui->materialList->setText(json["materials"].toObject().value("value").toString());

    ui->castingUnitsBox->setCurrentIndex(m_casting[json["activation"].toObject().value("type").toString()]);
    ui->castingTimeBox->setValue(json["activation"].toObject().value("cost").toInt());

    ui->durationBox->setValue(json["duration"].toObject().value("value").toInt());
    ui->durationUnitsBox->setCurrentIndex(m_duration[json["duration"].toObject().value("units").toString()]);

    ui->rangeBox->setValue(json["range"].toObject().value("value").toInt());
    ui->rangeUnitsBox->setCurrentIndex(m_units[json["range"].toObject().value("units").toString()]);

    ui->aoeBox->setValue(json["target"].toObject().value("value").toInt());
    ui->aoeUnitsBox->setCurrentIndex(m_units[json["target"].toObject().value("units").toString()]);
    ui->shapeComboBox->setCurrentIndex(m_shapes[json["target"].toObject().value("type").toString()]);

    return true;
}
