#ifndef DM_ASSIST_SETTINGS_H
#define DM_ASSIST_SETTINGS_H

#include <QString>
#include <QSettings>

#define ORGANIZATION_NAME "Technohamster"
#define ORGANIZATION_DOMAIN "github.com/Technohamster-py"
#define APPLICATION_NAME "DM-assist"

#define HELP_URL "https://github.com/Technohamster-py/dm-assist?tab=readme-ov-file#dm-assist"
#define DONATE_URL "https://pay.cloudtips.ru/p/8f6d339a"
#define ISSUES_URL  "https://github.com/Technohamster-py/dm-assist/issues"
#define RELEASES_URL "https://api.github.com/repos/Technohamster-py/dm-assist/releases/latest"
#define VERSION "1.2.2"

enum hpBarMode {
    bar = 0,
    value,
    status

};

enum iniFields{
    name = 1,
    init = 2,
    ac = 4,
    hp = 8,
    maxHp = 16,
    del = 32
};


struct Settings{
    struct General{
        QString audioDevice = "common/audioDevice";    ///< int Выбранное аудиоустройство
        QString dir = "common/dir";                    ///< String рабочая папка
        QString lang = "common/lang";                  ///< String язык
        QString volume = "common/volume";              ///< int основной ползунок громкости
        QString defaultCampaignDir = "common/defaultCampaignDir"; ///< String папка в которую будут сохраняться кампании по-умолчанию
    };
    General general;

    struct Appearance{
        QString theme = "appearance/theme";
        QString style = "appearance/style";
    };
    Appearance appearance;

    struct Initiative {
        QString autoInitiative = "initiative/autoRoll";            ///< uint8 (0:7) Автоматические броски инициативы для NPC/Монстров/Игроков
        QString fields = "initiative/fields";                      ///< uint8 Режим отображения полей в трекере инициативы
        QString hpBarMode = "initiative/hpBar";                    ///< uint8 (0:2) Режим отображения здоровья
        QString showHpComboBox = "initiative/showHpCombo";         ///< bool Показывать комбобокс с выбором режима в основном виджете
        QString sharedWindows = "initiative/sharedWindowsCount";   ///< uint8 - Количество открытых расшаренных окон (UNUSED)
        QString customStatuses = "initiative/customStatuses";      ///< list - Список кастомных статусов с названиями и иконками
        QString customStatusTitle = "title";                       ///<
        QString customStatusIcon = "icon";                         ///<
    };
    Initiative initiative;

    struct Map {
        QString brightRadius = "map/light/brightRadius";    ///< int Радиус яркий
        QString dimRadius = "map/light/dimRadius";          ///< int Радиус тусклый
        QString lightColor = "map/light/color";             ///< string цвет света
        QString color = "map/color";                        ///< string цвет инструментов
    };
    Map map;

    struct LastSession {
        QString campaign = "session/campaign";
    };
    LastSession session;

    struct Rolls {
        QString compactMode = "rolls/compact";    ///< bool Компактный режим;
    };
    Rolls rolls;
};

#endif //DM_ASSIST_SETTINGS_H
