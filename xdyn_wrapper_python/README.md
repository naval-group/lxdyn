# lxdyn

Python bindings for [xdyn](https://github.com/naval-group/lxdyn), a lightweight time-domain ship
simulator modelling the dynamic behaviour of a ship at sea. The import name stays `xdyn`; only the
PyPI distribution is called `lxdyn`.

```bash
pip install lxdyn
```

Linux x86_64 (glibc 2.28 or newer), CPython 3.11 to 3.15: one wheel per Python version, each
built against that interpreter's ABI.

```python
import xdyn

print(xdyn.__version__)
```

For the input-file format, tutorials and the full API, see the
[project README](https://github.com/naval-group/lxdyn#readme).
