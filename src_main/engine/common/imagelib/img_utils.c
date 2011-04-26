/*
img_utils.c - image common tools
Copyright (C) 2007 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "imagelib.h"
#include "mathlib.h"

convar_t *gl_round_down;

#define LERPBYTE( i )	r = resamplerow1[i]; out[i] = (byte)(((( resamplerow2[i] - r ) * lerp)>>16 ) + r )

uint d_8toQ1table[256];
uint d_8toHLtable[256];
uint d_8to24table[256];

qboolean q1palette_init = false;
qboolean hlpalette_init = false;

static byte palette_q1[768] =
{
0,0,0,15,15,15,31,31,31,47,47,47,63,63,63,75,75,75,91,91,91,107,107,107,123,123,123,139,139,139,155,155,155,171,
171,171,187,187,187,203,203,203,219,219,219,235,235,235,15,11,7,23,15,11,31,23,11,39,27,15,47,35,19,55,43,23,63,
47,23,75,55,27,83,59,27,91,67,31,99,75,31,107,83,31,115,87,31,123,95,35,131,103,35,143,111,35,11,11,15,19,19,27,
27,27,39,39,39,51,47,47,63,55,55,75,63,63,87,71,71,103,79,79,115,91,91,127,99,99,139,107,107,151,115,115,163,123,
123,175,131,131,187,139,139,203,0,0,0,7,7,0,11,11,0,19,19,0,27,27,0,35,35,0,43,43,7,47,47,7,55,55,7,63,63,7,71,71,
7,75,75,11,83,83,11,91,91,11,99,99,11,107,107,15,7,0,0,15,0,0,23,0,0,31,0,0,39,0,0,47,0,0,55,0,0,63,0,0,71,0,0,79,
0,0,87,0,0,95,0,0,103,0,0,111,0,0,119,0,0,127,0,0,19,19,0,27,27,0,35,35,0,47,43,0,55,47,0,67,55,0,75,59,7,87,67,7,
95,71,7,107,75,11,119,83,15,131,87,19,139,91,19,151,95,27,163,99,31,175,103,35,35,19,7,47,23,11,59,31,15,75,35,19,
87,43,23,99,47,31,115,55,35,127,59,43,143,67,51,159,79,51,175,99,47,191,119,47,207,143,43,223,171,39,239,203,31,255,
243,27,11,7,0,27,19,0,43,35,15,55,43,19,71,51,27,83,55,35,99,63,43,111,71,51,127,83,63,139,95,71,155,107,83,167,123,
95,183,135,107,195,147,123,211,163,139,227,179,151,171,139,163,159,127,151,147,115,135,139,103,123,127,91,111,119,
83,99,107,75,87,95,63,75,87,55,67,75,47,55,67,39,47,55,31,35,43,23,27,35,19,19,23,11,11,15,7,7,187,115,159,175,107,
143,163,95,131,151,87,119,139,79,107,127,75,95,115,67,83,107,59,75,95,51,63,83,43,55,71,35,43,59,31,35,47,23,27,35,
19,19,23,11,11,15,7,7,219,195,187,203,179,167,191,163,155,175,151,139,163,135,123,151,123,111,135,111,95,123,99,83,
107,87,71,95,75,59,83,63,51,67,51,39,55,43,31,39,31,23,27,19,15,15,11,7,111,131,123,103,123,111,95,115,103,87,107,
95,79,99,87,71,91,79,63,83,71,55,75,63,47,67,55,43,59,47,35,51,39,31,43,31,23,35,23,15,27,19,11,19,11,7,11,7,255,
243,27,239,223,23,219,203,19,203,183,15,187,167,15,171,151,11,155,131,7,139,115,7,123,99,7,107,83,0,91,71,0,75,55,
0,59,43,0,43,31,0,27,15,0,11,7,0,0,0,255,11,11,239,19,19,223,27,27,207,35,35,191,43,43,175,47,47,159,47,47,143,47,
47,127,47,47,111,47,47,95,43,43,79,35,35,63,27,27,47,19,19,31,11,11,15,43,0,0,59,0,0,75,7,0,95,7,0,111,15,0,127,23,
7,147,31,7,163,39,11,183,51,15,195,75,27,207,99,43,219,127,59,227,151,79,231,171,95,239,191,119,247,211,139,167,123,
59,183,155,55,199,195,55,231,227,87,127,191,255,171,231,255,215,255,255,103,0,0,139,0,0,179,0,0,215,0,0,255,0,0,255,
243,147,255,247,199,255,255,255,159,91,83
};

// this is used only for particle colors
static byte palette_hl[768] =
{
0,0,0,15,15,15,31,31,31,47,47,47,63,63,63,75,75,75,91,91,91,107,107,107,123,123,123,139,139,139,155,155,155,171,
171,171,187,187,187,203,203,203,219,219,219,235,235,235,15,11,7,23,15,11,31,23,11,39,27,15,47,35,19,55,43,23,63,
47,23,75,55,27,83,59,27,91,67,31,99,75,31,107,83,31,115,87,31,123,95,35,131,103,35,143,111,35,11,11,15,19,19,27,
27,27,39,39,39,51,47,47,63,55,55,75,63,63,87,71,71,103,79,79,115,91,91,127,99,99,139,107,107,151,115,115,163,123,
123,175,131,131,187,139,139,203,0,0,0,7,7,0,11,11,0,19,19,0,27,27,0,35,35,0,43,43,7,47,47,7,55,55,7,63,63,7,71,71,
7,75,75,11,83,83,11,91,91,11,99,99,11,107,107,15,7,0,0,15,0,0,23,0,0,31,0,0,39,0,0,47,0,0,55,0,0,63,0,0,71,0,0,79,
0,0,87,0,0,95,0,0,103,0,0,111,0,0,119,0,0,127,0,0,19,19,0,27,27,0,35,35,0,47,43,0,55,47,0,67,55,0,75,59,7,87,67,7,
95,71,7,107,75,11,119,83,15,131,87,19,139,91,19,151,95,27,163,99,31,175,103,35,35,19,7,47,23,11,59,31,15,75,35,19,
87,43,23,99,47,31,115,55,35,127,59,43,143,67,51,159,79,51,175,99,47,191,119,47,207,143,43,223,171,39,239,203,31,255,
243,27,11,7,0,27,19,0,43,35,15,55,43,19,71,51,27,83,55,35,99,63,43,111,71,51,127,83,63,139,95,71,155,107,83,167,123,
95,183,135,107,195,147,123,211,163,139,227,179,151,171,139,163,159,127,151,147,115,135,139,103,123,127,91,111,119,
83,99,107,75,87,95,63,75,87,55,67,75,47,55,67,39,47,55,31,35,43,23,27,35,19,19,23,11,11,15,7,7,187,115,159,175,107,
143,163,95,131,151,87,119,139,79,107,127,75,95,115,67,83,107,59,75,95,51,63,83,43,55,71,35,43,59,31,35,47,23,27,35,
19,19,23,11,11,15,7,7,219,195,187,203,179,167,191,163,155,175,151,139,163,135,123,151,123,111,135,111,95,123,99,83,
107,87,71,95,75,59,83,63,51,67,51,39,55,43,31,39,31,23,27,19,15,15,11,7,111,131,123,103,123,111,95,115,103,87,107,
95,79,99,87,71,91,79,63,83,71,55,75,63,47,67,55,43,59,47,35,51,39,31,43,31,23,35,23,15,27,19,11,19,11,7,11,7,255,
243,27,239,223,23,219,203,19,203,183,15,187,167,15,171,151,11,155,131,7,139,115,7,123,99,7,107,83,0,91,71,0,75,55,
0,59,43,0,43,31,0,27,15,0,11,7,0,0,0,255,11,11,239,19,19,223,27,27,207,35,35,191,43,43,175,47,47,159,47,47,143,47,
47,127,47,47,111,47,47,95,43,43,79,35,35,63,27,27,47,19,19,31,11,11,15,43,0,0,59,0,0,75,7,0,95,7,0,111,15,0,127,23,
7,147,31,7,163,39,11,183,51,15,195,75,27,207,99,43,219,127,59,227,151,79,231,171,95,239,191,119,247,211,139,167,123,
59,183,155,55,199,195,55,231,227,87,0,255,0,171,231,255,215,255,255,103,0,0,139,0,0,179,0,0,215,0,0,255,0,0,255,243,
147,255,247,199,255,255,255,159,91,83
};

/*
=============================================================================

	XASH3D LOAD IMAGE FORMATS

=============================================================================
*/
// stub
static const loadpixformat_t load_null[] =
{
{ NULL, NULL, NULL, IL_HINT_NO }
};

