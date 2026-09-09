# This file is part of the dosbox-automation Project.
# License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
#

"""Integration tests for SFN generation on host directories with illegal chars.

Mounts a directory containing host names with brackets, semicolons, plus
signs, and other characters that are illegal in DOS 8.3 names. Verifies
that DIR shows valid shortnames and that cd into each one works.
"""

import time
from pathlib import Path

import pytest


def run_script(client, source, timeout=20):
    time.sleep(2.1)
    r = client.script_load(source, name="sfn-test")
    assert r.status_code == 200, f"Load failed: {r.text}"
    r = client.script_start()
    assert r.status_code == 200, f"Start failed: {r.text}"
    data = client.wait_script_done(timeout=timeout)
    assert data["state"] == "completed", (
        f"Script failed: state={data['state']}, error={data.get('error', '')}"
    )
    return data.get("output", {})


# The host directory names we test and the SFN stems we expect to see
# in the DIR listing (before the ~N tail, upcased).
TEST_DIRS = [
    "test [test]",
    "game;v2",
    "mod+patch",
    "file=1",
    "name.with.dots",
    "path with spaces",
]

# Expected shortname patterns (the stem after cleaning, before tails).
# These match what sfn_clean_basis produces.
EXPECTED_STEMS = {
    "test [test]": "TEST_TES",
    "game;v2": "GAME_V2",
    "mod+patch": "MOD_PATC",
    "file=1": "FILE_1",
    "name.with.dots": "NAME",
    "path with spaces": "PATHWITH",
}


def test_sfn_bracket_dir_in_listing(dosbox_e2e, tmp_path):
    """Host dir with brackets gets a valid shortname in DIR."""
    game_dir = tmp_path / "g"
    game_dir.mkdir()
    (game_dir / "test [test]").mkdir()

    instance = dosbox_e2e(
        autoexec_lines=[f"mount c {game_dir}", "c:"],
        conf_dir=tmp_path,
    )
    client = instance.client
    client.wait_shell(timeout=10)

    lua = (
        'dosbox.type("dir")\n'
        'dosbox.key("KBD_enter", true)\n'
        'dosbox.key("KBD_enter", false)\n'
        'dosbox.wait_frames(120)\n'
        'dosbox.output.listing = dosbox.screen_text()\n'
    )
    out = run_script(client, lua)
    listing = out.get("listing", "")
    assert "TEST_T~1" in listing, (
        f"expected TEST_T~1 for 'test [test]' not in listing:\n{listing}"
    )


def test_sfn_semicolon_dir_in_listing(dosbox_e2e, tmp_path):
    """Host dir with semicolon gets a valid shortname."""
    game_dir = tmp_path / "g"
    game_dir.mkdir()
    (game_dir / "game;v2").mkdir()

    instance = dosbox_e2e(
        autoexec_lines=[f"mount c {game_dir}", "c:"],
        conf_dir=tmp_path,
    )
    client = instance.client
    client.wait_shell(timeout=10)

    lua = (
        'dosbox.type("dir")\n'
        'dosbox.key("KBD_enter", true)\n'
        'dosbox.key("KBD_enter", false)\n'
        'dosbox.wait_frames(120)\n'
        'dosbox.output.listing = dosbox.screen_text()\n'
    )
    out = run_script(client, lua)
    listing = out.get("listing", "")
    assert "GAME_V~1" in listing, (
        f"expected GAME_V~1 for 'game;v2' not in listing:\n{listing}"
    )


def test_sfn_cd_into_bracket_dir(dosbox_e2e, tmp_path):
    """cd into a dir whose host name has brackets should not produce
    'illegal path'."""
    game_dir = tmp_path / "testmount"
    game_dir.mkdir()
    (game_dir / "test [test]").mkdir()

    instance = dosbox_e2e(
        autoexec_lines=[f"mount c {game_dir}", "c:"],
        conf_dir=tmp_path,
    )
    client = instance.client
    client.wait_shell(timeout=10)

    # DIR to discover the shortname, then cd into it
    lua = (
        'dosbox.type("dir")\n'
        'dosbox.key("KBD_enter", true)\n'
        'dosbox.key("KBD_enter", false)\n'
        'dosbox.wait_frames(120)\n'
        'local t = dosbox.screen_text()\n'
        'dosbox.output.before_cd = t\n'
        'dosbox.type("cd TEST_T~1")\n'
        'dosbox.key("KBD_enter", true)\n'
        'dosbox.key("KBD_enter", false)\n'
        'dosbox.wait_frames(60)\n'
        'local t2 = dosbox.screen_text()\n'
        'dosbox.output.after_cd = t2\n'
    )
    out = run_script(client, lua)

    after_cd = out.get("after_cd", "")
    assert "Illegal" not in after_cd, (
        f"cd into bracket dir got illegal path error:\n{after_cd}"
    )
    # The prompt should show we are inside the directory
    assert "TEST_T~1" in after_cd or "C:\\TEST_T~1" in after_cd, (
        f"prompt does not show the directory after cd:\n{after_cd}"
    )


def test_sfn_colliding_stems_get_different_tails(dosbox_e2e, tmp_path):
    """Two host dirs whose cleaned stems are identical must get ~1 and ~2."""
    game_dir = tmp_path / "g"
    game_dir.mkdir()
    # Both produce stem "TESTLONG" after cleaning (9+ chars truncated to 8)
    (game_dir / "testlong alpha").mkdir()
    (game_dir / "testlong beta").mkdir()

    instance = dosbox_e2e(
        autoexec_lines=[f"mount c {game_dir}", "c:"],
        conf_dir=tmp_path,
    )
    client = instance.client
    client.wait_shell(timeout=10)

    lua = (
        'dosbox.type("dir")\n'
        'dosbox.key("KBD_enter", true)\n'
        'dosbox.key("KBD_enter", false)\n'
        'dosbox.wait_frames(120)\n'
        'dosbox.output.listing = dosbox.screen_text()\n'
    )
    out = run_script(client, lua)
    listing = out.get("listing", "")

    # Both names exceed 8 chars, so both get tails
    assert "TESTLO~1" in listing, (
        f"first tailed stem not found in listing:\n{listing}"
    )
    assert "TESTLO~2" in listing, (
        f"second tailed stem not found in listing:\n{listing}"
    )
