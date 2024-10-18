/**
    @author Kenta Suzuki
*/

#include "ListedWidget.h"
#include <QBoxLayout>
#include <QListWidgetItem>

using namespace cnoid;


ListedWidget::ListedWidget(QWidget* parent)
    : QWidget(parent)
{
    stackedWidget = new QStackedWidget(this);

    listWidget = new QListWidget(this);
    listWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(listWidget, &QListWidget::currentRowChanged,
        [&](int currentRow){ stackedWidget->setCurrentIndex(currentRow); });

    elementLayout = new QHBoxLayout;

    auto layout = new QVBoxLayout;
    layout->addWidget(listWidget);
    layout->addLayout(elementLayout);

    auto layout2 = new QHBoxLayout;
    layout2->addLayout(layout);
    layout2->addWidget(stackedWidget);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout2);
    setLayout(mainLayout);

    setWindowTitle("");
}


ListedWidget::~ListedWidget()
{

}


void ListedWidget::addWidget(const QString& text, QWidget* widget)
{
    auto item = new QListWidgetItem(listWidget);
    item->setText(text);
    stackedWidget->addWidget(widget);

    listWidget->setCurrentRow(0);
    stackedWidget->setCurrentIndex(0);
}


void ListedWidget::addWidget(const QIcon& icon, const QString& text, QWidget* widget)
{
    auto item = new QListWidgetItem(listWidget);
    item->setIcon(icon);
    item->setText(text);
    stackedWidget->addWidget(widget);

    listWidget->setCurrentRow(0);
    stackedWidget->setCurrentIndex(0);
}


void ListedWidget::addLayout(const QString& text, QLayout* layout)
{
    auto widget = new QWidget;
    widget->setLayout(layout);
    this->addWidget(text, widget);
}


void ListedWidget::addLayout(const QIcon& icon, const QString& text, QLayout* layout)
{
    auto widget = new QWidget;
    widget->setLayout(layout);
    this->addWidget(icon, text, widget);
}


QPushButton* ListedWidget::addButton(const QString& text)
{
    auto pushButton = new QPushButton(text);
    elementLayout->addWidget(pushButton);
    return pushButton;
}


QPushButton* ListedWidget::addButton(const QIcon& icon, const QString& text)
{
    auto pushButton = new QPushButton(icon, text);
    elementLayout->addWidget(pushButton);
    return pushButton;
}