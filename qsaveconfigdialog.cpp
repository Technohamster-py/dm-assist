#include "qsaveconfigdialog.h"
#include "QStandardPaths"

SaveConfigDialog::SaveConfigDialog(QWidget *parent)  : QDialog(parent), warningLabel(nullptr) {
    setWindowTitle(tr("Сохранение проекта"));
    resize(400, 200);

    // Метки
    QLabel* projectNameLabel = new QLabel(tr("Имя проекта:"), this);
    QLabel* rootFolderLabel = new QLabel(tr("Корневая папка:"), this);

    // Поля ввода
    projectNameEdit = new QLineEdit(this);
    rootFolderEdit = new QLineEdit(this);

    // Кнопка выбора директории
    QPushButton* browseButton = new QPushButton(tr("Выбрать папку..."), this);
    connect(browseButton, &QPushButton::clicked, this, &SaveConfigDialog::onBrowseClicked);

    // Кнопки действий
    QPushButton* saveButton = new QPushButton(tr("Сохранить"), this);
    connect(saveButton, &QPushButton::clicked, this, &SaveConfigDialog::onSaveClicked);
    QPushButton* cancelButton = new QPushButton(tr("Отмена"), this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Метка для предупреждений
    warningLabel = new QLabel(this);
    warningLabel->setStyleSheet("color: red;");
    warningLabel->hide();

    // Компоновка
    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout* projectNameLayout = new QHBoxLayout();
    projectNameLayout->addWidget(projectNameLabel);
    projectNameLayout->addWidget(projectNameEdit);
    layout->addLayout(projectNameLayout);

    QHBoxLayout* rootFolderLayout = new QHBoxLayout();
    rootFolderLayout->addWidget(rootFolderLabel);
    rootFolderLayout->addWidget(rootFolderEdit);
    rootFolderLayout->addWidget(browseButton);
    layout->addLayout(rootFolderLayout);

    layout->addWidget(warningLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    setLayout(layout);

    rootFolderEdit->setText(QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/dm_assis_files/saves");
}

void SaveConfigDialog::onBrowseClicked() {
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          tr("Выберите папку"),
                                                          rootFolderEdit->text().trimmed());
    if (!directory.isEmpty()) {
        showWarning(QString(tr("Проект будет сохранен в папку %1/%2")).arg(directory, projectNameEdit->text().trimmed()));
        rootFolderEdit->setText(directory);
    }
}

void SaveConfigDialog::onSaveClicked() {
    QString projectName = projectNameEdit->text().trimmed();
    QString rootFolder = rootFolderEdit->text().trimmed() + "/" + projectName;

    // Проверяем введенные данные
    if (projectName.isEmpty()) {
        showWarning(tr("Введите имя проекта."));
        return;
    }
    if (rootFolder.isEmpty()) {
        showWarning(tr("Укажите корневую папку."));
        return;
    }

    QDir dir(rootFolder);
    if (!dir.exists())
        dir.mkpath(".");

    // Проверяем, пуста ли папка
    if (!dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
        showWarning(tr("Указанная папка не пуста."));
        return;
    }

    // Сохраняем файл с названием "ИмяПроекта.xml"
    filename = dir.filePath(projectName + ".xml");
    QFile configFile(filename);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showWarning(tr("Не удалось создать файл."));
        return;
    }

    configFile.close();
    accept(); // Закрытие диалога при успешном сохранении
}

void SaveConfigDialog::showWarning(const QString &message)
{
    warningLabel->setText(message);
    warningLabel->show();
}


