/*
 * BrewTargetTreeModel.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
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

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <Qt>
#include <QObject>

#include "brewtarget.h"
#include "BrewTargetTreeItem.h"
#include "BrewTargetTreeModel.h"
#include "BrewTargetTreeView.h"
#include "database.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "recipe.h"
#include "yeast.h"
#include "brewnote.h"

BrewTargetTreeModel::BrewTargetTreeModel(BrewTargetTreeView *parent, TypeMasks type)
   : QAbstractItemModel(parent)
{
   // Initialize the tree structure
   int items = 0;

   rootItem = new BrewTargetTreeItem();

   if ( type & RECIPEMASK)
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::RECIPE);
      trees.insert(RECIPEMASK, items++);
      connect( &(Database::instance()), SIGNAL(newRecipeSignal(Recipe*)),this, SLOT(recipeAdded(Recipe*)));
      connect( &(Database::instance()), SIGNAL(deletedRecipeSignal(Recipe*)),this, SLOT(recipeRemoved(Recipe*)));
   }

   if ( type & EQUIPMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::EQUIPMENT);
      trees.insert(EQUIPMASK, items++);
      connect( &(Database::instance()), SIGNAL(newEquipmentSignal(Equipment*)),this, SLOT(equipmentAdded(Equipment*)));
      connect( &(Database::instance()), SIGNAL(deletedEquipmentSignal(Equipment*)),this, SLOT(equipmentRemoved(Equipment*)));
   }

   if ( type & FERMENTMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::FERMENTABLE);
      trees.insert(FERMENTMASK,items++);
      connect( &(Database::instance()), SIGNAL(newFermentableSignal(Fermentable*)),this, SLOT(fermentableAdded(Fermentable*)));
      connect( &(Database::instance()), SIGNAL(deletedFermentableSignal(Fermentable*)),this, SLOT(fermentableRemoved(Fermentable*)));
   }

   if ( type & HOPMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::HOP);
      trees.insert(HOPMASK,items++);
      connect( &(Database::instance()), SIGNAL(newHopSignal(Hop*)),this, SLOT(hopAdded(Hop*)));
      connect( &(Database::instance()), SIGNAL(deletedHopSignal(Hop*)),this, SLOT(hopRemoved(Hop*)));
   }

   if ( type & MISCMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::MISC);
      trees.insert(MISCMASK,items++);
      connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)),this, SLOT(miscAdded(Misc*)));
      connect( &(Database::instance()), SIGNAL(deletedMiscSignal(Misc*)),this, SLOT(miscRemoved(Misc*)));
   }

   if ( type & YEASTMASK )
   {
      rootItem->insertChildren(items,1,BrewTargetTreeItem::YEAST);
      trees.insert(YEASTMASK,items++);

      connect( &(Database::instance()), SIGNAL(newYeastSignal(Yeast*)),this, SLOT(yeastAdded(Yeast*)));
      connect( &(Database::instance()), SIGNAL(deletedYeastSignal(Yeast*)),this, SLOT(yeastRemoved(Yeast*)));
   }

   treeMask = type;
   parentTree = parent;
   loadTreeModel();
}

BrewTargetTreeModel::~BrewTargetTreeModel()
{
   delete rootItem;
}

BrewTargetTreeItem *BrewTargetTreeModel::getItem( const QModelIndex &index ) const
{
   if ( index.isValid())
   {
      BrewTargetTreeItem *item = static_cast<BrewTargetTreeItem*>(index.internalPointer());
      if (item)
         return item;
   }

   return rootItem;
}

int BrewTargetTreeModel::rowCount(const QModelIndex &parent) const
{
   if (! parent.isValid())
      return rootItem->childCount();
   
   BrewTargetTreeItem *pItem = getItem(parent);

   return pItem->childCount();
}

int BrewTargetTreeModel::columnCount( const QModelIndex &parent) const
{
   switch(treeMask)
   {
   case RECIPEMASK:
      return BrewTargetTreeItem::RECIPENUMCOLS;
   case EQUIPMASK:
      return BrewTargetTreeItem::EQUIPMENTNUMCOLS;
   case FERMENTMASK:
      return BrewTargetTreeItem::FERMENTABLENUMCOLS;
   case HOPMASK:
      return BrewTargetTreeItem::HOPNUMCOLS;
   case MISCMASK:
      return BrewTargetTreeItem::MISCNUMCOLS;
   case YEASTMASK:
      return BrewTargetTreeItem::YEASTNUMCOLS;
   case ALLMASK:
      return BrewTargetTreeItem::RECIPENUMCOLS;
   default:
      return 0;
   }
   // Backwards compatibility. This MUST be fixed before the code goes live.
   return BrewTargetTreeItem::RECIPENUMCOLS;
}

Qt::ItemFlags BrewTargetTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return 0;

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex BrewTargetTreeModel::index( int row, int column, const QModelIndex &parent) const
{
   BrewTargetTreeItem *pItem, *cItem;

   if ( parent.isValid() && parent.column() != 0)
      return QModelIndex();

   pItem = getItem(parent);
   cItem = pItem->child(row);

   if (cItem)
      return createIndex(row,column,cItem);
   else
      return QModelIndex();
}

QModelIndex BrewTargetTreeModel::parent(const QModelIndex &index) const
{
   BrewTargetTreeItem *pItem, *cItem;

   if (!index.isValid())
      return QModelIndex();

   cItem = getItem(index);

   if ( cItem == 0 )
      return QModelIndex();

   pItem = cItem->parent();

   if (pItem == rootItem || pItem == 0 )
      return QModelIndex();

   return createIndex(pItem->childNumber(),0,pItem);
}

QModelIndex BrewTargetTreeModel::getFirst(int type)
{
   QModelIndex parent;
   BrewTargetTreeItem* pItem; 
   TypeMasks index;

   switch(type) 
   {
      case BrewTargetTreeItem::RECIPE:
         index = RECIPEMASK;
         break;
      case BrewTargetTreeItem::EQUIPMENT:
         index = EQUIPMASK;
         break;
      case BrewTargetTreeItem::FERMENTABLE:
         index = FERMENTMASK;
         break;
      case BrewTargetTreeItem::HOP:
         index = HOPMASK;
         break;
      case BrewTargetTreeItem::MISC:
         index = MISCMASK;
         break;
      case BrewTargetTreeItem::YEAST:
         index = YEASTMASK;
         break;
      default:
         index  = treeMask;
   }
   pItem = rootItem->child(trees.value(index));
   if ( pItem->childCount() > 0 )
      return createIndex(0,0,pItem->child(0));

   return QModelIndex();
}

QVariant BrewTargetTreeModel::data(const QModelIndex &index, int role) const
{
   int maxColumns;

   switch(treeMask)
   {
   case RECIPEMASK:
      maxColumns = BrewTargetTreeItem::RECIPENUMCOLS;
      break;
   case EQUIPMASK:
      maxColumns = BrewTargetTreeItem::EQUIPMENTNUMCOLS;
      break;
   case FERMENTMASK:
      maxColumns = BrewTargetTreeItem::FERMENTABLENUMCOLS;
      break;
   case HOPMASK:
      maxColumns = BrewTargetTreeItem::HOPNUMCOLS;
      break;
   case MISCMASK:
      maxColumns = BrewTargetTreeItem::MISCNUMCOLS;
      break;
   case YEASTMASK:
      maxColumns = BrewTargetTreeItem::YEASTNUMCOLS;
      break;
   default:
      // Backwards compatibility. This MUST be fixed prior to releasing the code
      maxColumns = BrewTargetTreeItem::RECIPENUMCOLS;
   }

   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns)
      return QVariant();

   if ( role != Qt::DisplayRole && role != Qt::EditRole)
      return QVariant();

   BrewTargetTreeItem *item = getItem(index);
   return item->getData(index.column());
}

// This is much better, assuming the rest can be made to work
QVariant BrewTargetTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
      return QVariant();

   switch(treeMask)
   {
   case RECIPEMASK:
      return getRecipeHeader(section);
   case EQUIPMASK:
      return getEquipmentHeader(section);
   case FERMENTMASK:
      return getFermentableHeader(section);
   case HOPMASK:
      return getHopHeader(section);
   case MISCMASK:
      return getMiscHeader(section);
   case YEASTMASK:
      return getYeastHeader(section);
   default: // This needs to be fixed later
      return getRecipeHeader(section);

   }
}

QVariant BrewTargetTreeModel::getRecipeHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::RECIPENAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::RECIPEBREWDATECOL:
      return QVariant(tr("Brew Date"));
   case BrewTargetTreeItem::RECIPESTYLECOL:
      return QVariant(tr("Style"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getRecipeHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getEquipmentHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::EQUIPMENTNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::EQUIPMENTBOILTIMECOL:
      return QVariant(tr("Boil Time"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getEquipmentHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getFermentableHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::FERMENTABLENAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::FERMENTABLECOLORCOL:
      return QVariant(tr("Color"));
   case BrewTargetTreeItem::FERMENTABLETYPECOL:
      return QVariant(tr("Type"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getFermentableHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getHopHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::HOPNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::HOPFORMCOL:
      return QVariant(tr("Type"));
   case BrewTargetTreeItem::HOPUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getHopHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getMiscHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::MISCNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::MISCTYPECOL:
      return QVariant(tr("Type"));
   case BrewTargetTreeItem::MISCUSECOL:
      return QVariant(tr("Use"));
   }

   Brewtarget::log(Brewtarget::WARNING, QString("BrewTargetTreeModel::getMiscHeader Bad column: %1").arg(section));
   return QVariant();
}

QVariant BrewTargetTreeModel::getYeastHeader(int section) const
{
   switch(section)
   {
   case BrewTargetTreeItem::YEASTNAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::YEASTTYPECOL:
      return QVariant(tr("Type"));
   case BrewTargetTreeItem::YEASTFORMCOL:
      return QVariant(tr("Form"));
   }

   Brewtarget::logW( QString("BrewTargetTreeModel::getYeastHeader Bad column: %1").arg(section) );
   return QVariant();
}

bool BrewTargetTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
   if ( ! parent.isValid() )
      return false;

   BrewTargetTreeItem *pItem = getItem(parent);
   int type = pItem->getType();
   int gpType;

   if ( treeMask == ALLMASK )
   {
      BrewTargetTreeItem *gpItem = pItem->parent();
      gpType = gpItem->getType();
   }
   else
      gpType = type;

   // This is somehwat tricky. Basically, if the current item and its grandparent are both
   // recipes, then this must be a brewnote. This makes some sense (not much, but some)
   // when doing an ALLMASK tree. I am not certain it makes sense for the new trees.
   if (gpType == BrewTargetTreeItem::RECIPE && type == BrewTargetTreeItem::RECIPE)
      type = BrewTargetTreeItem::BREWNOTE;

   bool success = true;

   beginInsertRows(parent, row, row + count - 1);
   success = pItem->insertChildren(row, count, type);
   endInsertRows();

   return success;
}

bool BrewTargetTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   BrewTargetTreeItem *pItem = getItem(parent);
   bool success = true;
    
   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

QModelIndex BrewTargetTreeModel::findRecipe(Recipe* rec)
{
   BrewTargetTreeItem* pItem = rootItem->child(trees.value(RECIPEMASK));
   int i;

   if (! rec || (treeMask & RECIPEMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getRecipe() == rec )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findEquipment(Equipment* kit)
{
   BrewTargetTreeItem* pItem = rootItem->child(trees.value(EQUIPMASK));
   int i;

   if (! kit || (treeMask & EQUIPMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getEquipment() == kit )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findFermentable(Fermentable* ferm)
{
   BrewTargetTreeItem* pItem = rootItem->child(trees.value(EQUIPMASK));
   int i;

   if (! ferm || (treeMask & FERMENTMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getFermentable() == ferm )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findHop(Hop* hop)
{
   BrewTargetTreeItem* pItem = rootItem->child(trees.value(HOPMASK));
   int i;

   if (! hop || (treeMask & HOPMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getHop() == hop )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findMisc(Misc* misc)
{
   BrewTargetTreeItem* pItem = rootItem->child(trees.value(MISCMASK));
   int i;

   if (! misc || (treeMask & MISCMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getMisc() == misc )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findYeast(Yeast* yeast)
{
   BrewTargetTreeItem* pItem = rootItem->child(trees.value(YEASTMASK));
   int i;

   if (! yeast || (treeMask & YEASTMASK) == 0)
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getYeast() == yeast )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

/* Important lesson here.  When building the index, the pointer needs to be to
 * the child's parent item, as understood by the model.  Not the pointer to
 * the actual object (e.g., the BrewNote) or the recipe, or the brewnote's
 * place in the recipe.
 */
