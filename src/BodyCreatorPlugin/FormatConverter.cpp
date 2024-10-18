/**
   @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/BodyItem>
#include <cnoid/Buttons>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/ItemManager>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/UTF8>
#include <cnoid/WorldItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/HamburgerMenu>
#include <QBoxLayout>
#include <QFile>
#include <QLabel>
#include <QTextStream>
#include "BodyCreatorDialog.h"
#include "FileDroppableWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

struct KeyInfo {
    QString oldKey;
    QString newKey;
};

KeyInfo keyInfo[] = {
    // for Header
    { "formatVersion: 1.0", "format_version: 2.0"    },
    { "angleUnit:",          "angle_unit:"           },
    { "rootLink:",           "root_link:"            },

    // for Link node
    { "jointId:",            "joint_id:"             },
    { "jointType:",          "joint_type:"           },
    { "jointAxis:",          "joint_axis:"           },
    { "jointAngle:",         "joint_angle:"          },
    { "jointDisplacement:",  "joint_displacement:"   },
    { "jointRange:",         "joint_range:"          },
    { "maxJointVelocity:",   "max_joint_velocity:"   },
    { "jointVelocityRange:", "joint_velocity_range:" },
    { "rotorInertia:",       "rotor_inertia:"        },
    { "gearRatio:",          "gear_ratio:"           },
    { "centerOfMass:",       "center_of_mass:"       },

    // for Extruction node
    { "crossSection:",       "cross_section:"        },
    { "creaseAngle:",        "crease_angle:"         },
    { "beginCap:",           "begin_cap:"            },
    { "endCap:",             "end_cap:"              },

    // for ElevationGrid node
    { "xDimension:",         "x_dimension:"          },
    { "zDimension:",         "z_dimension:"          },
    { "xSpacing:",           "x_spacing:"            },
    { "zSpacing:",           "z_spacing:"            },
    { "creaseAngle:",        "crease_angle:"         },

    // for IndexedFaceSet node
    { "texCoords:",          "tex_coords:"           },
    { "texCoordIndices:",    "tex_coord_indices:"    },
    { "creaseAngle:",        "crease_angle:"         },

    // for Appearance node
    { "textureTransform:",   "texture_transform:"    },

    // for Material node
    { "ambientIntensity:",   "ambient:"              },
    { "diffuseColor:",       "diffuse:"              },
    { "emissiveColor:",      "emissive:"             },
    { "specularColor:",      "specular:"             },

    // for Texture node
    { "repeatS:",            "repeat_s:"             },
    { "repeatT:",            "repeat_t:"             },

    // for AccelerationSensor node
    { "maxAcceleration:",    "max_acceleration:"     },

    // for RateGyroSensor node
    { "maxAngularVelocity:", "max_angular_velocity:" },

    // for Imu node
    { "maxAcceleration:",    "max_acceleration:"     },
    { "maxAngularVelocity:", "max_angular_velocity:" },

    // for ForceSensor node
    { "maxForce:",           "max_force:"            },
    { "maxTorque:",          "max_torque:"           },

    // for Camera node
    { "fieldOfView:",        "field_of_view:"        },
    { "nearClipDistance:",   "near_clip_distance:"   },
    { "farClipDistance:",    "far_clip_distance:"    },
    { "frameRate:",          "frame_rate:"           },

    // for RangeSensor node
    { "yawRange:",           "yaw_range:"            },
    { "yawStep:",            "yaw_step:"             },
    { "pitchRange:",         "pitch_range:"          },
    { "pitchStep:",          "pitch_step:"           },
    { "scanRate:",           "scan_rate:"            },
    { "minDistance:",        "min_distance:"         },
    { "maxDistance:",        "max_distance:"         },

    // for SpotLight node
    { "beamWidth:",          "beam_width:"           },
    { "cutOffAngle:",        "cut_off_angle:"        },
    { "cutOffExponent:",     "cut_off_exponent:"     },

    // for ExtraJoint node
    { "link1Name:",          "link1_name:"           },
    { "link2Name:",          "link2_name:"           },
    { "link1LocalPos:",      "link1_local_pos:"      },
    { "link2LocalPos:",      "link2_local_pos:"      },
    { "jointType:",          "joint_type:"           }
};

class ConverterWidget : public QWidget
{
public:
    ConverterWidget(QWidget* parent = nullptr);

private:
    void onFileDropped(const string& filename);
    QString convert(const QString& line) const;
    void saveFile(const QString& fileName);

    QCheckBox* convertCheckBox;
    QComboBox* formatComboBox;
};

}


void FormatConverter::initializeClass(ExtensionManager* ext)
{
    static ConverterWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new ConverterWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("Format Converter"), widget);
    }
}


FormatConverter::FormatConverter()
{

}


FormatConverter::~FormatConverter()
{

}


ConverterWidget::ConverterWidget(QWidget* parent)
    : QWidget(parent)
{
    convertCheckBox = new QCheckBox;
    convertCheckBox->setText(_("Format conversion"));

    formatComboBox = new QComboBox;
    formatComboBox->addItems(QStringList() << _("1.0") << _("2.0"));
    formatComboBox->setCurrentIndex(1);

    auto dropWidget = new FileDroppableWidget;
    dropWidget->setFixedHeight(200);

    auto vbox = new QVBoxLayout;
    auto label = new QLabel(_("Drop body(*.body) here."));
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);
    dropWidget->setLayout(vbox);
    dropWidget->sigFileDropped().connect(
        [this](const string& filename){ onFileDropped(filename); });

    auto layout = new QHBoxLayout;
    layout->addWidget(convertCheckBox);
    layout->addWidget(formatComboBox);
    layout->addStretch();

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(dropWidget);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setWindowTitle(_("Format Converter"));
}


void ConverterWidget::onFileDropped(const string& filename)
{
    filesystem::path path(fromUTF8(filename));
    string ext = path.extension().string();

    if(ext == ".body") {
        if(!convertCheckBox->isChecked()) {
            auto bodyItem = new BodyItem;
            bodyItem->load(filename);

            RootItem* rootItem = RootItem::instance();
            ItemList<WorldItem> worldItems = rootItem->selectedItems();
            if(!worldItems.size()) {
                rootItem->addChildItem(bodyItem);
            } else {
                worldItems[0]->addChildItem(bodyItem);
            }
        } else {
            saveFile(filename.c_str());
        }
    } else if(ext == ".cnoid") {
        ProjectManager* projectManager = ProjectManager::instance();
        if(projectManager->tryToCloseProject()) {
            projectManager->clearProject();
            projectManager->loadProject(filename);
        }
    } else {
        MessageView::instance()->putln(formatR(_("{0} is not supported."), filename));
    }
}


QString ConverterWidget::convert(const QString& line) const
{
    QString newLine(line);

    for(int i = 0; i < 58; ++i) {
        KeyInfo info = keyInfo[i];
        if(formatComboBox->currentIndex() == 0) {
            if(newLine.contains(info.newKey)) {
                newLine.replace(info.newKey, info.oldKey);
            }
        } else {
            if(newLine.contains(info.oldKey)) {
                newLine.replace(info.oldKey, info.newKey);
            }
        }
    }

    return newLine;
}


void ConverterWidget::saveFile(const QString& fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QString fileName2(fileName);
    fileName2.replace(".", "_c.");

    QFile file2(fileName2);
    if(!file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QTextStream out(&file2);
    while(!in.atEnd()) {
        QString line = in.readLine();
        out << convert(line) << "\n";
    }

    MessageView::instance()->putln(formatR(_("{0} has been generated."), fileName2.toStdString()));
}