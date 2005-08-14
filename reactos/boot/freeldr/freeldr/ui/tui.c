/*
 *  FreeLoader
 *  Copyright (C) 1998-2003  Brian Palmer  <brianp@sginet.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <freeldr.h>
#include <ui.h>
#include "tui.h"
#include "keycodes.h"
#include <rtl.h>
#include <mm.h>
#include <debug.h>
#include <inifile.h>
#include <version.h>
#include <video.h>
#include <machine.h>


PVOID	TextVideoBuffer = NULL;

BOOL TuiInitialize(VOID)
{
	MachVideoClearScreen(ATTR(COLOR_WHITE, COLOR_BLACK));
	MachVideoHideShowTextCursor(FALSE);

	TextVideoBuffer = VideoAllocateOffScreenBuffer();
	if (TextVideoBuffer == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

VOID TuiUnInitialize(VOID)
{
	if (UiUseSpecialEffects)
	{
		TuiFadeOut();
	}
	else
	{
		MachVideoSetDisplayMode(NULL, FALSE);
	}

	//VideoClearScreen();
	MachVideoHideShowTextCursor(TRUE);
}

VOID TuiDrawBackdrop(VOID)
{
	//
	// Fill in the background (excluding title box & status bar)
	//
	TuiFillArea(0,
			TUI_TITLE_BOX_CHAR_HEIGHT,
			UiScreenWidth - 1,
			UiScreenHeight - 2,
			UiBackdropFillStyle,
			ATTR(UiBackdropFgColor, UiBackdropBgColor));

	//
	// Draw the title box
	//
	TuiDrawBox(0,
			0,
			UiScreenWidth - 1,
			TUI_TITLE_BOX_CHAR_HEIGHT - 1,
			D_VERT,
			D_HORZ,
			TRUE,
			FALSE,
			ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));

	//
	// Draw version text
	//
	TuiDrawText(2,
			1,
			GetFreeLoaderVersionString(),
			ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));

	//
	// Draw copyright
	//
	TuiDrawText(2,
			2,
			BY_AUTHOR,
			ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));
	TuiDrawText(2,
			3,
			AUTHOR_EMAIL,
			ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));

	//
	// Draw help text
	//
	TuiDrawText(UiScreenWidth - 16, 3, /*"F1 for Help"*/"F8 for Options", ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));

	//
	// Draw title text
	//
	TuiDrawText( (UiScreenWidth / 2) - (strlen(UiTitleBoxTitleText) / 2),
			2,
			UiTitleBoxTitleText,
			ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));

	//
	// Draw status bar
	//
	TuiDrawStatusText("Welcome to FreeLoader!");

	//
	// Update the date & time
	//
	TuiUpdateDateTime();

	VideoCopyOffScreenBufferToVRAM();
}

/*
 * FillArea()
 * This function assumes coordinates are zero-based
 */
VOID TuiFillArea(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, CHAR FillChar, UCHAR Attr /* Color Attributes */)
{
	PUCHAR	ScreenMemory = (PUCHAR)TextVideoBuffer;
	ULONG		i, j;

	// Clip the area to the screen
	// FIXME: This code seems to have problems... Uncomment and view ;-)
	/*if ((Left >= UiScreenWidth) || (Top >= UiScreenHeight))
	{
		return;
	}
	if ((Left + Right) >= UiScreenWidth)
	{
		Right = UiScreenWidth - Left;
	}
	if ((Top + Bottom) >= UiScreenHeight)
	{
		Bottom = UiScreenHeight - Top;
	}*/

	// Loop through each line and fill it in
	for (i=Top; i<=Bottom; i++)
	{
		// Loop through each character (column) in the line and fill it in
		for (j=Left; j<=Right; j++)
		{
			ScreenMemory[((i*2)*UiScreenWidth)+(j*2)] = (UCHAR)FillChar;
			ScreenMemory[((i*2)*UiScreenWidth)+(j*2)+1] = Attr;
		}
	}
}

/*
 * DrawShadow()
 * This function assumes coordinates are zero-based
 */
