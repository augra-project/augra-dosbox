// This file is part of the dosbox-automation Project.
// License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
//

#include "dos/programs/docs_location.h"

#include <string>

#include <gtest/gtest.h>

#include "dosbox_config.h"

namespace {

TEST(DocsLocation, OnlineUrlUsesTheFullVersion)
{
	EXPECT_EQ(DocsLocation::OnlineUrl("introduction/about-this-manual/"),
	          std::string("https://dosbox-automation.org/") +
	                  DOSBOX_VERSION + "/introduction/about-this-manual/");
}

TEST(DocsLocation, OnlineUrlOfTheRootEndsWithTheVersion)
{
	EXPECT_EQ(DocsLocation::OnlineUrl(""),
	          std::string("https://dosbox-automation.org/") +
	                  DOSBOX_VERSION + "/");
}

TEST(DocsLocation, OnlineUrlNeverCarriesTheShortVersion)
{
	// The site has no /0.85/ tree, only /0.85.1/; the short form was
	// what MANUAL and GUIDE sent before and it answered 404.
	const auto url = DocsLocation::OnlineUrl("using-dosbox-automation/");
	EXPECT_EQ(url.find(std::string("/") + DOSBOX_VERSION_SHORT + "/"),
	          std::string::npos);
}

TEST(DocsLocation, BundledPathMirrorsTheSiteLayout)
{
	EXPECT_EQ(DocsLocation::BundledRelativePath("using-dosbox-automation/"),
	          std::string("docs/") + DOSBOX_VERSION +
	                  "/using-dosbox-automation/index.html");
	EXPECT_EQ(DocsLocation::BundledRelativePath(""),
	          std::string("docs/") + DOSBOX_VERSION + "/index.html");
}

} // namespace
