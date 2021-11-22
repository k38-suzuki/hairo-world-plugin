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
    CrawlerGenerator(ExtensionManager* ext);
    virtual ~CrawlerGenerator();

    static void initialize(ExtensionManager* ext);

private:
    CrawlerGeneratorImpl* impl;
    friend class CrawlerGeneratorImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_CRAWLERGENERATOR_H
