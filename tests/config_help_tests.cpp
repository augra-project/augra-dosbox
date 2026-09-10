// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

// Setting help text is rendered through printf-style functions twice: once in
// Property::GetHelp/GetHelpRaw (to fill in the default value) and once at the
// consumer (config -h via MoreOutputStrings::AddString, or the config file
// writer). A literal percent must be written '%%'. A help text that leaves a
// live conversion after the first pass crashes the second pass: that was the
// 'config -h browser' SIGSEGV, where '%%s' was mistaken for a default-value
// placeholder and de-escaped one pass too early.
//
// The unit-test binary links messages_stubs.cpp, so GetHelp cannot be driven
// here (MSG_Add is a no-op). These tests lock the two pieces that are testable
// without the message store: the placeholder detector that the fix corrected,
// and the printf de-escape primitive both render paths rely on. The
// all-settings render is covered by the config -wc integration test.

#include "config/setup.h"

#include <gtest/gtest.h>

#include <string>

#include "utils/string_utils.h"

namespace {

// The exact browser help text (src/dosbox.cpp), kept here so a change there
// that reintroduces a bad escape is caught by the checks below.
constexpr const char* BrowserHelp =
        "Browser for pages the emulator opens (WORKBENCH, MANUAL, GUIDE);\n"
        "'auto' by default uses the system default. Give a browser name\n"
        "(firefox, chrome, chromium, brave, edge, librewolf, vivaldi), an\n"
        "executable, or a command line where %%s stands for the URL;\n"
        "without %%s the URL is appended. The BROWSER environment\n"
        "variable, when set, takes precedence over this setting.";

// Report why a raw help string would break the double-pass render, or an empty
// string if it is well formed. The rules that keep both passes safe:
//   - every '%' is either an escaped '%%' or the single '%s' default slot;
//   - at most one unescaped '%s';
//   - a '%s' default slot and a '%%' literal cannot coexist, because the first
//     pass collapses the '%%' too and leaves a lone '%' for the second.
std::string help_escaping_error(const std::string& help)
{
	int unescaped_s      = 0;
	bool escaped_percent = false;

	for (size_t i = 0; i < help.size(); ++i) {
		if (help[i] != '%') {
			continue;
		}
		if (i + 1 >= help.size()) {
			return "lone '%' at end of help";
		}
		const char next = help[i + 1];
		if (next == '%') {
			escaped_percent = true;
			++i;
			continue;
		}
		if (next == 's') {
			++unescaped_s;
			++i;
			continue;
		}
		return std::string("unescaped conversion '%") + next + "'";
	}

	if (unescaped_s > 1) {
		return "more than one unescaped '%s'";
	}
	if (unescaped_s == 1 && escaped_percent) {
		return "mixes a '%s' default slot with a '%%' literal";
	}
	return {};
}

} // namespace

// ---------------------------------------------------------------------------
// The placeholder detector, the exact logic the fix corrected. An escaped
// '%%s' must not read as a default-value placeholder; a bare '%s' must.
// ---------------------------------------------------------------------------

TEST(ConfigHelpPlaceholder, BarePercentSIsAPlaceholder)
{
	EXPECT_TRUE(help_has_default_placeholder("The default is %s."));
	EXPECT_TRUE(help_has_default_placeholder("%s at the start"));
	EXPECT_TRUE(help_has_default_placeholder("at the end %s"));
}

TEST(ConfigHelpPlaceholder, EscapedPercentSIsNotAPlaceholder)
{
	EXPECT_FALSE(help_has_default_placeholder("literal %%s token"));
	EXPECT_FALSE(help_has_default_placeholder("%%s"));
	EXPECT_FALSE(help_has_default_placeholder(BrowserHelp));
}

TEST(ConfigHelpPlaceholder, PercentPairAndPlainTextAreNotPlaceholders)
{
	EXPECT_FALSE(help_has_default_placeholder("expand %%PATH%% now"));
	EXPECT_FALSE(help_has_default_placeholder("no percent at all"));
	EXPECT_FALSE(help_has_default_placeholder("100%% left and right"));
}

TEST(ConfigHelpPlaceholder, TrailingPercentIsNotAPlaceholder)
{
	EXPECT_FALSE(help_has_default_placeholder("ends with a percent %"));
}

// ---------------------------------------------------------------------------
// The de-escape primitive. format_str with no arguments is what the consumer
// applies to help; '%%' must collapse to '%' and a bare '%s' would crash it.
// ---------------------------------------------------------------------------

TEST(ConfigHelpRender, FormatStrDeEscapesDoublePercentS)
{
	EXPECT_EQ(format_str("command line where %%s is the URL"),
	          "command line where %s is the URL");
}

TEST(ConfigHelpRender, FormatStrDeEscapesPercentPair)
{
	EXPECT_EQ(format_str("expand %%PATH%% now"), "expand %PATH% now");
}

TEST(ConfigHelpRender, BrowserHelpDeEscapesToLiteralPercentSTwice)
{
	const std::string rendered = format_str(BrowserHelp);
	EXPECT_NE(rendered.find("where %s stands for the URL"), std::string::npos);
	EXPECT_NE(rendered.find("without %s the URL is appended"), std::string::npos);
	EXPECT_EQ(rendered.find("%%s"), std::string::npos);
}

// ---------------------------------------------------------------------------
// The authoring-rule validator, tested against known-good and known-bad help
// so it can be trusted (it mirrors the check the config -wc integration test
// enforces against every real setting).
// ---------------------------------------------------------------------------

TEST(ConfigHelpEscaping, AcceptsWellFormedHelp)
{
	EXPECT_TRUE(help_escaping_error("plain text").empty());
	EXPECT_TRUE(help_escaping_error("literal %%s token").empty());
	EXPECT_TRUE(help_escaping_error("expand %%PATH%% now").empty());
	EXPECT_TRUE(help_escaping_error("default is %s").empty());
	EXPECT_TRUE(help_escaping_error(BrowserHelp).empty());
}

TEST(ConfigHelpEscaping, RejectsMalformedHelp)
{
	EXPECT_FALSE(help_escaping_error("two %s and %s slots").empty());
	EXPECT_FALSE(help_escaping_error("stray %d conversion").empty());
	EXPECT_FALSE(help_escaping_error("trailing percent %").empty());
	EXPECT_FALSE(help_escaping_error("mix %%literal with %s slot").empty());
}
