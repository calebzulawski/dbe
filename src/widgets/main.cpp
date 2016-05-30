#include "main.hpp"

#include<QMenuBar>
#include<QMenu>

using namespace gui;

Main::Main() {
    createMenuBar();
}

void Main::createFileMenu() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
}

void Main::createPluginsMenu() {
    QMenu* pluginMenu = menuBar()->addMenu(tr("&Plugins"));
}

void Main::createMenuBar() {
    createFileMenu();
    createPluginsMenu();
}
