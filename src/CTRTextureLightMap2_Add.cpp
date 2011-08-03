/*
 * LTE Game Engine
 * Copyright (C) 2006-2008 SiberianSTAR <haxormail@gmail.com>
 * http://www.ltestudios.com
 *  
 * The LTE Game Engine is based on Irrlicht 1.0
 * Irrlicht Engine is Copyright (C) 2002-2006 Nikolaus Gebhardt
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */
 
// Copyright (C) 2002-2006 Nikolaus Gebhardt/Alten Thomas
// This file is part of the "engine Engine".
// For conditions of distribution and use, see copyright notice in engine.h

#include "ITriangleRenderer2.h"

// compile flag for this file

#undef USE_Z
#undef IPOL_Z
#undef CMP_Z
#undef WRITE_Z

#undef IPOL_W
#undef CMP_W
#undef WRITE_W

#undef SUBTEXEL

#undef IPOL_C
#undef IPOL_T0
#undef IPOL_T1

// define render case


#define SUBTEXEL

#define IPOL_W
//#define CMP_W
//#define WRITE_W

//#define IPOL_C
#define IPOL_T0
#define IPOL_T1

// apply global override

#ifndef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
        #undef IPOL_W
#endif

#ifndef SOFTWARE_DRIVER_2_SUBTEXEL
        #undef SUBTEXEL
#endif


namespace engine
{

namespace video
{

class CTRTextureLightMap2_Add : public ITriangleRenderer2
{
public:

//! constructor
CTRTextureLightMap2_Add(IZBuffer2* zbuffer);

//! draws an indexed triangle list
virtual void drawTriangle ( const s4DVertex *a,const s4DVertex *b,const s4DVertex *c );


private:
void scanline_bilinear ( sScanLineData * data ) const;

};

//! constructor
CTRTextureLightMap2_Add::CTRTextureLightMap2_Add(IZBuffer2* zbuffer)
	: ITriangleRenderer2(zbuffer)
{
	#ifdef _DEBUG
	setDebugName("CTRTextureLightMap2_Add");
	#endif
}



/*!
 */
inline void CTRTextureLightMap2_Add::scanline_bilinear ( sScanLineData * data ) const
{


}

void CTRTextureLightMap2_Add::drawTriangle ( const s4DVertex *a,const s4DVertex *b,const s4DVertex *c )
{


}






//! creates a flat triangle renderer
ITriangleRenderer2* createTriangleRendererTextureLightMap2_Add(IZBuffer2* zbuffer)
{
	return new CTRTextureLightMap2_Add(zbuffer);
}


} // end namespace video
} // end namespace engine



