///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/textmeasurecmn.cpp
// Purpose:     wxTextMeasureBase implementation
// Author:      Manuel Martin
// Created:     2012-10-05
// RCS-ID:      $Id:
// Copyright:   (c) 1997-2012 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/window.h"
#endif //WX_PRECOMP

#include "wx/private/textmeasure.h"

// ============================================================================
// wxTextMeasureBase implementation
// ============================================================================

wxTextMeasureBase::wxTextMeasureBase(const wxDC *dc, const wxFont *theFont)
    : m_dc(dc),
      m_win(NULL),
      m_font(theFont)
{
    wxASSERT_MSG( dc, wxS("wxTextMeasure needs a valid wxDC") );

    // By default, use wxDC version, we'll explicitly reset this to false in
    // the derived classes if the DC is of native variety.
    m_useDCImpl = true;
}

wxTextMeasureBase::wxTextMeasureBase(const wxWindow *win, const wxFont *theFont)
    : m_dc(NULL),
      m_win(win),
      m_font(theFont)
{
    wxASSERT_MSG( win, wxS("wxTextMeasure needs a valid wxWindow") );

    // We don't have any wxDC so we can't forward to it.
    m_useDCImpl = false;
}

wxFont wxTextMeasureBase::GetFont() const
{
    return m_font ? *m_font
                  : m_win ? m_win->GetFont()
                          : m_dc->GetFont();
}

void wxTextMeasureBase::CallGetTextExtent(const wxString& string,
                                          wxCoord *width,
                                          wxCoord *height,
                                          wxCoord *descent,
                                          wxCoord *externalLeading)
{
    if ( m_useDCImpl )
        m_dc->GetTextExtent(string, width, height, descent, externalLeading);
    else
        DoGetTextExtent(string, width, height, descent, externalLeading);
}

void wxTextMeasureBase::GetTextExtent(const wxString& string,
                                      wxCoord *width,
                                      wxCoord *height,
                                      wxCoord *descent,
                                      wxCoord *externalLeading)
{
    // To make the code simpler, make sure that the width and height pointers
    // are always valid, even if they point to dummy variables.
    int unusedWidth, unusedHeight;
    if ( !width )
        width = &unusedWidth;
    if ( !height )
        height = &unusedHeight;

    if ( string.empty() )
    {
        *width =
        *height = 0;

        return;
    }

    MeasuringGuard guard(*this);

    CallGetTextExtent(string, width, height, descent, externalLeading);
}

void wxTextMeasureBase::GetMultiLineTextExtent(const wxString& text,
                                               wxCoord *width,
                                               wxCoord *height,
                                               wxCoord *heightOneLine)
{
    MeasuringGuard guard(*this);

    wxCoord widthTextMax = 0, widthLine,
            heightTextTotal = 0, heightLineDefault = 0, heightLine = 0;

    wxString curLine;
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == wxS('\n') )
        {
            if ( curLine.empty() )
            {
                // we can't use GetTextExtent - it will return 0 for both width
                // and height and an empty line should count in height
                // calculation

                // assume that this line has the same height as the previous
                // one
                if ( !heightLineDefault )
                    heightLineDefault = heightLine;

                if ( !heightLineDefault )
                {
                    // but we don't know it yet - choose something reasonable
                    int dummy;
                    CallGetTextExtent(wxS("W"), &dummy, &heightLineDefault);
                }

                heightTextTotal += heightLineDefault;
            }
            else
            {
                CallGetTextExtent(curLine, &widthLine, &heightLine);
                if ( widthLine > widthTextMax )
                    widthTextMax = widthLine;
                heightTextTotal += heightLine;
            }

            if ( pc == text.end() )
            {
               break;
            }
            else // '\n'
            {
               curLine.clear();
            }
        }
        else
        {
            curLine += *pc;
        }
    }

    if ( width )
        *width = widthTextMax;
    if ( height )
        *height = heightTextTotal;
    if ( heightOneLine )
        *heightOneLine = heightLine;
}

wxSize wxTextMeasureBase::GetLargestStringExtent(size_t n,
                                                 const wxString* strings)
{
    MeasuringGuard guard(*this);

    wxCoord w, h, widthMax = 0, heightMax = 0;
    for ( size_t i = 0; i < n; ++i )
    {
        CallGetTextExtent(strings[i], &w, &h);

        if ( w > widthMax )
            widthMax = w;
        if ( h > heightMax )
            heightMax = h;
    }

    return wxSize(widthMax, heightMax);
}

bool wxTextMeasureBase::GetPartialTextExtents(const wxString& text,
                                              wxArrayInt& widths,
                                              double scaleX)
{
    widths.Empty();
    if ( text.empty() )
        return true;

    MeasuringGuard guard(*this);

    widths.Add(0, text.length());

    return DoGetPartialTextExtents(text, widths, scaleX);
}