VOID TuiDrawShadow(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom)
{
	PUCHAR	ScreenMemory = (PUCHAR)TextVideoBuffer;
	ULONG		Idx;

	// Shade the bottom of the area
	if (Bottom < (UiScreenHeight - 1))
	{
		if (UiScreenHeight < 34)
		{
			Idx=Left + 2;
		}
		else
		{
			Idx=Left + 1;
		}

		for (; Idx<=Right; Idx++)
		{
			ScreenMemory[(((Bottom+1)*2)*UiScreenWidth)+(Idx*2)+1] = ATTR(COLOR_GRAY, COLOR_BLACK);
		}
	}

	// Shade the right of the area
	if (Right < (UiScreenWidth - 1))
	{
		for (Idx=Top+1; Idx<=Bottom; Idx++)
		{
			ScreenMemory[((Idx*2)*UiScreenWidth)+((Right+1)*2)+1] = ATTR(COLOR_GRAY, COLOR_BLACK);
		}
	}
	if (UiScreenHeight < 34)
	{
		if ((Right + 1) < (UiScreenWidth - 1))
		{
			for (Idx=Top+1; Idx<=Bottom; Idx++)
			{
				ScreenMemory[((Idx*2)*UiScreenWidth)+((Right+2)*2)+1] = ATTR(COLOR_GRAY, COLOR_BLACK);
			}
		}
	}

	// Shade the bottom right corner
	if ((Right < (UiScreenWidth - 1)) && (Bottom < (UiScreenHeight - 1)))
	{
		ScreenMemory[(((Bottom+1)*2)*UiScreenWidth)+((Right+1)*2)+1] = ATTR(COLOR_GRAY, COLOR_BLACK);
	}
	if (UiScreenHeight < 34)
	{
		if (((Right + 1) < (UiScreenWidth - 1)) && (Bottom < (UiScreenHeight - 1)))
		{
			ScreenMemory[(((Bottom+1)*2)*UiScreenWidth)+((Right+2)*2)+1] = ATTR(COLOR_GRAY, COLOR_BLACK);
		}
	}
}

/*
 * DrawBox()
 * This function assumes coordinates are zero-based
 */
VOID TuiDrawBox(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, UCHAR VertStyle, UCHAR HorzStyle, BOOL Fill, BOOL Shadow, UCHAR Attr)
{
	UCHAR	ULCorner, URCorner, LLCorner, LRCorner;

	// Calculate the corner values
	if (HorzStyle == HORZ)
	{
		if (VertStyle == VERT)
		{
			ULCorner = UL;
			URCorner = UR;
			LLCorner = LL;
			LRCorner = LR;
		}
		else // VertStyle == D_VERT
		{
			ULCorner = VD_UL;
			URCorner = VD_UR;
			LLCorner = VD_LL;
			LRCorner = VD_LR;
		}
	}
	else // HorzStyle == D_HORZ
	{
		if (VertStyle == VERT)
		{
			ULCorner = HD_UL;
			URCorner = HD_UR;
			LLCorner = HD_LL;
			LRCorner = HD_LR;
		}
		else // VertStyle == D_VERT
		{
			ULCorner = D_UL;
			URCorner = D_UR;
			LLCorner = D_LL;
			LRCorner = D_LR;
		}
	}

	// Fill in box background
	if (Fill)
	{
		TuiFillArea(Left, Top, Right, Bottom, ' ', Attr);
	}

	// Fill in corners
	TuiFillArea(Left, Top, Left, Top, ULCorner, Attr);
	TuiFillArea(Right, Top, Right, Top, URCorner, Attr);
	TuiFillArea(Left, Bottom, Left, Bottom, LLCorner, Attr);
	TuiFillArea(Right, Bottom, Right, Bottom, LRCorner, Attr);

	// Fill in left line
	TuiFillArea(Left, Top+1, Left, Bottom-1, VertStyle, Attr);
	// Fill in top line
	TuiFillArea(Left+1, Top, Right-1, Top, HorzStyle, Attr);
	// Fill in right line
	TuiFillArea(Right, Top+1, Right, Bottom-1, VertStyle, Attr);
	// Fill in bottom line
	TuiFillArea(Left+1, Bottom, Right-1, Bottom, HorzStyle, Attr);

	// Draw the shadow
	if (Shadow)
	{
		TuiDrawShadow(Left, Top, Right, Bottom);
	}
}

