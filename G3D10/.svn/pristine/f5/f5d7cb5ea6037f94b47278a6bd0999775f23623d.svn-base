### To add pyg3d module:

- cd into `tools/pyG3D.dll/pyg3d`
- run `python setup.py sdist bdist_wheel`
- run `pip install dist/<wheel file>`, e.g. `pyg3d-0.1-py3-none-any.whl`. Add flag `--force-reinstall` if module needs to be updated
- Update: Has been added to buildg3d script, but if no C++ changes were made then it is faster to run the commands manually.

### To run sample script / verify bindings are working:
- cd to `$g3d$/G3D10`
- run `python3 tools/pyG3D.dll/samples/unittest.py`
- should print "TESTS COMPLETE" at the end
- segmentation fault after TESTS COMPLETE is ok.
