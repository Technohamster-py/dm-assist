#ifndef DM_ASSIST_TEXTUREPICKERDIALOG_H
#define DM_ASSIST_TEXTUREPICKERDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QFileInfoList>
#include <QToolButton>
#include <QDir>

class TexturePickerDialog : public QDialog {
Q_OBJECT
public:

    static QString getTexture(QWidget* parent){
        TexturePickerDialog dlg(parent);
        if (dlg.exec() == QDialog::Accepted) return dlg.selectedTexture();
        return "";
    }

    explicit TexturePickerDialog(QWidget* parent = nullptr)
            : QDialog(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        QDir dir("textures");
        QStringList filters = {"*.png", "*.jpg", "*.bmp"};
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

        int row = 0, col = 0;
        for (const QFileInfo& info : files) {
            QToolButton* btn = new QToolButton;
            btn->setIcon(QIcon(info.absoluteFilePath()));
            btn->setIconSize(QSize(64, 64));
            btn->setToolTip(info.baseName());
            layout->addWidget(btn, row, col++);
            if (col >= 5) { col = 0; row++; }
            connect(btn, &QToolButton::clicked, this, [this, info]() {
                m_selected = info.absoluteFilePath();
                accept();
            });
        }
    }

    QString selectedTexture() const { return m_selected; }

private:
    QString m_selected;
};



#endif //DM_ASSIST_TEXTUREPICKERDIALOG_H