static const loadpixformat_t load_game[] =
{
{ "%s%s.%s", "tga", Image_LoadTGA, IL_HINT_NO },	// hl vgui menus
{ "%s%s.%s", "bmp", Image_LoadBMP, IL_HINT_NO },	// hl skyboxes
{ "%s%s.%s", "jpg", Image_LoadJPG, IL_HINT_NO },	// hl skyboxes
{ "%s%s.%s", "mip", Image_LoadMIP, IL_HINT_NO },	// hl textures from wad or buffer
{ "%s%s.%s", "mdl", Image_LoadMDL, IL_HINT_HL },	// hl studio model skins
{ "%s%s.%s", "spr", Image_LoadSPR, IL_HINT_HL },	// hl sprite frames
{ "%s%s.%s", "lmp", Image_LoadLMP, IL_HINT_HL },	// hl menu images (cached.wad etc)
{ "%s%s.%s", "fnt", Image_LoadFNT, IL_HINT_HL },	// hl menu images (cached.wad etc)
{ "%s%s.%s", "pal", Image_LoadPAL, IL_HINT_NO },	// install studio palette
{ NULL, NULL, NULL, IL_HINT_NO }
};

/*
=============================================================================

	XASH3D SAVE IMAGE FORMATS

=============================================================================
*/
// stub
static const savepixformat_t save_null[] =
{
{ NULL, NULL, NULL }
};

// Xash3D normal instance
static const savepixformat_t save_game[] =
{
{ "%s%s.%s", "tga", Image_SaveTGA },		// tga screenshots
{ "%s%s.%s", "bmp", Image_SaveBMP },		// bmp levelshots or screenshots
{ NULL, NULL, NULL }
};

void Image_Init( void )
{
	// init pools
	host.imagepool = Mem_AllocPool( "ImageLib Pool" );
	gl_round_down = Cvar_Get( "gl_round_down", "0", CVAR_RENDERINFO, "down size non-power of two textures" );

	// install image formats (can be re-install later by Image_Setup)
	switch( host.type )
	{
	case HOST_NORMAL:
		image.cmd_flags = IL_USE_LERPING|IL_ALLOW_OVERWRITE;		
		image.loadformats = load_game;
		image.saveformats = save_game;
		break;
	default:	// all other instances not using imagelib or will be reinstalling later
		image.loadformats = load_null;
		image.saveformats = save_null;
		break;
	}
	image.tempbuffer = NULL;
}

void Image_Shutdown( void )
{
	Mem_Check(); // check for leaks
	Mem_FreePool( &host.imagepool );
}

byte *Image_Copy( size_t size )
{
	byte	*out;

	out = Mem_Alloc( host.imagepool, size );
	Q_memcpy( out, image.tempbuffer, size );
	return out; 
}

