/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_CRAWLER_GENERATOR_DIALOG_H
#define CNOID_BODY_GENERATOR_PLUGIN_CRAWLER_GENERATOR_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class CrawlerGeneratorDialogImpl;

class CrawlerGeneratorDialog : public Dialog
{
public:
    CrawlerGeneratorDialog();
    virtual ~CrawlerGeneratorDialog();

    static CrawlerGeneratorDialog* instance();

private:
    CrawlerGeneratorDialogImpl* impl;
    friend class CrawlerGeneratorDialogImpl;
};

}

#endif // CNOID_BODY_GENERATOR_PLUGIN_CRAWLER_GENERATOR_DIALOG_H