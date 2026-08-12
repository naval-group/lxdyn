# lxdyn

Python bindings for [xdyn](https://github.com/naval-group/lxdyn), a lightweight time-domain ship
simulator modelling the dynamic behaviour of a ship at sea. The import name stays `xdyn`; only the
PyPI distribution is called `lxdyn`.

```bash
pip install lxdyn
```

Linux only, and only under CPython 3.10 — the wheel is built against that interpreter's ABI, not the
stable ABI, so `pip install` on another version fails with no matching distribution.

```python
import xdyn

print(xdyn.__version__)
```

For the input-file format, tutorials and the full API, see the
[project README](https://github.com/naval-group/lxdyn#readme).
