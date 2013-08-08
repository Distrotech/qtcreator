/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
** Author: Milian Wolff, KDAB (milian.wolff@kdab.com)
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "valgrindplugin.h"

#include "callgrindtool.h"
#include "memchecktool.h"
#include "valgrindruncontrolfactory.h"

#include <analyzerbase/analyzermanager.h>
#include <analyzerbase/analyzersettings.h>

#include <valgrind/valgrindsettings.h>

#include <utils/hostosinfo.h>

#include <QtPlugin>

using namespace Analyzer;

namespace Valgrind {
namespace Internal {

bool ValgrindPlugin::initialize(const QStringList &, QString *)
{
    AnalyzerGlobalSettings::registerConfig(new ValgrindGlobalSettings());

    IAnalyzerTool *memcheckTool = new MemcheckTool(this);
    IAnalyzerTool *callgrindTool = new CallgrindTool(this);
    if (!Utils::HostOsInfo::isWindowsHost()) {
        AnalyzerManager::addTool(memcheckTool, StartLocal);
        AnalyzerManager::addTool(callgrindTool, StartLocal);
    }
    AnalyzerManager::addTool(memcheckTool, StartRemote);
    AnalyzerManager::addTool(callgrindTool, StartRemote);

    addAutoReleasedObject(new ValgrindRunControlFactory());

    return true;
}

} // namespace Internal
} // namespace Valgrind


Q_EXPORT_PLUGIN(Valgrind::Internal::ValgrindPlugin)
