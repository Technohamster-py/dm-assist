#include "spelleditwidget.h"
#include "ui_spelleditwidget.h"

#include <QJsonObject>
#include "src/modules/map/dialogs/texturepickerdialog.h"

SpellEditWidget::SpellEditWidget(QWidget *parent) :
        QDialog(parent), ui(new Ui::SpellEditWidget) {
    ui->setupUi(this);
    connect(ui->buttons, &QDialogButtonBox::accepted, this, [this] {
        if (!ui->titleEdit->text().trimmed().isEmpty() && ui->aoeBox->value() > 0) accept();
    });
    connect(ui->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    ui->aoeBox->setMinimum(1);
    for (auto box : findChildren<QSpinBox*>()) box->setMaximum(100000);
    ui->levelBox->setMaximum(9);


    connect(ui->materialCheckBox, &QCheckBox::toggled, ui->materialList, &QTextEdit::setEnabled);
    connect(ui->textureButton, &QPushButton::clicked, [=](){
        QString textureName = TexturePickerDialog::getTexture(this);
        if (textureName.isEmpty()) return;
        texture = textureName;
        ui->textureButton->setIcon(QIcon(textureName));
    });
}

SpellEditWidget::~SpellEditWidget() {
    delete ui;
}

bool SpellEditWidget::parseFromJson(QJsonObject json) {
    original = json;
    texture = json["texture"].toString();
    ui->textureButton->setIcon(QIcon(texture));
    ui->schoolEdit->setText(json["school"].toString());
    ui->widthBox->setValue(json["target"].toObject()["width"].toInt(5));
    ui->ritualCheckBox->setChecked(json["components"].toObject()["ritual"].toBool());
    ui->titleEdit->setText(json.value("name").toString());
    ui->levelBox->setValue(json.value("level").toInt());
    ui->descriptionEdit->setHtml(json["description"].toObject().value("value").toString());

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

QJsonObject SpellEditWidget::toJson() const {
    auto json = original;
    json["name"] = ui->titleEdit->text().trimmed();
    json["level"] = ui->levelBox->value();
    json["school"] = ui->schoolEdit->text();
    json["texture"] = texture;
    auto set = [&json](QString key, QString field, QJsonValue value) {
        auto object = json[key].toObject(); object[field] = value; json[key] = object;
    };
    set("description", "value", ui->descriptionEdit->toHtml());
    set("materials", "value", ui->materialList->toPlainText());
    set("components", "vocal", ui->verbalCheckBox->isChecked());
    set("components", "somatic", ui->somaticCheckBox->isChecked());
    set("components", "material", ui->materialCheckBox->isChecked());
    set("components", "ritual", ui->ritualCheckBox->isChecked());
    set("activation", "cost", ui->castingTimeBox->value());
    set("activation", "type", m_casting.key(ui->castingUnitsBox->currentIndex()));
    set("duration", "value", ui->durationBox->value());
    set("duration", "units", m_duration.key(ui->durationUnitsBox->currentIndex()));
    set("range", "value", ui->rangeBox->value());
    set("range", "units", m_units.key(ui->rangeUnitsBox->currentIndex()));
    set("target", "value", ui->aoeBox->value());
    set("target", "width", ui->widthBox->value());
    set("target", "units", m_units.key(ui->aoeUnitsBox->currentIndex()));
    set("target", "type", QStringList{"line","cone","sphere","cube","cylinder"}.value(ui->shapeComboBox->currentIndex()));
    return json;
}
