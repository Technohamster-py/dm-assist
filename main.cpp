#include <QApplication>

#include "random"
#include "mainwindow.h"
#include "encounter.h"


#include <QDebug>


int main(int argc, char *argv[]) {
    std::random_device rd;
    std::mt19937 d20(rd());
    std::uniform_int_distribution <> dis(1, 20);

    QApplication a(argc, argv);
    Encounter encounter("Боссфайт");

    dndMonster monster1("Werwolf");
    monster1.setAc(15);
    monster1.setMaxHp(48);
    monster1.setFullHp();
    monster1.setInitiativeBonus(-2);

    dndMonster monster2("Dolboeb");
    monster2.setAc(13);
    monster2.setMaxHp(100);
    monster2.setFullHp();
    monster2.setInitiativeBonus(3);


    DndCharacter char1("Gandalf");
    char1.setAc(18);
    char1.setMaxHp(78);
    char1.setFullHp();
    char1.setInitiativeBonus(2);


    DndCharacter char2("Shrek");
    char2.setAc(50);
    char2.setMaxHp(53);
    char2.setFullHp();
    char2.setInitiativeBonus(5);


    encounter.addCharacter(&char1, dis(d20), true);
    encounter.addCharacter(&char2, dis(d20), true);
    encounter.addMonster(&monster1, dis(d20), true);
    encounter.addMonster(&monster2, dis(d20), true);

    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}