/*
 * DrawText()
 * This function assumes coordinates are zero-based
 */
VOID TuiDrawText(ULONG X, ULONG Y, PCHAR Text, UCHAR Attr)
{
	PUCHAR	ScreenMemory = (PUCHAR)TextVideoBuffer;
	ULONG		i, j;

	// Draw the text
	for (i=X, j=0; Text[j]  && i<UiScreenWidth; i++,j++)
	{
		ScreenMemory[((Y*2)*UiScreenWidth)+(i*2)] = (UCHAR)Text[j];
		ScreenMemory[((Y*2)*UiScreenWidth)+(i*2)+1] = Attr;
	}
}

VOID TuiDrawCenteredText(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, PCHAR TextString, UCHAR Attr)
{
	ULONG		TextLength;
	ULONG		BoxWidth;
	ULONG		BoxHeight;
	ULONG		LineBreakCount;
	ULONG		Index;
	ULONG		LastIndex;
	ULONG		RealLeft;
	ULONG		RealTop;
	ULONG		X;
	ULONG		Y;
	CHAR	Temp[2];

	TextLength = strlen(TextString);

	// Count the new lines and the box width
	LineBreakCount = 0;
	BoxWidth = 0;
	LastIndex = 0;
	for (Index=0; Index<TextLength; Index++)
	{
		if (TextString[Index] == '\n')
		{
			LastIndex = Index;
			LineBreakCount++;
		}
		else
		{
			if ((Index - LastIndex) > BoxWidth)
			{
				BoxWidth = (Index - LastIndex);
			}
		}
	}

	BoxHeight = LineBreakCount + 1;

	RealLeft = (((Right - Left) - BoxWidth) / 2) + Left;
	RealTop = (((Bottom - Top) - BoxHeight) / 2) + Top;

	LastIndex = 0;
	for (Index=0; Index<TextLength; Index++)
	{
		if (TextString[Index] == '\n')
		{
			RealTop++;
			LastIndex = 0;
		}
		else
		{
			X = RealLeft + LastIndex;
			Y = RealTop;
			LastIndex++;
			Temp[0] = TextString[Index];
			Temp[1] = 0;
			TuiDrawText(X, Y, Temp, Attr);
		}
	}
}

VOID TuiDrawStatusText(PCHAR StatusText)
{
	ULONG		i;

	TuiDrawText(0, UiScreenHeight-1, " ", ATTR(UiStatusBarFgColor, UiStatusBarBgColor));
	TuiDrawText(1, UiScreenHeight-1, StatusText, ATTR(UiStatusBarFgColor, UiStatusBarBgColor));

	for (i=strlen(StatusText)+1; i<UiScreenWidth; i++)
	{
		TuiDrawText(i, UiScreenHeight-1, " ", ATTR(UiStatusBarFgColor, UiStatusBarBgColor));
	}

	VideoCopyOffScreenBufferToVRAM();
}

