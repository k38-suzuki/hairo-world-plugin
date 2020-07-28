/**
   \file
   \author Kenta Suzuki
*/

#include "MarkerPointItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <bitset>
#include "gettext.h"

using namespace std;
using namespace cnoid;

int colorIndex = 1;
bool colorRotation = true;

namespace  {

void putKeyVector3(YAMLWriter* writer, const string key, const Vector3 value)
{
    writer->putKey(key);
    writer->startFlowStyleListing();
    for(int i = 0; i < 3; i++) {
        writer->putScalar(value[i]);
    }
    writer->endListing();
}


vector<string> csplit(const string str)
{
    vector<string> items;
    QString qstr = QString::fromStdString(str);
    QStringList qstrs = qstr.split(",");
    for(QString qitem: qstrs) {
        items.push_back(qitem.toStdString());
    }
    return items;
}


bool loadCsv(MarkerPointItem* item, const string fileName)
{
    if(!fileName.empty()) {
        stdx::filesystem::path name(fileName);
        item->setName(name.stem().c_str());
        ifstream ifs(fileName.c_str());
        if(ifs) {
            int i = 0;
            while(!ifs.eof()) {
                string line;
                ifs >> line;
                if(line.empty()) {
                    break;
                }
                if(i == 0) {
                    vector<string> data = csplit(line);
                    if(data[0] != "Trajectories") {
                        return false;
                    }
                }
                else if(i == 1) {

                }
                else if(i == 2) {

                }
                else if(i == 3) {

                }
                else if(i == 4) {

                }
                else {
                    vector<string> data = csplit(line);
                    data.erase(data.begin(), data.begin() + 2);
                    int rcolorIndex = 1;
                    for(int j = 0; j < data.size(); j += 3) {
                        Vector3 record(atof(data[j].c_str()), atof(data[j + 1].c_str()), atof(data[j + 2].c_str()));

                        int index;
                        if(!colorRotation) {
                            index = colorIndex;
                        }
                        else {
                            index = rcolorIndex;
                        }
                        stringstream ss;
                        ss << bitset<3>(index);
                        string s = ss.str();
                        string colors[3] { { s[0] }, { s[1] }, { s[2] } };

                        double f = (double)j / ((double)data.size() + 3.0);
                        Vector3f color = Vector3f(atof(colors[0].c_str()), atof(colors[1].c_str()), atof(colors[2].c_str()));

                        if(!colorRotation) {
                            color += Vector3f(f, f, f);
                        }
                        for(int k = 0; k < 3; k++) {
                            if(color[k] > 1.0) {
                                color[k] = 1.0;
                            }
                            else if(color[k] < 0.0) {
                                color[k] = 0.0;
                            }
                        }

                        item->addPoint(record * 0.001, 0.03, color, 0.7);

                        rcolorIndex++;
                        if(rcolorIndex > 6) {
                            rcolorIndex = 1;
                        }
                    }
                }
                i++;
            }
        }
        ifs.close();
    }
    colorIndex++;
    if(colorIndex > 6) {
        colorIndex = 1;
    }
    return true;
}


bool loadItem(Mapping& node, MarkerPointItem* item)
{
    Listing* markerList = node.findListing("markers");
    if(markerList->isValid()) {
        for(int i = 0; i < markerList->size(); i++) {
            Mapping& info = *markerList->at(i)->toMapping();
            Vector3 point;
            if(read(info, "point", point)) {}
            Vector3 color;
            if(read(info, "color", color)) {}
            double transparency;
            if(node.read("transparency", transparency)) {}
            item->addPoint(point, 0.03, Vector3f(color[0], color[1], color[2]), transparency);
        }
    }
    return true;
}

}


namespace cnoid {

class MarkerPointItemImpl
{
public:
    MarkerPointItemImpl(MarkerPointItem* self);
    MarkerPointItemImpl(MarkerPointItem* self, const MarkerPointItemImpl& org);

    MarkerPointItem* self;
    SgGroupPtr scene;

    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
    SgGroupPtr points() { return scene; }
    void addPoint(const Vector3 point, const double radius, const Vector3f color, const double transparency);
};

}


MarkerPointItem::MarkerPointItem()
{
    impl = new MarkerPointItemImpl(this);
}


MarkerPointItemImpl::MarkerPointItemImpl(MarkerPointItem* self)
    : self(self)
{   
    scene = new SgGroup();
}


MarkerPointItem::MarkerPointItem(const MarkerPointItem& org)
    : Item(org),
      impl(new MarkerPointItemImpl(this, *org.impl))

{

}


