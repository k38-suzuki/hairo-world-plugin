/**
   \file
   \author Kenta Suzuki
*/

#include "MotionPlanner.h"
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/EigenTypes>
#include <cnoid/JointPath>
#include <cnoid/MenuManager>
#include <cnoid/MeshGenerator>
#include <cnoid/MessageView>
#include <cnoid/PositionDragger>
#include <cnoid/RootItem>
#include <cnoid/SceneDrawables>
#include <cnoid/SceneView>
#include <cnoid/SceneWidget>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/Timer>
#include <cnoid/ViewManager>
#include <cnoid/WorldItem>
#include <fmt/format.h>
#include <QColor>
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

using namespace std;
using namespace cnoid;

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
    CheckBox* statesCheck;
    CheckBox* solutionCheck;
    CheckBox* cubicCheck;
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
    CheckBox* startCheck;
    CheckBox* goalCheck;
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

    SgPosTransformPtr scene;
    PlannerConfigDialog* config;
    SgSwitchableGroupPtr startScene;
    SgSwitchableGroupPtr goalScene;
    SgSwitchableGroupPtr statesScene;
    SgSwitchableGroupPtr solutionScene;
    ItemList<BodyItem> bodyItems;
    BodyItem* bodyItem;
    Body* body;
    Link* baseLink;
    Link* endLink;
    WorldItem* worldItem;
    MessageView* mv;
    vector<Vector3> solutions;
    Interpolator<VectorXd> interpolator;
    double time;
    double timeStep;
    double timeLength;
    Timer* timer;
    bool isSolved;
    PositionDraggerPtr startDragger;
    PositionDraggerPtr goalDragger;

    void createScene();
    void onTargetLinkChanged();
    void onStartButtonClicked();
    void onGoalButtonClicked();
    void onGenerateButtonClicked();
    void onPreviewButtonToggled(bool on);
    void onPreviewTimeout();
    void onCheckToggled();
    void onCurrentIndexChanged(int index);
    void onStartValueChanged();
    void onGoalValueChanged();
    void onStartPositionDragged();
    void onGoalPositionDragged();
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
    worldItem = nullptr;
    solutions.clear();
    interpolator.clear();
    time = 0.0;
    timeStep = 0.001;
    timeLength = 1.0;
    timer = new Timer;
    timer->start(1);
    isSolved = false;
    startDragger = nullptr;
    goalDragger = nullptr;

    createScene();

    config->generateButton->sigClicked().connect([&](){ onGenerateButtonClicked(); });
    RootItem::instance()->sigCheckToggled().connect([&](Item* item, bool on){
        onCheckToggled();
    });
    config->previewButton->sigToggled().connect([&](bool on){ onPreviewButtonToggled(on); });
    config->bodyCombo->sigCurrentIndexChanged().connect([&](int index){ onCurrentIndexChanged(index); });
    config->cubicCheck->sigToggled().connect([&](bool on){
        config->cubicSpin->setEnabled(on);
        config->xminSpin->setEnabled(!on);
        config->xmaxSpin->setEnabled(!on);
        config->yminSpin->setEnabled(!on);
        config->ymaxSpin->setEnabled(!on);
        config->zminSpin->setEnabled(!on);
        config->zmaxSpin->setEnabled(!on);
    });
    config->cubicSpin->sigValueChanged().connect([&](double value){
        config->xminSpin->setValue(-1.0 * value);
        config->xmaxSpin->setValue(value);
        config->yminSpin->setValue(-1.0 * value);
        config->ymaxSpin->setValue(value);
        config->zminSpin->setValue(-1.0 * value);
        config->zmaxSpin->setValue(value);
    });
    config->statesCheck->sigToggled().connect([&](bool on){
        statesScene->setTurnedOn(on);
        statesScene->notifyUpdate();
    });
    config->solutionCheck->sigToggled().connect([&](bool on){
        solutionScene->setTurnedOn(on);
        solutionScene->notifyUpdate();
    });
    config->startCheck->sigToggled().connect([&](bool on){
        startScene->setTurnedOn(on);
        startScene->notifyUpdate();
    });
    config->goalCheck->sigToggled().connect([&](bool on){
        goalScene->setTurnedOn(on);
        goalScene->notifyUpdate();
    });
    config->startxSpin->sigValueChanged().connect([&](double value){ onStartValueChanged(); });
    config->startySpin->sigValueChanged().connect([&](double value){ onStartValueChanged(); });
    config->startzSpin->sigValueChanged().connect([&](double value){ onStartValueChanged(); });
    config->goalxSpin->sigValueChanged().connect([&](double value){ onGoalValueChanged(); });
    config->goalySpin->sigValueChanged().connect([&](double value){ onGoalValueChanged(); });
    config->goalzSpin->sigValueChanged().connect([&](double value){ onGoalValueChanged(); });

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


