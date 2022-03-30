/**
   \file
   \author Kenta Suzuki
*/

#include "MotionPlanner.h"
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/EigenTypes>
#include <cnoid/JointPath>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/PointSetItem>
#include <cnoid/RootItem>
#include <cnoid/SceneDrawables>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/Timer>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRTstar.h>
#include <ompl/geometric/planners/rrt/pRRT.h>
#include <ompl/geometric/SimpleSetup.h>
#include <ompl/config.h>
#include "sample/SimpleController/Interpolator.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace {

MotionPlanner* plannerInstance = nullptr;

}

namespace cnoid {

class PlannerConfigDialog : public Dialog
{
public:
    PlannerConfigDialog();

    ComboBox* bodyCombo;
    ComboBox* baseCombo;
    ComboBox* endCombo;
    ComboBox* plannerCombo;
    DoubleSpinBox* cubicSpin;
    DoubleSpinBox* xminSpin;
    DoubleSpinBox* xmaxSpin;
    DoubleSpinBox* yminSpin;
    DoubleSpinBox* ymaxSpin;
    DoubleSpinBox* zminSpin;
    DoubleSpinBox* zmaxSpin;
    DoubleSpinBox* startxSpin;
    DoubleSpinBox* startySpin;
    DoubleSpinBox* startzSpin;
    DoubleSpinBox* goalxSpin;
    DoubleSpinBox* goalySpin;
    DoubleSpinBox* goalzSpin;
    DoubleSpinBox* timeLengthSpin;
    DoubleSpinBox* timeSpin;
    ToggleButton* previewButton;
    PushButton* startButton;
    PushButton* goalButton;
    PushButton* generateButton;
};

class MotionPlannerImpl
{
public:
    MotionPlannerImpl(MotionPlanner* self);
    MotionPlanner* self;

    PlannerConfigDialog* config;
    ItemList<BodyItem> bodyItems;
    BodyItem* bodyItem;
    Body* body;
    Link* baseLink;
    Link* endLink;
    MessageView* mv;
    vector<Vector3> solutions;
    Interpolator<VectorXd> interpolator;
    double time;
    double timeStep;
    Timer* timer;
    bool isSolved;
    PointSetItem* statePointSetItem;

    void onTargetLinkChanged();
    void onStartButtonClicked();
    void onGoalButtonClicked();
    void onGenerateButtonClicked();
    void onPreviewButtonToggled(bool on);
    void onPreviewTimeout();
    void onCheckToggled();
    void onCurrentIndexChanged(int index);
    void planWithSimpleSetup();
    bool isStateValid(const ob::State* state);
};

}


MotionPlanner::MotionPlanner()
{
    impl = new MotionPlannerImpl(this);
}


MotionPlannerImpl::MotionPlannerImpl(MotionPlanner* self)
    : self(self),
      mv(MessageView::instance()),
      config(new PlannerConfigDialog)
{
    bodyItems.clear();
    bodyItem = nullptr;
    body = nullptr;
    baseLink = nullptr;
    endLink = nullptr;
    solutions.clear();
    interpolator.clear();
    time = 0.0;
    timeStep = 0.001;
    timer = new Timer;
    timer->start(1);
    isSolved = false;
    statePointSetItem = nullptr;

    config->generateButton->sigClicked().connect([&](){ onGenerateButtonClicked(); });
    RootItem::instance()->sigCheckToggled().connect([&](Item* item, bool on){
        onCheckToggled();
    });
    config->previewButton->sigToggled().connect([&](bool on){ onPreviewButtonToggled(on); });
    config->bodyCombo->sigCurrentIndexChanged().connect([&](int index){ onCurrentIndexChanged(index); });
    config->cubicSpin->sigValueChanged().connect([&](double value){
        config->xminSpin->setValue(-1.0 * value);
        config->xmaxSpin->setValue(value);
        config->yminSpin->setValue(-1.0 * value);
        config->ymaxSpin->setValue(value);
        config->zminSpin->setValue(-1.0 * value);
        config->zmaxSpin->setValue(value);
    });

    timer->sigTimeout().connect([&](){ onPreviewTimeout(); });
    config->startButton->sigClicked().connect([&](){ onStartButtonClicked(); });
    config->goalButton->sigClicked().connect([&](){ onGoalButtonClicked(); });
}


MotionPlanner::~MotionPlanner()
{
    delete impl;
}


