/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_CRAWLER_GENERATOR_H
#define CNOID_BODYEDIT_PLUGIN_CRAWLER_GENERATOR_H

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

#endif // CNOID_BODYEDIT_PLUGIN_CRAWLER_GENERATOR_H
