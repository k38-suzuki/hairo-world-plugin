/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkWidget.h"
#include <cnoid/Action>
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/SimulationBar>
#include <cnoid/TreeWidget>
#include <QHBoxLayout>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "KIOSKManager.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

struct ButtonInfo {
    int row;
    int column;
    int rowSpan;
    int columnSpan;
    const char* icon;
};

ButtonInfo buttonInfo[] = {
    { 0, 0, 1, 1,                                 "" },
    { 0, 1, 1, 1,                                 "" },
    { 0, 2, 1, 1,                                 "" },
    { 0, 3, 1, 1, ":/Body/icon/start-simulation.svg" },
    { 0, 4, 1, 1,  ":/Body/icon/stop-simulation.svg" }
};

}

namespace cnoid {

class BookmarkWidgetImpl
{
public:
    BookmarkWidgetImpl(BookmarkWidget* self);
    BookmarkWidget* self;

    enum ButtonID { ADD, REMOVE, LOCK, START, NUM_BUTTONS };
    enum InfoID { BOOKMARK, FILE, NUM_INFO };

    TreeWidget* treeWidget;
    PushButton* buttons[NUM_BUTTONS];
    Menu menu;

    void addItem(const string& filename, const string& text);
    void removeItem();
    void onAddButtonClicked();
    void onLockButtonToggled(const bool& on);
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


BookmarkWidget::BookmarkWidget()
{
    impl = new BookmarkWidgetImpl(this);
}


BookmarkWidgetImpl::BookmarkWidgetImpl(BookmarkWidget* self)
    : self(self)
{
    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(false);

    const QStringList header_labels = { _("Bookmark"), _("File") };
    treeWidget->setHeaderLabels(header_labels);

    static const char* labels[] = { _("+"), _("-"), _("Lock") };

    MainWindow* mw = MainWindow::instance();
    QHBoxLayout* hbox = new QHBoxLayout;
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        buttons[i] = new PushButton;
        PushButton* button = buttons[i];
        ButtonInfo info = buttonInfo[i];
        if(i < START) {
            button->setText(labels[i]);
        } else if(info.icon != "") {
            button->setIconSize(mw->iconSize());
            button->setIcon(QIcon(info.icon));
        }
        hbox->addWidget(button);
        if(i == LOCK) {
            hbox->addStretch();
        }
    }

    buttons[LOCK]->setCheckable(true);

    buttons[ADD]->sigClicked().connect([&](){ onAddButtonClicked(); });
    buttons[REMOVE]->sigClicked().connect([&](){ removeItem(); });
    buttons[LOCK]->sigToggled().connect([&](bool on){ onLockButtonToggled(on); });
    buttons[START]->sigClicked().connect([&](){ onStartButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(treeWidget);
    self->setLayout(vbox);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    Action* addAct = new Action;
    addAct->setText(_("Add"));
    menu.addAction(addAct);
    Action* removeAct = new Action;
    removeAct->setText(_("Remove"));
    menu.addAction(removeAct);
    Action* openAct = new Action;
    openAct->setText(_("Open"));
    menu.addAction(openAct);

    addAct->sigTriggered().connect([&](){ onAddButtonClicked(); });
    removeAct->sigTriggered().connect([&](){ removeItem(); });
    openAct->sigTriggered().connect([&](){ onStartButtonClicked(); });
    self->connect(treeWidget, &TreeWidget::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });
}


BookmarkWidget::~BookmarkWidget()
{
    delete impl;
}


void BookmarkWidgetImpl::addItem(const string& filename, const string& text)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(FILE, filename.c_str());
    treeWidget->setCurrentItem(item);
    LineEdit* bookmarkLine = new LineEdit;
    bookmarkLine->setPlaceholderText(_("Bookmark"));
    if(!text.empty()) {
        bookmarkLine->setText(text.c_str());
    }
    treeWidget->setItemWidget(item, BOOKMARK, bookmarkLine);
}


void BookmarkWidgetImpl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void BookmarkWidgetImpl::onAddButtonClicked()
{
    MainWindow* mw = MainWindow::instance();
    FileDialog dialog(mw);
    dialog.setWindowTitle(_("Select a Choreonoid project file"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("Project files (*.cnoid)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    if(dialog.exec()) {
        int numFiles = dialog.selectedFiles().size();
        for(int i = 0; i < numFiles; ++i) {
            QString filename = dialog.selectedFiles()[i];
            addItem(filename.toStdString(), "");
        }
    }
}


void BookmarkWidgetImpl::onLockButtonToggled(const bool& on)
{
    buttons[ADD]->setEnabled(!on);
    buttons[REMOVE]->setEnabled(!on);

    int size = treeWidget->topLevelItemCount();
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            LineEdit* bookmarkLine = dynamic_cast<LineEdit*>(treeWidget->itemWidget(item, BOOKMARK));
            if(bookmarkLine) {
                bookmarkLine->setEnabled(!on);
            }
        }
    }
}


void BookmarkWidgetImpl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(FILE).toStdString();
        ProjectManager* pm = ProjectManager::instance();
        bool result = pm->tryToCloseProject();
        if(result) {
            pm->clearProject();
            MessageView::instance()->flush();
            pm->loadProject(filename);
            SimulationBar::instance()->startSimulation(true);
        }
    }
}


void BookmarkWidgetImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    menu.exec(treeWidget->mapToGlobal(pos));
}


void BookmarkWidget::store(Mapping& archive)
{
    impl->store(archive);
}


void BookmarkWidgetImpl::store(Mapping& archive)
{
    int size = treeWidget->topLevelItemCount();
    archive.write("num_bookmarks", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            LineEdit* bookmarkLine = dynamic_cast<LineEdit*>(treeWidget->itemWidget(item, BOOKMARK));
            if(bookmarkLine) {
                string bookmark = bookmarkLine->text().toStdString();
                string bookmarkKey = "bookmark" + to_string(i);
                archive.write(bookmarkKey, bookmark);
            }
            string filename = item->text(FILE).toStdString();
            string fileKey = "bookmark_file" + to_string(i);
            archive.write(fileKey, filename);
        }
    }
    archive.write("bookmark_lock", buttons[LOCK]->isChecked());
}


void BookmarkWidget::restore(const Mapping &archive)
{
    impl->restore(archive);
}


void BookmarkWidgetImpl::restore(const Mapping& archive)
{
    int size = archive.get("num_bookmarks", 0);
    for(int i = 0; i < size; ++i) {
        string bookmarkKey = "bookmark" + to_string(i);
        string bookmark = archive.get(bookmarkKey, "");
        string fileKey = "bookmark_file" + to_string(i);
        string filename = archive.get(fileKey, "");
        if(!filename.empty()) {
            addItem(filename, bookmark);
        }
    }
    buttons[LOCK]->setChecked(archive.get("bookmark_lock", false));
}


bool BookmarkWidget::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool BookmarkWidgetImpl::storeState(Archive& archive)
{
    return true;
}


bool BookmarkWidget::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool BookmarkWidgetImpl::restoreState(const Archive& archive)
{
    return true;
}