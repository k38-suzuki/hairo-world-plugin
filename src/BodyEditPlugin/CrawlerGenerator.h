/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_CRAWLER_GENERATOR_H
#define CNOID_BODY_EDIT_PLUGIN_CRAWLER_GENERATOR_H

namespace cnoid {

class ExtensionManager;

class CrawlerGenerator
{
public:
    CrawlerGenerator();
    virtual ~CrawlerGenerator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
