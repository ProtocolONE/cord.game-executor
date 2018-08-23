#pragma once

#include <QtCore/qglobal.h>

#ifdef GAMEEXECUTOR_STATIC_LIB
# define GAMEEXECUTOR_EXPORT 
#else

#ifdef GAMEEXECUTOR_LIB
# define GAMEEXECUTOR_EXPORT Q_DECL_EXPORT
#else
# define GAMEEXECUTOR_EXPORT Q_DECL_IMPORT
#endif

#endif

#define SIGNAL_CONNECT_CHECK(X) { bool result = X; Q_ASSERT_X(result, __FUNCTION__ , #X); }

#define CRITICAL_LOG qCritical() << __FILE__ << __LINE__ << __FUNCTION__
#define WARNING_LOG qWarning() << __FILE__ << __LINE__ << __FUNCTION__
#define DEBUG_LOG qDebug() << __FILE__ << __LINE__ << __FUNCTION__
