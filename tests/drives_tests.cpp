// SPDX-FileCopyrightText:  2020-2025 The DOSBox Staging Team
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dos/drives.h"

#include <gtest/gtest.h>

#include <string>

std::string run_Set_Label(char const * const input, bool cdrom) {
    char output[32] = { 0 };
    Set_Label(input, output, cdrom);
    std::cout << "Set_Label " << "CD-ROM? " << (cdrom ? 'y' : 'n') << \
        " Input: " << input << " Output: " << output << '\n';
    return std::string(output);
}

namespace {

TEST(wild_file_cmp, wild_match)
{
    EXPECT_EQ(true, wild_match("TEST", "*"));
    EXPECT_EQ(true, wild_match("TEST", "T*"));
    EXPECT_EQ(true, wild_match("TEST", "T*T"));
    EXPECT_EQ(true, wild_match("TEST", "TES?"));
    EXPECT_EQ(false, wild_match("TEST LONG NAME", "TEST*long*"));
    EXPECT_EQ(false, wild_match("TEST LONG NAME", "*NONE*"));
    EXPECT_EQ(true, wild_match("TEST LONG LONG NAME", "*LONG?NAME"));
    EXPECT_EQ(true, wild_match("TEST LONG LONG NAME", "*LONG*LONG*"));
    EXPECT_EQ(false, wild_match("TEST LONG LONG NAME", "*LONGLONG*"));
    EXPECT_EQ(false, wild_match("TEST", "Z*"));
}

TEST(wild_file_cmp, ExactMatch)
{
	EXPECT_EQ(true, wild_file_cmp("", ""));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "TEST.EXE"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "TEST"));
	EXPECT_EQ(false, wild_file_cmp("TEST.EXE", ".EXE"));
	EXPECT_EQ(true, wild_file_cmp(".EXE", ".EXE"));
}

TEST(wild_file_cmp, WildDotWild)
{
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "*.*"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "*.*"));
	EXPECT_EQ(true, wild_file_cmp(".EXE", "*.*"));
}

TEST(wild_file_cmp, WildcardNoExt)
{
	EXPECT_EQ(false, wild_file_cmp("TEST.EXE", "*"));
	EXPECT_EQ(false, wild_file_cmp(".EXE", "*"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "*"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "T*"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "*Y*"));
	EXPECT_EQ(false, wild_file_cmp("TEST", "Z*"));
}

TEST(wild_file_cmp, QuestionMark)
{
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "?EST.EXE"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "?EST"));
	EXPECT_EQ(false, wild_file_cmp("TEST", "???Z"));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "TEST.???"));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "TEST.?XE"));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "???T.EXE"));
	EXPECT_EQ(true, wild_file_cmp("TEST", "???T.???"));
}

TEST(wild_file_cmp, LongCompare)
{
	EXPECT_EQ(false, wild_file_cmp("TEST", "", true));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "*", true));
	EXPECT_EQ(true, wild_file_cmp("TEST", "?EST", true));
	EXPECT_EQ(false, wild_file_cmp("TEST", "???Z", true));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "T*T.*", true));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "T*T.?X?", true));
	EXPECT_EQ(true, wild_file_cmp("TEST.EXE", "T??T.E*E", true));
	EXPECT_EQ(true, wild_file_cmp("Test.exe", "*ST.E*", true));
	EXPECT_EQ(true, wild_file_cmp("Test long name", "*NAME", true));
	EXPECT_EQ(true, wild_file_cmp("Test long name", "*T*L*M*", true));
	EXPECT_EQ(true, wild_file_cmp("Test long name.txt", "T*long*.T??", true));
	EXPECT_EQ(true, wild_file_cmp("Test long name.txt", "??st*name.*t", true));
	EXPECT_EQ(true,
	          wild_file_cmp("Test long name.txt", "Test?long?????.*t", true));
	EXPECT_EQ(true,
	          wild_file_cmp("Test long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long.txt",
	                        "Test*long.???",
	                        true));
	EXPECT_EQ(true,
	          wild_file_cmp("Test long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long.txt",
	                        "Test long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long.txt",
	                        true));
	EXPECT_EQ(false,
	          wild_file_cmp("Test long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long.txt",
	                        "Test long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long.txt",
	                        true));
	EXPECT_EQ(false, wild_file_cmp("TEST", "Z*", true));
	EXPECT_EQ(false, wild_file_cmp("TEST FILE NAME", "*Y*", true));
	EXPECT_EQ(false, wild_file_cmp("TEST FILE NAME", "*F*X*", true));
}

