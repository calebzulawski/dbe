#include "main.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <sstream>
#include <Python.h>

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
        ss << "<center><font size = 16>Databending Editor</font><br>";
        ss << "a free databending image editor<br>" << std::endl;
        ss << "dbe " << DBE_VERSION << "<br>";
        ss << "Python version " << PY_MAJOR_VERSION << "."
                                << PY_MINOR_VERSION << "."
                                << PY_MICRO_VERSION << "<br><br>";
        ss << "Copyright (C) 2016  Caleb Zulawski<br></center>";
        
        ss << "This program is free software: you can redistribute it and/or modify<br>";
        ss << "it under the terms of the GNU General Public License as published by<br>";
        ss << "the Free Software Foundation, either version 3 of the License, or<br>";
        ss << "(at your option) any later version.<br><br>";

        ss << "This program is distributed in the hope that it will be useful,<br>";
        ss << "but WITHOUT ANY WARRANTY; without even the implied warranty of<br>";
        ss << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>";
        ss << "GNU General Public License for more details.<br><br>";

        ss << "You should have received a copy of the GNU General Public License<br>";
        ss << "along with this program.  If not, see";
        ss << "<a href='http://www.gnu.org/licenses/'>&lt;http://www.gnu.org/licenses/&gt;</a>.<br>";

        QMessageBox::about(this, "About DBE", QString::fromStdString(ss.str()));
    });
    helpMenu->addAction(aboutAction);
}

void Main::createMenuBar() {
    createFileMenu();
    createPluginsMenu();
    createHelpMenu();
}
