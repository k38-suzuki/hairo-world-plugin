/**
    @author Kenta Suzuki
*/

#include "GainTestView.h"
#include <cnoid/Archive>
#include <cnoid/BodyItem>
#include <cnoid/Buttons>
#include <cnoid/ConnectionSet>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/NullOut>
#include <cnoid/RootItem>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/ValueTree>
#include <cnoid/ViewManager>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <cnoid/HamburgerMenu>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QStyle>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class GainTestView::Impl
{
public:
    GainTestView* self;

    Impl(GainTestView* self);
    ~Impl();

    void updateInterface(int index);
    void onNewButtonClicked();
    void onOpenButtonClicked();
    void onSaveButtonClicked();
    void onPrintButtonClicked();
    bool load(const string& filename, ostream& os = nullout());
    bool save(const string& filename);

    struct InterfaceUnit
    {
        Link* joint;
        QLabel* nameLabel;
        QLabel* pgainLabel;
        QLabel* dgainLabel;
        DoubleSpinBox* pgainSpinBox;
        DoubleSpinBox* dgainSpinBox;
        DoubleSpinBox* pstepSpinBox;
        DoubleSpinBox* dstepSpinBox;
        PushButton* pplusButton;
        PushButton* dplusButton;
        PushButton* pminusButton;
        PushButton* dminusButton;
        ConnectionSet connections;

        ~InterfaceUnit();
    };

    QLabel targetLabel;
    ScopedConnection modelUpdateConnection;
    ScopedConnectionSet itemConnections;

    BodyItem* bodyItem;
    YAMLWriter yamlWriter;
    vector<std::unique_ptr<InterfaceUnit>> interfaceUnits;

    QGridLayout* grid;
};

}


void GainTestView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<GainTestView>(
        N_("GainTestView"), N_("Gain Test"));
}


GainTestView* GainTestView::instance()
{
    static GainTestView* instance_ = ViewManager::findView<GainTestView>();
    return instance_;
}


GainTestView::GainTestView()
    : View()
{
    impl = new Impl(this);
}


GainTestView::Impl::Impl(GainTestView* self)
    : self(self)
{
    self->setDefaultLayoutArea(CenterArea);

    yamlWriter.setKeyOrderPreservationMode(true);

    auto style = self->style();
    int lmargin = style->pixelMetric(QStyle::PM_LayoutLeftMargin);
    int rmargin = style->pixelMetric(QStyle::PM_LayoutRightMargin);
    int tmargin = style->pixelMetric(QStyle::PM_LayoutTopMargin);
    int bmargin = style->pixelMetric(QStyle::PM_LayoutBottomMargin);

    auto vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    self->setLayout(vbox);

    const QIcon newIcon = QIcon::fromTheme("document-new");
    auto newButton = new PushButton(newIcon, _("New"), self);
    newButton->sigClicked().connect([&](){ onNewButtonClicked(); });

    const QIcon openIcon = QIcon::fromTheme("document-open");
    auto openButton = new PushButton(openIcon, _("Open"), self);
    openButton->sigClicked().connect([&](){ onOpenButtonClicked(); });

    const QIcon saveIcon = QIcon::fromTheme("document-save");
    auto saveButton = new PushButton(saveIcon, _("Save"), self);
    saveButton->sigClicked().connect([&](){ onSaveButtonClicked(); });

    const QIcon printIcon = QIcon::fromTheme("document-print");
    auto printButton = new PushButton(printIcon, _("Print"), self);
    printButton->sigClicked().connect([&](){ onPrintButtonClicked(); });

    auto hbox = new QHBoxLayout;
    hbox->setContentsMargins(lmargin, tmargin / 2, rmargin, bmargin / 2);
    targetLabel.setStyleSheet("font-weight: bold");
    targetLabel.setAlignment(Qt::AlignLeft);
    hbox->addWidget(&targetLabel);
    hbox->addStretch();
    hbox->addWidget(newButton);
    hbox->addWidget(openButton);
    hbox->addWidget(saveButton);
    hbox->addWidget(printButton);
    vbox->addLayout(hbox);

    auto baseWidget = new QWidget;

    auto scrollArea = new QScrollArea;
    scrollArea->setStyleSheet("QScrollArea {background: transparent;}");
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidget(baseWidget);
    baseWidget->setAutoFillBackground(false);
    vbox->addWidget(scrollArea);

    grid = new QGridLayout;
    hbox = new QHBoxLayout;
    hbox->addLayout(grid);
    // hbox->addStretch();

    vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();
    baseWidget->setLayout(vbox);
}


