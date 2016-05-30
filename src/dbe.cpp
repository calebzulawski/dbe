#include <QApplication>
#include <Python.h>
#include <iostream>

#include "widgets/main.hpp"

int main(int argv, char **args) {
    std::cout << "hello world" << std::endl;

    Py_Initialize();

    QApplication app(argv, args);
    gui::Main main;
    main.show();

    Py_Finalize();

    return app.exec();
}

// // add plugin directory
// PyObject* sys = PyImport_ImportModule("sys");
// PyObject* sys_path = PyObject_GetAttrString(sys, "path");
// PyObject* plugin_folder = PyUnicode_FromString(pluginDir);
// PyList_Append(sys_path, plugin_folder);
