/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VRJ.NET is (C) Copyright 2004 by Patrick Hartling
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

#ifndef _VRJ_NET_GMTL_LINE_SEG_OPS_H_
#define _VRJ_NET_GMTL_LINE_SEG_OPS_H_

#include <gmtl/LineSeg.h>
#include <gmtl/LineSegOps.h>


namespace gmtl
{
   template float distanceSquared(const gmtl::LineSegf&, const gmtl::Point3f&);
   template double distanceSquared(const gmtl::LineSegd&, const gmtl::Point3d&);

   template gmtl::Point3f findNearestPt(const gmtl::LineSegf&,
                                        const gmtl::Point3f&);
   template gmtl::Point3d findNearestPt(const gmtl::LineSegd&,
                                        const gmtl::Point3d&);

   template float distance(const gmtl::LineSegf&, const gmtl::Point3f&);
   template double distance(const gmtl::LineSegd&, const gmtl::Point3d&);
}

#endif /* _VRJ_NET_GMTL_LINE_SEG_OPS_H_ */
