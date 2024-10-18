/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BODY_CREATOR_PLUGIN_BODY_CREATOR_H
#define CNOID_BODY_CREATOR_PLUGIN_BODY_CREATOR_H

namespace cnoid {

class ExtensionManager;

class BentPipeCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    BentPipeCreator();
    virtual ~BentPipeCreator();
};

class CrawlerCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    CrawlerCreator();
    virtual ~CrawlerCreator();
};

class GratingCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    GratingCreator();
    virtual ~GratingCreator();

};

class PipeCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    PipeCreator();
    virtual ~PipeCreator();
};

class SlopeCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    SlopeCreator();
    virtual ~SlopeCreator();
};

class StairsCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    StairsCreator();
    virtual ~StairsCreator();
};

class TerrainCreator
{
public:
    static void initializeClass(ExtensionManager* ext);

    TerrainCreator();
    virtual ~TerrainCreator();
};

class FormatConverter
{
public:
    static void initializeClass(ExtensionManager* ext);

    FormatConverter();
    virtual ~FormatConverter();
};

class InertiaCalculator
{
public:
    static void initializeClass(ExtensionManager* ext);

    InertiaCalculator();
    virtual ~InertiaCalculator();
};

}

#endif // CNOID_BODY_CREATOR_PLUGIN_BODY_CREATOR_H