QModelIndex BrewTargetTreeModel::findBrewNote(BrewNote* bNote)
{
   // Get the brewnote's parent
   Recipe *parent = Database::instance().getParentRecipe(bNote);
   // Find that recipe in the list
   QModelIndex pInd = findRecipe(parent);
   // and get the associated treeItem
   BrewTargetTreeItem* pItem = getItem(pInd);

   if (! bNote )
      return QModelIndex();

   QList<BrewNote*> notes = parent->brewNotes();
   int i = notes.indexOf(bNote);
   if( i > 0 )
      return createIndex(i,0,pItem->child(i));
   else
      return QModelIndex();
}

/*
void BrewTargetTreeModel::addObserved( BeerXMLElement* element )
{
   connect( element, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
}

void BrewTargetTreeModel::removeObserved( BeerXMLElement* element )
{
   disconnect( element, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
}
*/

// Under construction -- I haven't figured out quite how to pay attention
// to the display and/or delete flag. The prototype here likely won't work, since there is now
// a difference between the lists in the database and the lists being displayed. This will almost
// always end up hurting me. Maybe I should do the filtering via the filterproxymodel instead?
// I will use this until I find that it does sigsegv.
void BrewTargetTreeModel::loadTreeModel(QString propName)
{
   int i;
   int rows;
   int inserted;
   BrewTargetTreeItem* temp;

   bool loadAll = (propName == "");
  
   if ( (treeMask & RECIPEMASK ) && 
        (loadAll || propName == "recipes" ) )
   {  
      BrewTargetTreeItem* local = rootItem->child(trees.value(RECIPEMASK));
      QList<Recipe*> recipes = Database::instance().recipes();
      
      rows = recipes.size();
      inserted = 0;

       // And set the data
      for( i = 0; i < rows; ++i )
      {
         insertRows(inserted,1, createIndex(trees.value(RECIPEMASK),0,local));

         temp = local->child(inserted++);
         Recipe* foo = recipes[i];

         // Watch the recipe for updates.

         temp->setData(BrewTargetTreeItem::RECIPE, foo);

         // If we have brewnotes, set them up here.
         // Ouch.  My head hurts.
         QList<BrewNote*> notes = foo->brewNotes();
         if ( notes.size() > 0 )
         {
            int bn_ins = 0;

            for (int j=0; j < notes.size(); ++j)
            {
               if ( notes[j]->display() && ! notes[j]->deleted() )
               {
                  insertRows(bn_ins, 1, createIndex(inserted,0,temp));
                  BrewTargetTreeItem* bar = temp->child(bn_ins++);
                  bar->setData(static_cast<int>(BrewTargetTreeItem::BREWNOTE), notes[j]);
               }
            }
         }
      }
   }

   if ( (treeMask & EQUIPMASK) &&
      (loadAll || propName == "equipments") )
   {
      BrewTargetTreeItem* local = rootItem->child(trees.value(EQUIPMASK));
      QList<Equipment*> equipments = Database::instance().equipments();

      rows = equipments.size();
      inserted = 0;
      for( i = 0; i < rows; ++i )
      {
          insertRows(inserted, 1, createIndex(trees.value(EQUIPMASK),0,local));
          temp = local->child(inserted++);
          temp->setData(BrewTargetTreeItem::EQUIPMENT, equipments[i]);
      }
   }

   if ( (treeMask & FERMENTMASK) &&
      (loadAll || propName == "fermentables") )
   {
      BrewTargetTreeItem* local = rootItem->child(trees.value(FERMENTMASK));
      QList<Fermentable*> ferms = Database::instance().fermentables();

      rows = ferms.size();
      inserted = 0;

      for( i = 0; i < rows; ++i )
      {
         insertRows(inserted,1,createIndex(trees.value(FERMENTMASK),0,local));
         temp = local->child(inserted++);
         temp->setData(BrewTargetTreeItem::FERMENTABLE, ferms[i]);
      }
   }

   if ( (treeMask & HOPMASK) &&
      (loadAll || propName == "hops") )
   {
      BrewTargetTreeItem* local = rootItem->child(trees.value(HOPMASK));
      QList<Hop*> hops = Database::instance().hops();

      rows = hops.size();
      inserted = 0;

      for( i = 0; i < rows; ++i )
      {
         insertRows(inserted,1,createIndex(trees.value(HOPMASK),0,local));
         temp = local->child(inserted++);
         temp->setData(BrewTargetTreeItem::HOP, hops[i]);
      }
   }

   if ( (treeMask & MISCMASK) &&
      (loadAll || propName == "miscs") )
   {
      BrewTargetTreeItem* local = rootItem->child(trees.value(MISCMASK));
      QList<Misc*> miscs = Database::instance().miscs();

      rows = miscs.size();
      inserted = 0;

      for( i = 0; i < rows; ++i )
      {
         insertRows(inserted,1,createIndex(trees.value(MISCMASK),0,local));
         temp = local->child(inserted++);
         temp->setData(BrewTargetTreeItem::MISC, miscs[i]);
      }
   }

   if ( (treeMask & YEASTMASK) &&
      (loadAll || propName == "yeasts") )
   {
      BrewTargetTreeItem* local = rootItem->child(trees.value(YEASTMASK));
      QList<Yeast*> yeasts = Database::instance().yeasts();

      rows = yeasts.size();
      inserted = 0;

      for( i = 0; i < rows; ++i )
      {
        insertRows(inserted,1,createIndex(trees.value(YEASTMASK),0,local));
        temp = local->child(inserted++);
        temp->setData(BrewTargetTreeItem::YEAST, yeasts[i]);
      }
   }
}

