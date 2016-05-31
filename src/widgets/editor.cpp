#include "editor.hpp"

Editor::Editor(QString& filename)
    : filename (filename)
{
    setWindowTitle(filename);
}
