#include "statuseditdialog.h"
#include "ui_statuseditdialog.h"

#include "iconpickerdialog.h"
#include "themediconmanager.h"
#include "statusmanager.h"

#include <utility>

StatusModel::StatusModel(QObject *parent) : QAbstractTableModel(parent) {

}

int StatusModel::rowCount(const QModelIndex &parent) const {
    return statuses.size();
}

int StatusModel::columnCount(const QModelIndex &parent) const {
    return fields::del+1;
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
            return statusIcons.at(index.row());
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
        case fields::del: return "";
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
        case fields::title: s.title = value.toString(); sort(); break;
        case fields::timer: s.remainingRounds = value.toInt(); break;
        default: return false;
    }

    emit dataChanged(index, index);
    return true;
}

void StatusModel::addStatus(const Status &status) {
    for (int row; row < statuses.size(); ++row) {
        if (status == statuses[row]){
            statuses[row].iconPath = status.iconPath;
            statuses[row].remainingRounds = status.remainingRounds;
            emit dataChanged(index(row, 0), index(row, fields::timer+1));
        }
    }


    beginInsertRows(QModelIndex(), statuses.size(), statuses.size());
    statuses.append(status);
    statusIcons.append(QIcon(status.iconPath));
    ThemedIconManager::instance().addPixmapTarget(status.iconPath, this, [=](const QPixmap& px){statusIcons[statusIcons.count()-1] = QIcon(px);});
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

void StatusModel::editStatusIcon(int row, QString newIconPath) {
    Status &s = statuses[row];
    s.iconPath = std::move(newIconPath);
    statusIcons[row] = QIcon(newIconPath);
    ThemedIconManager::instance().addPixmapTarget(s.iconPath, this, [=](const QPixmap& px){statusIcons[row] = QIcon(px);});
    emit dataChanged(index(row, fields::icon), index(row, fields::icon+1));

}

void StatusModel::sort() {
    std::sort(statuses.begin(), statuses.end(), [](const Status &a, const Status &b) {
        return a.title < b.title;
    });
    emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
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
    setupIcons();
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);


    connect(ui->customStatusesView, &QTableView::clicked, this, [=](const QModelIndex &index) {
        if (index.column() == StatusModel::fields::icon)
        {
            QString iconPath = IconPickerDialog::getSelectedIcon(this);
            model->editStatusIcon(index.row(), iconPath);
            StatusManager::instance().addStatus(model->statusAt(index.row()));  ///< Update status icon in manager
        }
    });

    connect(ui->customStatusesView, &QTableView::clicked, this, [=](const QModelIndex &index) {
        if (index.column() == StatusModel::fields::del)
        {
            StatusManager::instance().removeStatus(model->statusAt(index.row()));
            model->remove(index.row());
        }
    });
}

StatusEditDialog::~StatusEditDialog() {
    delete ui;
}

void StatusEditDialog::on_addButton_clicked() {
    Status status;
    status.title = ui->titleEdit->text();
    status.iconPath = m_currentIconPath;
    model->addStatus(status);
    StatusManager::instance().addStatus(status);
    ui->titleEdit->clear();
    ui->iconButton->setIcon(QIcon());
    model->sort();
}

void StatusEditDialog::on_iconButton_clicked() {
    m_currentIconPath = IconPickerDialog::getSelectedIcon(this);
    ThemedIconManager::instance().addIconTarget<QPushButton>(m_currentIconPath, ui->iconButton, &QPushButton::setIcon);
}

void StatusEditDialog::populate() {
    auto available = StatusManager::instance().availableStatuses();
    for (const Status& status : available) {
        model->addStatus(status);
    }

    ui->customStatusesView->setModel(model);
    ui->customStatusesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->customStatusesView->horizontalHeader()->setSectionResizeMode(StatusModel::fields::title, QHeaderView::Stretch);

    for (const auto& status : m_statuses) {
        if (standardStatuses.contains(status.title, Qt::CaseInsensitive))
            m_standardStatusesMap[status.title]->setValue(status.remainingRounds);
        else
            model->addStatus(status);
    }
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

void StatusEditDialog::closeEvent(QCloseEvent *event) {
    accept();
    QDialog::closeEvent(event);
}

void StatusEditDialog::setupIcons() {
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-blinded.svg", ui->blindedCheckBox, [label = ui->blindedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-charmed.svg", ui->charmedCheckBox, [label = ui->charmedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-deafened.svg", ui->deafenedCheckBox, [label = ui->deafenedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-exhaustion.svg", ui->exhaustionCheckBox, [label = ui->exhaustionCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-frightened.svg", ui->frightenedCheckBox, [label = ui->frightenedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-grappled.svg", ui->grappledCheckBox, [label = ui->grappledCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-incapacitated.svg", ui->incapacitatedCheckBox, [label = ui->incapacitatedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-invisible.svg", ui->invisibleCheckBox, [label = ui->invisibleCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-paralyzed.svg", ui->paralyzedCheckBox, [label = ui->paralyzedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-petrified.svg", ui->petrifiedCheckBox, [label = ui->petrifiedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-poisoned.svg", ui->poisonedCheckBox, [label = ui->poisonedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-prone.svg", ui->proneCheckBox, [label = ui->proneCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-restrained.svg", ui->restrainedCheckBox, [label = ui->restrainedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-stunned.svg", ui->stunnedCheckBox, [label = ui->stunnedCheckBox](const QPixmap& px){label->setPixmap(px);});
    ThemedIconManager::instance().addPixmapTarget(":/statuses/status-unconscious.svg", ui->unconsciousCheckBox, [label = ui->unconsciousCheckBox](const QPixmap& px){label->setPixmap(px);});
}
