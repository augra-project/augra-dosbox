# This file is part of the dosbox-automation Project.
# License: GPL-2.0-or-later. Contact: dosbox-automation-project@trinity2k.net
#

"""Config help rendering, in the real engine with the real message store.

The unit tests (tests/config_help_tests.cpp) cannot drive this path because
the test binary stubs out the message system. These run the built binary so
every setting's help goes through the actual printf-style render.

'config -wc' writes the help for every registered setting into a config file,
so a bad escape anywhere (the class behind the 'config -h browser' SIGSEGV)
crashes the process here and fails the test, named by the missing output.
"""

import subprocess
from pathlib import Path

import pytest
from conftest import DOSBOX_BIN

pytestmark = pytest.mark.skipif(
    not Path(DOSBOX_BIN).exists(),
    reason=f"dosbox binary not built at {DOSBOX_BIN}",
)


def run_dosbox(commands, extra_args=None):
    args = [DOSBOX_BIN, "-noprimaryconf", "-nolocalconf"]
    if extra_args:
        args += extra_args
    for cmd in commands:
        args += ["-c", cmd]
    args += ["-c", "exit"]
    env = {
        "SDL_VIDEODRIVER": "offscreen",
        "SDL_AUDIODRIVER": "dummy",
        "HOME": str(Path.home()),
        "PATH": "/usr/bin:/bin",
    }
    return subprocess.run(
        args, env=env, capture_output=True, text=True, timeout=60
    )


def test_config_h_browser_does_not_crash():
    # The exact reported repro. A format-string fault returns -signal.SIGSEGV.
    result = run_dosbox(["config -h browser"])
    assert result.returncode == 0, (
        f"config -h browser exited {result.returncode}\n{result.stderr}"
    )


def test_config_write_renders_every_setting(tmp_path):
    conf = tmp_path / "out.conf"
    result = run_dosbox([f"config -wc {conf}"])
    assert result.returncode == 0, (
        f"config -wc exited {result.returncode}\n{result.stderr}"
    )
    assert conf.exists(), "config -wc wrote no file"

    text = conf.read_text(encoding="utf-8")

    # The browser help must show a literal '%s', not the escaped '%%s'.
    assert "command line where %s stands for the URL" in text
    assert "%%s" not in text

    # A '%%' literal elsewhere must de-escape to a single percent.
    assert "%PATH%" in text
    assert "%%PATH%%" not in text
