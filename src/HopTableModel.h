/*
 * HopTableModel.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _HOPTABLEMODEL_H
#define   _HOPTABLEMODEL_H

class HopTableModel;
class HopItemDelegate;

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QItemDelegate>
#include <QVector>
#include "hop.h"
#include "HopTableWidget.h"
#include "recipe.h"

enum{HOPNAMECOL, HOPALPHACOL, HOPAMOUNTCOL, HOPFORMCOL, HOPUSECOL, HOPTIMECOL, HOPNUMCOLS /*This one MUST be last*/};

class HopTableModel : public QAbstractTableModel
{
   Q_OBJECT
           
public:
   HopTableModel(HopTableWidget* parent=0);
   virtual ~HopTableModel();
   //! Observe a recipe's list of fermentables.
   void observeRecipe(Recipe* rec);
   //! Whether or not we should be looking at the database.
   void observeDatabase(bool val);
   void setShowIBUs( bool var ); // If you want to show IBUs.
   void addHop(Hop* hop);
   //! Watch all the \b ferms for changes.
   void addHops(QList<Hop*> hops);
   Hop* getHop(unsigned int i);
   //! \returns true if "hop" is successfully found and removed.
   bool removeHop(Hop* hop);
   void removeAll();
   
   //! Reimplemented from QAbstractTableModel.
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! Reimplemented from QAbstractTableModel.
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   //! Reimplemented from QAbstractTableModel.
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   //! Reimplemented from QAbstractTableModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   //! Reimplemented from QAbstractTableModel.
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   //! Reimplemented from QAbstractTableModel.
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
   
public slots:
   void changed(QMetaProperty, QVariant);
   
private:
   QList<Hop*> hopObs;
   Recipe* recObs;
   HopTableWidget* parentTableWidget;
   bool showIBUs; // True if you want to show the IBU contributions in the table rows.
};

class HopItemDelegate : public QItemDelegate
{
   Q_OBJECT
           
public:
   HopItemDelegate(QObject* parent = 0);
   
   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   
private:
};

#endif   /* _HOPTABLEMODEL_H */

