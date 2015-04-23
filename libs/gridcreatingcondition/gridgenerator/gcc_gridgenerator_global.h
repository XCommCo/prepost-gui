#ifndef GCC_GRIDGENERATOR_GLOBAL_H
#define GCC_GRIDGENERATOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GCC_GRIDGENERATOR_LIBRARY)
#  define GCC_GRIDGENERATOR_EXPORT Q_DECL_EXPORT
#else
#  define GCC_GRIDGENERATOR_EXPORT Q_DECL_IMPORT
#endif

#endif // GCC_GRIDGENERATOR_GLOBAL_H