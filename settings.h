//
// Created by arsen on 10.04.2025.
//

#ifndef DM_ASSIST_SETTINGS_H
#define DM_ASSIST_SETTINGS_H

#include <QString>

struct Settings{
    struct General{
        QString audioDevice = "general/audioDevice";    ///< int
        QString dir = "general/dir";                    ///< String
        QString lang = "general/lang";                  ///< String
    };

    struct Appearance{
        QString theme = "appearance/theme";
    };

    General general;
    Appearance appearance;
};

#endif //DM_ASSIST_SETTINGS_H
