#ifndef __DBE_EDITOR_HPP__
#define __DBE_EDITOR_HPP__

#include <QScrollArea>

class Editor : public QScrollArea {
public:
    Editor(QString& filename);

private:
    QString filename;

};

#endif /* __DBE_EDITOR_HPP__ */
