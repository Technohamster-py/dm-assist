#ifndef DM_ASSIST_MAPTABWIDGET_H
#define DM_ASSIST_MAPTABWIDGET_H

#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>

#include <themediconmanager.h>

/**
 * @class TabWidget
 * @brief A custom widget derived from QTabWidget with added functionality such as "new tab"
 *        button and a context menu for tab management.
 *
 * The TabWidget class provides a specialized QTabWidget that includes:
 * - A corner "+" button to add new tabs.
 * - A context menu appearing on right-clicking a tab for performing actions like "share" and "save".
 * - Integration with signals for handling tab-related actions externally.
 */
class TabWidget : public QTabWidget {
Q_OBJECT

public:
    explicit TabWidget(QWidget *parent = nullptr);

signals:
    void newTabRequested();
    void dropAccepted(QString file);
    void share(int tab);
    void save(int tab);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void addNewTab() {
        emit newTabRequested();
    }

    void showContextMenu(const QPoint &pos);
};

#endif //DM_ASSIST_MAPTABWIDGET_H
