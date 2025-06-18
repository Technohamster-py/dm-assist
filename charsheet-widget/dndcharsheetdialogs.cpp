#include "dndcharsheetdialogs.h"
#include "ui_resourcedialog.h"
#include "ui_attackdialog.h"


ResourceDialog::ResourceDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::ResourceDialog) {
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ResourceDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ResourceDialog::reject);
    connect(ui->maximumBox, &QSpinBox::valueChanged, [=](int value){
        ui->amountBox->setMaximum(value);
    });
}

ResourceDialog::~ResourceDialog() {
    delete ui;
}

Resource ResourceDialog::getCreatedResource() {
    Resource resource;

    resource.title = ui->titleEdit->text();
    resource.current = ui->amountBox->value();
    resource.max = ui->maximumBox->value();
    resource.refillOnLongRest = ui->longRestCheckBox->isChecked();
    resource.refillOnLongRest = ui->longRestRadioButton->isChecked();
    resource.refillOnShortRest = ui->shortRestRadioButton->isChecked();

    return resource;
}


AttackDialog::AttackDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::AttackDialog) {
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ResourceDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ResourceDialog::reject);
}

AttackDialog::~AttackDialog() {
    delete ui;
}

Attack AttackDialog::getCreatedAttack() {
    Attack attack;

    attack.title = ui->titleEdit->text();
    attack.ability = statsMap[ui->statComboBox->currentIndex()];
    attack.bonus = ui->bonusSpinBox->value();
    attack.damage = ui->damageEdit->text();
    attack.notes = ui->notesEdit->text();

    return attack;
}
