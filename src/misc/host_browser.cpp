// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

#include "misc/host_browser.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include <SDL3/SDL.h>

#include "augra/log.h"
#include "config/config.h"
#include "config/setup.h"
#include "utils/checks.h"
#include "utils/env_utils.h"

CHECK_NARROWING();

namespace {

struct BrowserName {
	std::string_view name;
	std::vector<std::string_view> executables;
};

const std::vector<BrowserName> known_browsers = {
        {  "firefox",                                         {"firefox"}},
        {   "chrome", {"google-chrome", "google-chrome-stable", "chrome"}},
        { "chromium",                    {"chromium", "chromium-browser"}},
        {    "brave",                          {"brave-browser", "brave"}},
        {     "edge",         {"microsoft-edge", "microsoft-edge-stable"}},
        {"librewolf",                                       {"librewolf"}},
        {  "vivaldi",                       {"vivaldi", "vivaldi-stable"}},
};

std::string ToLower(const std::string_view text)
{
	std::string lower(text);
	std::ranges::transform(lower, lower.begin(), [](const unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return lower;
}

void AppendEntry(const std::string_view entry, std::vector<HostBrowser::Argv>& out)
{
	const auto words = HostBrowser::SplitCommand(entry);
	if (words.empty()) {
		return;
	}
	if (words.size() == 1) {
		const auto lower = ToLower(words[0]);
		for (const auto& known : known_browsers) {
			if (known.name == lower) {
				for (const auto exe : known.executables) {
					out.push_back({std::string(exe)});
				}
				return;
			}
		}
	}
	out.push_back(words);
}

// BROWSER is colon-separated; a colon right after a single leading
// letter (quoted or not) is a Windows drive, not a separator.
std::vector<std::string> SplitBrowserList(const std::string_view list)
{
	std::vector<std::string> entries = {};
	std::string current              = {};
	for (const char c : list) {
		const auto stem = current.starts_with('"') ? current.substr(1)
		                                           : current;
		const bool drive_colon = (c == ':' && stem.size() == 1 &&
		                          std::isalpha(static_cast<unsigned char>(
		                                  stem[0])));
		if (c == ':' && !drive_colon) {
			entries.push_back(current);
			current.clear();
			continue;
		}
		current.push_back(c);
	}
	entries.push_back(current);
	return entries;
}

SDL_Environment* MakeChildEnvironment()
{
	HostBrowser::Environment vars = {};

	auto* current = SDL_CreateEnvironment(true);
	if (current) {
		if (char** list = SDL_GetEnvironmentVariables(current)) {
			for (char** p = list; *p; ++p) {
				const std::string_view kv(*p);
				const auto eq = kv.find('=');
				if (eq == std::string_view::npos) {
					continue;
				}
				vars[std::string(kv.substr(0, eq))] = std::string(
				        kv.substr(eq + 1));
			}
			SDL_free(list);
		}
		SDL_DestroyEnvironment(current);
	}

	HostBrowser::CleanChildEnvironment(vars);

	auto* env = SDL_CreateEnvironment(false);
	for (const auto& [name, value] : vars) {
		SDL_SetEnvironmentVariable(env, name.c_str(), value.c_str(), true);
	}
	return env;
}

bool Spawn(const HostBrowser::Argv& argv)
{
	if (argv.empty()) {
		return false;
	}
	std::vector<char*> args = {};
	args.reserve(argv.size() + 1);
	for (const auto& arg : argv) {
		args.push_back(const_cast<char*>(arg.c_str()));
	}
	args.push_back(nullptr);

	auto* env        = MakeChildEnvironment();
	const auto props = SDL_CreateProperties();
	SDL_SetPointerProperty(props,
	                       SDL_PROP_PROCESS_CREATE_ARGS_POINTER,
	                       args.data());
	SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ENVIRONMENT_POINTER, env);
	SDL_SetBooleanProperty(props, SDL_PROP_PROCESS_CREATE_BACKGROUND_BOOLEAN, true);

	auto* process = SDL_CreateProcessWithProperties(props);
	SDL_DestroyProperties(props);
	SDL_DestroyEnvironment(env);
	if (!process) {
		return false;
	}
	// The handle is ours to drop; the browser keeps running.
	SDL_DestroyProcess(process);
	return true;
}

std::string ConfiguredBrowser()
{
	if (!control) {
		return "auto";
	}
	const auto* section = static_cast<const SectionProp*>(
	        control->GetSection("dosbox"));
	if (!section) {
		return "auto";
	}
	return section->GetString("browser");
}

} // namespace

HostBrowser::Argv HostBrowser::SplitCommand(const std::string_view command)
{
	Argv words          = {};
	std::string current = {};
	bool in_word        = false;
	bool in_quotes      = false;

	for (const char c : command) {
		if (in_quotes) {
			if (c == '"') {
				in_quotes = false;
			} else {
				current.push_back(c);
			}
			continue;
		}
		if (c == '"') {
			in_quotes = true;
			in_word   = true;
			continue;
		}
		if (c == ' ' || c == '\t') {
			if (in_word) {
				words.push_back(current);
				current.clear();
				in_word = false;
			}
			continue;
		}
		current.push_back(c);
		in_word = true;
	}
	if (in_word) {
		words.push_back(current);
	}
	return words;
}

std::vector<HostBrowser::Argv> HostBrowser::Candidates(
        const std::string_view env_browser, const std::string_view conf_browser)
{
	std::vector<Argv> out = {};
	for (const auto& entry : SplitBrowserList(env_browser)) {
		AppendEntry(entry, out);
	}
	const auto conf_words = SplitCommand(conf_browser);
	if (!conf_words.empty() &&
	    !(conf_words.size() == 1 && ToLower(conf_words[0]) == "auto")) {
		AppendEntry(conf_browser, out);
	}
	return out;
}

HostBrowser::Argv HostBrowser::BuildArgv(const Argv& tmpl, const std::string_view url)
{
	Argv argv   = {};
	bool placed = false;
	for (const auto& arg : tmpl) {
		const auto pos = arg.find("%s");
		if (pos == std::string::npos) {
			argv.push_back(arg);
			continue;
		}
		std::string filled = arg;
		filled.replace(pos, 2, url);
		argv.push_back(filled);
		placed = true;
	}
	if (!placed) {
		argv.emplace_back(url);
	}
	return argv;
}

void HostBrowser::CleanChildEnvironment(Environment& env)
{
	const auto appdir = env.contains("APPDIR") ? env.at("APPDIR")
	                                           : std::string{};

	for (const auto* name :
	     {"APPDIR", "APPIMAGE", "OWD", "ARGV0", "SHARUN_DIR", "LD_PRELOAD"}) {
		env.erase(name);
	}

	if (appdir.empty() || !env.contains("XDG_DATA_DIRS")) {
		return;
	}
	const auto& dirs = env.at("XDG_DATA_DIRS");
	if (dirs.empty()) {
		return;
	}

	// sharun prepends its share dir and two NixOS paths and keeps no
	// copy of the original list.
	const std::vector<std::string> dropped = {
	        appdir + "/share",
	        "/run/current-system/sw/share",
	        "/run/opengl-driver/share",
	};

	std::string kept  = {};
	std::string entry = {};
	auto flush        = [&]() {
                if (!entry.empty() &&
                    std::ranges::find(dropped, entry) == dropped.end()) {
                        if (!kept.empty()) {
                                kept.push_back(':');
                        }
                        kept += entry;
                }
                entry.clear();
	};
	for (const char c : dirs) {
		if (c == ':') {
			flush();
		} else {
			entry.push_back(c);
		}
	}
	flush();

	if (kept.empty()) {
		env.erase("XDG_DATA_DIRS");
	} else {
		env["XDG_DATA_DIRS"] = kept;
	}
}

bool HostBrowser::OpenUrl(const std::string_view url)
{
	const auto env_browser  = get_env_var("BROWSER");
	const auto conf_browser = ConfiguredBrowser();

	for (const auto& tmpl : Candidates(env_browser, conf_browser)) {
		const auto argv = BuildArgv(tmpl, url);
		if (Spawn(argv)) {
			augra::log_info("host_browser",
			                "opened %s with %s",
			                std::string(url).c_str(),
			                argv.front().c_str());
			return true;
		}
		augra::log_warn("host_browser",
		                "could not start %s: %s",
		                argv.front().c_str(),
		                SDL_GetError());
	}
	return SDL_OpenURL(std::string(url).c_str());
}
