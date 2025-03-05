/**
  \file G3D-app.lib/include/G3D-app/PythonInterpreter.h
  @author Zander Majercik

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#include "G3D-base/Table.h"


/**
    We want to forward declare PyObject so as not to include Python.h in the header file.
    However, we also want to avoid declaring PyObject inside the G3D namespace so that
    it does not conflict when linking against other libraries that also use the Python/C
    API. The solution is fragile: https://mail.python.org/pipermail/python-dev/2003-August/037601.html
    I have modified it slightly to use the include guard in "object.h" in the python includes,
    so at least if PyObject is not defined here then the full "object.h" file has been previously
    included.
*/
#ifndef Py_OBJECT_H
struct _object;
typedef _object PyObject;
#endif

namespace G3D {

/** This class implements the Singleton design pattern.
 It is *not* thread-safe.
 For more, see: https://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf (Scott Meyer's paper on the thread safety of
 singletons, where he argues that making a Singleton truly thread-safe in a portable way is (nearly?) impossible.)
 Even so, we choose Singleton for the following reasons:
 + We want to enforce the constraint (inherited from the API) that there can only be one python interpreter running at a time.
 + Providing a fully static interface without initializing an object would make it the resposibility of the programmer to call finishPython() at
   the correct point in their program, or the interpreter would never close.
 Making a singleton class ensures that there is only ever one python interpreter *and* it is automatically closed
 when the managing object is destroyed.
 */
class PythonInterpreter {

private:
    // Private Constructor
    PythonInterpreter() {}

protected:
    // Check if python is running so we don't attempt to start the interpreter twice.
    bool m_pythonRunning = false;

    // Import modules and functions from the interpreter
    Table<int, int> test;
    Table<String, PyObject*> m_modules;
    Table<String, PyObject*> m_functions;

    // Convert data to Py_MemoryView
    template<typename T>
    PyObject* convertToPythonArgument(const std::vector<size_t>& dimensions, T* data);

    // TODO: build out this template
    template<typename T>
    PyObject* convertToPythonArgument(const T& input) {
        return nullptr;
    }

    // TODO: build out this template
    template<typename T>
    T convertToCObject(PyObject* input) {
        return nullptr;
    }

    
public:

    /** Stop the compiler generating methods of copying the object.
        Make these deleted functions public for more useful error messages.
    */
    PythonInterpreter(PythonInterpreter const& copy) = delete;
    PythonInterpreter& operator=(PythonInterpreter const& copy) = delete;


    /**
        Examine python error state.
        variable is the name of the most recent python object ptr that was intialized by a call to the Python API.
    */
    static const char* assertPythonOk(const char* variable);

    static shared_ptr<PythonInterpreter>& interpreterHandle() {
        static shared_ptr<PythonInterpreter> instance(new PythonInterpreter);
        return instance;
    }

    // For now, python functions may return values but not modify values passed directly
    // to python.
    // TODO: Use a variadic template to capture multiple input arguments to a python function
    template<typename Out, typename In>
    void call(const String& name, Out& output, const In& input);

    virtual ~PythonInterpreter() {
        if (m_pythonRunning) {
            finishPython();
        }
    }

    /**
    Initialize Python interpreter.
    */
    void startPython(const String& pythonHome);


    // Import modules and functions from modules into the interpreter
    void importModule(const String& module);
    void importFunctionFromModule(const String& function, const String& module);

    /** Close python interpreter. All resources should be released with Py_DECREF
    at this point. */
    void finishPython();
};

}