/*
=================
Image_NearestPOW
=================
*/
int Image_NearestPOW( int value, qboolean roundDown )
{
	int	n = 1;

	if( value <= 0 ) return 1;
	while( n < value ) n <<= 1;

	if( roundDown )
	{
		if( n > value ) n >>= 1;
	}
	return n;
}

void Image_RoundDimensions( int *width, int *height )
{
	// find nearest power of two, rounding down if desired
	*width = Image_NearestPOW( *width, gl_round_down->integer );
	*height = Image_NearestPOW( *height, gl_round_down->integer );
}

qboolean Image_ValidSize( const char *name )
{
	if( image.width > IMAGE_MAXWIDTH || image.height > IMAGE_MAXHEIGHT || image.width <= 0 || image.height <= 0 )
	{
		MsgDev( D_ERROR, "Image: %s has invalid sizes %i x %i\n", name, image.width, image.height );
		return false;
	}
	return true;
}

qboolean Image_LumpValidSize( const char *name )
{
	if( image.width > LUMP_MAXWIDTH || image.height > LUMP_MAXHEIGHT || image.width <= 0 || image.height <= 0 )
	{
		MsgDev(D_WARN, "Image_LumpValidSize: (%s) dims out of range[%dx%d]\n", name, image.width,image.height );
		return false;
	}
	return true;
}

/*
=============
Image_ComparePalette
=============
*/
int Image_ComparePalette( const byte *pal )
{
	if( pal == NULL )
		return PAL_INVALID;
	else if( !memcmp( palette_q1, pal, 768 ))
		return PAL_QUAKE1;
	else if( !memcmp( palette_hl, pal, 768 ))
		return PAL_HALFLIFE;
	return PAL_CUSTOM;		
}

void Image_SetPalette( const byte *pal, uint *d_table )
{
	int	i;
	byte	rgba[4];
	
	// setup palette
	switch( image.d_rendermode )
	{
	case LUMP_DECAL:
		for( i = 0; i < 256; i++ )
		{
			rgba[0] = pal[765];
			rgba[1] = pal[766];
			rgba[2] = pal[767];
			rgba[3] = i;
			d_table[i] = *(uint *)rgba;
		}
		break;
	case LUMP_TRANSPARENT:
		for( i = 0; i < 256; i++ )
		{
			rgba[0] = pal[i*3+0];
			rgba[1] = pal[i*3+1];
			rgba[2] = pal[i*3+2];
			rgba[3] = pal[i] == 255 ? pal[i] : 0xFF;
			d_table[i] = *(uint *)rgba;
		}
		break;
	case LUMP_QFONT:
		for( i = 0; i < 256; i++ )
		{
			rgba[0] = pal[i*3+0];
			rgba[1] = pal[i*3+1];
			rgba[2] = pal[i*3+2];
			rgba[3] = 0xFF;
			d_table[i] = *(uint *)rgba;
		}
		break;
	case LUMP_NORMAL:
		for( i = 0; i < 256; i++ )
		{
			rgba[0] = pal[i*3+0];
			rgba[1] = pal[i*3+1];
			rgba[2] = pal[i*3+2];
			rgba[3] = 0xFF;
			d_table[i] = *(uint *)rgba;
		}
		break;
	case LUMP_EXTENDED:
		for( i = 0; i < 256; i++ )
		{
			rgba[0] = pal[i*4+0];
			rgba[1] = pal[i*4+1];
			rgba[2] = pal[i*4+2];
			rgba[3] = pal[i*4+3];
			d_table[i] = *(uint *)rgba;
		}
		break;	
	}
}

void Image_GetPaletteQ1( void )
{
	image.d_rendermode = LUMP_NORMAL;

	if( !q1palette_init )
	{
		Image_SetPalette( palette_q1, d_8toQ1table );
		d_8toQ1table[255] = 0; // 255 is transparent
		q1palette_init = true;
	}
	image.d_currentpal = d_8toQ1table;
}

void Image_GetPaletteHL( void )
{
	image.d_rendermode = LUMP_NORMAL;

	if( !hlpalette_init )
	{
		Image_SetPalette( palette_hl, d_8toHLtable );
		d_8toHLtable[255] = 0; // 255 is transparent
		hlpalette_init = true;
	}
	image.d_currentpal = d_8toHLtable;
}

void Image_GetPaletteBMP( const byte *pal )
{
	image.d_rendermode = LUMP_EXTENDED;

	if( pal )
	{
		Image_SetPalette( pal, d_8to24table );
		image.d_currentpal = d_8to24table;
	}
}

void Image_GetPaletteLMP( const byte *pal, int rendermode )
{
	image.d_rendermode = rendermode;

	if( pal )
	{
		Image_SetPalette( pal, d_8to24table );
		if( rendermode != LUMP_DECAL )
			d_8to24table[255] &= 0xFFFFFF;
		image.d_currentpal = d_8to24table;
	}
	else if( rendermode == LUMP_QFONT )
	{
		// quake1 base palette and font palette have some diferences
		Image_SetPalette( palette_q1, d_8to24table );
		d_8to24table[0] = 0;
		image.d_currentpal = d_8to24table;
	}
	else Image_GetPaletteHL(); // default half-life palette          
}

