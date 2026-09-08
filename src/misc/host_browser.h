// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

#ifndef DOSBOX_MISC_HOST_BROWSER_H
#define DOSBOX_MISC_HOST_BROWSER_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace HostBrowser {

using Argv        = std::vector<std::string>;
using Environment = std::map<std::string, std::string>;

Argv SplitCommand(std::string_view command);

std::vector<Argv> Candidates(std::string_view env_browser,
                             std::string_view conf_browser);

Argv BuildArgv(const Argv& tmpl, std::string_view url);

void CleanChildEnvironment(Environment& env);

bool OpenUrl(std::string_view url);

} // namespace HostBrowser

#endif