GainTestView::~GainTestView()
{
    delete impl;
}


GainTestView::Impl::~Impl()
{

}


double GainTestView::p(int index) const
{
    if(index >= impl->interfaceUnits.size()) {
        return 0.0;
    }
    return impl->interfaceUnits[index]->pgainSpinBox->value();
}


double GainTestView::d(int index) const
{
    if(index >= impl->interfaceUnits.size()) {
        return 0.0;
    }
    return impl->interfaceUnits[index]->dgainSpinBox->value();
}


void GainTestView::setP(int index, const int value)
{
    impl->interfaceUnits[index]->pgainSpinBox->setValue(value);
}


void GainTestView::setD(int index, const int value)
{
    impl->interfaceUnits[index]->dgainSpinBox->setValue(value);
}


void GainTestView::onActivated()
{

}


void GainTestView::onDeactivated()
{
    impl->modelUpdateConnection.disconnect();
}


void GainTestView::onAttachedMenuRequest(MenuManager& menuManager)
{

}


void GainTestView::Impl::updateInterface(int index)
{
    auto& unit = interfaceUnits[index];
    auto joint = unit->joint;
    unit->connections.block();

    unit->connections.unblock();
}


void GainTestView::Impl::onNewButtonClicked()
{
    bodyItem = nullptr;
    ItemList<BodyItem> items = RootItem::instance()->selectedItems<BodyItem>();
    if(items.size() > 0) {
        bodyItem = items[0];
    }

    modelUpdateConnection.disconnect();

    if(!bodyItem) {
        targetLabel.setText("------");
        interfaceUnits.resize(0);
    } else {
        bool updated = false;
        auto candidate = bodyItem;
        while(candidate) {
            if(candidate->body()->numAllJoints() > 0) {
                targetLabel.setText(candidate->displayName().c_str());

                auto body = bodyItem->body();
                int numJoints = body->numJoints();

                size_t prevSize = interfaceUnits.size();
                interfaceUnits.resize(numJoints);

                for(size_t i = prevSize; i < interfaceUnits.size(); ++i) {
                    auto& unit = interfaceUnits[i];
                    unit.reset(new InterfaceUnit);

                    unit->nameLabel = new QLabel(self);
                    unit->pgainLabel = new QLabel(self);
                    unit->pgainLabel->setAlignment(Qt::AlignCenter);
                    unit->dgainLabel = new QLabel(self);
                    unit->dgainLabel->setAlignment(Qt::AlignCenter);

                    unit->pgainSpinBox = new DoubleSpinBox(self);
                    unit->pgainSpinBox->setAlignment(Qt::AlignCenter);
                    unit->pgainSpinBox->setDecimals(3);
                    unit->pgainSpinBox->setRange(0.000, 99999.999);
                    unit->pgainSpinBox->setValue(0.0);

                    unit->dgainSpinBox = new DoubleSpinBox(self);
                    unit->dgainSpinBox->setAlignment(Qt::AlignCenter);
                    unit->dgainSpinBox->setDecimals(3);
                    unit->dgainSpinBox->setRange(0.000, 99999.999);
                    unit->dgainSpinBox->setValue(0.0);

                    unit->pstepSpinBox = new DoubleSpinBox(self);
                    unit->pstepSpinBox->setAlignment(Qt::AlignCenter);
                    unit->pstepSpinBox->setDecimals(3);
                    unit->pstepSpinBox->setRange(0.001, 99999.999);
                    unit->pstepSpinBox->setValue(1.0);

                    unit->dstepSpinBox = new DoubleSpinBox(self);
                    unit->dstepSpinBox->setAlignment(Qt::AlignCenter);
                    unit->dstepSpinBox->setDecimals(3);
                    unit->dstepSpinBox->setRange(0.001, 99999.999);
                    unit->dstepSpinBox->setValue(1.0);

                    unit->pplusButton = new PushButton("+", self);
                    unit->pplusButton->sigClicked().connect(
                        [&](){
                            double gain = unit->pgainSpinBox->value();
                            double step = unit->pstepSpinBox->value();
                            unit->pgainSpinBox->setValue(gain + step);
                        });

                    unit->pminusButton = new PushButton("-", self);
                    unit->pminusButton->sigClicked().connect(
                        [&](){
                            double gain = unit->pgainSpinBox->value();
                            double step = unit->pstepSpinBox->value();
                            unit->pgainSpinBox->setValue(gain - step);
                        });

                    unit->dplusButton = new PushButton("+", self);
                    unit->dplusButton->sigClicked().connect(
                        [&](){
                            double gain = unit->dgainSpinBox->value();
                            double step = unit->dstepSpinBox->value();
                            unit->dgainSpinBox->setValue(gain + step);
                        });

                    unit->dminusButton = new PushButton("-", self);
                    unit->dminusButton->sigClicked().connect(
                        [&](){
                            double gain = unit->dgainSpinBox->value();
                            double step = unit->dstepSpinBox->value();
                            unit->dgainSpinBox->setValue(gain - step);
                        });

                    grid->addWidget(unit->nameLabel, i, 0);
                    grid->addWidget(unit->pgainLabel, i, 1);
                    grid->addWidget(unit->pgainSpinBox, i, 2);
                    grid->addWidget(unit->pstepSpinBox, i, 3);
                    grid->addWidget(unit->pplusButton, i, 4);
                    grid->addWidget(unit->pminusButton, i, 5);
                    grid->addWidget(unit->dgainLabel, i, 6);
                    grid->addWidget(unit->dgainSpinBox, i, 7);
                    grid->addWidget(unit->dstepSpinBox, i, 8);
                    grid->addWidget(unit->dplusButton, i, 9);
                    grid->addWidget(unit->dminusButton, i, 10);
                }

                itemConnections.disconnect();
                for(size_t i = 0; i < body->numJoints(); ++i) {
                    Link* joint = body->joint(i);
                    auto& unit = interfaceUnits[i];
                    unit->joint = joint;
                    QString name = QString("%1: %2")
                        .arg(joint->jointId()).arg(joint->name().c_str());
                    unit->nameLabel->setText(name);
                    unit->pgainLabel->setText("P");
                    unit->dgainLabel->setText("D");
                    itemConnections.add(
                        bodyItem->sigUpdated().connect(
                            [=](){ updateInterface(i); }));
                    updateInterface(i);
                }

                updated = true;
                break;
            }
            candidate = candidate->parentBodyItem();
        }
        if(!updated) {
            modelUpdateConnection =
                bodyItem->sigModelUpdated().connect(
                    [this](int flags){
                        if(flags & (BodyItem::LinkSetUpdate | BodyItem::LinkSpecUpdate)) {
                            onNewButtonClicked();
                        }
                    });
        }
    }
}