VOID TuiUpdateDateTime(VOID)
{
	ULONG	Year, Month, Day;
	ULONG	Hour, Minute, Second;
	CHAR	DateString[40];
	CHAR	TimeString[40];
	CHAR	TempString[20];
	BOOL	PMHour = FALSE;

	MachRTCGetCurrentDateTime(&Year, &Month, &Day, &Hour, &Minute, &Second);
	// Get the month name
	strcpy(DateString, UiMonthNames[Month - 1]);
	// Get the day
	itoa(Day, TempString, 10);
	// Get the day postfix
	if (1 == Day || 21 == Day || 31 == Day)
	{
		strcat(TempString, "st");
	}
	else if (2 == Day || 22 == Day)
	{
		strcat(TempString, "nd");
	}
	else if (3 == Day || 23 == Day)
	{
		strcat(TempString, "rd");
	}
	else
	{
		strcat(TempString, "th");
	}

	// Add the day to the date
	strcat(DateString, TempString);
	strcat(DateString, " ");

	// Get the year and add it to the date
	itoa(Year, TempString, 10);
	strcat(DateString, TempString);

	// Draw the date
	TuiDrawText(UiScreenWidth-strlen(DateString)-2, 1, DateString, ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));

	// Get the hour and change from 24-hour mode to 12-hour
	if (Hour > 12)
	{
		Hour -= 12;
		PMHour = TRUE;
	}
	if (Hour == 0)
	{
		Hour = 12;
	}
	itoa(Hour, TempString, 10);
	strcpy(TimeString, "    ");
	strcat(TimeString, TempString);
	strcat(TimeString, ":");
	itoa(Minute, TempString, 10);
	if (Minute < 10)
	{
		strcat(TimeString, "0");
	}
	strcat(TimeString, TempString);
	strcat(TimeString, ":");
	itoa(Second, TempString, 10);
	if (Second < 10)
	{
		strcat(TimeString, "0");
	}
	strcat(TimeString, TempString);
	if (PMHour)
	{
		strcat(TimeString, " PM");
	}
	else
	{
		strcat(TimeString, " AM");
	}

	// Draw the time
	TuiDrawText(UiScreenWidth-strlen(TimeString)-2, 2, TimeString, ATTR(UiTitleBoxFgColor, UiTitleBoxBgColor));
}

VOID TuiSaveScreen(PUCHAR Buffer)
{
	PUCHAR	ScreenMemory = (PUCHAR)TextVideoBuffer;
	ULONG		i;

	for (i=0; i < (UiScreenWidth * UiScreenHeight * 2); i++)
	{
		Buffer[i] = ScreenMemory[i];
	}
}

VOID TuiRestoreScreen(PUCHAR Buffer)
{
	PUCHAR	ScreenMemory = (PUCHAR)TextVideoBuffer;
	ULONG		i;

	for (i=0; i < (UiScreenWidth * UiScreenHeight * 2); i++)
	{
		ScreenMemory[i] = Buffer[i];
	}
}

VOID TuiMessageBox(PCHAR MessageText)
{
	PVOID	ScreenBuffer;

	// Save the screen contents
	ScreenBuffer = MmAllocateMemory(UiScreenWidth * UiScreenHeight * 2);
	TuiSaveScreen(ScreenBuffer);

	// Display the message box
	TuiMessageBoxCritical(MessageText);

	// Restore the screen contents
	TuiRestoreScreen(ScreenBuffer);
	MmFreeMemory(ScreenBuffer);
}

VOID TuiMessageBoxCritical(PCHAR MessageText)
{
	int		width = 8;
	unsigned int	height = 1;
	int		curline = 0;
	int		k;
	size_t		i , j;
	int		x1, x2, y1, y2;
	char	temp[260];
	char	key;

	// Find the height
	for (i=0; i<strlen(MessageText); i++)
	{
		if (MessageText[i] == '\n')
			height++;
	}

	// Find the width
	for (i=0,j=0,k=0; i<height; i++)
	{
		while ((MessageText[j] != '\n') && (MessageText[j] != 0))
		{
			j++;
			k++;
		}

		if (k > width)
			width = k;

		k = 0;
		j++;
	}

	// Calculate box area
	x1 = (UiScreenWidth - (width+2))/2;
	x2 = x1 + width + 3;
	y1 = ((UiScreenHeight - height - 2)/2) + 1;
	y2 = y1 + height + 4;

	// Draw the box
	TuiDrawBox(x1, y1, x2, y2, D_VERT, D_HORZ, TRUE, TRUE, ATTR(UiMessageBoxFgColor, UiMessageBoxBgColor));

	// Draw the text
	for (i=0,j=0; i<strlen(MessageText)+1; i++)
	{
		if ((MessageText[i] == '\n') || (MessageText[i] == 0))
		{
			temp[j] = 0;
			j = 0;
			UiDrawText(x1+2, y1+1+curline, temp, ATTR(UiMessageBoxFgColor, UiMessageBoxBgColor));
			curline++;
		}
		else
			temp[j++] = MessageText[i];
	}

	// Draw OK button
	strcpy(temp, "   OK   ");
	UiDrawText(x1+((x2-x1)/2)-3, y2-2, temp, ATTR(COLOR_BLACK, COLOR_GRAY));

	// Draw status text
	UiDrawStatusText("Press ENTER to continue");

	VideoCopyOffScreenBufferToVRAM();

	for (;;)
	{
		if (MachConsKbHit())
		{
			key = MachConsGetCh();
			if(key == KEY_EXTENDED)
				key = MachConsGetCh();

			if(key == KEY_ENTER)
				break;
			else if(key == KEY_SPACE)
				break;
			else if(key == KEY_ESC)
				break;
		}

		TuiUpdateDateTime();

		VideoCopyOffScreenBufferToVRAM();
	}

}


