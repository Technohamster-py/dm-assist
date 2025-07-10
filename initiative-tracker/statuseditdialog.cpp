#include "statuseditdialog.h"
#include "ui_statuseditdialog.h"


StatusEditDialog::StatusEditDialog(const InitiativeCharacter& character, QWidget *parent) :
        QDialog(parent), ui(new Ui::StatusEditDialog), m_character(character) {
    ui->setupUi(this);

    m_standardStatusesMap = {
            {"charmed", ui->charmedSpinBox},
            {"deafened", ui->deafenedSpinBox},
            {"exhaustion", ui->exhaustionSpinBox},
            {"frightened", ui->frightenedSpinBox},
            {"grappled", ui->grappledSpinBox},
            {"incapacitated", ui->incapacitatedSpinBox},
            {"invisible", ui->invisibleSpinBox},
            {"paralyzed", ui->paralyzedSpinBox},
            {"petrified", ui->petrifiedSpinBox},
            {"poisoned", ui->poisonedSpinBox},
            {"prone", ui->pronedSpinBox},
            {"restrained", ui->restrainedSpinBox},
            {"stunned", ui->stunnedSpinBox},
            {"unconscious", ui->unconsciousSpinBox}
    };

    m_statuses = character.statuses;
    setWindowTitle(tr("%1 status edit").arg(character.name));
}

StatusEditDialog::~StatusEditDialog() {
    delete ui;
}

void StatusEditDialog::on_addButton_clicked() {
    Status status;
    status.title = ui->titleEdit->text();
    status.iconPath = m_currentIconPath;
    addRow(status);
}

void StatusEditDialog::on_iconButton_clicked() {
    m_currentIconPath = "";
}

InitiativeCharacter StatusEditDialog::getUpdatedCharacter() const {
    return InitiativeCharacter();
}

void StatusEditDialog::populate() {
    for (const auto& status : m_statuses) {
        if (standardStatuses.contains(status.title, Qt::CaseInsensitive))
            m_standardStatusesMap[status.title]->setValue(status.remainingRounds);
        else
            addRow(status);
    }
}

void StatusEditDialog::addRow(Status status) {
    int row = ui->customStatusesView->rowCount();

    ui->customStatusesView->insertRow(row);

    QTableWidgetItem* iconItem = new QTableWidgetItem(QIcon(status.iconPath), "");
    ui->customStatusesView->setItem(row, 0, iconItem);

    QTableWidgetItem* titleItem = new QTableWidgetItem(status.title);
    ui->customStatusesView->setItem(row, 0, titleItem);

    QTableWidgetItem* roundsItem = new QTableWidgetItem(status.remainingRounds);
    ui->customStatusesView->setItem(row, 0, roundsItem);
}
