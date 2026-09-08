// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

#include "dos/programs/docs_location.h"

#include "dosbox_config.h"

// The site publishes one tree per full release version, never per
// major.minor, so DOSBOX_VERSION_SHORT has no page behind it.
std::string DocsLocation::OnlineUrl(const std::string_view page)
{
	return std::string("https://dosbox-automation.org/") + DOSBOX_VERSION +
	       "/" + std::string(page);
}

std::string DocsLocation::BundledRelativePath(const std::string_view page)
{
	return std::string("docs/") + DOSBOX_VERSION + "/" + std::string(page) +
	       "index.html";
}