void MotionPlanner::initializeClass(ExtensionManager* ext)
{
    string version = OMPL_VERSION;
    MessageView::instance()->putln(fmt::format("OMPL version: {0}", version));

    if(!plannerInstance) {
        plannerInstance = ext->manage(new MotionPlanner);
    }

    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
    mm.addItem(_("Motion Planner"))->sigTriggered().connect(
                [&](){ plannerInstance->impl->config->show(); });
}


void MotionPlannerImpl::onTargetLinkChanged()
{
    bodyItem = bodyItems[config->bodyCombo->currentIndex()];
    body = bodyItem->body();
    endLink = body->link(config->endCombo->currentIndex());
}


void MotionPlannerImpl::onStartButtonClicked()
{
    onTargetLinkChanged();
    if(endLink) {
        Vector3 translation = endLink->T().translation();
        config->startxSpin->setValue(translation[0]);
        config->startySpin->setValue(translation[1]);
        config->startzSpin->setValue(translation[2]);
    }
}


void MotionPlannerImpl::onGoalButtonClicked()
{
    onTargetLinkChanged();
    if(endLink) {
        Vector3 translation = endLink->T().translation();
        config->goalxSpin->setValue(translation[0]);
        config->goalySpin->setValue(translation[1]);
        config->goalzSpin->setValue(translation[2]);
    }
}


void MotionPlannerImpl::onGenerateButtonClicked()
{
    if(bodyItems.size()) {
        bodyItem = bodyItems[config->bodyCombo->currentIndex()];
        body = bodyItem->body();
        bodyItem->restoreInitialState(true);
        baseLink = body->link(config->baseCombo->currentIndex());
        endLink = body->link(config->endCombo->currentIndex());
    }
    planWithSimpleSetup();
}


void MotionPlannerImpl::onPreviewButtonToggled(bool on)
{
    if(on && isSolved) {
        time = 0.0;
        interpolator.clear();
        int numPoints = solutions.size();
        double timeLength = config->timeLengthSpin->value();
        double dt = timeLength / (double)numPoints;

        for(size_t i = 0; i < solutions.size(); i++) {
            interpolator.appendSample(dt * (double)i, solutions[i]);
        }
        interpolator.update();
    }
}


void MotionPlannerImpl::onPreviewTimeout()
{
    if(body && baseLink && endLink) {
        if(config->previewButton->isChecked()) {
            auto path = JointPath::getCustomPath(body, baseLink, endLink);
            VectorXd p(6);
            p = interpolator.interpolate(time);
            Vector3 pref = Vector3(p.head<3>());
            Matrix3 rref = endLink->R();
            Isometry3 T;
            T.linear() = rref;
            T.translation() = pref;
            if(path->calcInverseKinematics(T)) {
                bodyItem->notifyKinematicStateChange(true);
            }
            time += timeStep;
        }
    }
}


void MotionPlannerImpl::onCheckToggled()
{
    bodyItems = RootItem::instance()->checkedItems<BodyItem>();
    config->bodyCombo->clear();

    for(size_t i = 0; i < bodyItems.size(); i++) {
        config->bodyCombo->addItem(bodyItems[i]->name().c_str());
    }
}


void MotionPlannerImpl::onCurrentIndexChanged(int index)
{
    config->baseCombo->clear();
    config->endCombo->clear();
    if(index >= 0) {
        Body* body = bodyItems[index]->body();
        for(size_t i = 0; i < body->numLinks(); i++) {
            Link* link = body->link(i);
            config->baseCombo->addItem(link->name().c_str());
            config->endCombo->addItem(link->name().c_str());
        }
    }
}


