/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_CRAWLERGENERATOR_H
#define CNOID_BODYGENERATORPLUGIN_CRAWLERGENERATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class CrawlerGeneratorImpl;

class CrawlerGenerator
{
public:
    CrawlerGenerator();
    virtual ~CrawlerGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    CrawlerGeneratorImpl* impl;
    friend class CrawlerGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_CRAWLERGENERATOR_H
