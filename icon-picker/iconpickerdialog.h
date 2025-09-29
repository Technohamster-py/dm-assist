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

/**
 * @class IconLabel
 * @brief A UI component that combines an icon with a text label.
 *
 * The IconLabel class is designed to display an icon alongside a text label.
 * This can be used in graphical user interfaces to provide a visual
 * representation (icon) paired with descriptive text, making interfaces more intuitive.
 *
 * The class provides methods to set and retrieve the icon and label text,
 * as well as to configure the overall appearance of the component.
 *
 * Responsibilities:
 * - Managing the icon image or graphic.
 * - Managing the label text.
 * - Controlling alignment and layout between the icon and text.
 * - Providing necessary methods to modify the appearance dynamically at runtime.
 *
 * Common usage scenarios include buttons with icons, toolbar items, menu entries,
 * or informational elements within a UI.
 */
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


/**
 * @class IconPickerDialog
 * @brief A dialog interface for selecting an icon from a collection.
 *
 * The IconPickerDialog class provides a user-friendly mechanism to browse,
 * preview, and select an icon from a predefined set of options. It is typically
 * used in graphical user interfaces where users need to customize or assign icons
 * to certain elements or features.
 *
 * This class handles the visual presentation of icons and supports interaction
 * mechanisms for searching, filtering, and choosing an appropriate icon.
 *
 * Responsibilities:
 * - Displaying a collection of icons in a structured layout.
 * - Enabling selection of an icon and returning the selected value.
 * - Providing optional search or filter functionality for easier navigation.
 * - Supporting callbacks or events to notify when a selection is made.
 *
 * Common usage scenarios include configuration dialogs, personalization settings,
 * or any context requiring the assignment or modification of icons.
 */
class IconPickerDialog : public QDialog {
Q_OBJECT
public:
    static QString getSelectedIcon(QWidget *parent = nullptr);
    explicit IconPickerDialog(QWidget *parent = nullptr);

signals:
    void iconSelected(QString path);

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