void Image_ConvertPalTo24bit( rgbdata_t *pic )
{
	byte	*pal32, *pal24;
	byte	*converted;
	int	i;

	if( !pic->palette )
	{
		MsgDev( D_ERROR, "Image_ConvertPalTo24bit: no palette found\n" );
		return;
	}

	if( pic->type == PF_INDEXED_24 )
	{
		MsgDev( D_ERROR, "Image_ConvertPalTo24bit: palette already converted\n" );
		return;
	}

	pal24 = converted = Mem_Alloc( host.imagepool, 768 );
	pal32 = pic->palette;

	for( i = 0; i < 256; i++, pal24 += 3, pal32 += 4 )
	{
		pal24[0] = pal32[0];
		pal24[1] = pal32[1];
		pal24[2] = pal32[2];
	}
	Mem_Free( pic->palette );
	pic->palette = converted;
	pic->type = PF_INDEXED_24;
}

void Image_CopyPalette32bit( void )
{
	if( image.palette ) return; // already created ?
	image.palette = Mem_Alloc( host.imagepool, 1024 );
	Q_memcpy( image.palette, image.d_currentpal, 1024 );
}

void Image_CopyParms( rgbdata_t *src )
{
	Image_Reset();

	image.width = src->width;
	image.height = src->height;
	image.type = src->type;
	image.flags = src->flags;
	image.size = src->size;
	image.palette = src->palette;	// may be NULL

	Q_memcpy( image.fogParams, src->fogParams, sizeof( image.fogParams ));
}

/*
============
Image_Copy8bitRGBA

NOTE: must call Image_GetPaletteXXX before used
============
*/
qboolean Image_Copy8bitRGBA( const byte *in, byte *out, int pixels )
{
	int	*iout = (int *)out;
	byte	*fin = (byte *)in;
	rgba_t	*col;
	int	i;

	if( !image.d_currentpal )
	{
		MsgDev( D_ERROR, "Image_Copy8bitRGBA: no palette set\n" );
		return false;
	}

	if( !in )
	{
		MsgDev( D_ERROR, "Image_Copy8bitRGBA: no input image\n" );
		return false;
	}

	// this is a base image with luma - clear luma pixels
	if( image.flags & IMAGE_HAS_LUMA )
	{
		for( i = 0; i < image.width * image.height; i++ )
			fin[i] = fin[i] < 224 ? fin[i] : 0;
	}

	while( pixels >= 8 )
	{
		iout[0] = image.d_currentpal[in[0]];
		iout[1] = image.d_currentpal[in[1]];
		iout[2] = image.d_currentpal[in[2]];
		iout[3] = image.d_currentpal[in[3]];
		iout[4] = image.d_currentpal[in[4]];
		iout[5] = image.d_currentpal[in[5]];
		iout[6] = image.d_currentpal[in[6]];
		iout[7] = image.d_currentpal[in[7]];

		col = (rgba_t *)iout;
		if( col[0] != col[1] || col[1] != col[2] )
			image.flags |= IMAGE_HAS_COLOR;

		in += 8;
		iout += 8;
		pixels -= 8;
	}

	if( pixels & 4 )
	{
		iout[0] = image.d_currentpal[in[0]];
		iout[1] = image.d_currentpal[in[1]];
		iout[2] = image.d_currentpal[in[2]];
		iout[3] = image.d_currentpal[in[3]];
		in += 4;
		iout += 4;
	}

	if( pixels & 2 )
	{
		iout[0] = image.d_currentpal[in[0]];
		iout[1] = image.d_currentpal[in[1]];
		in += 2;
		iout += 2;
	}

	if( pixels & 1 ) // last byte
		iout[0] = image.d_currentpal[in[0]];

	image.type = PF_RGBA_32;	// update image type;
	return true;
}

static void Image_Resample32LerpLine( const byte *in, byte *out, int inwidth, int outwidth )
{
	int	j, xi, oldx = 0, f, fstep, endx, lerp;

	fstep = (int)(inwidth * 65536.0f / outwidth);
	endx = (inwidth-1);

	for( j = 0, f = 0; j < outwidth; j++, f += fstep )
	{
		xi = f>>16;
		if( xi != oldx )
		{
			in += (xi - oldx) * 4;
			oldx = xi;
		}
		if( xi < endx )
		{
			lerp = f & 0xFFFF;
			*out++ = (byte)((((in[4] - in[0]) * lerp)>>16) + in[0]);
			*out++ = (byte)((((in[5] - in[1]) * lerp)>>16) + in[1]);
			*out++ = (byte)((((in[6] - in[2]) * lerp)>>16) + in[2]);
			*out++ = (byte)((((in[7] - in[3]) * lerp)>>16) + in[3]);
		}
		else // last pixel of the line has no pixel to lerp to
		{
			*out++ = in[0];
			*out++ = in[1];
			*out++ = in[2];
			*out++ = in[3];
		}
	}
}

static void Image_Resample24LerpLine( const byte *in, byte *out, int inwidth, int outwidth )
{
	int	j, xi, oldx = 0, f, fstep, endx, lerp;

	fstep = (int)(inwidth * 65536.0f / outwidth);
	endx = (inwidth-1);

	for( j = 0, f = 0; j < outwidth; j++, f += fstep )
	{
		xi = f>>16;
		if( xi != oldx )
		{
			in += (xi - oldx) * 3;
			oldx = xi;
		}
		if( xi < endx )
		{
			lerp = f & 0xFFFF;
			*out++ = (byte)((((in[3] - in[0]) * lerp)>>16) + in[0]);
			*out++ = (byte)((((in[4] - in[1]) * lerp)>>16) + in[1]);
			*out++ = (byte)((((in[5] - in[2]) * lerp)>>16) + in[2]);
		}
		else // last pixel of the line has no pixel to lerp to
		{
			*out++ = in[0];
			*out++ = in[1];
			*out++ = in[2];
		}
	}
}

