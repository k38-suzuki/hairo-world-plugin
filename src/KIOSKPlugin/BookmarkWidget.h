/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSKPLUGIN_BOOKMARKWIDGET_H
#define CNOID_KIOSKPLUGIN_BOOKMARKWIDGET_H

#include <cnoid/Archive>
#include <cnoid/Widget>

namespace cnoid {

class BookmarkWidgetImpl;

class BookmarkWidget : public Widget
{
public:
    BookmarkWidget();
    virtual ~BookmarkWidget();

    std::string memo() const;

    void store(Mapping& archive);
    void restore(const Mapping& archive);

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);

private:
    BookmarkWidgetImpl* impl;
    friend class BookmarkWidgetImpl;
};

}

#endif // CNOID_KIOSKPLUGIN_BOOKMARKWIDGET_H
