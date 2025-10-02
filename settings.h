#ifndef DM_ASSIST_SETTINGS_H
#define DM_ASSIST_SETTINGS_H

#include <QString>
#include <QSettings>

#define ORGANIZATION_NAME "Technohamster"
#define ORGANIZATION_DOMAIN "github.com/Technohamster-py"
#define APPLICATION_NAME "DM-assist"

#define HELP_URL "https://github.com/Technohamster-py/dm-assist?tab=readme-ov-file#dm-assist"
#define DONATE_URL "https://technohamster.taplink.ws"
#define ISSUES_URL  "https://github.com/Technohamster-py/dm-assist/issues"
#define RELEASES_URL "https://api.github.com/repos/Technohamster-py/dm-assist/releases/latest"
#define VERSION "1.3"

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

enum startActions{
    openLast = 0,
    showRecent,
    showEmptyWindow
};

struct Settings{
    struct General{
        QString audioDevice = "common/audioDevice";                ///< int Выбранное аудиоустройство
        QString dir = "common/dir";                                ///< String рабочая папка
        QString lang = "common/lang";                              ///< String язык
        QString volume = "common/volume";                          ///< int основной ползунок громкости
        QString defaultCampaignDir = "common/defaultCampaignDir";  ///< String папка в которую будут сохраняться кампании по-умолчанию
        QString openLastMap = "common/openLastMap";                ///< bool
        QString startAction = "common/doOnStart";                  ///< int
        QString checkForUpdates = "common/checkForUpdates";        ///< bool
    };
    General general;

    struct Appearance{
        QString theme = "appearance/theme";                        ///< string
        QString style = "appearance/style";                        ///< string
        QString stretch = "appearance/mainStretch";                ///< byteArray
        QString scale = "appearance/scale";                        ///< int
    };
    Appearance appearance;

    struct Initiative {
        QString autoInitiative = "initiative/autoRoll/character";  ///< bool Автоматические броски инициативы для Игроков
        QString beastAutoInitiative = "initiative/autoRoll/beast"; ///< bool Автоматические броски инициативы для Монстров
        QString fields = "initiative/fields";                      ///< uint8 Режим отображения полей в трекере инициативы
        QString hpBarMode = "initiative/hpBar";                    ///< uint8 (0:2) Режим отображения здоровья
        QString showHpComboBox = "initiative/showHpCombo";         ///< bool Показывать комбобокс с выбором режима в основном виджете
        QString sharedWindows = "initiative/sharedWindowsCount";   ///< uint8 - Количество открытых расшаренных окон (UNUSED)
        QString customStatuses = "initiative/customStatuses";      ///< list - Список кастомных статусов с названиями и иконками
            QString customStatusTitle = "title";                   ///< QString - Название кастомного статуса в списке
            QString customStatusIcon = "icon";                     ///< QString - Путь к иконке кастомного статуса в списке
    };
    Initiative initiative;

    struct Map {
        QString brightRadius = "map/light/brightRadius";    ///< int Радиус яркий
        QString dimRadius = "map/light/dimRadius";          ///< int Радиус тусклый
        QString lightColor = "map/light/color";             ///< string цвет света
        QString color = "map/color";                        ///< string цвет инструментов
        QString tokenTitleMode = "map/tokenTitleMode";      ///< int режим отображения имени у токенов на карте
        QString tokenFontSize = "map/tokenFontSize";        ///< int размер шрифта токенов
        QString masterFogOpacity = "map/FogOpacity/master"; ///< int %
        QString playerFogOpacity = "map/FogOpacity/player"; ///< int %
        QString fogColor = "map/fogColor";                  ///< string
        QString defaultGridSize = "map/defaultGridSize";    ///< int
    };
    Map map;

    struct Hotkeys{
        QString ruler = "hotkeys/tools/ruler";              ///< string
        QString height = "hotkeys/tools/height";            ///< string
        QString brush = "hotkeys/tools/brush";              ///< string
        QString fogHide = "hotkeys/tools/fogHide";          ///< string
        QString fogReveal = "hotkeys/tools/fogReveal";      ///< string
        QString light = "hotkeys/tools/light";              ///< string
        QString line = "hotkeys/tools/line";                ///< string
        QString circle = "hotkeys/tools/circle";            ///< string
        QString square = "hotkeys/tools/square";            ///< string
        QString triangle = "hotkeys/tools/triangle";        ///< string
    };
    Hotkeys hotkeys;

    struct LastSession {
        QString campaign = "session/campaign";              ///< string
        QString recent = "session/recent";                  ///< list
            QString path = "path";                          ///< string
    };
    LastSession session;

    struct Rolls {
        QString compactMode = "rolls/compact";    ///< bool Компактный режим;
    };
    Rolls rolls;
};

#endif //DM_ASSIST_SETTINGS_H