void Image_Resample32Lerp( const void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight )
{
	int	i, j, r, yi, oldy = 0, f, fstep, lerp, endy = (inheight - 1);
	int	inwidth4 = inwidth * 4;
	int	outwidth4 = outwidth * 4;
	const byte *inrow;
	byte	*out;
	byte	*resamplerow1;
	byte	*resamplerow2;

	out = (byte *)outdata;
	fstep = (int)(inheight * 65536.0f/outheight);

	resamplerow1 = (byte *)Mem_Alloc( host.imagepool, outwidth * 4 * 2);
	resamplerow2 = resamplerow1 + outwidth * 4;

	inrow = (const byte *)indata;

	Image_Resample32LerpLine( inrow, resamplerow1, inwidth, outwidth );
	Image_Resample32LerpLine( inrow + inwidth4, resamplerow2, inwidth, outwidth );

	for( i = 0, f = 0; i < outheight; i++, f += fstep )
	{
		yi = f>>16;

		if( yi < endy )
		{
			lerp = f & 0xFFFF;
			if( yi != oldy )
			{
				inrow = (byte *)indata + inwidth4 * yi;
				if (yi == oldy+1) Q_memcpy( resamplerow1, resamplerow2, outwidth4 );
				else Image_Resample32LerpLine( inrow, resamplerow1, inwidth, outwidth );
				Image_Resample32LerpLine( inrow + inwidth4, resamplerow2, inwidth, outwidth );
				oldy = yi;
			}
			j = outwidth - 4;
			while( j >= 0 )
			{
				LERPBYTE( 0);
				LERPBYTE( 1);
				LERPBYTE( 2);
				LERPBYTE( 3);
				LERPBYTE( 4);
				LERPBYTE( 5);
				LERPBYTE( 6);
				LERPBYTE( 7);
				LERPBYTE( 8);
				LERPBYTE( 9);
				LERPBYTE(10);
				LERPBYTE(11);
				LERPBYTE(12);
				LERPBYTE(13);
				LERPBYTE(14);
				LERPBYTE(15);
				out += 16;
				resamplerow1 += 16;
				resamplerow2 += 16;
				j -= 4;
			}
			if( j & 2 )
			{
				LERPBYTE( 0);
				LERPBYTE( 1);
				LERPBYTE( 2);
				LERPBYTE( 3);
				LERPBYTE( 4);
				LERPBYTE( 5);
				LERPBYTE( 6);
				LERPBYTE( 7);
				out += 8;
				resamplerow1 += 8;
				resamplerow2 += 8;
			}
			if( j & 1 )
			{
				LERPBYTE( 0);
				LERPBYTE( 1);
				LERPBYTE( 2);
				LERPBYTE( 3);
				out += 4;
				resamplerow1 += 4;
				resamplerow2 += 4;
			}
			resamplerow1 -= outwidth4;
			resamplerow2 -= outwidth4;
		}
		else
		{
			if( yi != oldy )
			{
				inrow = (byte *)indata + inwidth4*yi;
				if( yi == oldy + 1 ) Q_memcpy( resamplerow1, resamplerow2, outwidth4 );
				else Image_Resample32LerpLine( inrow, resamplerow1, inwidth, outwidth);
				oldy = yi;
			}
			Q_memcpy( out, resamplerow1, outwidth4 );
		}
	}

	Mem_Free( resamplerow1 );
	resamplerow1 = NULL;
	resamplerow2 = NULL;
}

void Image_Resample32Nolerp( const void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight )
{
	int	i, j;
	uint	frac, fracstep;
	int	*inrow, *out = (int *)outdata; // relies on int being 4 bytes

	fracstep = inwidth * 0x10000/outwidth;

	for( i = 0; i < outheight; i++)
	{
		inrow = (int *)indata + inwidth * (i * inheight/outheight);
		frac = fracstep>>1;
		j = outwidth - 4;

		while( j >= 0 )
		{
			out[0] = inrow[frac >> 16];frac += fracstep;
			out[1] = inrow[frac >> 16];frac += fracstep;
			out[2] = inrow[frac >> 16];frac += fracstep;
			out[3] = inrow[frac >> 16];frac += fracstep;
			out += 4;
			j -= 4;
		}
		if( j & 2 )
		{
			out[0] = inrow[frac >> 16];frac += fracstep;
			out[1] = inrow[frac >> 16];frac += fracstep;
			out += 2;
		}
		if( j & 1 )
		{
			out[0] = inrow[frac >> 16];frac += fracstep;
			out += 1;
		}
	}
}

