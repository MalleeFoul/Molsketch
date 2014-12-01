/***************************************************************************
 *   Copyright (C) 2009 Tim Vandermeersch                                  *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SMILESITEM_H
#define SMILESITEM_H

#ifdef QMAKEBUILD
#include <itemplugin.h>
#else
#include <molsketch/itemplugin.h>
#endif

namespace Molsketch {

  class Molecule;

  class SmilesItem : public ItemPlugin
  {
    public:
      SmilesItem();
      virtual ~SmilesItem();

      QString input() const { return "Molecule"; }
      QString output() const { return "SMILES"; }

      QRectF boundingRect() const;
      void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

      void dropEvent(QGraphicsSceneDragDropEvent *event);

   private:
      Molecule *m_molecule;
      QRectF m_rect;
  };

  class SmilesItemFactory : public ItemPluginFactory
  {
    public:
      SmilesItemFactory();
      QString input() const { return "Molecule"; }
      QString output() const { return "SMILES"; }
      ItemPlugin* createInstance() const { return new SmilesItem; }
  };

}

#endif // SMILESITEM_H