TEST(generate_8x3, SFNTest)
{
	EXPECT_EQ("TESTLO~1", generate_8x3("test long name...", 1, true));
	EXPECT_EQ("TESTLO~2.TXT", generate_8x3("test long name.txt", 2, true));
	EXPECT_EQ("TESTL~20.TEX", generate_8x3("test long name.txt.text", 20, true));
	EXPECT_EQ("TEST__~2.TEX", generate_8x3("test[ ]long name.text", 2, true));
	EXPECT_EQ("TEST_~20", generate_8x3("... test[ ]long name ...", 20, true));
	EXPECT_EQ("TEST~200.TT", generate_8x3("test long name.txt.tt", 200, true));
	EXPECT_EQ("TES~2000.TXT", generate_8x3("test[]long name.txt", 2000, true));
	EXPECT_EQ("TE~20000.EXT", generate_8x3("test long long name.txt..ext..", 20000, true));
	EXPECT_EQ("T~200000.TXT", generate_8x3("test long long name.txt", 200000, true));
	EXPECT_EQ("", generate_8x3("test long long name.txt", 2000000, true));
}

TEST(filename_not_8x3, NameTest)
{
	EXPECT_FALSE(filename_not_8x3("testfile.txt"));
	EXPECT_TRUE(filename_not_8x3("test_file.txt"));
	EXPECT_FALSE(filename_not_8x3("myfile.t"));
	EXPECT_TRUE(filename_not_8x3("my file.txt"));
	EXPECT_TRUE(filename_not_8x3("my+file.txt"));
	EXPECT_TRUE(filename_not_8x3("myfile.text"));
	EXPECT_TRUE(filename_not_8x3("myfile..txt"));
}

/**
 * Set_Labels tests. These test the conversion of a FAT/CD-ROM volume
 * label to an MS-DOS 8.3 label with a variety of edge cases & oddities.
 */
TEST(Set_Label, Daggerfall)
{
    std::string output = run_Set_Label("Daggerfall", false);
    EXPECT_EQ("DAGGERFA.LL", output);
}
TEST(Set_Label, DaggerfallCD)
{
    std::string output = run_Set_Label("Daggerfall", true);
    EXPECT_EQ("Daggerfa.ll", output);
}

TEST(Set_Label, LongerThan11)
{
    std::string output = run_Set_Label("a123456789AAA", false);
    EXPECT_EQ("A1234567.89A", output);
}
TEST(Set_Label, LongerThan11CD)
{
    std::string output = run_Set_Label("a123456789AAA", true);
    EXPECT_EQ("a1234567.89A", output);
}

TEST(Set_Label, ShorterThan8)
{
    std::string output = run_Set_Label("a123456", false);
    EXPECT_EQ("A123456", output);
}
TEST(Set_Label, ShorterThan8CD)
{
    std::string output = run_Set_Label("a123456", true);
    EXPECT_EQ("a123456", output);
}

// Tests that the CD-ROM version adds a trailing dot when
// input is 8 chars plus one dot (9 chars total)
TEST(Set_Label, EqualTo8)
{
    std::string output = run_Set_Label("a1234567", false);
    EXPECT_EQ("A1234567", output);
}
TEST(Set_Label, EqualTo8CD)
{
    std::string output = run_Set_Label("a1234567", true);
    EXPECT_EQ("a1234567.", output);
}

// A test to ensure non-CD-ROM function strips trailing dot
TEST(Set_Label, StripEndingDot)
{
    std::string output = run_Set_Label("a1234567.", false);
    EXPECT_EQ("A1234567", output);
}
TEST(Set_Label, NoStripEndingDotCD)
{
    std::string output = run_Set_Label("a1234567.", true);
    EXPECT_EQ("a1234567.", output);
}

// Just to make sure this function doesn't clean invalid DOS labels
TEST(Set_Label, InvalidCharsEndingDot)
{
    std::string output = run_Set_Label("?*':&@(..", false);
    EXPECT_EQ("?*':&@(.", output);
}
TEST(Set_Label, InvalidCharsEndingDotCD)
{
    std::string output = run_Set_Label("?*':&@(..", true);
    EXPECT_EQ("?*':&@(..", output);
}

TEST(generate_8x3, stem_is_identical_across_sequential_calls)
{
	auto r1 = generate_8x3("test long name.txt", 1);
	auto r2 = generate_8x3("test long name.txt", 2);
	auto r3 = generate_8x3("test long name.txt", 3);
	EXPECT_EQ("TESTLO~1.TXT", r1);
	EXPECT_EQ("TESTLO~2.TXT", r2);
	EXPECT_EQ("TESTLO~3.TXT", r3);
}