void Image_Resample24Lerp( const void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight )
{
	int	i, j, r, yi, oldy, f, fstep, lerp, endy = (inheight - 1);
	int	inwidth3 = inwidth * 3;
	int	outwidth3 = outwidth * 3;
	const byte *inrow;
	byte	*out = (byte *)outdata;
	byte	*resamplerow1;
	byte	*resamplerow2;
	
	fstep = (int)(inheight * 65536.0f / outheight);

	resamplerow1 = (byte *)Mem_Alloc( host.imagepool, outwidth * 3 * 2 );
	resamplerow2 = resamplerow1 + outwidth*3;

	inrow = (const byte *)indata;
	oldy = 0;
	Image_Resample24LerpLine( inrow, resamplerow1, inwidth, outwidth );
	Image_Resample24LerpLine( inrow + inwidth3, resamplerow2, inwidth, outwidth );

	for( i = 0, f = 0; i < outheight; i++, f += fstep )
	{
		yi = f>>16;

		if( yi < endy )
		{
			lerp = f & 0xFFFF;
			if( yi != oldy )
			{
				inrow = (byte *)indata + inwidth3 * yi;
				if( yi == oldy + 1) Q_memcpy( resamplerow1, resamplerow2, outwidth3 );
				else Image_Resample24LerpLine( inrow, resamplerow1, inwidth, outwidth );
				Image_Resample24LerpLine( inrow + inwidth3, resamplerow2, inwidth, outwidth );
				oldy = yi;
			}
			j = outwidth - 4;
			while( j >= 0 )
			{
				LERPBYTE( 0);
				LERPBYTE( 1);
				LERPBYTE( 2);
				LERPBYTE( 3);
				LERPBYTE( 4);
				LERPBYTE( 5);
				LERPBYTE( 6);
				LERPBYTE( 7);
				LERPBYTE( 8);
				LERPBYTE( 9);
				LERPBYTE(10);
				LERPBYTE(11);
				out += 12;
				resamplerow1 += 12;
				resamplerow2 += 12;
				j -= 4;
			}
			if( j & 2 )
			{
				LERPBYTE( 0);
				LERPBYTE( 1);
				LERPBYTE( 2);
				LERPBYTE( 3);
				LERPBYTE( 4);
				LERPBYTE( 5);
				out += 6;
				resamplerow1 += 6;
				resamplerow2 += 6;
			}
			if( j & 1 )
			{
				LERPBYTE( 0);
				LERPBYTE( 1);
				LERPBYTE( 2);
				out += 3;
				resamplerow1 += 3;
				resamplerow2 += 3;
			}
			resamplerow1 -= outwidth3;
			resamplerow2 -= outwidth3;
		}
		else
		{
			if( yi != oldy )
			{
				inrow = (byte *)indata + inwidth3*yi;
				if( yi == oldy + 1) Q_memcpy( resamplerow1, resamplerow2, outwidth3 );
				else Image_Resample24LerpLine( inrow, resamplerow1, inwidth, outwidth );
				oldy = yi;
			}
			Q_memcpy( out, resamplerow1, outwidth3 );
		}
	}
	Mem_Free( resamplerow1 );
	resamplerow1 = NULL;
	resamplerow2 = NULL;
}

void Image_Resample24Nolerp( const void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight )
{
	int	i, j, f, inwidth3 = inwidth * 3;
	uint	frac, fracstep;
	byte	*inrow, *out = (byte *)outdata;

	fracstep = inwidth * 0x10000 / outwidth;

	for( i = 0; i < outheight; i++)
	{
		inrow = (byte *)indata + inwidth3 * (i * inheight / outheight);
		frac = fracstep>>1;
		j = outwidth - 4;

		while( j >= 0 )
		{
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			j -= 4;
		}
		if( j & 2 )
		{
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			out += 2;
		}
		if( j & 1 )
		{
			f = (frac >> 16)*3;
			*out++ = inrow[f+0];
			*out++ = inrow[f+1];
			*out++ = inrow[f+2];
			frac += fracstep;
			out += 1;
		}
	}
}

void Image_Resample8Nolerp( const void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight )
{
	int	i, j;
	byte	*in, *inrow;
	uint	frac, fracstep;
	byte	*out = (byte *)outdata;

	in = (byte *)indata;
	fracstep = inwidth * 0x10000 / outwidth;
	for( i = 0; i < outheight; i++, out += outwidth )
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep>>1;
		for( j = 0; j < outwidth; j++ )
		{
			out[j] = inrow[frac>>16];
			frac += fracstep;
		}
	}
}

/*
================
Image_Resample
================
*/
byte *Image_ResampleInternal( const void *indata, int inwidth, int inheight, int outwidth, int outheight, int type, qboolean *resampled )
{
	qboolean	quality = (image.cmd_flags & IL_USE_LERPING);

	// nothing to resample ?
	if( inwidth == outwidth && inheight == outheight )
	{
		*resampled = false;
		return (byte *)indata;
	}

	// alloc new buffer
	switch( type )
	{
	case PF_INDEXED_24:
	case PF_INDEXED_32:
		image.tempbuffer = (byte *)Mem_Realloc( host.imagepool, image.tempbuffer, outwidth * outheight );
		Image_Resample8Nolerp( indata, inwidth, inheight, image.tempbuffer, outwidth, outheight );
		break;		
	case PF_RGB_24:
		image.tempbuffer = (byte *)Mem_Realloc( host.imagepool, image.tempbuffer, outwidth * outheight * 3 );
		if( quality ) Image_Resample24Lerp( indata, inwidth, inheight, image.tempbuffer, outwidth, outheight );
		else Image_Resample24Nolerp( indata, inwidth, inheight, image.tempbuffer, outwidth, outheight );
		break;
	case PF_RGBA_32:
		image.tempbuffer = (byte *)Mem_Realloc( host.imagepool, image.tempbuffer, outwidth * outheight * 4 );
		if( quality ) Image_Resample32Lerp( indata, inwidth, inheight, image.tempbuffer, outwidth, outheight );
		else Image_Resample32Nolerp( indata, inwidth, inheight, image.tempbuffer, outwidth, outheight );
		break;
	default:
		MsgDev( D_WARN, "Image_Resample: unsupported format %s\n", PFDesc[type].name );
		*resampled = false;
		return (byte *)indata;	
	}

	*resampled = true;
	return image.tempbuffer;
}

