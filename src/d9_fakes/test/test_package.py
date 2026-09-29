# Smoke test so `colcon test` has something to run: pytest exits with code 5
# ("no tests collected") otherwise, which colcon reports as a failure.
# Real tests arrive with fake_server, fake_drone and fake_hq in M2+.


def test_package_imports():
    import d9_fakes

    assert d9_fakes.__name__ == 'd9_fakes'