void BrewTargetTreeModel::unobserve( QList<Recipe*>& objects )
{
   int i, size;
   size = objects.size();
   for( i = 0; i < size; ++i )
      qDebug() << "unobserved";
      // removeObserved(objects[i]);
}

void BrewTargetTreeModel::unloadTreeModel(QString propName)
{
   int breadth;
   QModelIndex parent;

   bool unloadAll = (propName=="");
   
   if ( (treeMask & RECIPEMASK) &&
        (unloadAll || propName == "recipes"))
   {
      unobserve(recipes);
      recipes.clear();
      
      parent = createIndex(BrewTargetTreeItem::RECIPE,0,rootItem->child(trees.value(RECIPEMASK)));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & EQUIPMASK) &&
      (unloadAll || propName == "equipments"))
   {
      parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(trees.value(EQUIPMASK)));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & FERMENTMASK) &&
      (unloadAll || propName == "fermentables"))
   {
      parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,rootItem->child(trees.value(FERMENTMASK)));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if ((treeMask & HOPMASK) &&
      (unloadAll || propName == "hops"))
   {
      parent = createIndex(BrewTargetTreeItem::HOP,0,rootItem->child(trees.value(HOPMASK)));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & MISCMASK) &&
      (unloadAll || propName == "miscs"))
   {
      parent = createIndex(BrewTargetTreeItem::MISC,0,rootItem->child(trees.value(MISCMASK)));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if ((treeMask & YEASTMASK) &&
      (unloadAll || propName == "yeasts"))
   {
      parent = createIndex(BrewTargetTreeItem::YEAST,0,rootItem->child(trees.value(YEASTMASK)));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
}
/*
void BrewTargetTreeModel::changed(QMetaProperty prop, QVariant value)
{
   // Notifier could be the database. 
   QString propName = prop.name();

   if( sender() == &(Database::instance()) )
   {
      unloadTreeModel(prop.name());
      loadTreeModel(prop.name());
   }
   else // Otherwise, we know that one of the recipes changed.
   {
      // Do nothing if this isn't a recipe tree
      if ( ! treeMask & RECIPEMASK )
         return;

      Recipe* foo = qobject_cast<Recipe*>(sender());
      // Make sure we could cast to a recipe
      if ( foo == 0 )
         return;

      QModelIndex changed = findRecipe(foo);

      if ( ! changed.isValid() )
         return;

      BrewTargetTreeItem* temp = getItem(changed);
      temp->setData(BrewTargetTreeItem::RECIPE,foo);

      // If the tree item has children -- that is, brew notes -- remove them
      // all
      if ( temp->childCount() )
         removeRows(0,temp->childCount(),changed);
      
      QList<BrewNote*> brewNotes = foo->brewNotes();
      if ( brewNotes.size() > 0 )
      {

         // Put back how many ever rows are left
         insertRows(0,brewNotes.size(),changed);
         for (int j=0; j < brewNotes.size(); ++j)
         {
            // And populate them.
            BrewTargetTreeItem* bar = temp->child(j);
            bar->setData(BrewTargetTreeItem::BREWNOTE,brewNotes[j]);
         }
      }
   }
}
*/

Recipe* BrewTargetTreeModel::getRecipe(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getRecipe();
}

Equipment* BrewTargetTreeModel::getEquipment(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getEquipment();
}

Fermentable* BrewTargetTreeModel::getFermentable(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getFermentable();
}

Hop* BrewTargetTreeModel::getHop(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getHop();
}

Misc* BrewTargetTreeModel::getMisc(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getMisc();
}

Yeast* BrewTargetTreeModel::getYeast(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getYeast();
}

BrewNote* BrewTargetTreeModel::getBrewNote(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getBrewNote();
}

bool BrewTargetTreeModel::isRecipe(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::RECIPE;
}

bool BrewTargetTreeModel::isEquipment(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::EQUIPMENT;
}

bool BrewTargetTreeModel::isFermentable(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::FERMENTABLE;
}

bool BrewTargetTreeModel::isHop(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::HOP;
}

bool BrewTargetTreeModel::isMisc(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::MISC;
}

bool BrewTargetTreeModel::isYeast(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::YEAST;
}

bool BrewTargetTreeModel::isBrewNote(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::BREWNOTE;
}

int BrewTargetTreeModel::getType(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType();
}

int BrewTargetTreeModel::getMask()
{
   return treeMask;
}

void BrewTargetTreeModel::equipmentAdded(Equipment* victim)
{
   BrewTargetTreeItem* temp;

   BrewTargetTreeItem* local = rootItem->child(trees.value(EQUIPMASK));
   QModelIndex parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

    insertRows(breadth, 1, createIndex(trees.value(EQUIPMASK),0,local));
    temp = local->child(breadth);
    temp->setData(BrewTargetTreeItem::EQUIPMENT, victim);
}

void BrewTargetTreeModel::equipmentRemoved(Equipment* victim)
{
   QModelIndex index = findEquipment(victim);

   if ( index.isValid() )
      removeRows(index.row(), 1, createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(trees.value(EQUIPMASK))));
}

