/**
  \file G3D-app.lib/source/PythonInterpreter.cpp
  @author Zander Majercik

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifdef G3D_WINDOWS
#include "G3D-app/PythonInterpreter.h"

// Undefine _DEBUG here so Python.h does not search for the debug library which is
// not included in the project (it doesn't come with most python installations).
#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace G3D {

void PythonInterpreter::startPython(const String& pythonHome) {

    alwaysAssertM(!m_pythonRunning, "Multiple initialization of the python interpreter!");

    // Convert to wchar_t*
    const size_t homeLength = pythonHome.length();
    wchar_t* wideHome = new wchar_t[homeLength + 1];
    std::mbstowcs(wideHome, pythonHome.c_str(), homeLength);
    wideHome[homeLength] = '\0';

    Py_SetPythonHome(wideHome);

    // Py_GetVersion echoes includes from the python files packaged with the example.
    // Thus, it will be wrong if you are using a different version of python.
    //const char* version = Py_GetVersion();

    // Initialize the Python interpreter. This will start the python
    // specified by Py_SetPythonHome above.
    // From the documentation, this is a no-op if called again before Py_Finalize()
    // https://docs.python.org/3/c-api/init.html
    // Therefore, the class implements the Singleton design paradigm (see comment)
    Py_Initialize();
    assertPythonOk("initialize");

    //wchar_t* path = Py_GetPath();
    m_pythonRunning = true;

    // Allow python to search the working directory and find the inference script.
    // PyRun_SimpleString will return 0 on success and -1 on an exception. You cannot
    // view the exception, but you can test variables and throw or not to get information from the interpreter.
    PyRun_SimpleString("import sys\n"
        "sys.path.append(\".\")\n");

    assertPythonOk("Add current directory");
}

void PythonInterpreter::importModule(const String& module) {
    if (m_modules.containsKey(module)) {
        return;
    }
    m_modules.set(module, PyImport_ImportModule(module.c_str()));
    assertPythonOk(module.c_str());
}

void PythonInterpreter::importFunctionFromModule(const String& function, const String& module) {
    // Check if the function is already imported.
    // TODO: what about functions from different modules that share a name?
    if (PythonInterpreter::m_functions.containsKey(function)) {
        return;
    }

    alwaysAssertM(m_modules.get(module), "Module " + module + " has not been imported!");
    PyObject* dict = PyModule_GetDict(m_modules[module]);
    assertPythonOk("dict");
    PythonInterpreter::m_functions.set(function, PyDict_GetItemString(dict, function.c_str()));
    assertPythonOk(function.c_str());
    Py_DECREF(dict);
}

void PythonInterpreter::finishPython() {
    // Decrement references to all imported functions
    for (Table<String, PyObject*>::Iterator iter = m_functions.begin(); iter != m_functions.end(); ++iter) {
        Py_DECREF(iter.value());
    }
    m_functions.clear();
    // And to all imported modules
    for (Table<String, PyObject*>::Iterator iter = m_modules.begin(); iter != m_modules.end(); ++iter) {
        Py_DECREF(iter.value());
    }
    m_modules.clear();
    Py_Finalize();
    m_pythonRunning = false;
}

const char* PythonInterpreter::assertPythonOk(const char* variable) {
    if (!PyErr_Occurred()) {
        return nullptr;
    }
    //https://stackoverflow.com/questions/1418015/how-to-get-python-exception-text
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    return PyUnicode_AsUTF8(pvalue);
}

template<>
PyObject* PythonInterpreter::convertToPythonArgument(const std::vector<size_t>& dimensions, float* data) {

    Py_buffer* imageBuffer = new Py_buffer{
        data, //buf
        nullptr, //obj
        Py_ssize_t(dimensions[0] * dimensions[1] * dimensions[2] * sizeof(float)), // len
        sizeof(float), // itemsize

        PyBUF_WRITE, // readonly
        (int)dimensions.size(), //ndims
	(char*)"f", //format
        // "dimensions" scope persists beyond inference
        (Py_ssize_t*)&dimensions[0], //shape
	nullptr, //stride
	nullptr, //suboffset
	nullptr //internal
    };

    return PyMemoryView_FromBuffer(imageBuffer);
}
template<>
PyObject* PythonInterpreter::convertToPythonArgument(const float& data) {
    return PyFloat_FromDouble(data);
}

template<>
String PythonInterpreter::convertToCObject<String>(PyObject* input) {
    return String(PyUnicode_AsUTF8(input));
}

template<>
float PythonInterpreter::convertToCObject<float>(PyObject* input) {
    return (float)PyFloat_AsDouble(input);
}

template<>
void PythonInterpreter::call(const String& name, String& output, const int& input) {
    PyObject* retVal = PyObject_CallObject(PythonInterpreter::m_functions[name], PythonInterpreter::convertToPythonArgument(input));
    output = convertToCObject<String>(retVal);
}

template<>
void PythonInterpreter::call(const String& name, float& output, const float& input) {
    PyObject* argTuple = PyTuple_New(1);
    PyTuple_SetItem(argTuple, 0, PythonInterpreter::convertToPythonArgument(input));
    PyObject* retVal = PyObject_CallObject(PythonInterpreter::m_functions[name], argTuple);
    output = convertToCObject<float>(retVal);
}

template<>
void PythonInterpreter::call(const String& name, float& output, const Array<float>& input) {
    PyObject* argTuple = PyTuple_New(input.size());
    for (int i = 0; i < input.size(); ++i) {
        PyTuple_SetItem(argTuple, i, PythonInterpreter::convertToPythonArgument(input[i]));
    }
    PyObject* retVal = PyObject_CallObject(PythonInterpreter::m_functions[name], argTuple);
    output = convertToCObject<float>(retVal);
}


//template<>
//void PythonInterpreter::call(const String& name, float& output, float* input) {//, const std::vector<size_t>& dims) {
//    PyObject* argTuple = PyTuple_New(1);
//    PyTuple_SetItem(argTuple, 0, PythonInterpreter::convertToPythonArgument(dims, input));
//    PyObject* retVal = PyObject_CallObject(PythonInterpreter::m_functions[name], argTuple);
//    output = convertToCObject<float>(retVal);
//}


};
#endif
