/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_CRAWLERBUILDERDIALOG_H
#define CNOID_BODYGENERATORPLUGIN_CRAWLERBUILDERDIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class CrawlerBuilderDialogImpl;

class CrawlerBuilderDialog : public Dialog
{
public:
    CrawlerBuilderDialog();
    virtual ~CrawlerBuilderDialog();

    static CrawlerBuilderDialog* instance();

    bool save(const std::string& filename);

protected:
    virtual void onAccepted();
    virtual void onRejected();

private:
    CrawlerBuilderDialogImpl* impl;
    friend class CrawlerBuilderDialogImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_CRAWLERBUILDERDIALOG_H
