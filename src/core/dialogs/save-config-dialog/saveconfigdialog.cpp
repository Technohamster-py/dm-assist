#include "saveconfigdialog.h"
#include "QStandardPaths"

/**
 * @class SaveConfigDialog
 * @brief A dialog window for saving project configurations.
 *
 * The SaveConfigDialog class provides a graphical user interface that allows
 * users to specify a project name and a root directory in order to save project
 * configurations. The dialog includes input fields, buttons for browsing directories,
 * saving the configuration, and canceling the operation, as well as a label for
 * displaying warnings or errors.
 */
SaveConfigDialog::SaveConfigDialog(QWidget *parent, const QString& baseDir) : QDialog(parent), warningLabel(nullptr) {
    setWindowTitle(tr("Сохранение проекта"));
    resize(400, 200);

    auto* projectNameLabel = new QLabel(tr("Имя проекта:"), this);
    auto* rootFolderLabel = new QLabel(tr("Корневая папка:"), this);

    projectNameEdit = new QLineEdit(this);
    rootFolderEdit = new QLineEdit(this);

    auto* browseButton = new QPushButton(tr("Выбрать папку..."), this);
    connect(browseButton, &QPushButton::clicked, this, &SaveConfigDialog::onBrowseClicked);

    auto* saveButton = new QPushButton(tr("Сохранить"), this);
    connect(saveButton, &QPushButton::clicked, this, &SaveConfigDialog::onSaveClicked);
    auto* cancelButton = new QPushButton(tr("Отмена"), this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    warningLabel = new QLabel(this);
    warningLabel->setStyleSheet("color: red;");
    warningLabel->hide();

    auto* layout = new QVBoxLayout(this);

    auto* projectNameLayout = new QHBoxLayout();
    projectNameLayout->addWidget(projectNameLabel);
    projectNameLayout->addWidget(projectNameEdit);
    layout->addLayout(projectNameLayout);

    auto* rootFolderLayout = new QHBoxLayout();
    rootFolderLayout->addWidget(rootFolderLabel);
    rootFolderLayout->addWidget(rootFolderEdit);
    rootFolderLayout->addWidget(browseButton);
    layout->addLayout(rootFolderLayout);

    layout->addWidget(warningLabel);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    setLayout(layout);

    connect(projectNameEdit, &QLineEdit::textChanged, this, [=](){
        showWarning(QString(tr("Проект будет сохранен в папку %1/%2")).arg(rootFolderEdit->text().trimmed(), projectNameEdit->text().trimmed()));
        projectName = projectNameEdit->text().trimmed();
        directoryPath = rootFolderEdit->text().trimmed() + "/" + projectNameEdit->text().trimmed();
    });

    connect(rootFolderEdit, &QLineEdit::textChanged, this, [=](){
        showWarning(QString(tr("Проект будет сохранен в папку %1/%2")).arg(rootFolderEdit->text().trimmed(), projectNameEdit->text().trimmed()));
        directoryPath = rootFolderEdit->text().trimmed() + "/" + projectNameEdit->text().trimmed();
    });

    if (baseDir.isEmpty())
        rootFolderEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/dm_assist_campaigns");
    else
        rootFolderEdit->setText(baseDir);


    projectNameEdit->setText("Untitled campaign");
}

/**
 * @brief Handles the "Browse" button click event in the SaveConfigDialog.
 *
 * This method opens a directory selection dialog for the user to choose a folder.
 * If the user selects a non-empty directory, it sets the selected directory path
 * to the rootFolderEdit field and displays a warning message notifying the user
 * that the project will be saved in the specified folder with the project name.
 *
 * The warning message is displayed using the showWarning method.
 */
void SaveConfigDialog::onBrowseClicked() {
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          tr("Выберите папку"),
                                                          rootFolderEdit->text().trimmed());
    if (!directory.isEmpty()) {
        rootFolderEdit->setText(directory);
        directoryPath = directory + "/" + projectNameEdit->text().trimmed();
    }
}

/**
 * Handles the actions to be performed when the "Save" button is clicked in the SaveConfigDialog.
 *
 * This method retrieves and validates input data from the dialog, including the project name
 * and root folder path. It ensures:
 * - The project name is not empty.
 * - The root folder path is not empty and exists.
 * - The specified folder is empty to prevent overwriting existing files.
 *
 * If the input validation fails, an appropriate warning message is displayed using the showWarning method.
 * Otherwise, the method creates the necessary directory structure, creates an empty configuration
 * file named `<projectName>.xml`, and sets its path to the `filename` variable.
 *
 * Upon successful file creation, the dialog is closed by calling accept().
 *
 * Warning scenarios include:
 * - If the project name field is empty.
 * - If the root folder field is empty.
 * - If an error occurs while creating the file or directory.
 * - If the specified directory is not empty.
 */
void SaveConfigDialog::onSaveClicked() {
    QString projectName = projectNameEdit->text().trimmed();
    QString rootFolder = rootFolderEdit->text().trimmed() + "/" + projectName;

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

    if (!dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
        showWarning(tr("Указанная папка не пуста."));
        return;
    }

    filename = dir.filePath(projectName + ".json");
    accept();
}

/**
 * Displays a warning message in the Save Config Dialog.
 *
 * This function sets the text of the `warningLabel` to the provided message
 * and makes the label visible in the dialog.
 *
 * @param message The warning message to display. This is expected to be a QString containing
 *                the message that informs or warns the user.
 */
void SaveConfigDialog::showWarning(const QString &message)
{
    warningLabel->setText(message);
    warningLabel->show();
}


