/**
    @author Kenta Suzuki
*/

#include "QRReader.h"
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/Image>
#include <cnoid/ImageView>
#include <cnoid/ImageableItem>
#include <cnoid/MessageView>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include "QRDecoder.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class QRReader::Impl
{
public:

    Impl();
    ~Impl();

    void onDecodeButtonClicked();

    ImageViewBar* imageViewBar;
    string decoded_image_file;

    QRDecoder* decoder;
};

}


void QRReader::initializeClass(ExtensionManager* ext)
{
    static QRReader* reader = nullptr;

    if(!reader) {
        reader = ext->manage(new QRReader);
    }
}


QRReader::QRReader()
{
    impl = new Impl;
}


QRReader::Impl::Impl()
    : imageViewBar(ImageViewBar::instance()),
      decoded_image_file("decoded_image.png")
{
    decoder = new QRDecoder;
    decoder->sigDecoded().connect([&](std::string text){
        MessageView::instance()->putln(formatR(_("\"{0}\" has been read."), text)); });

    auto button = imageViewBar->addButton(":/ZBarPlugin/icon/qr-code-scanner.svg");
    button->setToolTip(_("Read QR"));
    button->sigClicked().connect([&](){ onDecodeButtonClicked(); });
}


QRReader::~QRReader()
{
    delete impl;
}


QRReader::Impl::~Impl()
{
    filesystem::path path(fromUTF8(decoded_image_file));
    if(filesystem::exists(path)) {
        filesystem::remove(path);
    }
}


void QRReader::Impl::onDecodeButtonClicked()
{
    auto imageableItem = imageViewBar->getSelectedImageableItem();
    if(imageableItem) {
        const Image* image = imageableItem->getImage();
        image->save(decoded_image_file);
        decoder->decode(decoded_image_file);
    }
}