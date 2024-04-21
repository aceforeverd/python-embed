#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: call pythonfile funcname [args]\n");
    return 1;
  }

  char exe_path[1024];
  if (realpath(argv[0], exe_path) == NULL) {
    perror("realpath");
    return 1;
  }
  // Extract the directory component from the executable path
  char *dir = dirname(exe_path);

  Py_Initialize();

  PyObject *sysPath = PySys_GetObject("path");
  if (PyList_Append(sysPath, PyUnicode_DecodeFSDefault(dir))) {
    perror("append python sys path");
    return 1;
  }
  PySys_SetObject("path", sysPath);

  PyObject *pName = PyUnicode_DecodeFSDefault(argv[1]);
  /* Error checking of pName left out */

  PyObject *pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pModule == NULL) {
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
    fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
  }

  PyObject *pFunc = PyObject_GetAttrString(pModule, argv[2]);

  if (pFunc == NULL || !PyCallable_Check(pFunc)) {
    PyErr_Print();
    fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
    return 1;
  }

  /* pFunc is a new reference */
  PyObject *pArgs = PyTuple_New(argc - 3);
  // three args
  PyObject *pValue;
  for (int i = 0; i < argc - 3; ++i) {
    if (i == 0) {
      // first arg string, else int
      pValue = PyUnicode_FromString(argv[i + 3]);
    } else {
      pValue = PyLong_FromLong(atoi(argv[i + 3]));
    }
    if (!pValue) {
      Py_DECREF(pArgs);
      Py_DECREF(pModule);
      fprintf(stderr, "Cannot convert argument\n");
      return 1;
    }
    /* pValue reference stolen here: */
    PyTuple_SetItem(pArgs, i, pValue);
  }
  PyObject *pRet = PyObject_CallObject(pFunc, pArgs);
  Py_DECREF(pArgs);

  if (pRet != NULL && PyUnicode_Check(pRet)) {
    PyObject *temp = PyUnicode_AsEncodedString(pRet, "utf-8", "strict");
    if (temp) {
      char *cResult = PyBytes_AsString(temp);
      printf("Result of call: %s\n", cResult);
      Py_DECREF(temp);
    }
  } else {
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
    PyErr_Print();
    fprintf(stderr, "Call failed\n");
    return 1;
  }
  Py_XDECREF(pFunc);
  Py_DECREF(pModule);

  if (Py_FinalizeEx() < 0) {
    return 120;
  }

  return 0;
}
