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
    static void initializeClass(ExtensionManager* ext);

    CrawlerGenerator();
    virtual ~CrawlerGenerator();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODYEDIT_PLUGIN_CRAWLER_GENERATOR_H
