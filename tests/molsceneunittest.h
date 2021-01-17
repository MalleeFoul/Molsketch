/***************************************************************************
 *   Copyright (C) 2017 - 2018 by Hendrik Vennekate                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <cxxtest/TestSuite.h>
#include "utilities.h"
#include <molscene.h>
#include <molecule.h>
#include <typeindex>
#include <frame.h>
#include <arrow.h>
#include <textitem.h>
#include <QDebug>
#include <scenesettings.h>
#include <QGraphicsSceneMouseEvent>
#include <QClipboard>
#include <QMimeData>

using namespace Molsketch;

const qreal BOND_ANGLE_FROM_SETTINGS = 4.5;
const QString SCENE_XML_WITH_ATTRIBUTE("<molscene MolsceneBondAngle=\"" + QString::number(BOND_ANGLE_FROM_SETTINGS) + "\"/>");
const QString MOLECULE_XML("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                           "<molecule name=\"\">"
                           "<atomArray>"
                           "<atom id=\"a1\" elementType=\"\" userCharge=\"0\" disableHydrogens=\"0\" hydrogens=\"0\" shapeType=\"0\" hydrogenAlignment=\"0\" colorR=\"0\" colorG=\"0\" colorB=\"0\" scalingParameter=\"1\" zLevel=\"3\" coordinates=\"5,5\"/>"
                           "<atom id=\"a2\" elementType=\"\" userCharge=\"0\" disableHydrogens=\"0\" hydrogens=\"0\" shapeType=\"0\" hydrogenAlignment=\"0\" colorR=\"0\" colorG=\"0\" colorB=\"0\" scalingParameter=\"1\" zLevel=\"3\" coordinates=\"0,0\"/>"
                           "</atomArray>"
                           "<bondArray"
                           "><bond atomRefs2=\"a2 a1\" type=\"10\" colorR=\"0\" colorG=\"0\" colorB=\"0\" scalingParameter=\"1\" zLevel=\"2\" coordinates=\"0,0;5,5\"/>"
                           "</bondArray>"
                           "</molecule>\n");
const QString ALTERNATIVE_MOLECULE_XML("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                       "<molecule name=\"\">"
                                       "<atomArray>"
                                       "<atom id=\"a1\" elementType=\"\" userCharge=\"0\" disableHydrogens=\"0\" hydrogens=\"0\" shapeType=\"0\" hydrogenAlignment=\"0\" colorR=\"0\" colorG=\"0\" colorB=\"0\" scalingParameter=\"1\" zLevel=\"3\" coordinates=\"0,0\"/>"
                                       "<atom id=\"a2\" elementType=\"\" userCharge=\"0\" disableHydrogens=\"0\" hydrogens=\"0\" shapeType=\"0\" hydrogenAlignment=\"0\" colorR=\"0\" colorG=\"0\" colorB=\"0\" scalingParameter=\"1\" zLevel=\"3\" coordinates=\"5,5\"/>"
                                       "</atomArray>"
                                       "<bondArray"
                                       "><bond atomRefs2=\"a1 a2\" type=\"10\" colorR=\"0\" colorG=\"0\" colorB=\"0\" scalingParameter=\"1\" zLevel=\"2\" coordinates=\"0,0;5,5\"/>"
                                       "</bondArray>"
                                       "</molecule>\n");

struct MolSceneForTesting : public MolScene {
  XmlObjectInterface* produceChild(const QString &childName, const QXmlStreamAttributes &attributes) override {
    return MolScene::produceChild(childName, attributes);
  }
  QList<const XmlObjectInterface*> children() const {
    return MolScene::children();
  }
  void sendMousePressEvent() {
    auto event = new QGraphicsSceneMouseEvent(QGraphicsSceneMouseEvent::MouseButtonPress);
    event->setButton(Qt::LeftButton);
    mousePressEvent(event);
  }
};

class AtomForTesting : public Atom {
public:
  void sendMouseDoubleClickEvent() {
    auto event = new QGraphicsSceneMouseEvent(QGraphicsSceneMouseEvent::MouseButtonDblClick);
    event->setButton(Qt::LeftButton);
    mouseDoubleClickEvent(event);
  }
};

class MolSceneUnitTest : public CxxTest::TestSuite {
  MolSceneForTesting *scene;

  Molecule *produceMolecule() {
    auto atomA = new AtomForTesting;
    auto atomB = new Atom(QPointF(5,5));
    auto bond = new Bond(atomA, atomB);
    return new Molecule({atomA, atomB}, {bond});
  }
public:
  void setUp() {
    scene = new MolSceneForTesting;
  }

  void tearDown() {
    delete scene;
  }

  void testDefaultProduceChild() {
    QS_ASSERT_EQUALS(scene->produceChild("Somestring", QXmlStreamAttributes()), nullptr);
    TS_ASSERT(scene->items().isEmpty());
  }

  void testProduceChild() {
    struct ExpectedChild{
      QString name;
      QString type;
      std::type_index expectedType;
      const char* toString() {
        return (name + type).toStdString().c_str();
      }
    };
    ExpectedChild children[] = {
      {"frame", "", typeid(Frame)},
      {"molecule", "", typeid(Molecule)},
      {"molecule", "test", typeid(Molecule)},
      {"arrow", "", typeid(Arrow)},
      {"object", "ReactionArrow", typeid(Arrow)},
      {"object", "MechanismArrow", typeid(Arrow)},
      {"textItem", "", typeid(TextItem)},
      {"settings", "", typeid(SceneSettings)},
      };
    for (auto child : children) {
      QXmlStreamAttributes attributes;
      attributes.append("type", child.type);
      XmlObjectInterface *sceneChild = scene->produceChild(child.name, attributes);
      TSM_ASSERT(child.toString(), sceneChild);
      TSM_ASSERT_EQUALS(child.toString(), child.expectedType, std::type_index(typeid(*(sceneChild))));
      TSM_ASSERT(child.toString(), scene->children().contains(sceneChild));
      if (QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(sceneChild))
        TSM_ASSERT(child.toString(), scene->items().contains(item));
    }
  }

  void testInitializingSettingsFromAttributes() {
    QXmlStreamReader reader(SCENE_XML_WITH_ATTRIBUTE);
    reader.readNextStartElement();
    scene->readXml(reader);
    TS_ASSERT_EQUALS(scene->bondAngle(), BOND_ANGLE_FROM_SETTINGS);
  }

  void testSelectingAllItems() {
    auto arrow = new Arrow;
    auto molecule = produceMolecule();
    (new Frame)->setParentItem(molecule);
    auto frame = new Frame;
    auto textItem = new TextItem;

    QList<QGraphicsItem*> items { arrow, molecule, frame, textItem };
    for (auto item : items) scene->addItem(item);

    scene->selectAll();
    QS_ASSERT_EQUALS(scene->selectedItems().toSet(),
                     QSet<QGraphicsItem*>() << arrow << molecule << frame << textItem); // TODO improve output
  }

  void testSelectingAllItemsDoesNotSelectGrid() {
    auto originalItems = scene->items().toSet();
    scene->setGrid(true);

    scene->selectAll();
    auto gridItemSet = scene->items().toSet() - originalItems;
    QS_ASSERT_EQUALS(gridItemSet.size(), 1);
    TSM_ASSERT("Grid item should not be contained in selected items",
               (scene->selectedItems().toSet() & gridItemSet).isEmpty());
  }

  void testSelectingAllItemsDoesNotSelectInputItem() {
    auto atom = new AtomForTesting;
    auto molecule = new Molecule({atom});
    scene->addItem(molecule);
    scene->addItem(atom);

    auto originalItems = scene->items().toSet();
    atom->sendMouseDoubleClickEvent();

    scene->selectAll();
    auto inputItemSet = scene->items().toSet() - originalItems;
    TS_ASSERT_EQUALS(inputItemSet.size(), 1);
    TSM_ASSERT("Input item should not be contained in selected items",
               (scene->selectedItems().toSet() & inputItemSet).isEmpty());
  }

  void testSelectingAllItemsDoesNotSelectSelectionRectangle() {
    auto originalItems = scene->items().toSet();
    scene->sendMousePressEvent();

    scene->selectAll();
    auto selectionRectangleSet = scene->items().toSet() - originalItems;
    TS_ASSERT_EQUALS(selectionRectangleSet.size(), 1);
    TSM_ASSERT("Selection rectangle should not be contained in selected items",
               (scene->selectedItems().toSet() & selectionRectangleSet).isEmpty());
  }

  void testCopyingMolecules() {
    scene->addItem(produceMolecule());
    scene->selectAll();
    scene->copy();
    auto mimeData = QApplication::clipboard()->mimeData();
    QS_ASSERT_EQUALS(mimeData->formats().toSet(), (QStringList() << "application/x-qt-image" << "molecule/molsketch" << "image/svg+xml").toSet());
    QS_ASSERT_EQUALS_OR_EQUALS(mimeData->data("molecule/molsketch"), MOLECULE_XML, ALTERNATIVE_MOLECULE_XML);
  }

  void testPastingMolecules() {
    auto mimeData = new QMimeData;
    mimeData->setData("molecule/molsketch", MOLECULE_XML.toUtf8());
    QApplication::clipboard()->setMimeData(mimeData);
    scene->paste();
    QS_ASSERT_EQUALS(scene->items().size(), 4);
  }
};
