/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Slider.h"
#include "CheckBox.h"

#define ART_BANNER	  	"gfx/shell/head_vidoptions"
#define ART_GAMMA		"gfx/shell/gamma"

class CMenuVidOptions : public CMenuFramework
{
private:
	void _Init() override;
	void _VidInit() override;

public:
	CMenuVidOptions() : CMenuFramework( "CMenuVidOptions" ) { }
	void SaveAndPopMenu() override;
	void GammaUpdate();
	void GammaGet();

	int		outlineWidth;

	class CMenuVidPreview : public CMenuBitmap
	{
		void Draw() override;
	} testImage;

	CMenuPicButton	done;

	CMenuSlider	screenSize;
	CMenuSlider	gammaIntensity;
	CMenuSlider	glareReduction;
#ifndef MAINUI_PSP
	CMenuCheckBox	fastSky;
	CMenuCheckBox	vbo;
	CMenuCheckBox	bump;
#endif
	HIMAGE		hTestImage;
};

/*
=================
CMenuVidOptions::GammaUpdate
=================
*/
void CMenuVidOptions::GammaUpdate( void )
{
	float val = RemapVal( gammaIntensity.GetCurrentValue(), 0.0, 1.0, 1.8, 7.0 );
	EngFuncs::CvarSetValue( "gamma", val );
	EngFuncs::ProcessImage( hTestImage, val );
}

void CMenuVidOptions::GammaGet( void )
{
	float val = EngFuncs::GetCvarFloat( "gamma" );

	gammaIntensity.SetCurrentValue( RemapVal( val, 1.8f, 7.0f, 0.0f, 1.0f ) );
	EngFuncs::ProcessImage( hTestImage, val );

	gammaIntensity.SetOriginalValue( val );
}

void CMenuVidOptions::SaveAndPopMenu( void )
{
	screenSize.WriteCvar();
	glareReduction.WriteCvar();
#ifndef MAINUI_PSP
	fastSky.WriteCvar();
	vbo.WriteCvar();
	bump.WriteCvar();
#endif
	// gamma is already written

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuVidOptions::Ownerdraw
=================
*/
void CMenuVidOptions::CMenuVidPreview::Draw( )
{
	int		color = 0xFFFF0000; // 255, 0, 0, 255
	int		viewport[4];
	int		viewsize, size, sb_lines;

	viewsize = EngFuncs::GetCvarFloat( "viewsize" );

	if( viewsize >= 120 )
		sb_lines = 0;	// no status bar at all
	else if( viewsize >= 110 )
		sb_lines = 24;	// no inventory
	else sb_lines = 48;

	size = Q_min( viewsize, 100 );

	viewport[2] = m_scSize.w * size / 100;
	viewport[3] = m_scSize.h * size / 100;

	if( viewport[3] > m_scSize.h - sb_lines )
		viewport[3] = m_scSize.h - sb_lines;
	if( viewport[3] > m_scSize.h )
		viewport[3] = m_scSize.h;

	viewport[2] &= ~7;
	viewport[3] &= ~1;

	viewport[0] = (m_scSize.w - viewport[2]) / 2;
	viewport[1] = (m_scSize.h - sb_lines - viewport[3]) / 2;

	UI_DrawPic( m_scPos.x + viewport[0], m_scPos.y + viewport[1], viewport[2], viewport[3], uiColorWhite, szPic );
	UI_DrawRectangleExt( m_scPos, m_scSize, color, ((CMenuVidOptions*)Parent())->outlineWidth );
}

/*
=================
CMenuVidOptions::Init
=================
*/
void CMenuVidOptions::_Init( void )
{
#ifdef PIC_KEEP_RGBDATA
	hTestImage = EngFuncs::PIC_Load( ART_GAMMA, PIC_KEEP_RGBDATA );
#else
	hTestImage = EngFuncs::PIC_Load( ART_GAMMA, PIC_KEEP_SOURCE );
#endif

	banner.SetPicture(ART_BANNER);

	testImage.iFlags = QMF_INACTIVE;
	testImage.SetRect( 390, 225, 480, 450 );
	testImage.SetPicture( ART_GAMMA );

	done.SetNameAndStatus( L( "GameUI_OK" ), L( "Go back to the Video Menu" ) );
	done.SetCoord( 72, 435 );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVidOptions::SaveAndPopMenu );

	screenSize.SetNameAndStatus( L( "Screen size" ), L( "Set the screen size" ) );
	screenSize.SetCoord( 72, 280 );
	screenSize.Setup( 30, 120, 10 );
	screenSize.onChanged = CMenuEditable::WriteCvarCb;

	gammaIntensity.SetNameAndStatus( L( "GameUI_Gamma" ), L( "Set gamma value (0.5 - 2.3)" ) );
	gammaIntensity.SetCoord( 72, 340 );
	gammaIntensity.Setup( 0.0, 1.0, 0.025 );
	gammaIntensity.onChanged = VoidCb( &CMenuVidOptions::GammaUpdate );
	gammaIntensity.onCvarGet = VoidCb( &CMenuVidOptions::GammaGet );

	glareReduction.SetCoord( 72, 400 );
	glareReduction.SetNameAndStatus( L( "GameUI_Brightness" ), L( "Set brightness level" ) );
	glareReduction.Setup( 0, 3, 0.1 );
#ifndef MAINUI_PSP
	bump.SetNameAndStatus( L( "Bump-mapping" ), L( "Enable bump mapping" ) );
	bump.SetCoord( 72, 515 );
	if( !EngFuncs::GetCvarFloat( "r_vbo" ) )
		bump.SetGrayed( true );

	vbo.SetNameAndStatus( L( "Use VBO" ), L( "Use new world renderer. Faster, but rarely glitchy" ) );
	vbo.SetCoord( 72, 565 );
	vbo.onChanged = CMenuCheckBox::BitMaskCb;
	vbo.onChanged.pExtra = &bump.iFlags;
	vbo.bInvertMask = true;
	vbo.iMask = QMF_GRAYED;

	fastSky.SetNameAndStatus( L( "Draw simple sky" ), L( "enable/disable fast sky rendering (for old computers)" ) );
	fastSky.SetCoord( 72, 615 );
#endif
	AddItem( background );
	AddItem( banner );
	AddItem( done );
	AddItem( screenSize );
	AddItem( gammaIntensity );
	AddItem( glareReduction );
#ifndef MAINUI_PSP
	if( UI_IsXashFWGS() )
	{
		AddItem( bump );
		AddItem( vbo );
	}
	AddItem( fastSky );
#endif
	AddItem( testImage );
	screenSize.LinkCvar( "viewsize" );
	gammaIntensity.LinkCvar( "gamma" );
	glareReduction.LinkCvar( "brightness" );
#ifndef MAINUI_PSP
	bump.LinkCvar( "r_bump" );
	vbo.LinkCvar( "r_vbo" );
	fastSky.LinkCvar( "r_fastsky" );
#endif

}

void CMenuVidOptions::_VidInit()
{
	outlineWidth = 2;
	UI_ScaleCoords( NULL, NULL, &outlineWidth, NULL );
}

ADD_MENU( menu_vidoptions, CMenuVidOptions, UI_VidOptions_Menu );
