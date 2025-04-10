//
// Created by arsen on 10.04.2025.
//

#ifndef DM_ASSIST_SETTINGS_H
#define DM_ASSIST_SETTINGS_H

#include <QString>

struct Settings{
    struct General{
        QString dir = "general/dir";                    ///< String
        QString audioDevice = "general/audioDevice";    ///< int
    };

    General general;
};

#endif //DM_ASSIST_SETTINGS_H
