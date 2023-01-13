/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_BOOKMARK_WIDGET_H
#define CNOID_KIOSK_PLUGIN_BOOKMARK_WIDGET_H

#include <cnoid/Archive>
#include <cnoid/Widget>

namespace cnoid {

class BookmarkWidgetImpl;

class BookmarkWidget : public Widget
{
public:
    BookmarkWidget();
    virtual ~BookmarkWidget();

    void store(Mapping& archive);
    void restore(const Mapping& archive);

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);

private:
    BookmarkWidgetImpl* impl;
    friend class BookmarkWidgetImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_BOOKMARK_WIDGET_H