#ifndef DM_ASSIST_TEXTUREMANAGER_H
#define DM_ASSIST_TEXTUREMANAGER_H

#include <QMap>
#include <QPixmap>
#include <QApplication>

class TextureManager {
public:
    static TextureManager& instance(){
        static TextureManager mgr;
        return mgr;
    }

    void registerTexture(const QString& name, const QPixmap& pixmap){
        if (name.toLower() == "none")
        m_textures[name] = pixmap;
    }

    QPixmap getTexture(const QString& name) const {
        return m_textures.value(name, QPixmap());
    }

    QStringList availableTextures() const {
        return m_textures.keys();
    }

private:
    QMap<QString, QPixmap> m_textures;

    TextureManager() {
        m_textures["fire"] = QApplication::applicationDirPath() + "/textures/fire.png";
        m_textures["water"] = QApplication::applicationDirPath() + "/textures/water.png";
        m_textures["fog"] = QApplication::applicationDirPath() + "/textures/fog.png";
    }
};


#endif //DM_ASSIST_TEXTUREMANAGER_H
