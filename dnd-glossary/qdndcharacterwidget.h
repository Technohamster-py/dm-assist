//
// Created by arseniy on 02.04.2024.
//

#ifndef DM_ASSIST_QDNDCHARACTERWIDGET_H
#define DM_ASSIST_QDNDCHARACTERWIDGET_H

#include <QWidget>
#include "dndcharacter.h"


QT_BEGIN_NAMESPACE
namespace Ui { class QDndCharacterWidget; }
QT_END_NAMESPACE

class QDndCharacterWidget : public QWidget {
Q_OBJECT

public:
    explicit QDndCharacterWidget(DndCharacter character, QWidget *parent = nullptr);

    ~QDndCharacterWidget() override;



private:
    Ui::QDndCharacterWidget *ui;
};


#endif //DM_ASSIST_QDNDCHARACTERWIDGET_H
