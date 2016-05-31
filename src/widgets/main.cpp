#include "main.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <sstream>

#include "dbe_info.hpp"

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

void Main::createHelpMenu() {
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, [this](){
        std::stringstream ss;
        ss << "Databending Editor (dbe)" << std::endl;
        ss << "Version " << DBE_VERSION << std::endl;
        QMessageBox::about(this, "About DBE", QString::fromStdString(ss.str()));
    });
    helpMenu->addAction(aboutAction);
}

void Main::createMenuBar() {
    createFileMenu();
    createPluginsMenu();
    createHelpMenu();
}
