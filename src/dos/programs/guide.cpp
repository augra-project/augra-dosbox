// SPDX-FileCopyrightText:  2026-2026 The DOSBox Staging Team
// SPDX-FileCopyrightText:  2026 dosbox-automation Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "guide.h"

#include <filesystem>

#include "dos/programs/docs_location.h"
#include "misc/host_browser.h"
#include "misc/support.h"
#include "more_output.h"
#include "utils/checks.h"
#include "utils/string_utils.h"

CHECK_NARROWING();

void GUIDE::Run(void)
{
	// Print usage
	if (HelpRequested()) {
		MoreOutputStrings output(*this);
		output.AddString(MSG_Get("PROGRAM_GUIDE_HELP"));
		output.AddString("\n");
		output.AddString(MSG_Get("PROGRAM_GUIDE_HELP_LONG"));
		output.Display();
		return;
	}

	const auto path = get_resource_path(
	        DocsLocation::BundledRelativePath("using-dosbox-automation/"));

	std::string url = {};
	if (std::filesystem::exists(path)) {
		url = std::string{"file://"} + path.string();
	} else {
		url = DocsLocation::OnlineUrl("using-dosbox-automation/");
	}
	if (!HostBrowser::OpenUrl(url)) {
		WriteOut("Could not open a browser. Set the BROWSER environment\n"
		         "variable or the 'browser' setting in the [dosbox] section.\n");
	}
}

void GUIDE::AddMessages()
{
	MSG_Add("PROGRAM_GUIDE_HELP",
	        "Open the dosbox-automation Getting Started guide in the default browser.\n");

	MSG_Add("PROGRAM_GUIDE_HELP_LONG",
	        "Usage:\n"
	        "  [color=light-green]guide[reset]\n"
	        "\n"
	        "Notes:\n"
	        "  - This will open a local offline copy of the Getting Started\n"
	        "    guide bundled with your dosbox-automation installation; you\n"
	        "    don't need an internet connection to read the bundled guide.\n"
	        "\n"
	        "  - The offline documentation is located in the 'docs' subfolder of your\n"
	        "    dosbox-automation 'resources' folder.\n"
	        "\n"
	        "  - If the offline documentation cannot be found, the command will open the\n"
	        "    guide on the project website (requires internet connection).");
}
