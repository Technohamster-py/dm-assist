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
#include <QTextStream>

class SaveConfigDialog : public QDialog {
Q_OBJECT

public:
    explicit SaveConfigDialog(QWidget *parent = nullptr);

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