/*
================
Image_Flood
================
*/
byte *Image_FloodInternal( const byte *indata, int inwidth, int inheight, int outwidth, int outheight, int type, qboolean *resampled )
{
	qboolean	quality = (image.cmd_flags & IL_USE_LERPING);
	int	samples = PFDesc[type].bpp;
	int	newsize, x, y, i;
	byte	*in, *out;

	// nothing to reflood ?
	if( inwidth == outwidth && inheight == outheight )
	{
		*resampled = false;
		return (byte *)indata;
	}

	// alloc new buffer
	switch( type )
	{
	case PF_INDEXED_24:
	case PF_INDEXED_32:
	case PF_RGB_24:
	case PF_BGR_24:
	case PF_BGRA_32:
	case PF_RGBA_32:
		in = ( byte *)indata;
		newsize = outwidth * outheight * samples;
		out = image.tempbuffer = (byte *)Mem_Realloc( host.imagepool, image.tempbuffer, newsize );
		break;
	default:
		MsgDev( D_WARN, "Image_Flood: unsupported format %s\n", PFDesc[type].name );
		*resampled = false;
		return (byte *)indata;	
	}

	if( samples == 1 ) Q_memset( out, 0xFF, newsize );	// last palette color
	else Q_memset( out, 0x00808080, newsize );		// gray (alpha leaved 0x00)

	for( y = 0; y < outheight; y++ )
	{
		for( x = 0; y < inheight && x < outwidth; x++ )
		{
			for( i = 0; i < samples; i++ )
			{
				if( x < inwidth )
					*out++ = *in++;
				else *out++;
			}
		}
	}

	*resampled = true;
	return image.tempbuffer;
}

/*
================
Image_Flip
================
*/
byte *Image_FlipInternal( const byte *in, word *srcwidth, word *srcheight, int type, int flags )
{
	int	i, x, y;
	word	width = *srcwidth;
	word	height = *srcheight; 
	int	samples = PFDesc[type].bpp;
	qboolean	flip_x = ( flags & IMAGE_FLIP_X ) ? true : false;
	qboolean	flip_y = ( flags & IMAGE_FLIP_Y ) ? true : false;
	qboolean	flip_i = ( flags & IMAGE_ROT_90 ) ? true : false;
	int	row_inc = ( flip_y ? -samples : samples ) * width;
	int	col_inc = ( flip_x ? -samples : samples );
	int	row_ofs = ( flip_y ? ( height - 1 ) * width * samples : 0 );
	int	col_ofs = ( flip_x ? ( width - 1 ) * samples : 0 );
	const byte *p, *line;
	byte	*out;

	// nothing to process
	if(!(flags & (IMAGE_FLIP_X|IMAGE_FLIP_Y|IMAGE_ROT_90)))
		return (byte *)in;

	switch( type )
	{
	case PF_INDEXED_24:
	case PF_INDEXED_32:
	case PF_RGB_24:
	case PF_RGBA_32:
		image.tempbuffer = Mem_Realloc( host.imagepool, image.tempbuffer, width * height * samples );
		break;
	default:
		// we can flip DXT without expanding to RGBA? hmmm...
		MsgDev( D_WARN, "Image_Flip: unsupported format %s\n", PFDesc[type].name );
		return (byte *)in;	
	}
	out = image.tempbuffer;

	if( flip_i )
	{
		for( x = 0, line = in + col_ofs; x < width; x++, line += col_inc )
			for( y = 0, p = line + row_ofs; y < height; y++, p += row_inc, out += samples )
				for( i = 0; i < samples; i++ )
					out[i] = p[i];
	}
	else
	{
		for( y = 0, line = in + row_ofs; y < height; y++, line += row_inc )
			for( x = 0, p = line + col_ofs; x < width; x++, p += col_inc, out += samples )
				for( i = 0; i < samples; i++ )
					out[i] = p[i];
	}

	// update dims
	if( flags & IMAGE_ROT_90 )
	{
		*srcwidth = height;
		*srcheight = width;		
	}
	else
	{
		*srcwidth = width;
		*srcheight = height;	
	}
	return image.tempbuffer;
}

byte *Image_CreateLumaInternal( const byte *fin, int width, int height, int type, int flags )
{
	byte	*out;
	int	i;

	if(!( flags & IMAGE_HAS_LUMA ))
	{
		MsgDev( D_WARN, "Image_MakeLuma: image doesn't has luma pixels\n" );
		return (byte *)fin;	  
	}

	switch( type )
	{
	case PF_INDEXED_24:
	case PF_INDEXED_32:
		out = image.tempbuffer = Mem_Realloc( host.imagepool, image.tempbuffer, width * height );
		for( i = 0; i < width * height; i++ )
			*out++ = fin[i] >= 224 ? fin[i] : 0;
		break;
	default:
		// another formats does ugly result :(
		MsgDev( D_WARN, "Image_MakeLuma: unsupported format %s\n", PFDesc[type].name );
		return (byte *)fin;	
	}
	return image.tempbuffer;
}

qboolean Image_AddIndexedImageToPack( const byte *in, int width, int height )
{
	int	mipsize = width * height;
	qboolean	expand_to_rgba = true;

	if( image.cmd_flags & IL_KEEP_8BIT )
		expand_to_rgba = false;
	else if( host.type == HOST_NORMAL && ( image.flags & ( IMAGE_HAS_LUMA|IMAGE_QUAKESKY )))
		expand_to_rgba = false;

	image.size = mipsize;

	if( expand_to_rgba ) image.size *= 4;
	else Image_CopyPalette32bit(); 

	// reallocate image buffer
	image.rgba = Mem_Alloc( host.imagepool, image.size );	
	if( expand_to_rgba == false ) Q_memcpy( image.rgba, in, image.size );
	else if( !Image_Copy8bitRGBA( in, image.rgba, mipsize ))
		return false; // probably pallette not installed

	return true;
}

