/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef QMLPROFILERDATAMODEL_H
#define QMLPROFILERDATAMODEL_H

#include <qmljsdebugclient/qmlprofilereventtypes.h>
#include <qmljsdebugclient/qmlprofilereventlocation.h>
#include "qv8profilerdatamodel.h"

#include <QHash>
#include <QObject>

namespace QmlProfiler {
namespace Internal {

// used for parents and children
struct QmlRangeEventRelative;

struct QmlRangeEventData
{
    QmlRangeEventData();
    ~QmlRangeEventData();

    int eventId;
    QString displayName;
    QString eventHashStr;
    QString details;
    QmlJsDebugClient::QmlEventLocation location;
    QmlJsDebugClient::QmlEventType eventType;

    QHash <QString, QmlRangeEventRelative *> parentHash;
    QHash <QString, QmlRangeEventRelative *> childrenHash;

    qint64 duration;
    qint64 calls;
    qint64 minTime;
    qint64 maxTime;
    double timePerCall;
    double percentOfTime;
    qint64 medianTime;

    bool isBindingLoop;

    QmlRangeEventData &operator=(const QmlRangeEventData &ref);
};

struct QmlRangeEventRelative {
    QmlRangeEventRelative(QmlRangeEventData *from) : reference(from), duration(0), calls(0), inLoopPath(false) {}
    QmlRangeEventRelative(QmlRangeEventRelative *from) : reference(from->reference), duration(from->duration), calls(from->calls), inLoopPath(from->inLoopPath) {}
    QmlRangeEventData *reference;
    qint64 duration;
    qint64 calls;
    bool inLoopPath;
};

class QmlProfilerDataModel : public QObject
{
    Q_OBJECT
public:
    enum State {
        Empty,
        AcquiringData,
        ProcessingData,
        Done
    };

    explicit QmlProfilerDataModel(QObject *parent = 0);
    ~QmlProfilerDataModel();

    QList<QmlRangeEventData *> getEventDescriptions() const;
    QmlRangeEventData *eventDescription(int eventId) const;
    QList<QV8EventData *> getV8Events() const;
    QV8EventData *v8EventDescription(int eventId) const;

    static QString getHashStringForQmlEvent(const QmlJsDebugClient::QmlEventLocation &location, int eventType);
    static QString getHashStringForV8Event(const QString &displayName, const QString &function);
    static QString rootEventName();
    static QString rootEventDescription();
    static QString qmlEventTypeAsString(QmlJsDebugClient::QmlEventType typeEnum);
    static QmlJsDebugClient::QmlEventType qmlEventTypeAsEnum(const QString &typeString);

    int findFirstIndex(qint64 startTime) const;
    int findFirstIndexNoParents(qint64 startTime) const;
    int findLastIndex(qint64 endTime) const;
    Q_INVOKABLE qint64 firstTimeMark() const;
    Q_INVOKABLE qint64 lastTimeMark() const;

    // data access
    Q_INVOKABLE int count() const;
    Q_INVOKABLE bool isEmpty() const;
    Q_INVOKABLE qint64 getStartTime(int index) const;
    Q_INVOKABLE qint64 getEndTime(int index) const;
    Q_INVOKABLE qint64 getDuration(int index) const;
    Q_INVOKABLE int getType(int index) const;
    Q_INVOKABLE int getNestingLevel(int index) const;
    Q_INVOKABLE int getNestingDepth(int index) const;
    Q_INVOKABLE QString getFilename(int index) const;
    Q_INVOKABLE int getLine(int index) const;
    Q_INVOKABLE int getColumn(int index) const;
    Q_INVOKABLE QString getDetails(int index) const;
    Q_INVOKABLE int getEventId(int index) const;
    Q_INVOKABLE int getBindingLoopDest(int index) const;
    Q_INVOKABLE int getFramerate(int index) const;
    Q_INVOKABLE int getAnimationCount(int index) const;
    Q_INVOKABLE int getMaximumAnimationCount() const;
    Q_INVOKABLE int getMinimumAnimationCount() const;


    // per-type data
    Q_INVOKABLE int uniqueEventsOfType(int type) const;
    Q_INVOKABLE int maxNestingForType(int type) const;
    Q_INVOKABLE QString eventTextForType(int type, int index) const;
    Q_INVOKABLE QString eventDisplayNameForType(int type, int index) const;
    Q_INVOKABLE int eventIdForType(int type, int index) const;
    Q_INVOKABLE int eventPosInType(int index) const;

    Q_INVOKABLE qint64 traceStartTime() const;
    Q_INVOKABLE qint64 traceEndTime() const;
    Q_INVOKABLE qint64 traceDuration() const;
    Q_INVOKABLE qint64 qmlMeasuredTime() const;
    Q_INVOKABLE qint64 v8MeasuredTime() const;

    void compileStatistics(qint64 startTime, qint64 endTime);
    State currentState() const;
    Q_INVOKABLE int getCurrentStateFromQml() const;

signals:
    void stateChanged();
    void countChanged();
    void error(const QString &error);

    void requestDetailsForLocation(int eventType, const QmlJsDebugClient::QmlEventLocation &location);
    void detailsChanged(int eventId, const QString &newString);
    void reloadDetailLabels();
    void reloadDocumentsForDetails();

public slots:
    void clear();

    void addRangedEvent(int type, qint64 startTime, qint64 length,
                        const QStringList &data, const QmlJsDebugClient::QmlEventLocation &location);
    void addV8Event(int depth,const QString &function,const QString &filename, int lineNumber, double totalTime, double selfTime);
    void addFrameEvent(qint64 time, int framerate, int animationcount);
    void setTraceStartTime(qint64 time);
    void setTraceEndTime(qint64 time);

    void complete();

    bool save(const QString &filename);
    void load(const QString &filename);
    void setFilename(const QString &filename);
    void load();

    void rewriteDetailsString(int eventType, const QmlJsDebugClient::QmlEventLocation &location, const QString &newString);
    void finishedRewritingDetails();

private:
    void setState(State state);
    void reloadDetails();

private:
    class QmlProfilerDataModelPrivate;
    QmlProfilerDataModelPrivate *d;

    friend class QV8ProfilerDataModel;
};


} // namespace Internal
} // namespace QmlProfiler

#endif // QMLPROFILERDATAMODEL_H