MarkerPointItemImpl::MarkerPointItemImpl(MarkerPointItem* self, const MarkerPointItemImpl& org)
    : self(self)
{
    scene = new SgGroup(*org.scene);
}


MarkerPointItem::~MarkerPointItem()
{
    delete impl;
}


SgNode* MarkerPointItem::getScene()
{
    return impl->scene;
}


void MarkerPointItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<MarkerPointItem>(N_("MarkerPointItem"));

    im.addLoaderAndSaver<MarkerPointItem>(
        _("Marker Point"), "MARKER-POINT-FILE", "yaml;yml;csv",
        [](MarkerPointItem* item, const std::string& filename, std::ostream& os, Item*){ return load(item, filename); },
        [](MarkerPointItem* item, const std::string& filename, std::ostream& os, Item*){ return save(item, filename); },
        ItemManager::PRIORITY_CONVERSION);
}


void MarkerPointItem::addPoint(const Vector3 point, const double radius, const Vector3f color, const double transparency)
{
    impl->addPoint(point, radius, color, transparency);
}


void MarkerPointItemImpl::addPoint(const Vector3 point, const double radius, const Vector3f color, const double transparency)
{
    MeshGenerator generator;
    SgShape* shape = new SgShape();
    shape->setMesh(generator.generateSphere(0.03));
    SgMaterial* material = new SgMaterial();
    material->setDiffuseColor(color);
    material->setTransparency(transparency);
    shape->setMaterial(material);
    SgPosTransform* transform = new SgPosTransform();
    transform->addChild(shape);
    transform->setTranslation(point);
    scene->addChild(transform, true);
}


bool MarkerPointItem::load(MarkerPointItem* item, const string fileName)
{
    stdx::filesystem::path name(fileName);
    string extension = name.extension().c_str();
    if((extension == ".yaml") || (extension == ".yml")) {
        YAMLReader reader;
        if(reader.load(fileName)) {
            Mapping* topNode = reader.loadDocument(fileName)->toMapping();
            loadItem(*topNode, item);
        }
    }
    else if(extension == ".csv") {
        loadCsv(item, fileName);
    }
    return true;
}


bool MarkerPointItem::save(MarkerPointItem* item, const string fileName)
{

    if(!fileName.empty()) {
        YAMLWriter writer(fileName);

        writer.startMapping();
        writer.putKey("markers");
        writer.startListing();

        SgGroupPtr scene = dynamic_cast<SgGroup*>(item->getScene());
        int numChildren = scene->numChildren();
        for(size_t i = 0; i < numChildren; i++) {
            SgPosTransform* pos = dynamic_cast<SgPosTransform*>(scene->child(i));
            SgShape* shape = dynamic_cast<SgShape*>(pos->child(0));
            SgMaterial* material = shape->material();
            writer.startMapping();
            putKeyVector3(&writer, "point", pos->translation());
            Vector3f color = material->diffuseColor();
            putKeyVector3(&writer, "color", Vector3(color[0], color[1], color[2]));
            writer.putKeyValue("transparency", material->transparency());
            writer.endMapping();
        }

        writer.endListing();
        writer.endMapping();
    }
    return true;
}


Item* MarkerPointItem::doDuplicate() const
{
    return new MarkerPointItem(*this);
}


void MarkerPointItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void MarkerPointItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{

}


bool MarkerPointItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool MarkerPointItemImpl::store(Archive& archive)
{
    int numChildren = scene->numChildren();
    archive.write("children", numChildren);
    for(size_t i = 0; i < numChildren; i++) {
        SgPosTransform* pos = dynamic_cast<SgPosTransform*>(scene->child(i));
        SgShape* shape = dynamic_cast<SgShape*>(pos->child(0));
        SgMaterial* material = shape->material();
        string name = "point" + to_string(i);
        write(archive, name, pos->translation());
        name = "color" + to_string(i);
        write(archive, name, material->diffuseColor());
        name = "transparency" + to_string(i);
        archive.write(name, material->transparency());
    }
    return true;
}


bool MarkerPointItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool MarkerPointItemImpl::restore(const Archive& archive)
{
    int numChildren;
    archive.read("children", numChildren);
    for(size_t i = 0; i < numChildren; i++) {
        string name = "point" + to_string(i);
        Vector3 point;
        read(archive, name, point);
        name = "color" + to_string(i);
        Vector3f color;
        read(archive, name, color);
        name = "transparency" + to_string(i);
        double transparency;
        archive.read(name, transparency);
        addPoint(point, 0.03, color, transparency);
    }
    return true;
}
