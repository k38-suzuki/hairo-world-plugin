/**
   @author Kenta Suzuki
*/

#include "FileExplorer.h"
#include <cnoid/BodyItem>
#include <cnoid/Dialog>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/Process>
#include <cnoid/SceneItem>
#include <cnoid/Separator>
#include <QAction>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QListView>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <vector>
#include "Notepad.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

FileExplorer* explorerInstance = nullptr;

class FileExplorerImpl : public Dialog
{
public:
    FileExplorerImpl(FileExplorer* self);
    FileExplorer* self;

    void execute(const Item* item, const int& id);
    void execute(const int argc, const char* argv[]);
    void kill();
    void openFile(const QString& fileName);
    void exit();
    void cascade();
    void tile();
    void remove(const int& index);

private:
    void on_listView_doubleClicked(const QModelIndex& index);
    void on_treeView_clicked(const QModelIndex& index);

    void createMenu();

    QMenuBar* menuBar;
    QMdiArea* mdiArea;
    QTabWidget* tabWidget;
    QFileSystemModel* fileModel;
    QFileSystemModel* dirModel;
    QListView* listView;
    QTreeView* treeView;
    QString fileName;

    QMenu* fileMenu;
    QMenu* viewMenu;
    QAction* exitAct;
    QAction* cascadeAct;
    QAction* tileAct;

    vector<Process*> processes;
};

}


FileExplorer::FileExplorer()
{
    impl = new FileExplorerImpl(this);
}


FileExplorerImpl::FileExplorerImpl(FileExplorer* self)
    : self(self)
{
    createMenu();

    mdiArea = new QMdiArea;
    tabWidget = new QTabWidget;
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);

    connect(tabWidget, &QTabWidget::tabCloseRequested, [&](int index){ remove(index); });

    fileModel = new QFileSystemModel;
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    dirModel = new QFileSystemModel;
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    listView = new QListView;
    listView->setModel(fileModel);
    treeView = new QTreeView;
    treeView->setModel(dirModel);

    connect(listView, &QListView::doubleClicked,
        [&](const QModelIndex& index){ on_listView_doubleClicked(index); });
    connect(treeView, &QTreeView::clicked,
        [&](const QModelIndex& index){ on_treeView_clicked(index); });

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(treeView);
    splitter->addWidget(listView);
    // splitter->addWidget(mdiArea);
    splitter->addWidget(tabWidget);

    fileName.clear();
    processes.clear();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    QVBoxLayout* mainLayout = new QVBoxLayout;
    // mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(_(""));
}


FileExplorer::~FileExplorer()
{
    impl->kill();
    delete impl;
}


void FileExplorer::initializeClass(ExtensionManager* ext)
{
    if(!explorerInstance) {
        explorerInstance = ext->manage(new FileExplorer);
    }

    ItemTreeView::instance()->customizeContextMenu<BodyItem>(
        [&](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            // menuManager.setPath("/").setPath("Open");
            // menuManager.addItem(_("File"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 0); });
            // menuManager.addItem(_("Directory"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 1); });
            menuManager.setPath("/");
            menuManager.addItem(_("Open a file"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->openFile(item->filePath().c_str()); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [&](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            // menuManager.setPath("/").setPath("Open");
            // menuManager.addItem(_("File"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 0); });
            // menuManager.addItem(_("Directory"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 1); });
            menuManager.setPath("/");
            menuManager.addItem(_("Open a file"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->openFile(item->filePath().c_str()); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void FileExplorerImpl::execute(const Item* item, const int& id)
{
    static const string program[] = { "gedit", "nautilus" };
    const int argc = 2;
    const char* argv[] = { program[id].c_str(), item->filePath().c_str() };
    execute(argc, argv);
}


void FileExplorerImpl::execute(const int argc, const char* argv[])
{
    string actualCommand = argv[0];
    QStringList arguments = { argv[1] };

    Process* process = new Process;
    process->start(actualCommand.c_str(), arguments);
    if(process->waitForStarted()) {}
    processes.push_back(process);
}


void FileExplorerImpl::kill()
{
    for(size_t i = 0; i < processes.size(); ++i) {
        Process* process = processes[i];
        if(process) {
            if(process->state() != QProcess::NotRunning) {
                process->kill();
                process->waitForFinished(100);
            }
        }
    }
}


void FileExplorerImpl::openFile(const QString& fileName)
{
    this->fileName = fileName;
    QFileInfo info(fileName);
    QString filePath = info.absolutePath();
    QString homePath = QDir::homePath();
    dirModel->setRootPath(homePath);
    treeView->setRootIndex(dirModel->setRootPath(homePath));

    const QModelIndex& index = dirModel->index(filePath);
    treeView->expand(index);
    treeView->scrollTo(index);
    treeView->setCurrentIndex(index);
    treeView->resizeColumnToContents(0);

    on_treeView_clicked(dirModel->index(filePath));
    on_listView_doubleClicked(fileModel->index(fileName));
}


void FileExplorerImpl::on_listView_doubleClicked(const QModelIndex& index)
{
    if(!fileModel->isDir(index)) {
        QString fileName = fileModel->filePath(index);
        if(fileName.isEmpty()) {
            return;
        }

        QFileInfo info(fileName);

        Notepad* notepad = new Notepad(this);
        notepad->loadFile(fileName);
        // mdiArea->addSubWindow(notepad);
        tabWidget->addTab(notepad, info.fileName());
        tabWidget->setCurrentWidget(notepad);

        this->show();
        notepad->show();
    }
}


void FileExplorerImpl::on_treeView_clicked(const QModelIndex& index)
{
    QString filePath = dirModel->fileInfo(index).absoluteFilePath();
    listView->setRootIndex(fileModel->setRootPath(filePath));

    const QModelIndex& index2 = fileModel->index(fileName);
    listView->scrollTo(index2);
    listView->setCurrentIndex(index2);

    setWindowTitle(filePath);
}


void FileExplorerImpl::exit()
{
    mdiArea->closeAllSubWindows();
    accept();
}


void FileExplorerImpl::cascade()
{
    mdiArea->cascadeSubWindows();
}


void FileExplorerImpl::tile()
{
    mdiArea->tileSubWindows();
}


void FileExplorerImpl::remove(const int& index)
{
    tabWidget->removeTab(index);
}


void FileExplorerImpl::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu(_("&File"), this);
    fileMenu->addSeparator();
    exitAct = fileMenu->addAction(_("E&xit"));
    menuBar->addMenu(fileMenu);

    viewMenu = new QMenu(_("&View"), this);
    cascadeAct = viewMenu->addAction(_("&Cascade"));
    tileAct = viewMenu->addAction(_("&Tile"));
    viewMenu->addSeparator();
    menuBar->addMenu(viewMenu);

    connect(exitAct, &QAction::triggered, [&](){ exit(); });
    connect(cascadeAct, &QAction::triggered, [&](){ cascade(); });
    connect(tileAct, &QAction::triggered, [&](){ tile(); });
}
