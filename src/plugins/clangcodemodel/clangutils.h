#ifndef CPPTOOLS_CLANGUTILS_H
#define CPPTOOLS_CLANGUTILS_H

#include "clang_global.h"
#include "utils.h"

#include <cpptools/ModelManagerInterface.h>

namespace ClangCodeModel {
namespace Utils {

CLANG_EXPORT ClangCodeModel::UnsavedFiles createUnsavedFiles(CPlusPlus::CppModelManagerInterface::WorkingCopy workingCopy);

CLANG_EXPORT QStringList createClangOptions(const CPlusPlus::CppModelManagerInterface::ProjectPart::Ptr &pPart);
CLANG_EXPORT QStringList createClangOptions(bool useCpp0x,
                                            bool useObjc,
                                            CPlusPlus::CppModelManagerInterface::ProjectPart::QtVersion qtVersion,
                                            const QList<QByteArray> &defines,
                                            const QStringList &includePaths,
                                            const QStringList &frameworkPaths);

} // namespace Utils
} // namespace Clang

#endif // CPPTOOLS_CLANGUTILS_H
