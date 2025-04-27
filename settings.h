//
//

#ifndef DM_ASSIST_SETTINGS_H
#define DM_ASSIST_SETTINGS_H

#include <QString>
#include <QSettings>

#define ORGANIZATION_NAME "Technohaster"
#define ORGANIZATION_DOMAIN "github.com/Technohamster-py"
#define APPLICATION_NAME "DM-assist"

enum hpBarMode {
    bar = 0,
    value,
    status

};

enum initiatiativeFields{
    hp = 1,
    ac = 2,
};

struct Settings{
    struct General{
        QString audioDevice = "general/audioDevice";    ///< int Выбранное аудиоустройство
        QString dir = "general/dir";                    ///< String рабочая папка
        QString lang = "general/lang";                  ///< String язык
    };

    General general;

    struct Inititiative {
        QString autoInitiative = "initiative/autoroll";            ///< uint8 (0:7) Автоматические броски инициативы для NPC/Монстров/Игроков (UNUSED)
        QString fields = "initiative/fields";                      ///< uint8 Режим отображения полей в трекере инициативы
        QString hpBarMode = "initiative/hpBar";                    ///< uint8 (0:2) Режим отображения здоровья
        QString sharedWindows = "initiative/sharedWindowsCount";   ///< uint8 - Количество открытых расшаренных окон (UNUSED)
    };
    Inititiative inititiative;
};

#endif //DM_ASSIST_SETTINGS_H
