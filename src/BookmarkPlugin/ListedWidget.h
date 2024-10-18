/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_LISTED_WIDGET_H
#define CNOID_BOOKMARK_PLUGIN_LISTED_WIDGET_H

#include <QBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT ListedWidget : public QWidget
{
public:
    ListedWidget(QWidget* parent = nullptr);
    virtual ~ListedWidget();

    void addWidget(const QString& text, QWidget* widget);
    void addWidget(const QIcon& icon, const QString& text, QWidget* widget);
    void addLayout(const QString& text, QLayout* layout);
    void addLayout(const QIcon& icon, const QString& text, QLayout* layout);

    QPushButton* addButton(const QString& text);
    QPushButton* addButton(const QIcon& icon, const QString& text);

private:
    QListWidget* listWidget;
    QStackedWidget* stackedWidget;
    QHBoxLayout* elementLayout;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_LISTED_WIDGET_H