VOID TuiDrawProgressBarCenter(ULONG Position, ULONG Range, PCHAR ProgressText)
{
	ULONG		Left, Top, Right, Bottom;
	ULONG		Width = 50; // Allow for 50 "bars"
	ULONG		Height = 2;

	Left = (UiScreenWidth - Width - 4) / 2;
	Right = Left + Width + 3;
	Top = (UiScreenHeight - Height - 2) / 2;
	Top += 2;
	Bottom = Top + Height + 1;

	TuiDrawProgressBar(Left, Top, Right, Bottom, Position, Range, ProgressText);
}

VOID TuiDrawProgressBar(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, ULONG Position, ULONG Range, PCHAR ProgressText)
{
	ULONG		i;
	ULONG		ProgressBarWidth = (Right - Left) - 3;

	// First make sure the progress bar text fits
	UiTruncateStringEllipsis(ProgressText, ProgressBarWidth - 4);

	if (Position > Range)
	{
		Position = Range;
	}

	// Draw the box
	TuiDrawBox(Left, Top, Right, Bottom, VERT, HORZ, TRUE, TRUE, ATTR(UiMenuFgColor, UiMenuBgColor));

	// Draw the "Loading..." text
	//TuiDrawText(70/2, Top+1, "Loading...", ATTR(UiTextColor, UiMenuBgColor));
	TuiDrawCenteredText(Left + 2, Top + 2, Right - 2, Top + 2, ProgressText, ATTR(UiTextColor, UiMenuBgColor));

	// Draw the percent complete
	for (i=0; i<(Position*ProgressBarWidth)/Range; i++)
	{
		TuiDrawText(Left+2+i, Top+2, "\xDB", ATTR(UiTextColor, UiMenuBgColor));
	}

	// Draw the rest
	for (; i<ProgressBarWidth; i++)
	{
		TuiDrawText(Left+2+i, Top+2, "\xB2", ATTR(UiTextColor, UiMenuBgColor));
	}

	TuiUpdateDateTime();

	VideoCopyOffScreenBufferToVRAM();
}

UCHAR TuiTextToColor(PCHAR ColorText)
{
	if (stricmp(ColorText, "Black") == 0)
		return COLOR_BLACK;
	else if (stricmp(ColorText, "Blue") == 0)
		return COLOR_BLUE;
	else if (stricmp(ColorText, "Green") == 0)
		return COLOR_GREEN;
	else if (stricmp(ColorText, "Cyan") == 0)
		return COLOR_CYAN;
	else if (stricmp(ColorText, "Red") == 0)
		return COLOR_RED;
	else if (stricmp(ColorText, "Magenta") == 0)
		return COLOR_MAGENTA;
	else if (stricmp(ColorText, "Brown") == 0)
		return COLOR_BROWN;
	else if (stricmp(ColorText, "Gray") == 0)
		return COLOR_GRAY;
	else if (stricmp(ColorText, "DarkGray") == 0)
		return COLOR_DARKGRAY;
	else if (stricmp(ColorText, "LightBlue") == 0)
		return COLOR_LIGHTBLUE;
	else if (stricmp(ColorText, "LightGreen") == 0)
		return COLOR_LIGHTGREEN;
	else if (stricmp(ColorText, "LightCyan") == 0)
		return COLOR_LIGHTCYAN;
	else if (stricmp(ColorText, "LightRed") == 0)
		return COLOR_LIGHTRED;
	else if (stricmp(ColorText, "LightMagenta") == 0)
		return COLOR_LIGHTMAGENTA;
	else if (stricmp(ColorText, "Yellow") == 0)
		return COLOR_YELLOW;
	else if (stricmp(ColorText, "White") == 0)
		return COLOR_WHITE;

	return COLOR_BLACK;
}

