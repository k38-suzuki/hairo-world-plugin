/**
   @author Kenta Suzuki
*/

#include "BodyConverter.h"
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <QAction>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace {

BodyConverter* instance_ = nullptr;

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

}

namespace cnoid {

class ConvertDialog : public Dialog
{
public:
    ConvertDialog(QWidget* parent = nullptr);

private:
    void open();
    void save();

    QString convert(const QString& line) const;
    void saveFile(const QString& fileName);

    void createMenu();
    void createFormGroupBox();

    QMenuBar* menuBar;
    QGroupBox* formGroupBox;
    ComboBox* formatCombo;
    QString bodyFileName;

    QMenu* fileMenu;
    QAction* openAct;
    QAction* saveAct;
    QAction* exitAct;
};


class BodyConverterImpl
{
public:
    BodyConverterImpl(BodyConverter* self);
    BodyConverter* self;

    void show();
};

}


BodyConverter::BodyConverter()
{
    impl = new BodyConverterImpl(this);
}


BodyConverterImpl::BodyConverterImpl(BodyConverter* self)
    : self(self)
{

}


BodyConverter::~BodyConverter()
{
    delete impl;
}


void BodyConverter::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new BodyConverter);

        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("Make Body File"));
        mm.addItem(_("Convert Body"))->sigTriggered().connect(
                    [&](){ instance_->impl->show(); });
    }
}


void BodyConverterImpl::show()
{
    ConvertDialog dialog(MainWindow::instance());

    if(dialog.exec()) {

    }
}


ConvertDialog::ConvertDialog(QWidget* parent)
    : Dialog(parent)
{
    createMenu();
    createFormGroupBox();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(formGroupBox);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(_("Body Converter"));
}


void ConvertDialog::open()
{
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Open a Body file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("Body files (*.body)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    if(dialog.exec()) {
        QString fileName = dialog.selectedFiles().front();
        bodyFileName = fileName;
    }
}


void ConvertDialog::save()
{
    if(!bodyFileName.isEmpty()) {
        saveFile(bodyFileName);
    }
}


QString ConvertDialog::convert(const QString& line) const
{
    QString newLine(line);

    for(int i = 0; i < 58; ++i) {
        KeyInfo info = keyInfo[i];
        if(formatCombo->currentIndex() == 0) {
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


void ConvertDialog::saveFile(const QString& fileName)
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

    QString status = QString("%1 has been generated.")
                    .arg(fileName2);

    QMessageBox msgBox;
    msgBox.setText(status);
    msgBox.exec();
}


void ConvertDialog::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu(_("&File"), this);
    openAct = fileMenu->addAction(_("&Open"));
    saveAct = fileMenu->addAction(_("&Save"));
    exitAct = fileMenu->addAction(_("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(openAct, &QAction::triggered, [&](){ open(); });
    connect(saveAct, &QAction::triggered, [&](){ save(); });
    connect(exitAct, &QAction::triggered, [&](){ accept(); });
}


void ConvertDialog::createFormGroupBox()
{
    formatCombo = new ComboBox;
    formatCombo->addItems(QStringList() << _("1.0") << _("2.0"));
    formatCombo->setCurrentIndex(1);

    formGroupBox = new QGroupBox(_("Convert to"));
    QFormLayout* layout = new QFormLayout;
    layout->addRow(_("Format:"), formatCombo);
    formGroupBox->setLayout(layout);
}
