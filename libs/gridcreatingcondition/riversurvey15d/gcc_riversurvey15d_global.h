#ifndef GCC_RIVERSURVEY15D_GLOBAL_H
#define GCC_RIVERSURVEY15D_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GCC_RIVERSURVEY15D_LIBRARY)
#  define GCC_RIVERSURVEY15D_EXPORT Q_DECL_EXPORT
#else
#  define GCC_RIVERSURVEY15D_EXPORT Q_DECL_IMPORT
#endif

#endif // GCC_RIVERSURVEY15D_GLOBAL_H