void MotionPlannerImpl::createScene()
{
    if(!scene) {
        scene = new SgPosTransform;
    } else {
        scene->clearChildren();
    }

    MeshGenerator generator;

    startScene = new SgSwitchableGroup;
    startScene->setTurnedOn(false);
    startDragger = new PositionDragger(
                PositionDragger::TranslationAxes, PositionDragger::WideHandle);
    startDragger->setDragEnabled(true);
    startDragger->setOverlayMode(true);
    startDragger->setPixelSize(48, 2);
    startDragger->setDisplayMode(PositionDragger::DisplayInEditMode);
    SgPosTransform* startPos = new SgPosTransform;
    SgShape* startShape = new SgShape;
    startShape->setMesh(generator.generateSphere(0.03));
    startShape->getOrCreateMaterial()->setDiffuseColor(Vector3(0.0, 1.0, 0.0));
    SgGroup* startGroup = new SgGroup;
    startGroup->addChild(startShape);
    startGroup->addChild(startDragger);
    startPos->addChild(startGroup);
    startScene->addChild(startPos);
    startDragger->adjustSize(startShape->boundingBox());
    startDragger->sigPositionDragged().connect([&](){ onStartPositionDragged(); });

    goalScene = new SgSwitchableGroup;
    goalScene->setTurnedOn(false);
    goalDragger = new PositionDragger(
                PositionDragger::TranslationAxes, PositionDragger::WideHandle);
    goalDragger->setDragEnabled(true);
    goalDragger->setOverlayMode(true);
    goalDragger->setPixelSize(48, 2);
    goalDragger->setDisplayMode(PositionDragger::DisplayInEditMode);
    SgPosTransform* goalPos = new SgPosTransform();
    SgShape* goalShape = new SgShape;
    goalShape->setMesh(generator.generateSphere(0.03));
    goalShape->getOrCreateMaterial()->setDiffuseColor(Vector3(1.0, 0.0, 0.0));
    SgGroup* goalGroup = new SgGroup;
    goalGroup->addChild(goalShape);
    goalGroup->addChild(goalDragger);
    goalPos->addChild(goalGroup);
    goalScene->addChild(goalPos);
    goalDragger->adjustSize(goalShape->boundingBox());
    goalDragger->sigPositionDragged().connect([&](){ onGoalPositionDragged(); });

    statesScene = new SgSwitchableGroup;
    statesScene->setTurnedOn(false);
    solutionScene = new SgSwitchableGroup;
    solutionScene->setTurnedOn(false);

    scene->addChild(startScene);
    scene->addChild(goalScene);
    scene->addChild(statesScene);
    scene->addChild(solutionScene);

    SceneWidget* sceneWidget = SceneView::instance()->sceneWidget();
    sceneWidget->sceneRoot()->addChild(scene);

    ViewManager::sigViewCreated().connect([&](View* view){
        SceneView* sceneView = dynamic_cast<SceneView*>(view);
        if(sceneView) {
            sceneView->sceneWidget()->sceneRoot()->addChildOnce(scene);
        }
    });
    ViewManager::sigViewRemoved().connect([&](View* view){
        SceneView* sceneView = dynamic_cast<SceneView*>(view);
        if(sceneView) {
            sceneView->sceneWidget()->sceneRoot()->removeChild(scene);
        }
    });
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
    statesScene->clearChildren();
    solutionScene->clearChildren();
    if(bodyItems.size()) {
        bodyItem = bodyItems[config->bodyCombo->currentIndex()];
        body = bodyItem->body();
        bodyItem->restoreInitialState(true);
        baseLink = body->link(config->baseCombo->currentIndex());
        endLink = body->link(config->endCombo->currentIndex());
        worldItem = bodyItem->findOwnerItem<WorldItem>();
    }
    planWithSimpleSetup();
}