UCHAR TuiTextToFillStyle(PCHAR FillStyleText)
{
	if (stricmp(FillStyleText, "Light") == 0)
	{
		return LIGHT_FILL;
	}
	else if (stricmp(FillStyleText, "Medium") == 0)
	{
		return MEDIUM_FILL;
	}
	else if (stricmp(FillStyleText, "Dark") == 0)
	{
		return DARK_FILL;
	}

	return LIGHT_FILL;
}

VOID TuiFadeInBackdrop(VOID)
{
	PPALETTE_ENTRY TuiFadePalette = NULL;

	if (UiUseSpecialEffects && ! MachVideoIsPaletteFixed())
	{
		TuiFadePalette = (PPALETTE_ENTRY)MmAllocateMemory(sizeof(PALETTE_ENTRY) * 64);

		if (TuiFadePalette != NULL)
		{
			VideoSavePaletteState(TuiFadePalette, 64);
			VideoSetAllColorsToBlack(64);
		}
	}

	// Draw the backdrop and title box
	TuiDrawBackdrop();

	if (UiUseSpecialEffects && ! MachVideoIsPaletteFixed() && TuiFadePalette != NULL)
	{
		VideoFadeIn(TuiFadePalette, 64);
		MmFreeMemory(TuiFadePalette);
	}
}

VOID TuiFadeOut(VOID)
{
	PPALETTE_ENTRY TuiFadePalette = NULL;

	if (UiUseSpecialEffects && ! MachVideoIsPaletteFixed())
	{
		TuiFadePalette = (PPALETTE_ENTRY)MmAllocateMemory(sizeof(PALETTE_ENTRY) * 64);

		if (TuiFadePalette != NULL)
		{
			VideoSavePaletteState(TuiFadePalette, 64);
		}
	}

	if (UiUseSpecialEffects && ! MachVideoIsPaletteFixed() && TuiFadePalette != NULL)
	{
		VideoFadeOut(64);
	}

	MachVideoSetDisplayMode(NULL, FALSE);

	if (UiUseSpecialEffects && ! MachVideoIsPaletteFixed() && TuiFadePalette != NULL)
	{
		VideoRestorePaletteState(TuiFadePalette, 64);
		MmFreeMemory(TuiFadePalette);
	}

}