void MotionPlannerImpl::planWithSimpleSetup()
{
    auto space(std::make_shared<ob::SE3StateSpace>());

    ob::RealVectorBounds bounds(3);
    bounds.setLow(0, config->xminSpin->value());
    bounds.setHigh(0, config->xmaxSpin->value());
    bounds.setLow(1, config->yminSpin->value());
    bounds.setHigh(1, config->ymaxSpin->value());
    bounds.setLow(2, config->zminSpin->value());
    bounds.setHigh(2, config->zmaxSpin->value());
    space->setBounds(bounds);

    og::SimpleSetup ss(space);

    ItemList<PointSetItem> pointSetItems = bodyItem->descendantItems<PointSetItem>();
    for(size_t i = 0; i < pointSetItems.size(); ++i) {
        pointSetItems[i]->removeFromParentItem();
    }

    statePointSetItem = new PointSetItem;
    statePointSetItem->setName("StatePointSet");
    statePointSetItem->setRenderingMode(PointSetItem::VOXEL);
    statePointSetItem->setVoxelSize(0.03);
    bodyItem->addSubItem(statePointSetItem);

    ss.setStateValidityChecker([&](const ob::State* state) { return isStateValid(state); });

    ob::ScopedState<ob::SE3StateSpace> start(space);
    start->setX(config->startxSpin->value());
    start->setY(config->startySpin->value());
    start->setZ(config->startzSpin->value());
    start->rotation().setIdentity();

    ob::ScopedState<ob::SE3StateSpace> goal(space);
    goal->setX(config->goalxSpin->value());
    goal->setY(config->goalySpin->value());
    goal->setZ(config->goalzSpin->value());
    goal->rotation().setIdentity();

    ss.setStartAndGoalStates(start, goal);

    int index = config->plannerCombo->currentIndex();
    switch (index) {
    case 0:
    {
        ob::PlannerPtr planner(new og::RRT(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case 1:
    {
        ob::PlannerPtr planner(new og::RRTConnect(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case 2:
    {
        ob::PlannerPtr planner(new og::RRTstar(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    case 3:
    {
        ob::PlannerPtr planner(new og::pRRT(ss.getSpaceInformation()));
        ss.setPlanner(planner);
    }
        break;
    default:
        break;
    }

    ss.setup();
//    ss.print(mv->cout());

    ob::PlannerStatus solved = ss.solve(config->timeSpin->value());

    if(solved) {
        mv->putln(_("Found solution:"));
        isSolved = true;

        og::PathGeometric pathes = ss.getSolutionPath();
        const int numPoints = pathes.getStateCount();
        solutions.clear();

        statePointSetItem->setChecked(true);
        PointSetItem* pointSetItem = new PointSetItem;
        pointSetItem->setName("SolvedPointSet");
        pointSetItem->setRenderingMode(PointSetItem::VOXEL);
        pointSetItem->setVoxelSize(0.04);
        pointSetItem->setChecked(true);
        bodyItem->addSubItem(pointSetItem);

        for(size_t i = 0; i < pathes.getStateCount(); i++) {
            ob::State* state = pathes.getState(i);
            float x = state->as<ob::SE3StateSpace::StateType>()->getX();
            float y = state->as<ob::SE3StateSpace::StateType>()->getY();
            float z = state->as<ob::SE3StateSpace::StateType>()->getZ();
            solutions.push_back(Vector3(x, y, z));

            if(bodyItem) {
                bodyItem->restoreInitialState(true);
                if(baseLink != endLink) {
                    auto path = JointPath::getCustomPath(body, baseLink, endLink);
                    Vector3 pref = Vector3(x, y, z);
                    Matrix3 rref = endLink->R();
                    Isometry3 T;
                    T.linear() = rref;
                    T.translation() = pref;
                    if(path->calcInverseKinematics(T)) {
                        bodyItem->notifyKinematicStateChange(true);
                    }
                }
            }
        }

        {
            SgVertexArray& points = *pointSetItem->pointSet()->getOrCreateVertices();
            SgColorArray& colors = *pointSetItem->pointSet()->getOrCreateColors();
    //        const int numPoints = src.size();
            points.resize(numPoints);
            colors.resize(numPoints);
            for(int i = 0; i < numPoints; ++i) {
                Vector3f point = Vector3f(solutions[i][0], solutions[i][1], solutions[i][2]);
                points[i] = point;
                Vector3f& c = colors[i];
                c[0] = 0.0;
                c[1] = 1.0;
                c[2] = 0.0;
            }
            pointSetItem->notifyUpdate();
        }

        {
            vector<Vector3> src;
            for(int i = 0; i < statePointSetItem->numAttentionPoints(); ++i) {
                Vector3 point = statePointSetItem->attentionPoint(i);
                src.push_back(point);
            }

            statePointSetItem->clearAttentionPoints();
            SgVertexArray& points = *statePointSetItem->pointSet()->getOrCreateVertices();
            SgColorArray& colors = *statePointSetItem->pointSet()->getOrCreateColors();
            const int numStates = src.size();
            points.resize(numStates);
            colors.resize(numStates);
            for(int i = 0; i < numStates; ++i) {
                Vector3f point = Vector3f(src[i][0], src[i][1], src[i][2]);
                points[i] = point;
                Vector3f& c = colors[i];
                c[0] = 1.0;
                c[1] = 0.0;
                c[2] = 0.0;
            }
            statePointSetItem->notifyUpdate();
        }

        ss.simplifySolution();
//        ss.getSolutionPath().print(mv->cout());
    } else {
        mv->putln(_("No solution found"));
        isSolved = false;
    }
}


bool MotionPlannerImpl::isStateValid(const ob::State* state)
{
    const auto *se3state = state->as<ob::SE3StateSpace::StateType>();
    const auto *pos = se3state->as<ob::RealVectorStateSpace::StateType>(0);
    const auto *rot = se3state->as<ob::SO3StateSpace::StateType>(1);

    bool solved = false;
    bool collided = false;

    float x = state->as<ob::SE3StateSpace::StateType>()->getX();
    float y = state->as<ob::SE3StateSpace::StateType>()->getY();
    float z = state->as<ob::SE3StateSpace::StateType>()->getZ();

    statePointSetItem->addAttentionPoint(Vector3(x, y, z));

    if(bodyItem) {
        bodyItem->restoreInitialState(true);
        if(baseLink != endLink) {
            auto path = JointPath::getCustomPath(body, baseLink, endLink);
            Vector3 pref = endLink->p();
            Matrix3 rref = endLink->R();
            pref = Vector3(x, y, z);
            Isometry3 T;
            T.linear() = rref;
            T.translation() = pref;
            if(path->calcInverseKinematics(T)) {
                bodyItem->notifyKinematicStateChange(true);
                solved = true;
                WorldItem* worldItem = bodyItem->findOwnerItem<WorldItem>();
                if(worldItem) {
                    worldItem->updateCollisions();
                    vector<CollisionLinkPairPtr> collisions = bodyItem->collisions();
                    for(size_t i = 0; i < collisions.size(); i++) {
                        CollisionLinkPairPtr collision = collisions[i];
                        if((collision->body[0] == body) || (collision->body[1] == body)) {
                            if(!collision->isSelfCollision()) {
                                collided = true;
                            }
                        }
                    }
                }
            }
        }
    }

    return ((const void*)rot != (const void*)pos) && solved && !collided;
}


PlannerConfigDialog::PlannerConfigDialog()
{
    setWindowTitle(_("Motion Planner"));

    QVBoxLayout* vbox = new QVBoxLayout;

    vbox->addLayout(new HSeparatorBox(new QLabel(_("Target Body"))));
    QGridLayout* tbgbox = new QGridLayout;
    bodyCombo = new ComboBox;
    baseCombo = new ComboBox;
    endCombo = new ComboBox;
    tbgbox->addWidget(new QLabel(_("Body")), 0, 0);
    tbgbox->addWidget(bodyCombo, 0, 1);
    tbgbox->addWidget(new QLabel(_("Base Link")), 1, 0);
    tbgbox->addWidget(baseCombo, 1, 1);
    tbgbox->addWidget(new QLabel(_("End Link")), 1, 2);
    tbgbox->addWidget(endCombo, 1, 3);
    vbox->addLayout(tbgbox);

    vbox->addLayout(new HSeparatorBox(new QLabel(_("Bounding Box"))));

    QGridLayout* bbgbox = new QGridLayout;
    cubicSpin = new DoubleSpinBox;
    cubicSpin->setRange(0.0, 1000.0);
    cubicSpin->setValue(1.0);
    cubicSpin->setAlignment(Qt::AlignCenter);
    xminSpin = new DoubleSpinBox;
    xminSpin->setRange(-1000.0, 0.0);
    xminSpin->setValue(-1.0);
    xminSpin->setAlignment(Qt::AlignCenter);
    xmaxSpin = new DoubleSpinBox;
    xmaxSpin->setRange(0.0, 1000.0);
    xmaxSpin->setValue(1.0);
    xmaxSpin->setAlignment(Qt::AlignCenter);

    yminSpin = new DoubleSpinBox;
    yminSpin->setRange(-1000.0, 0.0);
    yminSpin->setValue(-1.0);
    yminSpin->setAlignment(Qt::AlignCenter);
    ymaxSpin = new DoubleSpinBox;
    ymaxSpin->setRange(0.0, 1000.0);
    ymaxSpin->setValue(1.0);
    ymaxSpin->setAlignment(Qt::AlignCenter);

    zminSpin = new DoubleSpinBox;
    zminSpin->setRange(-1000.0, 0.0);
    zminSpin->setValue(-1.0);
    zminSpin->setAlignment(Qt::AlignCenter);
    zmaxSpin = new DoubleSpinBox;
    zmaxSpin->setRange(0.0, 1000.0);
    zmaxSpin->setValue(1.0);
    zmaxSpin->setAlignment(Qt::AlignCenter);

    bbgbox->addWidget(new QLabel(_("Cubic BB")), 0, 0);
    bbgbox->addWidget(cubicSpin, 0, 1);
    bbgbox->addWidget(new QLabel(_("min[x, y, z]")), 1, 0);
    bbgbox->addWidget(xminSpin, 1, 1);
    bbgbox->addWidget(yminSpin, 1, 2);
    bbgbox->addWidget(zminSpin, 1, 3);
    bbgbox->addWidget(new QLabel(_("max[x, y, z]")), 2, 0);
    bbgbox->addWidget(xmaxSpin, 2, 1);
    bbgbox->addWidget(ymaxSpin, 2, 2);
    bbgbox->addWidget(zmaxSpin, 2, 3);
    vbox->addLayout(bbgbox);

    vbox->addLayout(new HSeparatorBox(new QLabel(_("Path Generation"))));

    QHBoxLayout* hbox = new QHBoxLayout;
    plannerCombo = new ComboBox;
    QStringList planners = { "RRT", "RRTConnect", "RRT*", "pRRT" };
    plannerCombo->addItems(planners);
    hbox->addWidget(new QLabel(_("Geometric planner")), 0, 0);
    hbox->addWidget(plannerCombo);
    vbox->addLayout(hbox);

    timeSpin = new DoubleSpinBox;
    timeSpin->setRange(0.0, 1000.0);
    timeSpin->setValue(1.0);
    timeSpin->setAlignment(Qt::AlignCenter);

    startxSpin = new DoubleSpinBox;
    startxSpin->setRange(-1000.0, 1000.0);
    startxSpin->setSingleStep(0.01);
    startxSpin->setValue(0.0);
    startxSpin->setAlignment(Qt::AlignCenter);
    startySpin = new DoubleSpinBox;
    startySpin->setRange(-1000.0, 1000.0);
    startySpin->setSingleStep(0.01);
    startySpin->setValue(0.0);
    startySpin->setAlignment(Qt::AlignCenter);
    startzSpin = new DoubleSpinBox;
    startzSpin->setRange(-1000.0, 1000.0);
    startzSpin->setSingleStep(0.01);
    startzSpin->setValue(0.0);
    startzSpin->setAlignment(Qt::AlignCenter);

    goalxSpin = new DoubleSpinBox;
    goalxSpin->setRange(-1000.0, 1000.0);
    goalxSpin->setSingleStep(0.01);
    goalxSpin->setValue(0.0);
    goalxSpin->setAlignment(Qt::AlignCenter);
    goalySpin = new DoubleSpinBox;
    goalySpin->setRange(-1000.0, 1000.0);
    goalySpin->setSingleStep(0.01);
    goalySpin->setValue(0.0);
    goalySpin->setAlignment(Qt::AlignCenter);
    goalzSpin = new DoubleSpinBox;
    goalzSpin->setRange(-1000.0, 1000.0);
    goalzSpin->setSingleStep(0.01);
    goalzSpin->setValue(0.0);
    goalzSpin->setAlignment(Qt::AlignCenter);

    startButton = new PushButton(_("Set start"));
    goalButton = new PushButton(_("Set goal"));

    generateButton = new PushButton(_("Generate"));
    previewButton = new ToggleButton(_("Preview"));
    timeLengthSpin = new DoubleSpinBox;
    timeLengthSpin->setRange(1.0, 1000.0);
    timeLengthSpin->setValue(1.0);
    timeLengthSpin->setAlignment(Qt::AlignCenter);

    QGridLayout* pgbox = new QGridLayout;
    pgbox->addWidget(new QLabel(_("Start[x, y, z]")), 0, 0);
    pgbox->addWidget(startxSpin, 0, 1);
    pgbox->addWidget(startySpin, 0, 2);
    pgbox->addWidget(startzSpin, 0, 3);
    pgbox->addWidget(new QLabel(_("Goal[x, y, z]")), 1, 0);
    pgbox->addWidget(goalxSpin, 1, 1);
    pgbox->addWidget(goalySpin, 1, 2);
    pgbox->addWidget(goalzSpin, 1, 3);
    pgbox->addWidget(new QLabel(_("Calculation time")), 2, 0);
    pgbox->addWidget(timeSpin, 2, 1);
    pgbox->addWidget(startButton, 3, 1);
    pgbox->addWidget(goalButton, 3, 2);
    pgbox->addWidget(generateButton, 3, 3);
    pgbox->addWidget(new QLabel(_("Time length")), 4, 0);
    pgbox->addWidget(timeLengthSpin, 4, 1);
    pgbox->addWidget(previewButton, 4, 3);
    vbox->addLayout(pgbox);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}
