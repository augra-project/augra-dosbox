// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

#ifndef DOSBOX_PROGRAM_DOCS_LOCATION_H
#define DOSBOX_PROGRAM_DOCS_LOCATION_H

#include <string>
#include <string_view>

// Where the MANUAL and GUIDE programs send the browser. A page is a
// path below the versioned manual root, with its trailing slash
// ("introduction/about-this-manual/"); empty means the root.
namespace DocsLocation {

std::string OnlineUrl(std::string_view page);

std::string BundledRelativePath(std::string_view page);

} // namespace DocsLocation

#endif
