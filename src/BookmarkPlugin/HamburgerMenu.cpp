/**
    @author Kenta Suzuki
*/

#include "HamburgerMenu.h"
#include <cnoid/ExtensionManager>
#include <cnoid/FileDialog>
#include <cnoid/ItemFileDialog>
#include <cnoid/ItemFileIO>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/ToolBar>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

Menu* menu_Tools = nullptr;
ToolBar* fileInstance = nullptr;

}


void HamburgerMenu::initializeClass(ExtensionManager* ext)
{
    if(!menu_Tools) {
        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
        menu_Tools = mm.currentMenu();
    }
}


HamburgerMenu* HamburgerMenu::instance()
{
    static HamburgerMenu* instance_ = new HamburgerMenu;
    return instance_;
}


HamburgerMenu::HamburgerMenu(QWidget* parent)
    : Menu(parent)
{
    initialize();
}


HamburgerMenu::HamburgerMenu(const QString& title, QWidget* parent)
    : Menu(title, parent)
{
    initialize();
}


HamburgerMenu::~HamburgerMenu()
{

}


void HamburgerMenu::initialize()
{
    contextMenu_ = new Menu;

    auto button = fileBar()->addButton(":/GoogleMaterialSymbols/icon/menu_24dp_5F6368_FILL1_wght400_GRAD0_opsz24.svg");
    button->setMenu(this);
    button->setPopupMode(QToolButton::InstantPopup);

    button->setContextMenuPolicy(Qt::CustomContextMenu);
    button->connect(button, &ToolButton::customContextMenuRequested,
        [&](const QPoint& pos){ contextMenu_->exec(QCursor::pos()); });
}


static QString makeNameFilterString(const std::string& caption, const string& extensions)
{
    QString filters =
        ItemFileDialog::makeNameFilter(
            caption, ItemFileIO::separateExtensions(extensions));

    filters += _(";;Any files (*)");
    return filters;
}


namespace cnoid {

Menu* get_Tools_Menu()
{
    return menu_Tools;
}


ToolBar* fileBar()
{
    if(!fileInstance) {
        vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        for(auto& toolBar : toolBars) {
            if(toolBar->name() == "FileBar") {
                fileInstance = toolBar;
            }
        }
    }
    return fileInstance;
}


bool loadProject(const string& filename)
{
    ProjectManager* pm = ProjectManager::instance();
    bool result = pm->tryToCloseProject();
    if(!filename.empty() && result) {
        pm->clearProject();
        MessageView::instance()->flush();
        pm->loadProject(filename);
    }
    return result;
}


string getSaveFileName(const string& caption, const string& extensions)
{
    string filename;
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(caption.c_str());
    dialog.setNameFilter(makeNameFilterString(caption, extensions));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Save"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));
    dialog.updatePresetDirectories();
    if(dialog.exec() == QDialog::Accepted) {
        filename = dialog.selectedFiles().front().toStdString();
    }
    return filename;
}


vector<string> getSaveFileNames(const string& caption, const string& extensions)
{
    vector<string> filenames;
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(caption.c_str());
    dialog.setNameFilter(makeNameFilterString(caption, extensions));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Save"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));
    dialog.updatePresetDirectories();
    if(dialog.exec() == QDialog::Accepted) {
        for(auto& file : dialog.selectedFiles()) {
            filenames.push_back(file.toStdString());
        }
    }
    return filenames;
}

}