void GainTestView::Impl::onOpenButtonClicked()
{
    string filename = getOpenFileName(_("YAML File"), "yaml");

    if(!filename.empty()) {
        load(filename);
    }
}


void GainTestView::Impl::onSaveButtonClicked()
{
    string filename = getSaveFileName(_("YAML File"), "yaml");

    if(!filename.empty()) {
       filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != ".yaml") {
            filename += ".yaml";
        }
        save(filename);
    }
}


void GainTestView::Impl::onPrintButtonClicked()
{
    QString text1 = "P = { ";
    QString text2 = "D = { ";
    for(size_t i = 0; i < interfaceUnits.size(); ++i) {
        auto& unit = interfaceUnits[i];
        text1 += QString::number(unit->pgainSpinBox->value(), 'f', 3);
        text2 += QString::number(unit->dgainSpinBox->value(), 'f', 3);
        if(i < interfaceUnits.size() - 1) {
            text1 += ", ";
            text2 += ", ";
        } else {
            text1 += " };";
            text2 += " };";
        }
    }
    MessageView::instance()->putln(formatR("{0}\n{1}", text1.toStdString(), text2.toStdString()));
}


bool GainTestView::Impl::load(const string& filename, ostream& os)
{
    if(!bodyItem) {
        return false;
    }

    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {

            string str;
            string name = bodyItem->body()->name();
            if(node->read("name", str)) {
                if(str != name) {
                    return false;
                }
            }

            int nj;
            int num_joints = bodyItem->body()->numJoints();
            if(node->read("num_joints", nj)) {
                if(nj != num_joints) {
                    return false;
                }
            }

            for(size_t i = 0; i < interfaceUnits.size(); ++i) {
                auto& unit = interfaceUnits[i];

                const Listing& pgains = *node->findListing("pgains");

                if(pgains.isValid() && !pgains.empty()) {
                    const ValueNode& value = pgains[i];
                    double pgain;
                    if(value.read(pgain)) {
                        unit->pgainSpinBox->setValue(pgain);
                    }
                }

                const Listing& psteps = *node->findListing("psteps");

                if(psteps.isValid() && !psteps.empty()) {
                    const ValueNode& value = psteps[i];
                    double pstep;
                    if(value.read(pstep)) {
                        unit->pstepSpinBox->setValue(pstep);
                    }
                }

                const Listing& dgains = *node->findListing("dgains");

                if(dgains.isValid() && !dgains.empty()) {
                    const ValueNode& value = dgains[i];
                    double dgain;
                    if(value.read(dgain)) {
                        unit->dgainSpinBox->setValue(dgain);
                    }
                }

                const Listing& dsteps = *node->findListing("dsteps");

                if(dsteps.isValid() && !dsteps.empty()) {
                    const ValueNode& value = dsteps[i];
                    double dstep;
                    if(value.read(dstep)) {
                        unit->dstepSpinBox->setValue(dstep);
                    }
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }

    return true;
}


bool GainTestView::Impl::save(const string& filename)
{
    if(!bodyItem) {
        return false;
    }

    MappingPtr node = new Mapping;

    string name = bodyItem->body()->name();
    node->write("name", name);

    int num_joints = bodyItem->body()->numJoints();
    node->write("num_joints", num_joints);

    Listing* pgains = node->createFlowStyleListing("pgains");
    Listing* psteps = node->createFlowStyleListing("psteps");
    Listing* dgains = node->createFlowStyleListing("dgains");
    Listing* dsteps = node->createFlowStyleListing("dsteps");

    for(size_t i = 0; i < interfaceUnits.size(); ++i) {
        auto& unit = interfaceUnits[i];
        pgains->append(unit->pgainSpinBox->value());
        psteps->append(unit->pstepSpinBox->value());
        dgains->append(unit->dgainSpinBox->value());
        dsteps->append(unit->dstepSpinBox->value());
    }

    if(yamlWriter.openFile(filename)) {
        yamlWriter.putNode(node);
        yamlWriter.closeFile();
    }

    return true;
}


bool GainTestView::storeState(Archive& archive)
{
    return true;
}


bool GainTestView::restoreState(const Archive& archive)
{
    return true;
}


GainTestView::Impl::InterfaceUnit::~InterfaceUnit()
{
    delete nameLabel;
    delete pgainLabel;
    delete dgainLabel;
    delete pgainSpinBox;
    delete dgainSpinBox;
    delete pstepSpinBox;
    delete dstepSpinBox;
    delete pplusButton;
    delete dplusButton;
    delete pminusButton;
    delete dminusButton;
}