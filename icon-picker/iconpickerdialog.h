#ifndef DM_ASSIST_ICONPICKERDIALOG_H
#define DM_ASSIST_ICONPICKERDIALOG_H

#define ICON_DIR "/iconset"

#include <QCoreApplication>
#include <QDialog>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QPointer>

class IconLabel : public QLabel {
Q_OBJECT
public:
    explicit IconLabel(const QString &iconPath, QWidget *parent = nullptr);

    QString iconPath() const;

signals:
    void clicked();
    void doubleClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QString m_iconPath;
};



class IconPickerDialog : public QDialog {
Q_OBJECT
public:
    static QString getSelectedIcon(QWidget *parent = nullptr);
    explicit IconPickerDialog(QWidget *parent = nullptr);

signals:
    void iconSelected(const QString path);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QString iconDirectory = QCoreApplication::applicationDirPath() + ICON_DIR;
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QString selectedIconPath;
    IconLabel *highlightedLabel = nullptr;

    void loadIcons();
    void highlightLabel(IconLabel *label);
    void addIconToGrid(const QString &filePath);
};




#endif //DM_ASSIST_ICONPICKERDIALOG_H