/*
=============
Image_Decompress

force to unpack any image to 32-bit buffer
=============
*/
qboolean Image_Decompress( const byte *data )
{
	byte	*fin, *fout;
	int	i, size; 

	if( !data ) return false;
	fin = (byte *)data;

	size = image.width * image.height * 4;
	image.tempbuffer = Mem_Realloc( host.imagepool, image.tempbuffer, size );
	fout = image.tempbuffer;

	switch( PFDesc[image.type].format )
	{
	case PF_INDEXED_24:
		if( image.flags & IMAGE_HAS_ALPHA )
		{
			if( image.flags & IMAGE_COLORINDEX )
				Image_GetPaletteLMP( image.palette, LUMP_DECAL ); 
			else Image_GetPaletteLMP( image.palette, LUMP_TRANSPARENT ); 
		}
		else Image_GetPaletteLMP( image.palette, LUMP_NORMAL );
		// intentional falltrough
	case PF_INDEXED_32:
		if( !image.d_currentpal ) image.d_currentpal = (uint *)image.palette;
		if( !Image_Copy8bitRGBA( fin, fout, image.width * image.height ))
			return false;
		break;
	case PF_RGB_24:
		for (i = 0; i < image.width * image.height; i++ )
		{
			fout[(i<<2)+0] = fin[i*3+0];
			fout[(i<<2)+1] = fin[i*3+1];
			fout[(i<<2)+2] = fin[i*3+2];
			fout[(i<<2)+3] = 255;
		}
		break;
	case PF_BGR_24:
		for (i = 0; i < image.width * image.height; i++ )
		{
			fout[(i<<2)+0] = fin[i*3+2];
			fout[(i<<2)+1] = fin[i*3+1];
			fout[(i<<2)+2] = fin[i*3+0];
			fout[(i<<2)+3] = 255;
		}
		break;
	case PF_RGBA_32:
		Q_memcpy( fout, fin, size );
		break;
	case PF_BGRA_32:
		for( i = 0; i < image.width * image.height; i++ )
		{
			fout[i*4+0] = fin[i*4+2];
			fout[i*4+1] = fin[i*4+1];
			fout[i*4+2] = fin[i*4+0];
			fout[i*4+3] = fin[i*4+3];
		}
		break;
	default: return false;
	}

	// set new size
	image.size = size;

	return true;
}

rgbdata_t *Image_DecompressInternal( rgbdata_t *pic )
{
	// quick case to reject unneeded conversions
	if( pic->type == PF_RGBA_32 )
		return pic;

	Image_CopyParms( pic );
	image.size = image.ptr = 0;

	Image_Decompress( pic->buffer );

	// now we can change type to RGBA
	pic->type = PF_RGBA_32;

	pic->buffer = Mem_Realloc( host.imagepool, pic->buffer, image.size );
	Q_memcpy( pic->buffer, image.tempbuffer, image.size );
	Mem_Free( pic->palette );
	pic->flags = image.flags;
	pic->palette = NULL;

	return pic;
}

qboolean Image_Process( rgbdata_t **pix, int width, int height, uint flags )
{
	rgbdata_t	*pic = *pix;
	qboolean	result = true;
	byte	*out;
				
	// check for buffers
	if( !pic || !pic->buffer )
	{
		MsgDev( D_WARN, "Image_Process: NULL image\n" );
		return false;
	}

	if( !flags ) return false;	// no operation specfied

	if( flags & IMAGE_MAKE_LUMA )
	{
		out = Image_CreateLumaInternal( pic->buffer, pic->width, pic->height, pic->type, pic->flags );
		if( pic->buffer != out ) Q_memcpy( pic->buffer, image.tempbuffer, pic->size );
		pic->flags &= ~IMAGE_HAS_LUMA;
	}

	// update format to RGBA if any
	if( flags & IMAGE_FORCE_RGBA ) pic = Image_DecompressInternal( pic );

	// NOTE: flip and resample algorythms can't difference palette size
	if( flags & IMAGE_PALTO24 ) Image_ConvertPalTo24bit( pic );
	out = Image_FlipInternal( pic->buffer, &pic->width, &pic->height, pic->type, flags );
	if( pic->buffer != out ) Q_memcpy( pic->buffer, image.tempbuffer, pic->size );

	if(( flags & IMAGE_RESAMPLE && width > 0 && height > 0 ) || flags & IMAGE_ROUND || flags & IMAGE_ROUNDFILLER )
	{
		int	w, h;
		qboolean	resampled = false;

		if( flags & IMAGE_ROUND || flags & IMAGE_ROUNDFILLER )
		{
			w = pic->width;
			h = pic->height;

			// round to nearest pow
			// NOTE: images with dims less than 8x8 may causing problems
			if( flags & IMAGE_ROUNDFILLER )
			{
				// roundfiller always must roundup
				w = Image_NearestPOW( w, false );
				h = Image_NearestPOW( h, false );
			}
			else Image_RoundDimensions( &w, &h );

			w = bound( 8, w, IMAGE_MAXWIDTH );	// 8 - 4096
			h = bound( 8, h, IMAGE_MAXHEIGHT);	// 8 - 4096
		}
		else
		{
			// custom size (user choise without limitations)
			w = bound( 1, width, IMAGE_MAXWIDTH );	// 1 - 4096
			h = bound( 1, height, IMAGE_MAXHEIGHT);	// 1 - 4096
		}

		if( flags & IMAGE_ROUNDFILLER )
	         		out = Image_FloodInternal( pic->buffer, pic->width, pic->height, w, h, pic->type, &resampled );
		else out = Image_ResampleInternal((uint *)pic->buffer, pic->width, pic->height, w, h, pic->type, &resampled );

		if( resampled ) // resampled or filled
		{
			MsgDev( D_NOTE, "Image_Resample: from[%d x %d] to [%d x %d]\n", pic->width, pic->height, w, h );
			pic->width = w, pic->height = h;
			pic->size = w * h * PFDesc[pic->type].bpp;
			Mem_Free( pic->buffer );		// free original image buffer
			pic->buffer = Image_Copy( pic->size );	// unzone buffer (don't touch image.tempbuffer)
		}
		else result = false; // not a resampled or filled
	}
	*pix = pic;

	return result;
}