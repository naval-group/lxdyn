# Checks filtered states (with duration 0) are equal to non-filtered states.
# This ensures that:
# - the output section concerning filtered states is properly parsed
# - the filtered states are properly initialized
# - there is no time shift or off-by-one errors

import csv


def assert_relatively_close(a, b, eps):
    div = 1 if a == 0 else abs(a)
    assert abs(a - b) / div < eps, f"a = {a}, b = {b}, (a-b)/a = {(a - b) / a} (should be zero)"


def assert_far(a, b, min_dist):
    assert abs(a - b) > min_dist, f"a = {a}, b = {b}, (a-b) = {a - b} (should be far)"


with open("filtered_states.csv", newline="") as csvfile:
    csv_reader = csv.DictReader(csvfile)
    read_one_line = False
    for row in csv_reader:
        # Skip first line
        if read_one_line:
            assert_relatively_close(
                float(row["x(TestShip)"]), float(row["x_filtered(TestShip)"]), 1e-2
            )
            assert_relatively_close(float(row["x(TestShip)"]), float(row["xf(TestShip)"]), 1e-2)
            assert_far(float(row["z(TestShip)"]), float(row["z_filtered(TestShip)"]), 1e-11)
        read_one_line = True
    assert read_one_line
