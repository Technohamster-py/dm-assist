//
// Created by akhomchenko on 13.03.25.
//

#ifndef DM_ASSIST_QSAVECONFIGDIALOG_H
#define DM_ASSIST_QSAVECONFIGDIALOG_H

#include <QFileDialog>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFile>

/**
 * @class SaveConfigDialog
 * @brief A dialog for configuring and saving a project file.
 *
 * This class provides a graphical user interface for users to specify
 * the project name and root folder where the project configuration
 * will be saved. The dialog allows browsing directories, displays warnings
 * for validation errors, and ensures the specified folder is suitable
 * for saving the project.
 */
class SaveConfigDialog : public QDialog {
Q_OBJECT

public:
    explicit SaveConfigDialog(QWidget *parent = nullptr);

    QString filename;
    QString directoryPath;
    QString projectName;

private slots:
    void onBrowseClicked();
    void onSaveClicked();
private:
    QLineEdit* projectNameEdit;
    QLineEdit* rootFolderEdit;
    QLabel* warningLabel;

    void showWarning(const QString& message);
};

#endif //DM_ASSIST_QSAVECONFIGDIALOG_H
