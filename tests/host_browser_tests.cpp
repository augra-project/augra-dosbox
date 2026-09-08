// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

#include "misc/host_browser.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#include "augra/log.h"

namespace {

using HostBrowser::Argv;

TEST(HostBrowser, SplitsOnWhitespace)
{
	EXPECT_EQ(HostBrowser::SplitCommand("brave-browser --new-window %s"),
	          (Argv{"brave-browser", "--new-window", "%s"}));
	EXPECT_EQ(HostBrowser::SplitCommand("  firefox\t-P  work "),
	          (Argv{"firefox", "-P", "work"}));
}

TEST(HostBrowser, DoubleQuotesGroupAWord)
{
	EXPECT_EQ(HostBrowser::SplitCommand(
	                  "\"C:\\Program Files\\Brave\\brave.exe\" --incognito %s"),
	          (Argv{"C:\\Program Files\\Brave\\brave.exe", "--incognito", "%s"}));
}

TEST(HostBrowser, EmptyInputGivesNoWords)
{
	EXPECT_TRUE(HostBrowser::SplitCommand("").empty());
	EXPECT_TRUE(HostBrowser::SplitCommand("   \t ").empty());
}

TEST(HostBrowser, UnterminatedQuoteTakesTheRest)
{
	EXPECT_EQ(HostBrowser::SplitCommand("\"open with spaces here"),
	          (Argv{"open with spaces here"}));
}

TEST(HostBrowser, BrowserNameExpandsToItsExecutables)
{
	const auto c = HostBrowser::Candidates("", "brave");
	ASSERT_EQ(c.size(), 2u);
	EXPECT_EQ(c[0], (Argv{"brave-browser"}));
	EXPECT_EQ(c[1], (Argv{"brave"}));
}

TEST(HostBrowser, NameLookupIgnoresCase)
{
	const auto c = HostBrowser::Candidates("", "Chromium");
	ASSERT_EQ(c.size(), 2u);
	EXPECT_EQ(c[0], (Argv{"chromium"}));
	EXPECT_EQ(c[1], (Argv{"chromium-browser"}));
}

TEST(HostBrowser, UnknownWordIsTheExecutable)
{
	const auto c = HostBrowser::Candidates("", "qutebrowser");
	ASSERT_EQ(c.size(), 1u);
	EXPECT_EQ(c[0], (Argv{"qutebrowser"}));
}

TEST(HostBrowser, CommandLineIsOneCandidateAsGiven)
{
	const auto c = HostBrowser::Candidates("", "brave-browser --new-window %s");
	ASSERT_EQ(c.size(), 1u);
	EXPECT_EQ(c[0], (Argv{"brave-browser", "--new-window", "%s"}));
}

TEST(HostBrowser, AutoAndEmptyContributeNothing)
{
	EXPECT_TRUE(HostBrowser::Candidates("", "auto").empty());
	EXPECT_TRUE(HostBrowser::Candidates("", "AUTO").empty());
	EXPECT_TRUE(HostBrowser::Candidates("", "").empty());
	EXPECT_TRUE(HostBrowser::Candidates("", "   ").empty());
}

TEST(HostBrowser, EnvEntriesComeBeforeTheSetting)
{
	const auto c = HostBrowser::Candidates("firefox:vivaldi %s", "brave");
	ASSERT_EQ(c.size(), 4u);
	EXPECT_EQ(c[0], (Argv{"firefox"}));
	EXPECT_EQ(c[1], (Argv{"vivaldi", "%s"}));
	EXPECT_EQ(c[2], (Argv{"brave-browser"}));
	EXPECT_EQ(c[3], (Argv{"brave"}));
}

TEST(HostBrowser, EnvListSkipsEmptyEntries)
{
	const auto c = HostBrowser::Candidates("::firefox::", "auto");
	ASSERT_EQ(c.size(), 1u);
	EXPECT_EQ(c[0], (Argv{"firefox"}));
}

TEST(HostBrowser, EnvEntryMayBeABrowserName)
{
	const auto c = HostBrowser::Candidates("edge", "auto");
	ASSERT_EQ(c.size(), 2u);
	EXPECT_EQ(c[0], (Argv{"microsoft-edge"}));
	EXPECT_EQ(c[1], (Argv{"microsoft-edge-stable"}));
}

TEST(HostBrowser, DriveLetterColonIsNotAListSeparator)
{
	const auto c = HostBrowser::Candidates("\"C:\\Program Files\\Brave\\brave.exe\" %s",
	                                       "auto");
	ASSERT_EQ(c.size(), 1u);
	EXPECT_EQ(c[0], (Argv{"C:\\Program Files\\Brave\\brave.exe", "%s"}));
}

TEST(HostBrowser, PlaceholderInsideAnArgumentTakesTheUrl)
{
	EXPECT_EQ(HostBrowser::BuildArgv({"x", "--url=%s"}, "http://127.0.0.1:8386/a"),
	          (Argv{"x", "--url=http://127.0.0.1:8386/a"}));
}

TEST(HostBrowser, UrlIsAppendedWhenNoPlaceholder)
{
	EXPECT_EQ(HostBrowser::BuildArgv({"firefox", "-P", "work"}, "http://h/p"),
	          (Argv{"firefox", "-P", "work", "http://h/p"}));
}

TEST(HostBrowser, PlaceholderReplacedOncePerArgument)
{
	// A URL containing %s must not be expanded again.
	EXPECT_EQ(HostBrowser::BuildArgv({"x", "%s"}, "http://h/?q=%s"),
	          (Argv{"x", "http://h/?q=%s"}));
	EXPECT_EQ(HostBrowser::BuildArgv({"x", "%s", "%s"}, "http://h/"),
	          (Argv{"x", "http://h/", "http://h/"}));
}

TEST(HostBrowser, UrlStaysOneArgumentWhateverItContains)
{
	const std::string hostile = "http://h/a b;c&&d$(e)`f`|g";
	const auto argv           = HostBrowser::BuildArgv({"x"}, hostile);
	ASSERT_EQ(argv.size(), 2u);
	EXPECT_EQ(argv[1], hostile);
}

using HostBrowser::Environment;

TEST(HostBrowser, AppImageVariablesAreRemoved)
{
	Environment env = {
	        {    "APPDIR",            "/mnt/app"},
	        {  "APPIMAGE", "/home/u/da.AppImage"},
	        {       "OWD",             "/home/u"},
	        {     "ARGV0",       "./da.AppImage"},
	        {"SHARUN_DIR",            "/mnt/app"},
	        {"LD_PRELOAD",  "libgamemodeauto.so"},
	        {      "HOME",             "/home/u"},
	        {      "PATH",            "/usr/bin"},
	};
	HostBrowser::CleanChildEnvironment(env);
	EXPECT_EQ(env,
	          (Environment{
	                  {"HOME",  "/home/u"},
                          {"PATH", "/usr/bin"}
        }));
}

TEST(HostBrowser, XdgDataDirsLosesTheAppImageAndNixEntries)
{
	Environment env = {
	        {       "APPDIR","/mnt/app"                                 },
	        {"XDG_DATA_DIRS",
	         "/mnt/app/share:/run/current-system/sw/share:"
	         "/run/opengl-driver/share:/usr/local/share:/usr/share"},
	};
	HostBrowser::CleanChildEnvironment(env);
	EXPECT_EQ(env.at("XDG_DATA_DIRS"), "/usr/local/share:/usr/share");
	EXPECT_FALSE(env.contains("APPDIR"));
}

TEST(HostBrowser, XdgDataDirsKeepsOrderOfTheRest)
{
	Environment env = {
	        {       "APPDIR",                "/mnt/app"},
	        {"XDG_DATA_DIRS", "/a:/mnt/app/share:/b:/c"},
	};
	HostBrowser::CleanChildEnvironment(env);
	EXPECT_EQ(env.at("XDG_DATA_DIRS"), "/a:/b:/c");
}

TEST(HostBrowser, EnvironmentWithoutAppImageIsUntouched)
{
	const Environment before = {
	        {         "HOME",                     "/home/u"},
	        {"XDG_DATA_DIRS", "/usr/local/share:/usr/share"},
	        {      "BROWSER",                       "brave"},
	};
	Environment env = before;
	HostBrowser::CleanChildEnvironment(env);
	EXPECT_EQ(env, before);
}

TEST(HostBrowser, EmptyXdgDataDirsStaysEmpty)
{
	Environment env = {
	        {       "APPDIR", "/mnt/app"},
                {"XDG_DATA_DIRS",         ""}
        };
	HostBrowser::CleanChildEnvironment(env);
	EXPECT_EQ(env.at("XDG_DATA_DIRS"), "");
}

TEST(HostBrowser, XdgDataDirsRemovedWhenNothingIsLeft)
{
	Environment env = {
	        {       "APPDIR",       "/mnt/app"},
                {"XDG_DATA_DIRS", "/mnt/app/share"}
        };
	HostBrowser::CleanChildEnvironment(env);
	EXPECT_FALSE(env.contains("XDG_DATA_DIRS"));
}

#if !defined(WIN32)
// Never let this reach SDL_OpenURL: on a desktop it starts the real
// browser. A BROWSER entry that exits at once proves the spawn path.
TEST(HostBrowser, OpenUrlSpawnsTheBrowserNamedInTheEnvironment)
{
	const char* previous    = std::getenv("BROWSER");
	const std::string saved = previous ? previous : "";
	setenv("BROWSER", "/bin/true", 1);

	std::vector<std::string> opened = {};
	augra::Logger::instance().add_sink([&](augra::LogLevel,
	                                       const char* component,
	                                       const std::string& message) {
		if (std::string_view(component) == "host_browser") {
			opened.push_back(message);
		}
	});

	EXPECT_TRUE(HostBrowser::OpenUrl("http://127.0.0.1:1/"));
	augra::Logger::instance().clear_sinks();

	ASSERT_EQ(opened.size(), 1u);
	EXPECT_EQ(opened[0], "opened http://127.0.0.1:1/ with /bin/true");

	if (previous) {
		setenv("BROWSER", saved.c_str(), 1);
	} else {
		unsetenv("BROWSER");
	}
}
#endif

} // namespace
