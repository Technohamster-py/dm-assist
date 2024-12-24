 //
// Created by arseniy on 11.10.2024.
//
#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "QFile"
#include "QFileInfo"
#include "QDomDocument"


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    playerWidgetsList = {ui->player1, ui->player2, ui->player3, ui->player4, ui->player5, ui->player6, ui->player7, ui->player8, ui->player9};
    playButtonsList = {ui->play1, ui->play2, ui->play3, ui->play4, ui->play5, ui->play6, ui->play7, ui->play8, ui->play9};
    editButtonList = {ui->edit1, ui->edit2, ui->edit3, ui->edit4, ui->edit5, ui->edit6, ui->edit7, ui->edit8, ui->edit9};
    configurePlayers();
}

MainWindow::~MainWindow() {
    saveConfigFile();
    delete ui;
}

void MainWindow::configurePlayers() {

    for (int i = 0; i < 9; ++i) {
        playerList.append(new QPlayer(playerWidgetsList[i], i+1));
    }

    for (int i = 0; i < 9; ++i) {
        connect(playerList[i], SIGNAL(playerStarted()), this, SLOT(stopAll()));
        playerList[i]->setPlayShortcut(QString("Ctrl+%1").arg(QString::number(i+1)));
        connect(playButtonsList[i], SIGNAL(clicked(bool)), playerList[i], SLOT(play()));
        connect(editButtonList[i], SIGNAL(clicked(bool)), playerList[i], SLOT(edit()));
    }
}

void MainWindow::stopAll() {
    for (int i = 0; i < 9; ++i) {
        playerList[i]->stop();
    }
}

void MainWindow::loadConfigFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open player config file"),
                                                    QDir::homePath(),
                                                    tr("Xml file (*.xml)"));
    if(fileName.isEmpty())
        return;
    else
    {
        QFile configFile(fileName);
        if (!configFile.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Open File error"), configFile.errorString());
            return;
        }
        QDomDocument configDocument;
        configDocument.setContent(&configFile);
        QFileInfo fileInfo(configFile.fileName());

        QDomElement mainNode = configDocument.documentElement();
        QDomNodeList players = mainNode.childNodes();

        for (int i=0; i<players.count(); i++){
            QDomElement playerNode = players.at(i).toElement();
            QString absolutePath = fileInfo.dir().canonicalPath() + "/" + playerNode.firstChild().toText().data();
            int playerId = playerNode.attribute("id").toInt();
            QFile playerFile(absolutePath);
            playerList[i]->loadFromXml(&playerFile);
        }
    }
}

void MainWindow::saveConfigFile() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save to"),
                                                    QDir::homePath(),
                                                    tr("xml file (*.xml)"));
    if(fileName.isEmpty())
        return;
    else{
        QFile configFile(fileName);
        if (!configFile.open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this, tr("Open file error"), configFile.errorString());
            return;
        }
        QFileInfo fileInfo(configFile.fileName());

        QTextStream xmlContent(&configFile);
        QDomDocument configDocument;

        QDomElement root = configDocument.createElement("music-player");
        configDocument.appendChild(root);

        QString baseDir = fileInfo.dir().canonicalPath() + "/";

        for (int i = 0; i < 9; ++i) {
            QString playerFileName = QString("playlist%1.xml").arg(QString::number(i));
            playerList[i]->saveToXml(baseDir + playerFileName);
            QDomElement playlist_node = configDocument.createElement("playlist");
            playlist_node.setAttribute("id", i);
            playlist_node.setNodeValue(playerFileName);
            root.appendChild(playlist_node);
        }
        xmlContent << configDocument.toString();
    }
}
