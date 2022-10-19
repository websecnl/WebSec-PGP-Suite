#pragma once

#include "Networks.h"

#include <wx/txtstrm.h>
#include <wx/sstream.h>
#include <wx/protocol/http.h>

namespace suite::ver
{
    inline bool compare(const wxString& a, const wxString& b)
    {
        return a == b;
    }

    /* Will write a proper parser once it is required to receive more info from server, for now enjoy this monstrosity */
    inline std::string parse_version(const std::string& response)
    {
        auto start = response.find(PGPSUITE_SIG);

        if (response.npos == start) return "";

        start = response.find('=', start);
        auto end = response.find(';', start);

        if (start == response.npos || end == response.npos) return "";

        return response.substr(start + 1, end - start - 1);
    }

    /* Retrieves newest version from network location and caches it */
    inline std::string retrieve_version()
    {
        static std::string cached_version{};
        if (!cached_version.empty()) 
            return cached_version;

        const auto response = net::get_network_data();
        cached_version = parse_version(response);

        if (cached_version.empty())
        {
            wxMessageBox(_("Could not reach host, check your internet connection."), _("Error."), wxICON_ERROR);
            return "";
        }

        return cached_version;
    }

    /* Retrieves local version and caches it */
    inline wxString get_local_version()
    {
        static wxString cached_version{};
        if (!cached_version.empty())
            return cached_version;

        wxFileInputStream fstream(_("version.txt"));
        
        if (!fstream.IsOk())
        {
            wxMessageBox(_("Could not find version file, does version.txt exist?"), _("Error."), wxICON_ERROR);
            return "";
        }
        
        wxTextInputStream input( fstream );

        return cached_version = input.ReadLine();
    }

    /* Checks if the local version is up-to-date, otherwise it displays a dialogue giving the latest version
    @return If local version is up-to-date */
    inline bool verify_local_version()
    {
        const auto local_version = get_local_version();
        const auto latest_version = retrieve_version();

        const auto res = compare(local_version, latest_version);

        if (!res)
        {
            wxMessageBox(_("Current version: ") + local_version + _("\nNew version: ") + latest_version, _("New version available."));
        }

        return res;
    }
}