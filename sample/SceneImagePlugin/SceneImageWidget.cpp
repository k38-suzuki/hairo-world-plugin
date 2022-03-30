/**
   \file
   \author Kenta Suzuki
*/

#include "SceneImageWidget.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/SceneView>
#include <cnoid/SceneWidget>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/TimeBar>
#include <cnoid/ViewManager>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class CursorConfigDialog : public Dialog
{
public:
    CursorConfigDialog();

    SpinBox* radiusSpin;
    ComboBox* colorCombo;
};

class SceneImageWidgetImpl
{
public:
    SceneImageWidgetImpl(SceneImageWidget* self);
    SceneImageWidget* self;

    QPoint pos;
    bool scribbling;
    bool brushing;
    CursorConfigDialog* config;
    SceneView* sceneView;

    void onTimeChanged(const double& time);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


SceneImageWidget::SceneImageWidget()
{
    impl = new SceneImageWidgetImpl(this);
}


SceneImageWidgetImpl::SceneImageWidgetImpl(SceneImageWidget* self)
    : self(self)
{
    pos = QPoint(0, 0);
    scribbling = false;
    brushing = false;
    config = new CursorConfigDialog;
    sceneView = nullptr;

    TimeBar::instance()->sigTimeChanged().connect(
                [&](double time){ onTimeChanged(time); return true; });
}


SceneImageWidget::~SceneImageWidget()
{
    delete impl;
}


void SceneImageWidgetImpl::onTimeChanged(const double& time)
{
    self->update();
}


void SceneImageWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    impl->mouseDoubleClickEvent(event);
}


void SceneImageWidgetImpl::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        brushing = !brushing;
    } else if(event->button() == Qt::RightButton) {
        config->show();
    }
}


void SceneImageWidget::mouseMoveEvent(QMouseEvent* event)
{
    impl->mouseMoveEvent(event);
}


void SceneImageWidgetImpl::mouseMoveEvent(QMouseEvent* event)
{
    if(scribbling) {
        pos = event->pos();
        self->update();
    }
}


void SceneImageWidget::mousePressEvent(QMouseEvent* event)
{
    impl->mousePressEvent(event);
}


void SceneImageWidgetImpl::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        pos = event->pos();
        scribbling = true;
        sceneView = SceneView::lastFocusSceneView();
        self->update();
    }
}


void SceneImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
    impl->mouseReleaseEvent(event);
}


void SceneImageWidgetImpl::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        pos = event->pos();
        scribbling = false;
        self->update();
    }
}


void SceneImageWidget::paintEvent(QPaintEvent* event)
{
    impl->paintEvent(event);
}


void SceneImageWidgetImpl::paintEvent(QPaintEvent* event)
{
    static const Qt::GlobalColor colors[] = { Qt::red, Qt::green, Qt::blue,
                                              Qt::cyan, Qt::magenta, Qt::yellow };
    QPainter painter(self);
    if(sceneView) {
        QImage image = sceneView->sceneWidget()->getImage();
        painter.drawImage(0, 0, image);
    }

    if(scribbling) {
        int index = config->colorCombo->currentIndex();
        Qt::GlobalColor color = colors[index];
        painter.setPen(color);
        if(brushing) {
            painter.setBrush(color);
        }
        int radius = config->radiusSpin->value();
        painter.drawEllipse(pos, radius * 2, radius * 2);
    }
}


bool SceneImageWidget::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool SceneImageWidgetImpl::storeState(Archive& archive)
{
    if(sceneView) {
        archive.write("scene_view", sceneView->name());
    }
    archive.write("brushing", brushing);
    archive.write("radius", config->radiusSpin->value());
    archive.write("color", config->colorCombo->currentIndex());
    return true;
}


bool SceneImageWidget::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool SceneImageWidgetImpl::restoreState(const Archive& archive)
{
    string name;
    archive.read("scene_view", name);
    sceneView = ViewManager::findView<SceneView>(name);
    archive.read("brushing", brushing);
    config->radiusSpin->setValue(archive.get("radius", 5));
    config->colorCombo->setCurrentIndex(archive.get("color", 0));
    return true;
}


CursorConfigDialog::CursorConfigDialog()
{
    setWindowTitle(_("Configuration of cursor"));

    QStringList items = { _("Red"), _("Green"), _("Blue"),
                          _("Cyan"), _("Magenta"), _("Yellow") };

    radiusSpin = new SpinBox;
    radiusSpin->setRange(1, 50);
    radiusSpin->setValue(5);
    colorCombo = new ComboBox;
    colorCombo->addItems(items);
    colorCombo->setCurrentIndex(0);

    QGridLayout* gbox = new QGridLayout;
    static const char* labels[] = { _("Radius"), _("Color") };
    for(int i = 0; i < 2; ++i) {
        gbox->addWidget(new QLabel(labels[i]), i, 0);
    }

    gbox->addWidget(radiusSpin, 0, 1);
    gbox->addWidget(colorCombo, 1, 1);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(new HSeparatorBox(new QLabel(_("Cursor"))));
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}
