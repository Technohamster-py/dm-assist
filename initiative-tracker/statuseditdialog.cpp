#include "statuseditdialog.h"
#include "ui_statuseditdialog.h"

#include "iconpickerdialog.h"

#include <QTimer>
#include <utility>

StatusModel::StatusModel(QObject *parent) : QAbstractTableModel(parent) {

}

int StatusModel::rowCount(const QModelIndex &parent) const {
    return statuses.size();
}

int StatusModel::columnCount(const QModelIndex &parent) const {
    return fields::del;
}

QVariant StatusModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= statuses.size())
        return {};

    const Status &s = statuses.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole){
        if (role == Qt::DisplayRole && index.column() == fields::del)
            return "❌";

        switch (index.column()) {
            case fields::title: return s.title;
            case fields::timer: return s.remainingRounds;
        }
    }

    if (index.column() == fields::icon)
    {
        if (role == Qt::DecorationRole)
            return QIcon(s.iconPath);
        if (role == Qt::UserRole)
            return s.iconPath;
    }



    return {};
}

QVariant StatusModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::DecorationRole || orientation != Qt::Horizontal)
        return {};
    switch (section) {
        case fields::title: return tr("Title");
        case fields::timer: return tr("Timer");
        default: return {};
    }
}

Qt::ItemFlags StatusModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == fields::del || index.column() == fields::icon)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool StatusModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= statuses.size())
        return false;

    Status &s = statuses[index.row()];

    switch (index.column()) {
        case fields::title: s.title = value.toString(); break;
        case fields::timer: s.remainingRounds = value.toInt(); break;
        default: return false;
    }

    emit dataChanged(index, index);
    return true;
}

void StatusModel::addStatus(const Status &status) {
    beginInsertRows(QModelIndex(), statuses.size(), statuses.size());
    statuses.append(status);
    endInsertRows();
}

void StatusModel::remove(int row) {
    if (row < 0 || row >= rowCount()) return;

    beginRemoveRows(QModelIndex(), row, row);
    statuses.removeAt(row);
    endRemoveRows();
}

Status StatusModel::statusAt(int row) {
    if (row >= 0 && row < statuses.size())
        return statuses[row];
    return {};
}


StatusEditDialog::StatusEditDialog(QList<Status> statuses, QWidget *parent) :
        QDialog(parent), ui(new Ui::StatusEditDialog), m_statuses(std::move(statuses)) {
    ui->setupUi(this);

    m_standardStatusesMap = {
            {"blinded", ui->blindedSpinBox},
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

    model = new StatusModel(this);
    populate();
    setWindowFlags(Qt::Popup);
}

StatusEditDialog::~StatusEditDialog() {
    delete ui;
}

void StatusEditDialog::on_addButton_clicked() {
    Status status;
    status.title = ui->titleEdit->text();
    status.iconPath = m_currentIconPath;
    model->addStatus(status);
}

void StatusEditDialog::on_iconButton_clicked() {
    m_currentIconPath = IconPickerDialog::getSelectedIcon(ui->iconButton);
}

void StatusEditDialog::populate() {
    ui->customStatusesView->setModel(model);
    ui->customStatusesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (const auto& status : m_statuses) {
        if (standardStatuses.contains(status.title, Qt::CaseInsensitive))
            m_standardStatusesMap[status.title]->setValue(status.remainingRounds);
        else
            model->addStatus(status);
    }
}

void StatusEditDialog::focusOutEvent(QFocusEvent *event) {
    QDialog::focusOutEvent(event);
    QTimer::singleShot(0, this, &QDialog::accept);
}

QList<Status> StatusEditDialog::statuses() const {
    QList<Status> result;

    for (auto standardStatus : standardStatuses) {
        Status status;
        status.iconPath = standardStatusIcons[standardStatus];
        status.title = standardStatus;
        status.remainingRounds = m_standardStatusesMap[standardStatus]->value();

        if (status.remainingRounds > 0)
            result.append(status);
    }

    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->statusAt(row).remainingRounds > 0)
            result.append(model->statusAt(row));
    }

    return result;
}