void MotionPlannerImpl::onPreviewButtonToggled(bool on)
{
    if(on && isSolved) {
        time = 0.0;
        interpolator.clear();
        int numPoints = solutions.size();
        timeLength = config->timeLengthSpin->value();
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
        config->bodyCombo->addItem(QString::fromStdString(bodyItems[i]->name()));
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
            config->baseCombo->addItem(QString::fromStdString(link->name()));
            config->endCombo->addItem(QString::fromStdString(link->name()));
        }
    }
}


void MotionPlannerImpl::onStartValueChanged()
{
    SgPosTransform* pos = dynamic_cast<SgPosTransform*>(startScene->child(0));
    Vector3 translation = Vector3(config->startxSpin->value(), config->startySpin->value(), config->startzSpin->value());
    pos->setTranslation(translation);
    startScene->notifyUpdate();
}


void MotionPlannerImpl::onGoalValueChanged()
{
    SgPosTransform* pos = dynamic_cast<SgPosTransform*>(goalScene->child(0));
    Vector3 translation = Vector3(config->goalxSpin->value(), config->goalySpin->value(), config->goalzSpin->value());
    pos->setTranslation(translation);
    goalScene->notifyUpdate();
}


void MotionPlannerImpl::onStartPositionDragged()
{
    Vector3 p = startDragger->globalDraggingPosition().translation();
    config->startxSpin->setValue(p[0]);
    config->startySpin->setValue(p[1]);
    config->startzSpin->setValue(p[2]);
}


