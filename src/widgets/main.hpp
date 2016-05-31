#ifndef __DBE_MAIN_HPP__
#define __DBE_MAIN_HPP__

#include <QMainWindow>

namespace gui {

class Main : public QMainWindow {
public:
    Main();
private:
    void createMenuBar();
    void createFileMenu();
    void createPluginsMenu();
    void createHelpMenu();
};

} // namespace gui

#endif /* __DBE_MAIN_HPP__ */
