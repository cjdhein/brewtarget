/*
 * RecipeExtrasWidgetPlugin.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2011.
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

#ifndef _RECIPEEXTRASWIDGETPLUGIN_H
#define	_RECIPEEXTRASWIDGETPLUGIN_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QIcon>
#include <QDesignerCustomWidgetInterface>

class RecipeExtrasWidgetPlugin;

class RecipeExtrasWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
           
public:
   RecipeExtrasWidgetPlugin(QObject* parent=0);
   
   bool isContainer() const;
   bool isInitialized() const;
   QIcon icon() const;
   QString domXml() const;
   QString group() const;
   QString includeFile() const;
   QString name() const;
   QString toolTip() const;
   QString whatsThis() const;
   QWidget *createWidget(QWidget *parent);
   void initialize(QDesignerFormEditorInterface *core);

private:
   bool initialized;
};

#endif	/* _RECIPEEXTRASWIDGETPLUGIN_H */

