/**
   @author Kenta Suzuki
*/

#include "Notepad.h"
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>
#include <QBoxLayout>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class Notepad::Impl
{
public:
    Notepad* self;

    Impl(Notepad* self);

    void newFile();
    void open();
    void save();
    void saveAs();
    void about();

    void createActions();

    QTextEdit* textEdit;
    QString currentFile;
};

}


Notepad::Notepad(QWidget* parent)
    : QMainWindow(parent)
{
    impl = new Impl(this);
}


Notepad::Impl::Impl(Notepad* self)
    : self(self)
{
    QWidget* widget = new QWidget;
    self->setCentralWidget(widget);

    textEdit = new QTextEdit;

    auto layout = new QVBoxLayout;
    layout->addWidget(textEdit);
    widget->setLayout(layout);

    createActions();

    self->setWindowTitle("");
}


Notepad::~Notepad()
{
    delete impl;
}


void Notepad::loadFile(const QString& fileName)
{
    if(fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    impl->currentFile = fileName;
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        return;
    }
    setWindowTitle(fileName);
    QTextStream in(&file);
    QString text = in.readAll();
    impl->textEdit->setText(text);
    file.close();
}


void Notepad::Impl::newFile()
{
    currentFile.clear();
    textEdit->setText(QString());
    self->setWindowTitle(currentFile);
}


void Notepad::Impl::open()
{
    QString fileName = QFileDialog::getOpenFileName(self, "Open a file");
    if(fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    currentFile = fileName;
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(self, "Warning", "Cannot open file: " + file.errorString());
        return;
    }
    self->setWindowTitle(fileName);
    QTextStream in(&file);
    QString text = in.readAll();
    textEdit->setText(text);
    file.close();
}


void Notepad::Impl::save()
{
    QString fileName;
    if(currentFile.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(self, "Save");
        if(fileName.isEmpty()) {
            return;
        }
        currentFile = fileName;
    } else {
        fileName = currentFile;
    }
    QFile file(fileName);
    currentFile = fileName;
    if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(self, "Warning", "Cannot save file: " + file.errorString());
        return;
    }
    self->setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = textEdit->toPlainText();
    out << text;
    file.close();
}


void Notepad::Impl::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(self, "Save as");
    if(fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    currentFile = fileName;
    if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(self, "Warning", "Cannot save file: " + file.errorString());
        return;
    }
    self->setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = textEdit->toPlainText();
    out << text;
    file.close();
}


void Notepad::Impl::about()
{
    QMessageBox::about(self, _("About Notepad"),
                    _("The <b>Notepad</b> example demonstrates now to code a basic "
                        "text editor using QtWidgets"));
}


void Notepad::Impl::createActions()
{
    QMenu* fileMenu = self->menuBar()->addMenu(_("&File"));
    QToolBar* fileToolBar = self->addToolBar(_("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new");
    QAction* newAct = new QAction(newIcon, _("&New"), self);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(_("Create a new file"));
    self->connect(newAct, &QAction::triggered, [&](){ newFile(); });
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open");
    QAction* openAct = new QAction(openIcon, _("&Open..."), self);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(_("Open an existing file"));
    self->connect(openAct, &QAction::triggered, [&](){ open(); });
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save");
    QAction* saveAct = new QAction(saveIcon, _("Save"), self);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip("Save the document to disk");
    self->connect(saveAct, &QAction::triggered, [&](){ save(); });
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction* saveAsAct = fileMenu->addAction(saveAsIcon, _("Save &As..."), [&](){ saveAs(); });
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(_("Save the document under a new name"));

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction* exitAct = fileMenu->addAction(exitIcon, _("E&xit"), [&](){ self->close(); });
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(_("Exit the application"));

    QMenu* editMenu = self->menuBar()->addMenu(_("&Edit"));
    QToolBar* editToolBar = self->addToolBar(_("Edit"));

    const QIcon copyIcon = QIcon::fromTheme("edit-copy");
    QAction* copyAct = new QAction(copyIcon, _("&Copy"), self);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(_("Copy the current selection's contents to the "
                            "clipboard"));
    self->connect(copyAct, &QAction::triggered, [&](){ textEdit->copy(); });
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon cutIcon = QIcon::fromTheme("edit-cut");
    QAction* cutAct = new QAction(cutIcon, _("Cu&t"), self);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(_("Cut the current selection's contents to the "
                            "clipboard"));
    self->connect(cutAct, &QAction::triggered, [&](){ textEdit->cut(); });
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste");
    QAction* pasteAct = new QAction(pasteIcon, _("&Paste"), self);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(_("Paste the clipboard's contents into the current "
                            "selection"));
    self->connect(pasteAct, &QAction::triggered, [&](){ textEdit->paste(); });
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    const QIcon undoIcon = QIcon::fromTheme("edit-undo");
    QAction* undoAct = new QAction(undoIcon, _("&Undo"), self);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(_("Undo the action"));
    self->connect(undoAct, &QAction::triggered, [&](){ textEdit->undo(); });
    editMenu->addAction(undoAct);
    editToolBar->addAction(undoAct);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo");
    QAction* redoAct = new QAction(redoIcon, _("&Redo"), self);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(_("Redo the action"));
    self->connect(redoAct, &QAction::triggered, [&](){ textEdit->redo(); });
    editMenu->addAction(redoAct);
    editToolBar->addAction(redoAct);

    self->menuBar()->addSeparator();

    QMenu* helpMenu = self->menuBar()->addMenu(_("&Help"));

    QAction* aboutAct = new QAction(_("&About"), self);
    aboutAct->setStatusTip(_("Show the application's about box"));
    self->connect(aboutAct, &QAction::triggered, [&](){ about(); });
    helpMenu->addAction(aboutAct);
}
