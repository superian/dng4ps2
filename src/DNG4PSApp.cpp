/*=============================================================================
  DNG4PS - software for converting Canon PS RAW files to DNG ones
  Copyright (C) 2007-2008 Denis Artyomov

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  =============================================================================
  File:      DNG4PSApp.cpp
  Author(s): Denis Artyomov (denis.artyomov@gmail.com)
  ===========================================================================*/

  /* Command line operation

  dng4ps-2 [-d <directory] [-p0 | -p1 | -p2] [-c | -u] input-filename...

  If any parameter is present, the program will run in command line.

  <input-filename> is the FULL path to one or more raw files to be processed. At least one is mandatory
  -d directory  Base output directory path
  -p0           No preview
  -p1           Medium preview
  -p2           Full preview
  -c            Compressed SNG file
  -u            Uncompressed DNG file

  All other parameters are taken from the users dng4ps-2 settings as set in the GUI
  If an option is not specified, then the default is take from the users dng4ps2 settings as set in the GUI

  Note: Where possible using the same parameter spec as Adobe DNG Converter are used
  http://www.adobe.com/products/dng/pdfs/dng_commandline.pdf

  TODO: Better error handling and logging, other parameters available in the settings dialog

  */

#include "pch.h"
#include <wx/filename.h>
#include <wx/log.h>

#include "DNG4PSApp.h"
#include "ProgramObjects.h"
#include "Options.h"
#include "CameraOpts.h"

//(*AppHeaders
#include "DNG4PSMain.h"
#include <wx/image.h>
//*)


//for command wx line parser
#include <wx/cmdline.h>
#include <iostream>
#include <iterator>
// -----------------
using namespace std;


IMPLEMENT_APP(DNG4PSApp);

wxIcon main_icon;
wxString exe_dir, path_sep;

int num_args;

bool DNG4PSApp::OnInit()
{
	 srand((unsigned)time(NULL));

#if defined(__WIN32__)
	main_icon.LoadFile(L"main_icon", wxBITMAP_TYPE_ICO_RESOURCE);
	path_sep = L"\\";
#else
	path_sep = L"/";
#endif

	exe_dir = wxFileName(argv[0]).GetPath();
	num_args = argc;

	wxFileName lang_path(exe_dir, L"langs");

	sys().options->load();
	sys().init_language(lang_path.GetFullPath());

	sys().cameras->load();
	sys().cameras->set_defaults(false);


	//(*AppInitialize
	bool wxsOK = true;
	wxInitAllImageHandlers();
	if ( wxsOK )
	{
        DNG4PSFrame* Frame = new DNG4PSFrame(0);
        if (num_args > 1) //Are we running the command line version?
        {

            wxCmdLineParser parser(argc, argv);
            wxString output_dir = _T("");
            wxArrayString files;

            wxLog *logger=new wxLogStderr();
            wxLog::SetActiveTarget(logger);

            static const wxCmdLineEntryDesc cmdLineDesc[] =
            {
                { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("Display this help message"),
                wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
                { wxCMD_LINE_OPTION, _T("d"), _T("output"),   _T("Base output directory") },
                { wxCMD_LINE_SWITCH, _T("p0"), _T("output"),  _T("No preview") },
                { wxCMD_LINE_SWITCH, _T("p1"), _T("output"),  _T("Medium Preview") },
                { wxCMD_LINE_SWITCH, _T("p2"), _T("output"),  _T("Full preview") },
                { wxCMD_LINE_SWITCH, _T("u"), _T("output"),   _T("Uncompressed DNG file") },
                { wxCMD_LINE_SWITCH, _T("c"), _T("output"),   _T("Compressed DNG file") },
                { wxCMD_LINE_PARAM,  NULL, NULL, _T("input files"),
                    wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE },
                { wxCMD_LINE_NONE }
            };

            parser.DisableLongOptions();
            parser.SetDesc(cmdLineDesc);
            if( parser.Parse() == 0 )
            {
                files.clear();

                wxString s = _T("DNG4PS-2 command line mode:\nInput files: ");  //message string

                size_t count = parser.GetParamCount();
                if (count > 0) // must be at least one filename
                {
                    for ( size_t param = 0; param < count; param++ )
                    {
                        files.Add(parser.GetParam(param));
                        s << parser.GetParam(param) << ' ';
                    }
                    s << '\n';

                    if ( parser.Found(_T("d"), &output_dir) )
                    {
                        s << _T("Output dir:\t") << output_dir << '\n';
                    }

                    if ( parser.Found(_T("p0")))
                    {
                        sys().options->preview_type=pt_None;
                    } else if ( parser.Found(_T("p1")))
                    {
                        sys().options->preview_type=pt_Medium;
                    } else if ( parser.Found(_T("p2")))
                    {
                        sys().options->preview_type=pt_Full;
                    }

                    if ( parser.Found(_T("u")))
                    {
                        sys().options->compress_dng=false;
                    } else if ( parser.Found(_T("c")))
                    {
                        sys().options->compress_dng=true;
                    }

                    wxLogMessage(s);
                    Frame->convertFiles(files, output_dir);
                }
                else
                {
                    wxsOK = false;
                }
            }
            else
            {
                wxsOK = false;
            }
            Frame->Destroy(); //get out of here
        }
        else //otherwise use GUI version
        {
            Frame->Show();
            SetTopWindow(Frame);
        }
	}
	//*)

	return wxsOK;
}