void BrewTargetTreeModel::fermentableAdded(Fermentable* victim)
{
   BrewTargetTreeItem* temp;

   BrewTargetTreeItem* local = rootItem->child(trees.value(FERMENTMASK));
   QModelIndex parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

    insertRows(breadth, 1, createIndex(trees.value(FERMENTMASK),0,local));
    temp = local->child(breadth);
    temp->setData(BrewTargetTreeItem::FERMENTABLE, victim);
}

void BrewTargetTreeModel::fermentableRemoved(Fermentable* victim)
{
   QModelIndex index = findFermentable(victim);
   QModelIndex parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,rootItem->child(trees.value(FERMENTMASK)));

   removeRows(index.row(),1,parent);
}

void BrewTargetTreeModel::hopAdded(Hop* victim)
{
   BrewTargetTreeItem* temp;

   BrewTargetTreeItem* local = rootItem->child(trees.value(HOPMASK));
   QModelIndex parent = createIndex(BrewTargetTreeItem::HOP,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

    insertRows(breadth, 1, createIndex(trees.value(HOPMASK),0,local));
    temp = local->child(breadth);
    temp->setData(BrewTargetTreeItem::HOP, victim);
}

void BrewTargetTreeModel::hopRemoved(Hop* victim)
{
   QModelIndex index = findHop(victim);
   QModelIndex parent = createIndex(BrewTargetTreeItem::HOP,0,rootItem->child(trees.value(HOPMASK)));

   removeRows(index.row(),1,parent);
}

void BrewTargetTreeModel::miscAdded(Misc* victim)
{
   BrewTargetTreeItem* temp;

   BrewTargetTreeItem* local = rootItem->child(trees.value(MISCMASK));
   QModelIndex parent = createIndex(BrewTargetTreeItem::MISC,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

    insertRows(breadth, 1, createIndex(trees.value(MISCMASK),0,local));
    temp = local->child(breadth);
    temp->setData(BrewTargetTreeItem::MISC, victim);
}

void BrewTargetTreeModel::miscRemoved(Misc* victim)
{
   QModelIndex index = findMisc(victim);
   QModelIndex parent = createIndex(BrewTargetTreeItem::MISC,0,rootItem->child(trees.value(MISCMASK)));

   removeRows(index.row(),1,parent);
}

void BrewTargetTreeModel::recipeAdded(Recipe* victim)
{
   BrewTargetTreeItem* temp;

   BrewTargetTreeItem* local = rootItem->child(trees.value(RECIPEMASK));
   QModelIndex parent = createIndex(BrewTargetTreeItem::RECIPE,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

    insertRows(breadth, 1, createIndex(trees.value(RECIPEMASK),0,local));
    temp = local->child(breadth);
    temp->setData(BrewTargetTreeItem::RECIPE, victim);
}

void BrewTargetTreeModel::recipeRemoved(Recipe* victim)
{
   QModelIndex index = findRecipe(victim);
   QModelIndex parent = createIndex(BrewTargetTreeItem::RECIPE,0,rootItem->child(trees.value(RECIPEMASK)));

   removeRows(index.row(),1,parent);
}

void BrewTargetTreeModel::yeastAdded(Yeast* victim)
{
   BrewTargetTreeItem* temp;

   BrewTargetTreeItem* local = rootItem->child(trees.value(YEASTMASK));
   QModelIndex parent = createIndex(BrewTargetTreeItem::YEAST,0,local);

   if ( ! parent.isValid() )
      return;

   int breadth = rowCount(parent);

    insertRows(breadth, 1, createIndex(trees.value(YEASTMASK),0,local));
    temp = local->child(breadth);
    temp->setData(BrewTargetTreeItem::YEAST, victim);
}

void BrewTargetTreeModel::yeastRemoved(Yeast* victim)
{
   QModelIndex index = findYeast(victim);
   QModelIndex parent = createIndex(BrewTargetTreeItem::YEAST,0,rootItem->child(trees.value(YEASTMASK)));

   removeRows(index.row(),1,parent);
}