void MotionPlannerImpl::onGoalPositionDragged()
{
    Vector3 p = goalDragger->globalDraggingPosition().translation();
    config->goalxSpin->setValue(p[0]);
    config->goalySpin->setValue(p[1]);
    config->goalzSpin->setValue(p[2]);
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
        mv->putln("Found solution:");
        isSolved = true;

        og::PathGeometric pathes = ss.getSolutionPath();
        const int numPoints = pathes.getStateCount();
        solutions.clear();
        for(size_t i = 0; i < pathes.getStateCount(); i++) {
            ob::State* state = pathes.getState(i);
            float x = state->as<ob::SE3StateSpace::StateType>()->getX();
            float y = state->as<ob::SE3StateSpace::StateType>()->getY();
            float z = state->as<ob::SE3StateSpace::StateType>()->getZ();
            solutions.push_back(Vector3(x, y, z));

            MeshGenerator generator;
            SgShape* shape = new SgShape;
            shape->setMesh(generator.generateSphere(0.02));
            SgMaterial* material = new SgMaterial;
            int hue = 240.0 * (1.0 - (double)i / (double)(numPoints - 1));
            QColor qColor = QColor::fromHsv(hue, 255, 255);
            Vector3f color((double)qColor.red() / 255.0, (double)qColor.green() / 255.0, (double)qColor.blue() / 255.0);
            material->setDiffuseColor(Vector3(color[0], color[1], color[2]));
            material->setTransparency(0.5);
            shape->setMaterial(material);
            SgPosTransform* transform = new SgPosTransform;
            transform->addChild(shape);
            transform->setTranslation(Vector3(x, y, z));
            solutionScene->addChild(transform);

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

        ss.simplifySolution();
//        ss.getSolutionPath().print(mv->cout());
    } else {
        mv->putln("No solution found");
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

    MeshGenerator generator;
    SgShape* shape = new SgShape;
    shape->setMesh(generator.generateSphere(0.02));
    SgMaterial* material = new SgMaterial;
    material->setDiffuseColor(Vector3(0.0, 1.0, 0.0));
    material->setTransparency(0.5);
    shape->setMaterial(material);
    SgPosTransform* transform = new SgPosTransform;
    transform->addChild(shape);
    transform->setTranslation(Vector3(x, y, z));
    statesScene->addChild(transform);

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

    HSeparatorBox* tbsbox = new HSeparatorBox(new QLabel(_("Target Body")));
    vbox->addLayout(tbsbox);
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

    HSeparatorBox* bbsbox = new HSeparatorBox(new QLabel(_("Bounding Box")));
    vbox->addLayout(bbsbox);

    QGridLayout* bbgbox = new QGridLayout;
    cubicCheck = new CheckBox;
    cubicCheck->setText(_("Cubic BB"));
    cubicSpin = new DoubleSpinBox;
    cubicSpin->setRange(0.0, 1000.0);
    cubicSpin->setValue(1.0);
    cubicSpin->setAlignment(Qt::AlignCenter);
    cubicSpin->setEnabled(false);
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

    bbgbox->addWidget(cubicCheck, 0, 0);
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

    HSeparatorBox* ctsbox = new HSeparatorBox(new QLabel(_("Path Generation")));
    vbox->addLayout(ctsbox);

    QGridLayout* pgbox = new QGridLayout;
    plannerCombo = new ComboBox;
    QStringList planners = { "RRT", "RRTConnect", "RRT*", "pRRT" };
    plannerCombo->addItems(planners);
    timeSpin = new DoubleSpinBox;
    timeSpin->setRange(0.0, 1000.0);
    timeSpin->setValue(1.0);
    timeSpin->setAlignment(Qt::AlignCenter);
    statesCheck = new CheckBox;
    statesCheck->setText(_("Show states"));
    statesCheck->setChecked(false);
    solutionCheck = new CheckBox;
    solutionCheck->setText(_("Show solution"));
    solutionCheck->setChecked(false);

    startCheck = new CheckBox;
    startCheck->setChecked(false);
    startCheck->setText(_("Start[x, y, z]"));
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

    goalCheck = new CheckBox;
    goalCheck->setChecked(false);
    goalCheck->setText(_("Goal[x, y, z]"));
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

    pgbox->addWidget(new QLabel(_("Geometric planner")), 0, 0);
    pgbox->addWidget(plannerCombo, 0, 1);
    pgbox->addWidget(startButton, 0, 2);
    pgbox->addWidget(goalButton, 0, 3);
    pgbox->addWidget(startCheck, 1, 0);
    pgbox->addWidget(startxSpin, 1, 1);
    pgbox->addWidget(startySpin, 1, 2);
    pgbox->addWidget(startzSpin, 1, 3);
    pgbox->addWidget(goalCheck, 2, 0);
    pgbox->addWidget(goalxSpin, 2, 1);
    pgbox->addWidget(goalySpin, 2, 2);
    pgbox->addWidget(goalzSpin, 2, 3);
    pgbox->addWidget(new QLabel(_("Calculation time")), 3, 0);
    pgbox->addWidget(timeSpin, 3, 1);
    vbox->addLayout(pgbox);

    HSeparatorBox* psbox = new HSeparatorBox(new QLabel(_("Preview")));
    vbox->addLayout(psbox);

    generateButton = new PushButton(_("Generate"));
    previewButton = new ToggleButton(_("Preview"));
    timeLengthSpin = new DoubleSpinBox;
    timeLengthSpin->setRange(1.0, 1000.0);
    timeLengthSpin->setValue(1.0);
    timeLengthSpin->setAlignment(Qt::AlignCenter);
    QGridLayout* pvbox = new QGridLayout;
    pvbox->addWidget(solutionCheck, 0, 0);
    pvbox->addWidget(statesCheck, 0, 1);
    pvbox->addWidget(generateButton, 0, 2);
    pvbox->addWidget(new QLabel(_("Time length")), 1, 0);
    pvbox->addWidget(timeLengthSpin, 1, 1);
    pvbox->addWidget(previewButton, 1, 2);
    vbox->addLayout(pvbox);

    vbox->addWidget(new HSeparator);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });
    vbox->addWidget(buttonBox);

    setLayout(vbox);
}
