from pathlib import Path

import pytest


@pytest.fixture(autouse=True)
def _in_tmp_path(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """What a test writes then depends neither on where pytest is started nor on earlier tests."""
    monkeypatch.chdir(tmp_path)