TEST(sfn_clean_basis, replaces_bracket_with_underscore)
{
	auto r = sfn_clean_basis("[test]");
	EXPECT_STREQ(r.name, "_TEST_");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_semicolon)
{
	auto r = sfn_clean_basis("game;v2");
	EXPECT_STREQ(r.name, "GAME_V2");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_plus_sign)
{
	auto r = sfn_clean_basis("A+B");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_equals)
{
	auto r = sfn_clean_basis("file=1");
	EXPECT_STREQ(r.name, "FILE_1");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_double_quote)
{
	auto r = sfn_clean_basis("a\"b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_backslash)
{
	auto r = sfn_clean_basis("a\\b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_forward_slash)
{
	auto r = sfn_clean_basis("a/b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_colon)
{
	auto r = sfn_clean_basis("a:b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_angle_brackets)
{
	auto r = sfn_clean_basis("a<b>c");
	EXPECT_STREQ(r.name, "A_B_C");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_pipe)
{
	auto r = sfn_clean_basis("a|b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_question_mark)
{
	auto r = sfn_clean_basis("a?b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_asterisk)
{
	auto r = sfn_clean_basis("a*b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_comma)
{
	auto r = sfn_clean_basis("a,b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_del_0x7f)
{
	auto r = sfn_clean_basis("a\x7f""b");
	EXPECT_STREQ(r.name, "A_B");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, replaces_control_chars)
{
	for (char c = 1; c < 0x20; ++c) {
		std::string input = std::string("A") + c + "B";
		auto r = sfn_clean_basis(input.c_str());
		EXPECT_STREQ(r.name, "A_B") << "control char 0x" << std::hex << (int)(unsigned char)c;
		EXPECT_TRUE(r.lossy) << "control char 0x" << std::hex << (int)(unsigned char)c;
	}
}

TEST(sfn_clean_basis, passes_high_bytes_unchanged)
{
	for (int c = 0x80; c <= 0xFF; ++c) {
		std::string input = std::string("A") + (char)c + "B";
		auto r = sfn_clean_basis(input.c_str());
		std::string expected = std::string("A") + (char)c + "B";
		EXPECT_STREQ(r.name, expected.c_str())
			<< "byte 0x" << std::hex << c << " should pass through";
		EXPECT_FALSE(r.lossy) << "byte 0x" << std::hex << c;
	}
}

TEST(sfn_clean_basis, utf8_cafe_passes_through)
{
	auto r = sfn_clean_basis("caf\xc3\xa9");
	EXPECT_STREQ(r.name, "CAF\xc3\xa9");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, combination_of_illegal_chars)
{
	auto r = sfn_clean_basis("a[b]c+d;");
	EXPECT_STREQ(r.name, "A_B_C_D_");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}


TEST(sfn_clean_basis, strips_leading_periods)
{
	auto r = sfn_clean_basis("...test");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, dot_git_becomes_GIT)
{
	auto r = sfn_clean_basis(".git");
	EXPECT_STREQ(r.name, "GIT");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, strips_trailing_periods)
{
	auto r = sfn_clean_basis("test...");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, strips_leading_spaces)
{
	auto r = sfn_clean_basis("  test");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, strips_trailing_spaces)
{
	auto r = sfn_clean_basis("test  ");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, strips_embedded_spaces)
{
	auto r = sfn_clean_basis("te st");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, space_separated_name_and_ext)
{
	auto r = sfn_clean_basis("a b.c d");
	EXPECT_STREQ(r.name, "AB");
	EXPECT_STREQ(r.ext, "CD");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}


TEST(sfn_clean_basis, multiple_dots_split_rule)
{
	auto r = sfn_clean_basis("A.B.C.D");
	EXPECT_STREQ(r.name, "A");
	EXPECT_STREQ(r.ext, "D");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, prettybg_big)
{
	auto r = sfn_clean_basis("prettybg.big");
	EXPECT_STREQ(r.name, "PRETTYBG");
	EXPECT_STREQ(r.ext, "BIG");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, the_quick_brown_fox)
{
	auto r = sfn_clean_basis("The quick brown.fox");
	EXPECT_STREQ(r.name, "THEQUICK");
	EXPECT_STREQ(r.ext, "FOX");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, name_with_dots_ext)
{
	auto r = sfn_clean_basis("name.with.dots");
	EXPECT_STREQ(r.name, "NAME");
	EXPECT_STREQ(r.ext, "DOT");
	EXPECT_FALSE(r.lossy);
}


TEST(sfn_clean_basis, exactly_8_char_name)
{
	auto r = sfn_clean_basis("ABCDEFGH.TXT");
	EXPECT_STREQ(r.name, "ABCDEFGH");
	EXPECT_STREQ(r.ext, "TXT");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, 9_char_name_truncated)
{
	auto r = sfn_clean_basis("ABCDEFGHI.TXT");
	EXPECT_STREQ(r.name, "ABCDEFGH");
	EXPECT_STREQ(r.ext, "TXT");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, no_extension)
{
	auto r = sfn_clean_basis("TESTFILE");
	EXPECT_STREQ(r.name, "TESTFILE");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, extension_longer_than_3)
{
	auto r = sfn_clean_basis("TEST.TEXT");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_STREQ(r.ext, "TEX");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}


TEST(sfn_clean_basis, empty_string_becomes_underscore)
{
	auto r = sfn_clean_basis("");
	EXPECT_STREQ(r.name, "_");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, only_periods_becomes_underscore)
{
	auto r = sfn_clean_basis("...");
	EXPECT_STREQ(r.name, "_");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, only_spaces_becomes_underscore)
{
	auto r = sfn_clean_basis("   ");
	EXPECT_STREQ(r.name, "_");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, single_dot_passthrough)
{
	auto r = sfn_clean_basis(".");
	EXPECT_STREQ(r.name, ".");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, double_dot_passthrough)
{
	auto r = sfn_clean_basis("..");
	EXPECT_STREQ(r.name, "..");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, single_char)
{
	auto r = sfn_clean_basis("A");
	EXPECT_STREQ(r.name, "A");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
}


TEST(sfn_clean_basis, clean_name_not_lossy)
{
	auto r = sfn_clean_basis("README.TXT");
	EXPECT_FALSE(r.lossy);
	EXPECT_FALSE(r.not_8x3);
}

TEST(sfn_clean_basis, upcase_not_lossy_and_fits_8x3)
{
	auto r = sfn_clean_basis("readme.txt");
	EXPECT_STREQ(r.name, "README");
	EXPECT_STREQ(r.ext, "TXT");
	EXPECT_FALSE(r.lossy);
	EXPECT_FALSE(r.not_8x3);
}

TEST(sfn_clean_basis, space_stripping_not_lossy_but_not_8x3)
{
	auto r = sfn_clean_basis("  test  ");
	EXPECT_STREQ(r.name, "TEST");
	EXPECT_FALSE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, illegal_char_past_8th_position_sets_lossy)
{
	auto r = sfn_clean_basis("testfile[1]");
	EXPECT_STREQ(r.name, "TESTFILE");
	EXPECT_TRUE(r.lossy);
	EXPECT_TRUE(r.not_8x3);
}

TEST(sfn_clean_basis, exactly_8x3_is_not_8x3_false)
{
	auto r = sfn_clean_basis("TESTFILE.TXT");
	EXPECT_STREQ(r.name, "TESTFILE");
	EXPECT_STREQ(r.ext, "TXT");
	EXPECT_FALSE(r.not_8x3);
}


TEST(sfn_clean_basis, test_bracket_test)
{
	auto r = sfn_clean_basis("test [test]");
	EXPECT_STREQ(r.name, "TEST_TES");
	EXPECT_STREQ(r.ext, "");
	EXPECT_TRUE(r.lossy);
}

TEST(sfn_clean_basis, test_paren_test)
{
	auto r = sfn_clean_basis("test (test)");
	EXPECT_STREQ(r.name, "TEST(TES");
	EXPECT_STREQ(r.ext, "");
	EXPECT_FALSE(r.lossy);
}


TEST(sfn_clean_basis, traversal_after_cleaning_dotdot_bracket)
{
	auto r = sfn_clean_basis("..[x]");
	// leading dots stripped, then _x_ -> never ".." or "."
	EXPECT_STRNE(r.name, ".");
	EXPECT_STRNE(r.name, "..");
	EXPECT_TRUE(r.name[0] != '.');
}

TEST(sfn_clean_basis, embedded_nul_truncates)
{
	auto r = sfn_clean_basis(std::string("AB\0CD", 5).c_str());
	EXPECT_STREQ(r.name, "AB");
}

TEST(sfn_clean_basis, reserved_device_names_pass_through)
{
	auto r = sfn_clean_basis("CON");
	EXPECT_STREQ(r.name, "CON");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, con_with_extension)
{
	auto r = sfn_clean_basis("CON.TXT");
	EXPECT_STREQ(r.name, "CON");
	EXPECT_STREQ(r.ext, "TXT");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, allowed_chars_not_replaced)
{
	auto r = sfn_clean_basis("$#@()!%");
	EXPECT_STREQ(r.name, "$#@()!%");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, more_allowed_chars)
{
	auto r = sfn_clean_basis("{}`~_-");
	EXPECT_STREQ(r.name, "{}`~_-");
	EXPECT_FALSE(r.lossy);
}

TEST(sfn_clean_basis, caret_and_ampersand_allowed)
{
	auto r = sfn_clean_basis("^&'");
	EXPECT_STREQ(r.name, "^&'");
	EXPECT_FALSE(r.lossy);
}

} // namespace
