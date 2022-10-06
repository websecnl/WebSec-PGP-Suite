#pragma once

#include <wx/txtstrm.h>
#include <wx/sstream.h>
#include <wx/protocol/http.h>

namespace suite::ver
{
    inline bool compare(const wxString& a, const wxString& b)
    {
        return a == b;
    }

    inline static wxString retrieve_version()
    {
        wxString res;
        wxHTTP get;
        get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
        get.SetTimeout(2);
        long seconds_passed{};

        while (!get.Connect(_T("www.example.com")) && seconds_passed < get.GetTimeout())
        {
            wxSleep(1);
            seconds_passed++;
        }

        wxApp::IsMainLoopRunning();

        wxInputStream* httpStream = get.GetInputStream(_T("/"));

        if (get.GetError() == wxPROTO_NOERR)
        {
            wxStringOutputStream out_stream(&res);
            httpStream->Read(out_stream);
        }

        wxDELETE(httpStream);
        get.Close();

        return res;
    }

    inline static wxString get_local_version()
    {
        wxFileInputStream fstream(_("version.txt"));
        wxTextInputStream input( fstream );
        return input.ReadLine();
    }
}