/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

package org.vrjuggler.vrjconfig.commoneditors.devicegraph;

import java.util.HashMap;
import java.util.Map;
import org.jgraph.graph.DefaultCellViewFactory;
import org.jgraph.graph.DefaultGraphCell;
import org.jgraph.graph.VertexView;

import org.vrjuggler.jccl.config.ConfigDefinition;
import org.vrjuggler.jccl.config.ConfigElement;


public class ProxiedDeviceCellViewFactory
   extends DefaultCellViewFactory
{
   public ProxiedDeviceCellViewFactory()
   {
   }

   public ProxiedDeviceCellViewFactory(Map creatorMap)
   {
      mCreatorMap = creatorMap;
   }

   public void registerCreator(ConfigDefinition def, Class creator)
   {
      mCreatorMap.put(def, creator);
   }

   protected VertexView createVertexView(Object cell)
   {
      VertexView view = null;

      if ( cell instanceof DefaultGraphCell &&
           ((DefaultGraphCell) cell).getUserObject() instanceof ConfigElementHolder )
      {
         ConfigElementHolder holder =
            (ConfigElementHolder) ((DefaultGraphCell) cell).getUserObject();

         ConfigElement elt    = holder.getElement();
         ConfigDefinition def = elt.getDefinition();
         Class view_class     = (Class) mCreatorMap.get(def);

         if ( null != view_class )
         {
            try
            {
               view = (VertexView) view_class.newInstance();
               view.setCell(cell);
            }
            // If we catch an exception trying to instantiate the custom
            // VertexView type, ignore it and leave view set to null.
            catch (Exception e)
            {
               e.printStackTrace();
            }
         }
      }

      if ( null == view )
      {
         view = super.createVertexView(cell);
      }

      return view;
   }

   private Map mCreatorMap = new HashMap();
}