BOOL TuiEditBox(PCHAR MessageText, PCHAR EditTextBuffer, ULONG Length)
{
	int		width = 8;
	unsigned int	height = 1;
	int		curline = 0;
	int		k;
	size_t		i , j;
	int		x1, x2, y1, y2;
	char	temp[260];
	char	key;
	int		EditBoxLine;
	ULONG		EditBoxStartX, EditBoxEndX;
	int		EditBoxCursorX;
	unsigned int	EditBoxTextCount;
	int		EditBoxTextDisplayIndex;
	BOOL	ReturnCode;
	PVOID	ScreenBuffer;

	// Save the screen contents
	ScreenBuffer = MmAllocateMemory(UiScreenWidth * UiScreenHeight * 2);
	TuiSaveScreen(ScreenBuffer);

	// Find the height
	for (i=0; i<strlen(MessageText); i++)
	{
		if (MessageText[i] == '\n')
			height++;
	}

	// Find the width
	for (i=0,j=0,k=0; i<height; i++)
	{
		while ((MessageText[j] != '\n') && (MessageText[j] != 0))
		{
			j++;
			k++;
		}

		if (k > width)
			width = k;

		k = 0;
		j++;
	}

	// Calculate box area
	x1 = (UiScreenWidth - (width+2))/2;
	x2 = x1 + width + 3;
	y1 = ((UiScreenHeight - height - 2)/2) + 1;
	y2 = y1 + height + 4;

	// Draw the box
	TuiDrawBox(x1, y1, x2, y2, D_VERT, D_HORZ, TRUE, TRUE, ATTR(UiMessageBoxFgColor, UiMessageBoxBgColor));

	// Draw the text
	for (i=0,j=0; i<strlen(MessageText)+1; i++)
	{
		if ((MessageText[i] == '\n') || (MessageText[i] == 0))
		{
			temp[j] = 0;
			j = 0;
			UiDrawText(x1+2, y1+1+curline, temp, ATTR(UiMessageBoxFgColor, UiMessageBoxBgColor));
			curline++;
		}
		else
			temp[j++] = MessageText[i];
	}

	EditBoxTextCount = 0;
	EditBoxLine = y2 - 2;
	EditBoxStartX = x1 + 3;
	EditBoxEndX = x2 - 3;
	UiFillArea(EditBoxStartX, EditBoxLine, EditBoxEndX, EditBoxLine, ' ', ATTR(UiEditBoxTextColor, UiEditBoxBgColor));

	// Show the cursor
	EditBoxCursorX = EditBoxStartX;
	MachVideoSetTextCursorPosition(EditBoxCursorX, EditBoxLine);
	MachVideoHideShowTextCursor(TRUE);

	// Draw status text
	UiDrawStatusText("Press ENTER to continue, or ESC to cancel");

	VideoCopyOffScreenBufferToVRAM();

	for (;;)
	{
		if (MachConsKbHit())
		{
			key = MachConsGetCh();
			if(key == KEY_EXTENDED)
			{
				key = MachConsGetCh();
			}

			if(key == KEY_ENTER)
			{
				ReturnCode = TRUE;
				break;
			}
			else if(key == KEY_ESC)
			{
				ReturnCode = FALSE;
				break;
			}
			else if (key == KEY_BACKSPACE) // Remove a character
			{
				if (EditBoxTextCount)
				{
					EditBoxTextCount--;
					EditTextBuffer[EditBoxTextCount] = 0;
				}
				else
				{
					beep();
				}
			}
			else // Add this key to the buffer
			{
				if (EditBoxTextCount < Length - 1)
				{
					EditTextBuffer[EditBoxTextCount] = key;
					EditBoxTextCount++;
					EditTextBuffer[EditBoxTextCount] = 0;
				}
				else
				{
					beep();
				}
			}
		}

		// Draw the edit box background
		UiFillArea(EditBoxStartX, EditBoxLine, EditBoxEndX, EditBoxLine, ' ', ATTR(UiEditBoxTextColor, UiEditBoxBgColor));

		// Fill the text in
		if (EditBoxTextCount > (EditBoxEndX - EditBoxStartX))
		{
			EditBoxTextDisplayIndex = EditBoxTextCount - (EditBoxEndX - EditBoxStartX);
			EditBoxCursorX = EditBoxEndX;
		}
		else
		{
			EditBoxTextDisplayIndex = 0;
			EditBoxCursorX = EditBoxStartX + EditBoxTextCount;
		}
		UiDrawText(EditBoxStartX, EditBoxLine, &EditTextBuffer[EditBoxTextDisplayIndex], ATTR(UiEditBoxTextColor, UiEditBoxBgColor));

		// Move the cursor
		MachVideoSetTextCursorPosition(EditBoxCursorX, EditBoxLine);

		TuiUpdateDateTime();

		VideoCopyOffScreenBufferToVRAM();
	}

	// Hide the cursor again
	MachVideoHideShowTextCursor(FALSE);

	// Restore the screen contents
	TuiRestoreScreen(ScreenBuffer);
	MmFreeMemory(ScreenBuffer);

	return ReturnCode;
}
