/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_GENERATOR_PLUGIN_CRAWLER_GENERATOR_H
#define CNOID_BODY_GENERATOR_PLUGIN_CRAWLER_GENERATOR_H

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

#endif // CNOID_BODY_GENERATOR_PLUGIN_CRAWLER_GENERATOR_H