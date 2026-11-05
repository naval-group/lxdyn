"""Functions used by all modules"""
from difflib import SequenceMatcher


def similar(first_string: str, second_string: str) -> float:
    """Return a score between 0 (strings are different) and 1 (identical)."""
    return SequenceMatcher(None, first_string, second_string).ratio()


def closest_match(expected_keys: list[str], unknown_key: str) -> str:
    """Give a suggestion for the parameter name."""
    if expected_keys:
        return (
            "\nMaybe you meant: "
            + max(expected_keys, key=lambda k: similar(unknown_key, k))
            + " <-> "
            + unknown_key
            + "?"
        )
    return ""
