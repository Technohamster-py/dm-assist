//
// Created by akhomchenko on 13.03.25.
//

#include "qsaveconfigdialog.h"

SaveConfigDialog::SaveConfigDialog(QWidget *parent): QDialog(parent), warningLabel(nullptr)
{
        setWindowTitle("Сохранение проекта");
        resize(400, 200);

        // Метки
        QLabel* projectNameLabel = new QLabel("Имя проекта:", this);
        QLabel* rootFolderLabel = new QLabel("Корневая папка:", this);

        // Поля ввода
        projectNameEdit = new QLineEdit(this);
        rootFolderEdit = new QLineEdit(this);

        // Кнопка выбора директории
        QPushButton* browseButton = new QPushButton("Выбрать папку...", this);
        connect(browseButton, &QPushButton::clicked, this, &SaveConfigDialog::onBrowseClicked);

        // Кнопки действий
        QPushButton* saveButton = new QPushButton("Сохранить", this);
        connect(saveButton, &QPushButton::clicked, this, &SaveConfigDialog::onSaveClicked);
        QPushButton* cancelButton = new QPushButton("Отмена", this);
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
}

void SaveConfigDialog::onBrowseClicked() {
        QString directory = QFileDialog::getExistingDirectory(this, "Выберите папку");
        if (!directory.isEmpty()) {
            rootFolderEdit->setText(directory);
        }
}

void SaveConfigDialog::onSaveClicked() {
        QString projectName = projectNameEdit->text().trimmed();
        QString rootFolder = rootFolderEdit->text().trimmed();

        // Проверяем введенные данные
        if (projectName.isEmpty()) {
            showWarning("Введите имя проекта.");
            return;
        }
        if (rootFolder.isEmpty()) {
            showWarning("Укажите корневую папку.");
            return;
        }

        QDir dir(rootFolder);
        if (!dir.exists()) {
            showWarning("Указанная папка не существует.");
            return;
        }

        // Проверяем, пуста ли папка
        if (!dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
            showWarning("Указанная папка не пуста.");
            return;
        }

        // Сохраняем файл с названием "ИмяПроекта.xml"
        QFile configFile(dir.filePath(projectName + ".xml"));
        if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            showWarning("Не удалось создать файл.");
            return;
        }
        accept(); // Закрытие диалога при успешном сохранении
}

void SaveConfigDialog::showWarning(const QString &message) {
        warningLabel->setText(message);
        warningLabel->show();
}


