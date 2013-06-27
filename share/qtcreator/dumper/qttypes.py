
#######################################################################
#
# Dumper Implementations
#
#######################################################################

from __future__ import with_statement

movableTypes = set([
    "QBrush", "QBitArray", "QByteArray", "QCustomTypeInfo", "QChar", "QDate",
    "QDateTime", "QFileInfo", "QFixed", "QFixedPoint", "QFixedSize",
    "QHashDummyValue", "QIcon", "QImage", "QLine", "QLineF", "QLatin1Char",
    "QLocale", "QMatrix", "QModelIndex", "QPoint", "QPointF", "QPen",
    "QPersistentModelIndex", "QResourceRoot", "QRect", "QRectF", "QRegExp",
    "QSize", "QSizeF", "QString", "QTime", "QTextBlock", "QUrl", "QVariant",
    "QXmlStreamAttribute", "QXmlStreamNamespaceDeclaration",
    "QXmlStreamNotationDeclaration", "QXmlStreamEntityDeclaration"
])

def checkSimpleRef(ref):
    count = int(ref["_q_value"])
    check(count > 0)
    check(count < 1000000)

def checkRef(ref):
    try:
        count = int(ref["atomic"]["_q_value"]) # Qt 5.
        minimum = -1
    except:
        count = int(ref["_q_value"]) # Qt 4.
        minimum = 0
    # Assume there aren't a million references to any object.
    check(count >= minimum)
    check(count < 1000000)


def qByteArrayDataData(d, value):
    checkRef(value['ref'])
    size = int(value['size'])
    alloc = int(value['alloc'])
    try:
        # Qt 5. Will fail on Qt 4 due to the missing 'offset' member.
        addr = d.addressOf(value) + int(value['offset'])
    except:
        # Qt 4:
        addr = value['data']
    return createPointerValue(value, addr, d.charType()), size, alloc

def qByteArrayData(d, value):
    return d.byteArrayDataData(value['d'].dereference())

def qEncodeByteArray(d, value, limit = None):
    data, size, alloc = d.byteArrayData(value)
    if alloc != 0:
        check(0 <= size and size <= alloc and alloc <= 100*1000*1000)
    limit = d.computeLimit(size, limit)
    s = d.readRawMemory(data, limit)
    if limit < size:
        s += "2e2e2e"
    return s

def qEncodeString(d, value, limit = 0):
    data, size, alloc = d.stringData(value)
    if alloc != 0:
        check(0 <= size and size <= alloc and alloc <= 100*1000*1000)
    limit = d.computeLimit(size, limit)
    s = d.readRawMemory(data, 2 * limit)
    if limit < size:
        s += "2e002e002e00"
    return s

def mapForms():
    return "Normal,Compact"

def arrayForms():
    if hasPlot():
        return "Normal,Plot"
    return "Normal"

def qMapCompact(format, keyType, valueType):
    if format == 2:
        return True # Compact.
    return isSimpleType(keyType) and isSimpleType(valueType)

def qPutMapName(d, value):
    if str(value.type) == d.ns + "QString":
        d.put('key="%s",' % d.encodeString(value))
        d.put('keyencoded="%s",' % Hex4EncodedLittleEndian)
    elif str(value.type) == d.ns + "QByteArray":
        d.put('key="%s",' % d.encodeByteArray(value))
        d.put('keyencoded="%s",' % Hex2EncodedLatin1)
    else:
        if lldbLoaded:
            d.put('name="%s",' % value.GetValue())
        else:
            d.put('name="%s",' % value)

Dumper.encodeByteArray = qEncodeByteArray
Dumper.byteArrayData = qByteArrayData
Dumper.byteArrayDataData = qByteArrayDataData
Dumper.putByteArrayValue = \
    lambda d, value: d.putValue(d.encodeByteArray(value), Hex2EncodedLatin1)
Dumper.putStringValue = \
    lambda d, value: d.putValue(d.encodeString(value), Hex4EncodedLittleEndian)

Dumper.encodeString = qEncodeString
Dumper.stringData = Dumper.byteArrayData
Dumper.putMapName = qPutMapName

Dumper.isMapCompact = \
    lambda d, keyType, valueType: qMapCompact(d.currentItemFormat(), keyType, valueType)


# Returns True when it encounters a QObject or derived class.
def tryPutObjectNameValue(d, value):
    try:
        # Is this derived from QObject?
        dd = value["d_ptr"]["d"]
        privateType = d.lookupType(d.ns + "QObjectPrivate")
        staticMetaObject = value["staticMetaObject"]
        d_ptr = dd.cast(privateType.pointer()).dereference()
        objectName = None
        try:
            objectName = d_ptr["objectName"]
        except: # Qt 5
            p = d_ptr["extraData"]
            if not isNull(p):
                objectName = p.dereference()["objectName"]
        if not objectName is None:
            data, size, alloc = d.stringData(objectName)
            if size > 0:
                str = d.readRawMemory(data, 2 * size)
                d.putValue(str, Hex4EncodedLittleEndian, 1)
        return True
    except:
        return False

Dumper.tryPutObjectNameValue = tryPutObjectNameValue

############################################################################################


def qdump__QAtomicInt(d, value):
    d.putValue(value["_q_value"])
    d.putNumChild(0)


def qdump__QBasicAtomicInt(d, value):
    d.putValue(value["_q_value"])
    d.putNumChild(0)


def qdump__QBasicAtomicPointer(d, value):
    d.putType(value.type)
    p = cleanAddress(value["_q_value"])
    d.putValue(p)
    d.putPointerValue(value.address)
    d.putNumChild(p)
    if d.isExpanded():
        with Children(d):
           d.putItem(value["_q_value"])

def qdump__QByteArrayData(d, value):
    data, size, alloc = d.byteArrayDataData(value)
    d.putValue(d.readRawMemory(data, size), Hex2EncodedLatin1)
    d.putNumChild(size)
    if d.isExpanded():
        d.putArrayData(d.charType(), data, size)

def qform__QByteArray():
    return "Inline,As Latin1 in Separate Window,As UTF-8 in Separate Window"

def qdump__QByteArray(d, value):
    d.putByteArrayValue(value)
    data, size, alloc = d.byteArrayData(value)
    d.putNumChild(size)
    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        d.putField("editformat", DisplayLatin1String)
        d.putField("editvalue", d.encodeByteArray(value, None))
    elif format == 3:
        d.putField("editformat", DisplayUtf8String)
        d.putField("editvalue", d.encodeByteArray(value, None))
    if d.isExpanded():
        d.putArrayData(d.charType(), data, size)


# Fails on Windows.
try:
    import curses.ascii
    def printableChar(ucs):
        if curses.ascii.isprint(ucs):
            return ucs
        return '?'
except:
    def printableChar(ucs):
        if ucs >= 32 and ucs <= 126:
            return ucs
        return '?'

def qdump__QChar(d, value):
    ucs = int(value["ucs"])
    d.putValue("'%c' (%d)" % (printableChar(ucs), ucs))
    d.putNumChild(0)


def qform__QAbstractItemModel():
    return "Normal,Enhanced"

def qdump__QAbstractItemModel(d, value):
    format = d.currentItemFormat()
    if format == 1:
        d.putPlainChildren(value)
        return
    #format == 2:
    # Create a default-constructed QModelIndex on the stack.
    try:
        ri = makeValue(d.ns + "QModelIndex", "-1, -1, 0, 0")
        this_ = makeExpression(value)
        ri_ = makeExpression(ri)
        rowCount = int(parseAndEvaluate("%s.rowCount(%s)" % (this_, ri_)))
        columnCount = int(parseAndEvaluate("%s.columnCount(%s)" % (this_, ri_)))
    except:
        d.putPlainChildren(value)
        return
    d.putValue("%d x %d" % (rowCount, columnCount))
    d.putNumChild(rowCount * columnCount)
    if d.isExpanded():
        with Children(d, numChild=rowCount * columnCount, childType=ri.type):
            i = 0
            for row in xrange(rowCount):
                for column in xrange(columnCount):
                    with SubItem(d, i):
                        d.putName("[%s, %s]" % (row, column))
                        mi = parseAndEvaluate("%s.index(%d,%d,%s)"
                            % (this_, row, column, ri_))
                        #warn("MI: %s " % mi)
                        #name = "[%d,%d]" % (row, column)
                        #d.putValue("%s" % mi)
                        d.putItem(mi)
                        i = i + 1
                        #warn("MI: %s " % mi)
                        #d.putName("[%d,%d]" % (row, column))
                        #d.putValue("%s" % mi)
                        #d.putNumChild(0)
                        #d.putType(mi.type)
    #gdb.execute("call free($ri)")

def qform__QModelIndex():
    return "Normal,Enhanced"

def qdump__QModelIndex(d, value):
    format = d.currentItemFormat()
    if format == 1:
        d.putPlainChildren(value)
        return
    r = value["r"]
    c = value["c"]
    try:
        p = value["p"]
    except:
        p = value["i"]
    m = value["m"]
    if isNull(m) or r < 0 or c < 0:
        d.putValue("(invalid)")
        d.putPlainChildren(value)
        return

    mm = m.dereference()
    mm = mm.cast(mm.type.unqualified())
    try:
        mi = makeValue(d.ns + "QModelIndex", "%s,%s,%s,%s" % (r, c, p, m))
        mm_ = makeExpression(mm)
        mi_ = makeExpression(mi)
        rowCount = int(parseAndEvaluate("%s.rowCount(%s)" % (mm_, mi_)))
        columnCount = int(parseAndEvaluate("%s.columnCount(%s)" % (mm_, mi_)))
    except:
        d.putEmptyValue()
        d.putPlainChildren(value)
        return

    try:
        # Access DisplayRole as value
        val = parseAndEvaluate("%s.data(%s, 0)" % (mm_, mi_))
        v = val["d"]["data"]["ptr"]
        d.putStringValue(makeValue(d.ns + 'QString', v))
    except:
        d.putValue("(invalid)")

    d.putNumChild(rowCount * columnCount)
    if d.isExpanded():
        with Children(d):
            i = 0
            for row in xrange(rowCount):
                for column in xrange(columnCount):
                    with UnnamedSubItem(d, i):
                        d.putName("[%s, %s]" % (row, column))
                        mi2 = parseAndEvaluate("%s.index(%d,%d,%s)"
                            % (mm_, row, column, mi_))
                        d.putItem(mi2)
                        i = i + 1
            #d.putCallItem("parent", val, "parent")
            #with SubItem(d, "model"):
            #    d.putValue(m)
            #    d.putType(d.ns + "QAbstractItemModel*")
            #    d.putNumChild(1)
    #gdb.execute("call free($mi)")


def qdump__QDate(d, value):
    jd = int(value["jd"])
    if int(jd):
        d.putValue(jd, JulianDate)
        d.putNumChild(1)
        if d.isExpanded():
            qt = d.ns + "Qt::"
            if lldbLoaded:
                qt += "DateFormat::" # FIXME: Bug?...
            # FIXME: This improperly uses complex return values.
            with Children(d):
                d.putCallItem("toString", value, "toString", qt + "TextDate")
                d.putCallItem("(ISO)", value, "toString", qt + "ISODate")
                d.putCallItem("(SystemLocale)", value, "toString",
                    qt + "SystemLocaleDate")
                d.putCallItem("(Locale)", value, "toString", qt + "LocaleDate")
    else:
        d.putValue("(invalid)")
        d.putNumChild(0)


def qdump__QTime(d, value):
    mds = value["mds"]
    if int(mds) >= 0:
        d.putValue(value["mds"], MillisecondsSinceMidnight)
        d.putNumChild(1)
        if d.isExpanded():
            qtdate = d.ns + "Qt::"
            qttime = d.ns + "Qt::"
            if lldbLoaded:
                qtdate += "DateFormat::" # FIXME: Bug?...
                qttime += "TimeSpec::"
            # FIXME: This improperly uses complex return values.
            with Children(d):
                d.putCallItem("toString", value, "toString", qtdate + "TextDate")
                d.putCallItem("(ISO)", value, "toString", qtdate + "ISODate")
                d.putCallItem("(SystemLocale)", value, "toString",
                     qtdate + "SystemLocaleDate")
                d.putCallItem("(Locale)", value, "toString", qtdate + "LocaleDate")
                d.putCallItem("toUTC", value, "toTimeSpec", qttime + "UTC")
    else:
        d.putValue("(invalid)")
        d.putNumChild(0)


# This relies on the Qt4/Qt5 internal structure layout:
# {sharedref(4), date(8), time(4+x)}
def qdump__QDateTime(d, value):
    intType = d.lookupType("int")
    intPtrType = intType.pointer()
    base = pointerValue(value.cast(intPtrType))
    mds = int(createReferenceValue(value, base + intPtrType.sizeof + 8, intType))
    if mds >= 0:
        jd = int(createReferenceValue(value, base + intPtrType.sizeof, intType))
        d.putValue("%s/%s" % (jd, mds), JulianDateAndMillisecondsSinceMidnight)
        d.putNumChild(1)
        if d.isExpanded():
            # FIXME: This improperly uses complex return values.
            with Children(d):
                qtdate = d.ns + "Qt::"
                qttime = d.ns + "Qt::"
                if lldbLoaded:
                    qtdate += "DateFormat::" # FIXME: Bug?...
                    qttime += "TimeSpec::" # FIXME: Bug?...
                d.putCallItem("toTime_t", value, "toTime_t")
                d.putCallItem("toString", value, "toString", qtdate + "TextDate")
                d.putCallItem("(ISO)", value, "toString", qtdate + "ISODate")
                d.putCallItem("(SystemLocale)", value, "toString", qtdate + "SystemLocaleDate")
                d.putCallItem("(Locale)", value, "toString", qtdate + "LocaleDate")
                d.putCallItem("toUTC", value, "toTimeSpec", qttime + "UTC")
                d.putCallItem("toLocalTime", value, "toTimeSpec", qttime + "LocalTime")
    else:
        d.putValue("(invalid)")
        d.putNumChild(0)


def qdump__QDir(d, value):
    d.putNumChild(1)
    data = value["d_ptr"]["d"].dereference()
    try:
        # Up to Qt 4.7
        d.putStringValue(data["path"])
    except:
        # Qt 4.8 and later.
        d.putStringValue(data["dirEntry"]["m_filePath"])
    if d.isExpanded():
        with Children(d):
            qdir = d.ns + "QDir::"
            d.putCallItem("absolutePath", value, "absolutePath")
            d.putCallItem("canonicalPath", value, "canonicalPath")
            d.putSubItem("entryList", data["files"])
            d.putSubItem("entryInfoList", data["fileInfos"])


def qdump__QFile(d, value):
    try:
        ptype = d.lookupType(d.ns + "QFilePrivate").pointer()
        d_ptr = value["d_ptr"]["d"]
        d.putStringValue(d_ptr.cast(ptype).dereference()["fileName"])
    except:
        d.putPlainChildren(value)
        return
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            base = fieldAt(value.type, 0).type
            d.putSubItem("[%s]" % str(base), value.cast(base), False)
            d.putCallItem("exists", value, "exists")


def qdump__QFileInfo(d, value):
    try:
        d.putStringValue(value["d_ptr"]["d"].dereference()["fileNames"][3])
    except:
        d.putPlainChildren(value)
        return
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d, childType=d.lookupType(d.ns + "QString")):
            d.putCallItem("absolutePath", value, "absolutePath")
            d.putCallItem("absoluteFilePath", value, "absoluteFilePath")
            d.putCallItem("canonicalPath", value, "canonicalPath")
            d.putCallItem("canonicalFilePath", value, "canonicalFilePath")
            d.putCallItem("completeBaseName", value, "completeBaseName")
            d.putCallItem("completeSuffix", value, "completeSuffix")
            d.putCallItem("baseName", value, "baseName")
            if False:
                #ifdef Q_OS_MACX
                d.putCallItem("isBundle", value, "isBundle")
                d.putCallItem("bundleName", value, "bundleName")
            d.putCallItem("fileName", value, "fileName")
            d.putCallItem("filePath", value, "filePath")
            # Crashes gdb (archer-tromey-python, at dad6b53fe)
            #d.putCallItem("group", value, "group")
            #d.putCallItem("owner", value, "owner")
            d.putCallItem("path", value, "path")

            d.putCallItem("groupid", value, "groupId")
            d.putCallItem("ownerid", value, "ownerId")

            #QFile::Permissions permissions () const
            perms = call(value, "permissions")
            if perms is None:
                d.putValue("<not available>")
            else:
                with SubItem(d, "permissions"):
                    d.putEmptyValue()
                    d.putType(d.ns + "QFile::Permissions")
                    d.putNumChild(10)
                    if d.isExpanded():
                        with Children(d, 10):
                            perms = perms['i']
                            d.putBoolItem("ReadOwner",  perms & 0x4000)
                            d.putBoolItem("WriteOwner", perms & 0x2000)
                            d.putBoolItem("ExeOwner",   perms & 0x1000)
                            d.putBoolItem("ReadUser",   perms & 0x0400)
                            d.putBoolItem("WriteUser",  perms & 0x0200)
                            d.putBoolItem("ExeUser",    perms & 0x0100)
                            d.putBoolItem("ReadGroup",  perms & 0x0040)
                            d.putBoolItem("WriteGroup", perms & 0x0020)
                            d.putBoolItem("ExeGroup",   perms & 0x0010)
                            d.putBoolItem("ReadOther",  perms & 0x0004)
                            d.putBoolItem("WriteOther", perms & 0x0002)
                            d.putBoolItem("ExeOther",   perms & 0x0001)

            #QDir absoluteDir () const
            #QDir dir () const
            d.putCallItem("caching", value, "caching")
            d.putCallItem("exists", value, "exists")
            d.putCallItem("isAbsolute", value, "isAbsolute")
            d.putCallItem("isDir", value, "isDir")
            d.putCallItem("isExecutable", value, "isExecutable")
            d.putCallItem("isFile", value, "isFile")
            d.putCallItem("isHidden", value, "isHidden")
            d.putCallItem("isReadable", value, "isReadable")
            d.putCallItem("isRelative", value, "isRelative")
            d.putCallItem("isRoot", value, "isRoot")
            d.putCallItem("isSymLink", value, "isSymLink")
            d.putCallItem("isWritable", value, "isWritable")
            d.putCallItem("created", value, "created")
            d.putCallItem("lastModified", value, "lastModified")
            d.putCallItem("lastRead", value, "lastRead")


def qdump__QFixed(d, value):
    v = int(value["val"])
    d.putValue("%s/64 = %s" % (v, v/64.0))
    d.putNumChild(0)


def qdump__QFiniteStack(d, value):
    alloc = int(value["_alloc"])
    size = int(value["_size"])
    check(0 <= size and size <= alloc and alloc <= 1000 * 1000 * 1000)
    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        innerType = d.templateArgument(value.type, 0)
        d.putArrayData(innerType, value["_array"], size)

# Stock gdb 7.2 seems to have a problem with types here:
#
#  echo -e "namespace N { struct S { enum E { zero, one, two }; }; }\n"\
#      "int main() { N::S::E x = N::S::one;\n return x; }" >> main.cpp
#  g++ -g main.cpp
#  gdb-7.2 -ex 'file a.out' -ex 'b main' -ex 'run' -ex 'step' \
#     -ex 'ptype N::S::E' -ex 'python print gdb.lookup_type("N::S::E")' -ex 'q'
#  gdb-7.1 -ex 'file a.out' -ex 'b main' -ex 'run' -ex 'step' \
#     -ex 'ptype N::S::E' -ex 'python print gdb.lookup_type("N::S::E")' -ex 'q'
#  gdb-cvs -ex 'file a.out' -ex 'b main' -ex 'run' -ex 'step' \
#     -ex 'ptype N::S::E' -ex 'python print gdb.lookup_type("N::S::E")' -ex 'q'
#
# gives as of 2010-11-02
#
#  type = enum N::S::E {N::S::zero, N::S::one, N::S::two} \n
#    Traceback (most recent call last): File "<string>", line 1,
#      in <module> RuntimeError: No type named N::S::E.
#  type = enum N::S::E {N::S::zero, N::S::one, N::S::two} \n  N::S::E
#  type = enum N::S::E {N::S::zero, N::S::one, N::S::two} \n  N::S::E
#
# i.e. there's something broken in stock 7.2 that is was ok in 7.1 and is ok later.

def qdump__QFlags(d, value):
    i = value["i"]
    try:
        enumType = d.templateArgument(value.type.unqualified(), 0)
        d.putValue("%s (%s)" % (i.cast(enumType), i))
    except:
        d.putValue("%s" % i)
    d.putNumChild(0)


def qform__QHash():
    return mapForms()

def qdump__QHash(d, value):

    def hashDataFirstNode(value):
        val = value.cast(hashDataType)
        bucket = val["buckets"]
        e = val.cast(hashNodeType)
        for n in xrange(int(val["numBuckets"]) - 1, -1, -1):
            n = n - 1
            if n < 0:
                break
            if bucket.dereference() != e:
                return bucket.dereference()
            bucket = bucket + 1
        return e;

    def hashDataNextNode(node):
        next = node["next"]
        if next["next"]:
            return next
        d = node.cast(hashDataType.pointer()).dereference()
        numBuckets = int(d["numBuckets"])
        start = (int(node["h"]) % numBuckets) + 1
        bucket = d["buckets"] + start
        for n in xrange(numBuckets - start):
            if bucket.dereference() != next:
                return bucket.dereference()
            bucket += 1
        return node

    keyType = d.templateArgument(value.type, 0)
    valueType = d.templateArgument(value.type, 1)

    d_ptr = value["d"]
    e_ptr = value["e"]
    size = int(d_ptr["size"])

    hashDataType = d_ptr.type
    hashNodeType = e_ptr.type

    check(0 <= size and size <= 100 * 1000 * 1000)
    checkRef(d_ptr["ref"])

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        isCompact = d.isMapCompact(keyType, valueType)
        node = hashDataFirstNode(value)
        innerType = e_ptr.dereference().type
        childType = innerType
        if isCompact:
            childType = valueType
        with Children(d, size, maxNumChild=1000, childType=childType):
            for i in d.childRange():
                it = node.dereference().cast(innerType)
                with SubItem(d, i):
                    if isCompact:
                        d.putMapName(it["key"])
                        d.putItem(it["value"])
                        d.putType(valueType)
                    else:
                        d.putItem(it)
                node = hashDataNextNode(node)


def qdump__QHashNode(d, value):
    keyType = d.templateArgument(value.type, 0)
    valueType = d.templateArgument(value.type, 1)
    key = value["key"]
    val = value["value"]

    #if isSimpleType(keyType) and isSimpleType(valueType):
    #    d.putName(key)
    #    d.putValue(val)
    #else:
    d.putEmptyValue()

    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putSubItem("key", key)
            d.putSubItem("value", val)


def qHashIteratorHelper(d, value):
    typeName = str(value.type)
    hashType = d.lookupType(typeName[0:typeName.rfind("::")])
    keyType = d.templateArgument(hashType, 0)
    valueType = d.templateArgument(hashType, 1)
    d.putNumChild(1)
    d.putEmptyValue()
    if d.isExpanded():
        with Children(d):
            typeName = "%sQHash<%s,%s>::Node" % (d.ns, keyType, valueType)
            node = value["i"].cast(d.lookupType(typeName).pointer())
            d.putSubItem("key", node["key"])
            d.putSubItem("value", node["value"])

def qdump__QHash__const_iterator(d, value):
    qHashIteratorHelper(d, value)

def qdump__QHash__iterator(d, value):
    qHashIteratorHelper(d, value)


def qdump__QHostAddress(d, value):
    data = value["d"]["d"].dereference()
    if int(data["ipString"]["d"]["size"]):
        d.putStringValue(data["ipString"])
    else:
        a = int(data["a"])
        a, n4 = divmod(a, 256)
        a, n3 = divmod(a, 256)
        a, n2 = divmod(a, 256)
        a, n1 = divmod(a, 256)
        d.putValue("%d.%d.%d.%d" % (n1, n2, n3, n4));
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
           d.putFields(data)

def qdump__QList(d, value):
    dptr = childAt(value, 0)["d"]
    private = dptr.dereference()
    begin = int(private["begin"])
    end = int(private["end"])
    array = private["array"]
    check(begin >= 0 and end >= 0 and end <= 1000 * 1000 * 1000)
    size = end - begin
    check(size >= 0)
    checkRef(private["ref"])

    innerType = d.templateArgument(value.type, 0)

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        innerSize = innerType.sizeof
        # The exact condition here is:
        #  QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic
        # but this data is available neither in the compiled binary nor
        # in the frontend.
        # So as first approximation only do the 'isLarge' check:
        stepSize = dptr.type.sizeof
        isInternal = innerSize <= stepSize and d.isMovableType(innerType)
        addr = d.addressOf(array) + begin * stepSize
        if isInternal:
            if innerSize == stepSize:
                p = createPointerValue(value, addr, innerType)
                d.putArrayData(innerType, p, size)
            else:
                with Children(d, size, childType=innerType):
                    for i in d.childRange():
                        p = createPointerValue(value, addr + i * stepSize, innerType)
                        d.putSubItem(i, p.dereference())
        else:
            p = createPointerValue(value, addr, innerType.pointer())
            # about 0.5s / 1000 items
            with Children(d, size, maxNumChild=2000, childType=innerType):
                for i in d.childRange():
                    d.putSubItem(i, p.dereference().dereference())
                    p += 1

def qform__QImage():
    return "Normal,Displayed"

def qdump__QImage(d, value):
    # This relies on current QImage layout
    intPtrType = d.lookupType("int").pointer()
    base = createReferenceValue(value, d.addressOf(value) + 3 * intPtrType.sizeof, intPtrType)
    width = int(base[1])
    height = int(base[2])
    d.putValue("(%dx%d)" % (width, height))
    d.putNumChild(0)
    #d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            with SubItem(d, "data"):
                d.putNoType()
                d.putNumChild(0)
                d.putValue("size: %s bytes" % nbytes);
    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        d_ptr = value["d"]
        bits = d_ptr["data"]
        nbytes = d_ptr["nbytes"]
        if False:
            # Take four bytes at a time, this is critical for performance.
            # In fact, even four at a time is too slow beyond 100x100 or so.
            d.putField("editformat", DisplayImageData)
            d.put('%s="' % name)
            d.put("%08x" % width)
            d.put("%08x" % height)
            d.put("%08x" % int(d_ptr["format"]))
            p = bits.cast(d.intType().pointer())
            for i in xrange(nbytes / 4):
                d.put("%08x" % int(p.dereference()))
                p += 1
            d.put('",')
        else:
            # Write to an external file. Much faster ;-(
            file = tempfile.mkstemp(prefix="gdbpy_")
            filename = file[1].replace("\\", "\\\\")
            p = bits.cast(d.charType().pointer())
            gdb.execute("dump binary memory %s %s %s" %
                (filename, cleanAddress(p), cleanAddress(p + nbytes)))
            d.putDisplay(DisplayImageFile, " %d %d %d %s"
                % (width, height, d_ptr["format"], filename))


def qdump__QLinkedList(d, value):
    d_ptr = value["d"]
    e_ptr = value["e"]
    n = int(d_ptr["size"])
    check(0 <= n and n <= 100*1000*1000)
    checkRef(d_ptr["ref"])
    d.putItemCount(n)
    d.putNumChild(n)
    if d.isExpanded():
        innerType = d.templateArgument(value.type, 0)
        with Children(d, n, maxNumChild=1000, childType=innerType):
            p = e_ptr["n"]
            for i in d.childRange():
                d.putSubItem(i, p["t"])
                p = p["n"]

qqLocalesCount = None

def qdump__QLocale(d, value):
    # Check for uninitialized 'index' variable. Retrieve size of
    # QLocale data array from variable in qlocale.cpp.
    # Default is 368 in Qt 4.8, 438 in Qt 5.0.1, the last one
    # being 'System'.
    #global qqLocalesCount
    #if qqLocalesCount is None:
    #    #try:
    #        qqLocalesCount = int(value(d.ns + 'locale_data_size'))
    #    #except:
    #        qqLocalesCount = 438
    #try:
    #    index = int(value["p"]["index"])
    #except:
    #    try:
    #        index = int(value["d"]["d"]["m_index"])
    #    except:
    #        index = int(value["d"]["d"]["m_data"]...)
    #check(index >= 0)
    #check(index <= qqLocalesCount)
    d.putStringValue(call(value, "name"))
    d.putNumChild(0)
    return
    # FIXME: Poke back for variants.
    if d.isExpanded():
        with Children(d, childType=d.lookupType(d.ns + "QChar"), childNumChild=0):
            d.putCallItem("country", value, "country")
            d.putCallItem("language", value, "language")
            d.putCallItem("measurementSystem", value, "measurementSystem")
            d.putCallItem("numberOptions", value, "numberOptions")
            d.putCallItem("timeFormat_(short)", value,
                "timeFormat", d.ns + "QLocale::ShortFormat")
            d.putCallItem("timeFormat_(long)", value,
                "timeFormat", d.ns + "QLocale::LongFormat")
            d.putCallItem("decimalPoint", value, "decimalPoint")
            d.putCallItem("exponential", value, "exponential")
            d.putCallItem("percent", value, "percent")
            d.putCallItem("zeroDigit", value, "zeroDigit")
            d.putCallItem("groupSeparator", value, "groupSeparator")
            d.putCallItem("negativeSign", value, "negativeSign")


def qdump__QMapNode(d, value):
    d.putEmptyValue()
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putSubItem("key", value["key"])
            d.putSubItem("value", value["value"])


def qdumpHelper__Qt4_QMap(d, value, forceLong):
    d_ptr = value["d"].dereference()
    e_ptr = value["e"].dereference()
    n = int(d_ptr["size"])
    check(0 <= n and n <= 100*1000*1000)
    checkRef(d_ptr["ref"])

    d.putItemCount(n)
    d.putNumChild(n)
    if d.isExpanded():
        if n > 10000:
            n = 10000

        keyType = d.templateArgument(value.type, 0)
        valueType = d.templateArgument(value.type, 1)
        isCompact = d.isMapCompact(keyType, valueType)

        it = e_ptr["forward"].dereference()

        # QMapPayloadNode is QMapNode except for the 'forward' member, so
        # its size is most likely the offset of the 'forward' member therein.
        # Or possibly 2 * sizeof(void *)
        nodeType = d.lookupType(d.ns + "QMapNode<%s, %s>" % (keyType, valueType))
        nodePointerType = nodeType.pointer()
        payloadSize = nodeType.sizeof - 2 * nodePointerType.sizeof

        if isCompact:
            innerType = valueType
        else:
            innerType = nodeType

        with Children(d, n, childType=innerType):
            for i in xrange(n):
                base = it.cast(d.charPtrType()) - payloadSize
                node = base.cast(nodePointerType).dereference()
                with SubItem(d, i):
                    d.putField("iname", d.currentIName)
                    if isCompact:
                        #d.putType(valueType)
                        if forceLong:
                            d.putName("[%s] %s" % (i, node["key"]))
                        else:
                            d.putMapName(node["key"])
                        d.putItem(node["value"])
                    else:
                        d.putItem(node)
                it = it.dereference()["forward"].dereference()


def qdumpHelper__Qt5_QMap(d, value, forceLong):
    d_ptr = value["d"].dereference()
    n = int(d_ptr["size"])
    check(0 <= n and n <= 100*1000*1000)
    checkRef(d_ptr["ref"])

    d.putItemCount(n)
    d.putNumChild(n)
    if d.isExpanded():
        if n > 10000:
            n = 10000

        keyType = d.templateArgument(value.type, 0)
        valueType = d.templateArgument(value.type, 1)
        isCompact = d.isMapCompact(keyType, valueType)
        nodeType = d.lookupType(d.ns + "QMapNode<%s, %s>" % (keyType, valueType))
        if isCompact:
            innerType = valueType
        else:
            innerType = nodeType

        with Children(d, n, childType=innerType):
            toDo = []
            i = -1
            node = d_ptr["header"]
            left = node["left"]
            if not isNull(left):
                toDo.append(left.dereference())
            right = node["right"]
            if not isNull(right):
                toDo.append(right.dereference())

            while len(toDo):
                node = toDo[0].cast(nodeType)
                toDo = toDo[1:]
                left = node["left"]
                if not isNull(left):
                    toDo.append(left.dereference())
                right = node["right"]
                if not isNull(right):
                    toDo.append(right.dereference())
                i += 1

                with SubItem(d, i):
                    d.putField("iname", d.currentIName)
                    if isCompact:
                        if forceLong:
                            d.putName("[%s] %s" % (i, node["key"]))
                        else:
                            d.putMapName(node["key"])
                        d.putItem(node["value"])
                    else:
                        qdump__QMapNode(d, node)


def qdumpHelper__QMap(d, value, forceLong):
    if fieldAt(value["d"].dereference().type, 0).name == "backward":
        qdumpHelper__Qt4_QMap(d, value, forceLong)
    else:
        qdumpHelper__Qt5_QMap(d, value, forceLong)

def qform__QMap():
    return mapForms()

def qdump__QMap(d, value):
    qdumpHelper__QMap(d, value, False)

def qform__QMultiMap():
    return mapForms()

def qdump__QMultiMap(d, value):
    qdumpHelper__QMap(d, value, True)


def extractCString(table, offset):
    result = ""
    while True:
        d = table[offset]
        if d == 0:
            break
        result += "%c" % d
        offset += 1
    return result


def qdump__QObject(d, value):
    #warn("OBJECT: %s " % value)
    d.tryPutObjectNameValue(value)

    try:
        privateTypeName = d.ns + "QObjectPrivate"
        privateType = d.lookupType(privateTypeName)
        staticMetaObject = value["staticMetaObject"]
    except:
        d.putPlainChildren(value)
        return
    #warn("SMO: %s " % staticMetaObject)
    #warn("SMO DATA: %s " % staticMetaObject["d"]["stringdata"])
    superData = staticMetaObject["d"]["superdata"]
    #warn("SUPERDATA: %s" % superData)
    #while not isNull(superData):
    #    superData = superData.dereference()["d"]["superdata"]
    #    warn("SUPERDATA: %s" % superData)

    if privateType is None:
        d.putNumChild(4)
        #d.putValue(cleanAddress(value.address))
        d.putPlainChildren(value)
        if d.isExpanded():
            with Children(d):
                d.putFields(value)
        return

    #warn("OBJECTNAME: %s " % objectName)
    dd = value["d_ptr"]["d"]
    d_ptr = dd.cast(privateType.pointer()).dereference()
    #warn("D_PTR: %s " % d_ptr)
    mo = d_ptr["metaObject"]
    if isNull(mo):
        mo = staticMetaObject
    #warn("MO: %s " % mo)
    #warn("MO.D: %s " % mo["d"])
    metaData = mo["d"]["data"]
    metaStringData = mo["d"]["stringdata"]
    # This is char * in Qt 4 and ByteArrayData * in Qt 5.
    # Force it to the char * data in the Qt 5 case.
    try:
        offset = metaStringData["offset"]
        metaStringData = metaStringData.cast(d.charPtrType()) + int(offset)
    except:
        pass

    #extradata = mo["d"]["extradata"]   # Capitalization!
    #warn("METADATA: %s " % metaData)
    #warn("STRINGDATA: %s " % metaStringData)
    #warn("TYPE: %s " % value.type)
    #warn("INAME: %s " % d.currentIName)
    d.putEmptyValue()
    #QSignalMapper::staticMetaObject
    #checkRef(d_ptr["ref"])
    d.putNumChild(4)
    if d.isExpanded():
      with Children(d):

        # Local data.
        if privateTypeName != d.ns + "QObjectPrivate":
            if not privateType is None:
              with SubItem(d, "data"):
                d.putEmptyValue()
                d.putNoType()
                d.putNumChild(1)
                if d.isExpanded():
                    with Children(d):
                        d.putFields(d_ptr, False)


        d.putFields(value)
        # Parent and children.
        if stripClassTag(str(value.type)) == d.ns + "QObject":
            d.putSubItem("parent", d_ptr["parent"])
            d.putSubItem("children", d_ptr["children"])

        # Properties.
        with SubItem(d, "properties"):
            # Prolog
            extraData = d_ptr["extraData"]   # Capitalization!
            if isNull(extraData):
                dynamicPropertyCount = 0
            else:
                extraDataType = d.lookupType(
                    d.ns + "QObjectPrivate::ExtraData").pointer()
                extraData = extraData.cast(extraDataType)
                ed = extraData.dereference()
                names = ed["propertyNames"]
                values = ed["propertyValues"]
                #userData = ed["userData"]
                namesBegin = names["d"]["begin"]
                namesEnd = names["d"]["end"]
                namesArray = names["d"]["array"]
                dynamicPropertyCount = namesEnd - namesBegin

            #staticPropertyCount = call(mo, "propertyCount")
            staticPropertyCount = metaData[6]
            #warn("PROPERTY COUNT: %s" % staticPropertyCount)
            propertyCount = staticPropertyCount + dynamicPropertyCount

            d.putNoType()
            d.putItemCount(propertyCount)
            d.putNumChild(propertyCount)

            if d.isExpanded():
                # FIXME: Make this global. Don't leak.
                variant = "'%sQVariant'" % d.ns
                # Avoid malloc symbol clash with QVector
                gdb.execute("set $d = (%s*)calloc(sizeof(%s), 1)"
                    % (variant, variant))
                gdb.execute("set $d.d.is_shared = 0")

                with Children(d):
                    # Dynamic properties.
                    if dynamicPropertyCount != 0:
                        dummyType = d.voidPtrType().pointer()
                        namesType = d.lookupType(d.ns + "QByteArray")
                        valuesBegin = values["d"]["begin"]
                        valuesEnd = values["d"]["end"]
                        valuesArray = values["d"]["array"]
                        valuesType = d.lookupType(d.ns + "QVariant")
                        p = namesArray.cast(dummyType) + namesBegin
                        q = valuesArray.cast(dummyType) + valuesBegin
                        for i in xrange(dynamicPropertyCount):
                            with SubItem(d, i):
                                pp = p.cast(namesType.pointer()).dereference();
                                d.putField("key", d.encodeByteArray(pp))
                                d.putField("keyencoded", Hex2EncodedLatin1)
                                qq = q.cast(valuesType.pointer().pointer())
                                qq = qq.dereference();
                                d.putField("addr", cleanAddress(qq))
                                d.putField("exp", "*(%s*)%s"
                                     % (variant, cleanAddress(qq)))
                                t = qdump__QVariant(d, qq)
                                # Override the "QVariant (foo)" output.
                                d.putBetterType(t)
                            p += 1
                            q += 1

                    # Static properties.
                    propertyData = metaData[7]
                    for i in xrange(staticPropertyCount):
                      with NoAddress(d):
                        offset = propertyData + 3 * i
                        propertyName = extractCString(metaStringData,
                                                      metaData[offset])
                        propertyType = extractCString(metaStringData,
                                                      metaData[offset + 1])
                        with SubItem(d, propertyName):
                            #flags = metaData[offset + 2]
                            #warn("FLAGS: %s " % flags)
                            #warn("PROPERTY: %s %s " % (propertyType, propertyName))
                            # #exp = '((\'%sQObject\'*)%s)->property("%s")' \
                            #     % (d.ns, value.address, propertyName)
                            #exp = '"((\'%sQObject\'*)%s)"' %
                            #(d.ns, value.address,)
                            #warn("EXPRESSION:  %s" % exp)
                            prop = call(value, "property",
                                str(cleanAddress(metaStringData + metaData[offset])))
                            value1 = prop["d"]
                            #warn("   CODE: %s" % value1["type"])
                            # Type 1 and 2 are bool and int.
                            # Try to save a few cycles in this case:
                            if int(value1["type"]) > 2:
                                # Poke back prop
                                gdb.execute("set $d.d.data.ull = %s"
                                        % value1["data"]["ull"])
                                gdb.execute("set $d.d.type = %s"
                                        % value1["type"])
                                gdb.execute("set $d.d.is_null = %s"
                                        % value1["is_null"])
                                prop = parseAndEvaluate("$d").dereference()
                            val, inner, innert, handled = \
                                qdumpHelper__QVariant(d, prop)

                            if handled:
                                pass
                            elif len(inner):
                                # Build-in types.
                                d.putType(inner)
                                d.putItem(val)
                            else:
                                # User types.
                           #    func = "typeToName(('%sQVariant::Type')%d)"
                           #       % (d.ns, variantType)
                           #    type = str(call(value, func))
                           #    type = type[type.find('"') + 1 : type.rfind('"')]
                           #    type = type.replace("Q", d.ns + "Q") # HACK!
                           #    data = call(value, "constData")
                           #    tdata = data.cast(d.lookupType(type).pointer())
                           #      .dereference()
                           #    d.putValue("(%s)" % tdata.type)
                           #    d.putType(tdata.type)
                           #    d.putNumChild(1)
                           #    if d.isExpanded():
                           #        with Children(d):
                           #           d.putSubItem("data", tdata)
                                warn("FIXME: CUSTOM QOBJECT PROPERTY: %s %s"
                                    % (propertyType, innert))
                                d.putType(propertyType)
                                d.putValue("...")
                                d.putNumChild(0)

        # Connections.
        with SubItem(d, "connections"):
            d.putNoType()
            connections = d_ptr["connectionLists"]
            connectionListCount = 0
            if not isNull(connections):
                connectionListCount = connections["d"]["size"]
            d.putItemCount(connectionListCount, 0)
            d.putNumChild(connectionListCount)
            if d.isExpanded():
                pp = 0
                with Children(d):
                    vectorType = fieldAt(connections.type.target(), 0).type
                    innerType = d.templateArgument(vectorType, 0)
                    # Should check:  innerType == ns::QObjectPrivate::ConnectionList
                    p = gdb.Value(connections["p"]["array"]).cast(innerType.pointer())
                    for i in xrange(connectionListCount):
                        first = p.dereference()["first"]
                        while not isNull(first):
                            with SubItem(d, pp):
                                connection = first.dereference()
                                d.putItem(connection)
                                d.putValue(connection["callFunction"])
                            first = first["nextConnectionList"]
                            # We need to enforce some upper limit.
                            pp += 1
                            if pp > 1000:
                                break
                        p += 1
                if pp < 1000:
                    d.putItemCount(pp)


        # Signals.
        signalCount = metaData[13]
        with SubItem(d, "signals"):
            d.putItemCount(signalCount)
            d.putNoType()
            d.putNumChild(signalCount)
            if signalCount:
                # FIXME: empty type does not work for childtype
                #d.putField("childtype", ".")
                d.putField("childnumchild", "0")
            if d.isExpanded():
                with Children(d):
                    for signal in xrange(signalCount):
                        with SubItem(d, signal):
                            offset = metaData[14 + 5 * signal]
                            d.putName("signal %d" % signal)
                            d.putNoType()
                            d.putValue(extractCString(metaStringData, offset))
                            d.putNumChild(0)  # FIXME: List the connections here.

        # Slots.
        with SubItem(d, "slots"):
            slotCount = metaData[4] - signalCount
            d.putItemCount(slotCount)
            d.putNoType()
            d.putNumChild(slotCount)
            if slotCount:
                #d.putField("childtype", ".")
                d.putField("childnumchild", "0")
            if d.isExpanded():
                with Children(d):
                    for slot in xrange(slotCount):
                        with SubItem(d, slot):
                            offset = metaData[14 + 5 * (signalCount + slot)]
                            d.putName("slot %d" % slot)
                            d.putNoType()
                            d.putValue(extractCString(metaStringData, offset))
                            d.putNumChild(0)  # FIXME: List the connections here.

        # Active connection.
        with SubItem(d, "currentSender"):
            d.putNoType()
            sender = d_ptr["currentSender"]
            d.putValue(cleanAddress(sender))
            if isNull(sender):
                d.putNumChild(0)
            else:
                d.putNumChild(1)
                if d.isExpanded():
                    with Children(d):
                        # Sending object
                        d.putSubItem("object", sender["sender"])
                        # Signal in sending object
                        with SubItem(d, "signal"):
                            d.putValue(sender["signal"])
                            d.putNoType()
                            d.putNumChild(0)

# QObject

#   static const uint qt_meta_data_QObject[] = {

#   int revision;
#   int className;
#   int classInfoCount, classInfoData;
#   int methodCount, methodData;
#   int propertyCount, propertyData;
#   int enumeratorCount, enumeratorData;
#   int constructorCount, constructorData; //since revision 2
#   int flags; //since revision 3
#   int signalCount; //since revision 4

#    // content:
#          4,       // revision
#          0,       // classname
#          0,    0, // classinfo
#          4,   14, // methods
#          1,   34, // properties
#          0,    0, // enums/sets
#          2,   37, // constructors
#          0,       // flags
#          2,       // signalCount

#  /* 14 */

#    // signals: signature, parameters, type, tag, flags
#          9,    8,    8,    8, 0x05,
#         29,    8,    8,    8, 0x25,

#  /* 24 */
#    // slots: signature, parameters, type, tag, flags
#         41,    8,    8,    8, 0x0a,
#         55,    8,    8,    8, 0x08,

#  /* 34 */
#    // properties: name, type, flags
#         90,   82, 0x0a095103,

#  /* 37 */
#    // constructors: signature, parameters, type, tag, flags
#        108,  101,    8,    8, 0x0e,
#        126,    8,    8,    8, 0x2e,

#          0        // eod
#   };

#   static const char qt_meta_stringdata_QObject[] = {
#       "QObject\0\0destroyed(QObject*)\0destroyed()\0"
#       "deleteLater()\0_q_reregisterTimers(void*)\0"
#       "QString\0objectName\0parent\0QObject(QObject*)\0"
#       "QObject()\0"
#   };


# QSignalMapper

#   static const uint qt_meta_data_QSignalMapper[] = {

#    // content:
#          4,       // revision
#          0,       // classname
#          0,    0, // classinfo
#          7,   14, // methods
#          0,    0, // properties
#          0,    0, // enums/sets
#          0,    0, // constructors
#          0,       // flags
#          4,       // signalCount

#    // signals: signature, parameters, type, tag, flags
#         15,   14,   14,   14, 0x05,
#         27,   14,   14,   14, 0x05,
#         43,   14,   14,   14, 0x05,
#         60,   14,   14,   14, 0x05,

#    // slots: signature, parameters, type, tag, flags
#         77,   14,   14,   14, 0x0a,
#         90,   83,   14,   14, 0x0a,
#        104,   14,   14,   14, 0x08,

#          0        // eod
#   };

#   static const char qt_meta_stringdata_QSignalMapper[] = {
#       "QSignalMapper\0\0mapped(int)\0mapped(QString)\0"
#       "mapped(QWidget*)\0mapped(QObject*)\0"
#       "map()\0sender\0map(QObject*)\0"
#       "_q_senderDestroyed()\0"
#   };

#   const QMetaObject QSignalMapper::staticMetaObject = {
#       { &QObject::staticMetaObject, qt_meta_stringdata_QSignalMapper,
#         qt_meta_data_QSignalMapper, 0 }
#   };



# // Meta enumeration helpers
# static inline void dumpMetaEnumType(QDumper &d, const QMetaEnum &me)
# {
#     QByteArray type = me.scope()
#     if !type.isEmpty())
#         type += "::"
#     type += me.name()
#     d.putField("type", type.constData())
# }
#
# static inline void dumpMetaEnumValue(QDumper &d, const QMetaProperty &mop,
#                                      int value)
# {
#
#     const QMetaEnum me = mop.enumerator()
#     dumpMetaEnumType(d, me)
#     if const char *enumValue = me.valueToKey(value)) {
#         d.putValue(enumValue)
#     } else {
#         d.putValue(value)
#     }
#     d.putField("numchild", 0)
# }
#
# static inline void dumpMetaFlagValue(QDumper &d, const QMetaProperty &mop,
#                                      int value)
# {
#     const QMetaEnum me = mop.enumerator()
#     dumpMetaEnumType(d, me)
#     const QByteArray flagsValue = me.valueToKeys(value)
#     if flagsValue.isEmpty():
#         d.putValue(value)
#     else:
#         d.putValue(flagsValue.constData())
#     d.putNumChild(0)
# }

def qdump__QPixmap(d, value):
    painters = int(value["painters"])
    check(0 <= painters and painters < 1000)
    d_ptr = value["data"]["d"]
    if isNull(d_ptr):
        d.putValue("(null)")
    else:
        checkSimpleRef(d_ptr["ref"])
        d.putValue("(%dx%d)" % (d_ptr["w"], d_ptr["h"]))
    d.putNumChild(0)


def qdump__QPoint(d, value):
    x = int(value["xp"])
    y = int(value["yp"])
    d.putValue("(%s, %s)" % (x, y))
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__QPointF(d, value):
    x = float(value["xp"])
    y = float(value["yp"])
    d.putValue("(%s, %s)" % (x, y))
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__QRect(d, value):
    def pp(l):
        if l >= 0: return "+%s" % l
        return l
    x1 = value["x1"]
    y1 = value["y1"]
    x2 = value["x2"]
    y2 = value["y2"]
    w = x2 - x1 + 1
    h = y2 - y1 + 1
    d.putValue("%sx%s%s%s" % (w, h, pp(x1), pp(y1)))
    d.putNumChild(4)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__QRectF(d, value):
    def pp(l):
        if l >= 0: return "+%s" % l
        return l
    x = value["xp"]
    y = value["yp"]
    w = value["w"]
    h = value["h"]
    # FIXME: workaround, see QPoint
    x = x.cast(x.type.strip_typedefs())
    y = y.cast(y.type.strip_typedefs())
    w = w.cast(w.type.strip_typedefs())
    h = h.cast(h.type.strip_typedefs())
    d.putValue("%sx%s%s%s" % (w, h, pp(x), pp(y)))
    d.putNumChild(4)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__QRegExp(d, value):
    d.putStringValue(value["priv"]["engineKey"]["pattern"])
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            # FIXME: Remove need to call
            call(value, "capturedTexts") # create cache
            with SubItem(d, "syntax"):
                d.putItem(value["priv"]["engineKey"]["patternSyntax"])
            with SubItem(d, "captures"):
                d.putItem(value["priv"]["capturedCache"])


def qdump__QRegion(d, value):
    p = value["d"].dereference()["qt_rgn"]
    if isNull(p):
        d.putValue("<empty>")
        d.putNumChild(0)
    else:
        try:
            # Fails without debug info.
            n = int(p.dereference()["numRects"])
            d.putItemCount(n)
            d.putNumChild(n)
            if d.isExpanded():
                with Children(d):
                    d.putFields(p.dereference())
        except:
            warn("NO DEBUG INFO")
            d.putValue(p)
            d.putPlainChildren(value)

# qt_rgn might be 0
# gdb.parse_and_eval("region")["d"].dereference()["qt_rgn"].dereference()

def qdump__QScopedPointer(d, value):
    d.putBetterType(d.currentType)
    d.putItem(value["d"])


def qdump__QSet(d, value):

    def hashDataFirstNode(value):
        val = value.cast(hashDataType)
        bucket = val["buckets"]
        e = value.cast(hashNodeType)
        for n in xrange(val["numBuckets"] - 1, -1, -1):
            n = n - 1
            if n < 0:
                break
            if bucket.dereference() != e:
                return bucket.dereference()
            bucket = bucket + 1
        return e

    def hashDataNextNode(node):
        next = node["next"]
        if next["next"]:
            return next
        d = node.cast(hashDataType.pointer()).dereference()
        numBuckets = d["numBuckets"]
        start = (node["h"] % numBuckets) + 1
        bucket = d["buckets"] + start
        for n in xrange(numBuckets - start):
            if bucket.dereference() != next:
                return bucket.dereference()
            bucket += 1
        return node

    keyType = d.templateArgument(value.type, 0)

    d_ptr = value["q_hash"]["d"]
    e_ptr = value["q_hash"]["e"]
    size = int(d_ptr["size"])

    hashDataType = d_ptr.type
    hashNodeType = e_ptr.type

    check(0 <= size and size <= 100 * 1000 * 1000)
    checkRef(d_ptr["ref"])

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        isSimpleKey = isSimpleType(keyType)
        node = hashDataFirstNode(value)
        innerType = e_ptr.dereference().type
        with Children(d, size, maxNumChild=1000, childType=innerType):
            for i in xrange(size):
                it = node.dereference().cast(innerType)
                with SubItem(d, i):
                    key = it["key"]
                    if isSimpleKey:
                        d.putType(keyType)
                        d.putItem(key)
                        d.putName(key)
                    else:
                        d.putItem(key)
                node = hashDataNextNode(node)


def qdump__QSharedData(d, value):
    d.putValue("ref: %s" % value["ref"]["_q_value"])
    d.putNumChild(0)


def qdump__QSharedDataPointer(d, value):
    d_ptr = value["d"]
    if isNull(d_ptr):
        d.putValue("(null)")
        d.putNumChild(0)
    else:
        # This replaces the pointer by the pointee, making the
        # pointer transparent.
        try:
            innerType = d.templateArgument(value.type, 0)
        except:
            d.putValue(d_ptr)
            d.putPlainChildren(value)
            return
        d.putBetterType(d.currentType)
        d.putItem(gdb.Value(d_ptr.cast(innerType.pointer())).dereference())
        # d.putItem(value.dereference())


def qdump__QSharedPointer(d, value):
    qdump__QWeakPointer(d, value)


def qdump__QSize(d, value):
    w = value["wd"]
    h = value["ht"]
    d.putValue("(%s, %s)" % (w, h))
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__QSizeF(d, value):
    qdump__QSize(d, value)


def qdump__QStack(d, value):
    qdump__QVector(d, value)


def qdump__QStandardItem(d, value):
    d.putBetterType(d.currentType)
    try:
        d.putItem(value["d_ptr"])
    except:
        d.putPlainChildren(value)


def qedit__QString(expr, value):
    cmd = "call (%s).resize(%d)" % (expr, len(value))
    gdb.execute(cmd)
    d = gdb.parse_and_eval(expr)["d"]["data"]
    cmd = "set {short[%d]}%s={" % (len(value), pointerValue(d))
    for i in range(len(value)):
        if i != 0:
            cmd += ','
        cmd += str(ord(value[i]))
    cmd += '}'
    gdb.execute(cmd)

def qform__QString():
    return "Inline,Separate Window"

def qdump__QString(d, value):
    d.putStringValue(value)
    d.putNumChild(0)
    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        d.putField("editformat", DisplayUtf16String)
        d.putField("editvalue", d.encodeString(value, None))


def qdump__QStringRef(d, value):
    s = value["m_string"].dereference()
    data, size, alloc = d.stringData(s)
    data += 2 * int(value["m_position"])
    size = value["m_size"]
    s = d.readRawMemory(data, 2 * size)
    d.putValue(s, Hex4EncodedLittleEndian)
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__QStringList(d, value):
    listType = directBaseClass(value.type).type
    qdump__QList(d, value.cast(listType))
    d.putBetterType(value.type)


def qdump__QTemporaryFile(d, value):
    qdump__QFile(d, value)


def qdump__QTextCodec(d, value):
    name = call(value, "name")
    d.putValue(d.encodeByteArray(d, name), 6)
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putCallItem("name", value, "name")
            d.putCallItem("mibEnum", value, "mibEnum")


def qdump__QTextCursor(d, value):
    dd = value["d"]["d"]
    if isNull(dd):
        d.putValue("(invalid)")
        d.putNumChild(0)
    else:
        try:
            p = dd.dereference()
            d.putValue(p["position"])
        except:
            d.putPlainChildren(value)
            return
        d.putNumChild(1)
        if d.isExpanded():
            with Children(d):
                d.putIntItem("position", p["position"])
                d.putIntItem("anchor", p["anchor"])
                d.putCallItem("selected", value, "selectedText")


def qdump__QTextDocument(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putCallItem("blockCount", value, "blockCount")
            d.putCallItem("characterCount", value, "characterCount")
            d.putCallItem("lineCount", value, "lineCount")
            d.putCallItem("revision", value, "revision")
            d.putCallItem("toPlainText", value, "toPlainText")


def qdump__QUrl(d, value):
    try:
        # Qt 4
        data = value["d"].dereference()
        d.putByteArrayValue(data["encodedOriginal"])
    except:
        try:
            # Qt 5
            data = value["d"].dereference()
            str = d.encodeString(data["scheme"])
            str += "3a002f002f00"
            str += d.encodeString(data["host"])
            str += d.encodeString(data["path"])
            d.putValue(str, Hex4EncodedLittleEndian)
        except:
            d.putPlainChildren(value)
            return
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
           d.putFields(data)


def qdumpHelper_QVariant_0(d, data):
    # QVariant::Invalid
    d.putBetterType("%sQVariant (invalid)" % d.ns)
    d.putValue("(invalid)")

def qdumpHelper_QVariant_1(d, data):
    # QVariant::Bool
    d.putBetterType("%sQVariant (bool)" % d.ns)
    if int(data["b"]):
        d.putValue("true")
    else:
        d.putValue("false")

def qdumpHelper_QVariant_2(d, data):
    # QVariant::Int
    d.putBetterType("%sQVariant (int)" % d.ns)
    d.putValue(data["i"])

def qdumpHelper_QVariant_3(d, data):
    # uint
    d.putBetterType("%sQVariant (uint)" % d.ns)
    d.putValue(data["u"])

def qdumpHelper_QVariant_4(d, data):
    # qlonglong
    d.putBetterType("%sQVariant (qlonglong)" % d.ns)
    d.putValue(data["ll"])

def qdumpHelper_QVariant_5(d, data):
    # qulonglong
    d.putBetterType("%sQVariant (qulonglong)" % d.ns)
    d.putValue(data["ull"])

def qdumpHelper_QVariant_6(d, data):
    # QVariant::Double
    d.putBetterType("%sQVariant (double)" % d.ns)
    d.putValue(data["d"])

qdumpHelper_QVariants_A = [
    qdumpHelper_QVariant_0,
    qdumpHelper_QVariant_1,
    qdumpHelper_QVariant_2,
    qdumpHelper_QVariant_3,
    qdumpHelper_QVariant_4,
    qdumpHelper_QVariant_5,
    qdumpHelper_QVariant_6
]


qdumpHelper_QVariants_B = [
    "QChar",       # 7
    None,          # 8, QVariantMap
    None,          # 9, QVariantList
    "QString",     # 10
    "QStringList", # 11
    "QByteArray",  # 12
    "QBitArray",   # 13
    "QDate",       # 14
    "QTime",       # 15
    "QDateTime",   # 16
    "QUrl",        # 17
    "QLocale",     # 18
    "QRect",       # 19
    "QRectF",      # 20
    "QSize",       # 21
    "QSizeF",      # 22
    "QLine",       # 23
    "QLineF",      # 24
    "QPoint",      # 25
    "QPointF",     # 26
    "QRegExp",     # 27
    None,          # 28, QVariantHash
]

qdumpHelper_QVariants_C = [
    "QFont",       # 64
    "QPixmap",     # 65
    "QBrush",      # 66
    "QColor",      # 67
    "QPalette",    # 68
    "QIcon",       # 69
    "QImage",      # 70
    "QPolygon",    # 71
    "QRegion",     # 72
    "QBitmap",     # 73
    "QCursor",     # 74
    "QSizePolicy", # 75
    "QKeySequence",# 76
    "QPen",        # 77
    "QTextLength", # 78
    "QTextFormat", # 79
    "X",           # 80
    "QTransform",  # 81
    "QMatrix4x4",  # 82
    "QVector2D",   # 83
    "QVector3D",   # 84
    "QVector4D",   # 85
    "QQuadernion"  # 86
]

def qdumpHelper__QVariant(d, value):
    data = value["d"]["data"]
    variantType = int(value["d"]["type"])
    #warn("VARIANT TYPE: %s : " % variantType)

    if variantType <= 6:
        qdumpHelper_QVariants_A[variantType](d, data)
        d.putNumChild(0)
        return (None, None, None, True)

    inner = ""
    innert = ""
    val = None

    if variantType <= 28:
        inner = qdumpHelper_QVariants_B[variantType - 7]
        if not inner is None:
            innert = inner
        elif variantType == 8:  # QVariant::VariantMap
            inner = d.ns + "QMap<" + d.ns + "QString, " + d.ns + "QVariant>"
            innert = d.ns + "QVariantMap"
        elif variantType == 9:  # QVariant::VariantList
            inner = d.ns + "QList<" + d.ns + "QVariant>"
            innert = d.ns + "QVariantList"
        elif variantType == 28: # QVariant::VariantHash
            inner = d.ns + "QHash<" + d.ns + "QString, " + d.ns + "QVariant>"
            innert = d.ns + "QVariantHash"

    elif variantType <= 86:
        inner = d.ns + qdumpHelper_QVariants_B[variantType - 64]
        innert = inner

    if len(inner):
        innerType = d.lookupType(inner)
        sizePD = d.lookupType(d.ns + 'QVariant::Private::Data').sizeof
        if innerType.sizeof > sizePD:
            sizePS = d.lookupType(d.ns + 'QVariant::PrivateShared').sizeof
            val = (sizePS + data.cast(d.charPtrType())) \
                .cast(innerType.pointer()).dereference()
        else:
            val = data.cast(innerType)

    return (val, inner, innert, False)


def qdump__QVariant(d, value):
    d_ptr = value["d"]
    d_data = d_ptr["data"]

    (val, inner, innert, handled) = qdumpHelper__QVariant(d, value)

    if handled:
        return

    if len(inner):
        innerType = d.lookupType(inner)
        # FIXME: Why "shared"?
        if innerType.sizeof > d_data.type.sizeof:
            v = d_data["shared"]["ptr"].cast(innerType.pointer()).dereference()
        else:
            v = d_data.cast(innerType)
        d.putEmptyValue(-99)
        d.putItem(v)
        d.putBetterType("%sQVariant (%s)" % (d.ns, innert))
        return innert

    # User types.
    if gdbLoaded:
        type = str(call(value, "typeToName",
            "('%sQVariant::Type')%d" % (d.ns, d_ptr["type"])))
    if lldbLoaded:
        type = str(call(value, "typeToName",
            "(%sQVariant::Type)%d" % (d.ns, d_ptr["type"])))
    type = type[type.find('"') + 1 : type.rfind('"')]
    type = type.replace("Q", d.ns + "Q") # HACK!
    type = type.replace("uint", "unsigned int") # HACK!
    type = type.replace("COMMA", ",") # HACK!
    #warn("TYPE: %s" % type)
    data = call(value, "constData")
    #warn("DATA: %s" % data)
    d.putEmptyValue(-99)
    d.putType("%sQVariant (%s)" % (d.ns, type))
    d.putNumChild(1)
    tdata = data.cast(d.lookupType(type).pointer()).dereference()
    if d.isExpanded():
        with Children(d):
            with NoAddress(d):
                d.putSubItem("data", tdata)
    return tdata.type


def qedit__QVector(expr, value):
    values = value.split(',')
    ob = gdb.parse_and_eval(expr)
    cmd = "call (%s).resize(%d)" % (expr, len(values))
    gdb.execute(cmd)
    innerType = d.templateArgument(ob.type, 0)
    ptr = ob["p"]["array"].cast(d.voidPtrType())
    cmd = "set {%s[%d]}%s={%s}" % (innerType, len(values), pointerValue(ptr), value)
    gdb.execute(cmd)


def qform__QVector():
    return arrayForms()


def qdump__QVector(d, value):
    private = value["d"]
    checkRef(private["ref"])
    alloc = int(private["alloc"])
    size = int(private["size"])
    innerType = d.templateArgument(value.type, 0)
    try:
        # Qt 5. Will fail on Qt 4 due to the missing 'offset' member.
        offset = private["offset"]
        data = private.cast(d.charPtrType()) + offset
    except:
        # Qt 4.
        data = value["p"]["array"]

    p = data.cast(innerType.pointer())

    check(0 <= size and size <= alloc and alloc <= 1000 * 1000 * 1000)
    d.putItemCount(size)
    d.putNumChild(size)
    d.putPlotData(innerType, p, size, 2)


def qdump__QWeakPointer(d, value):
    d_ptr = value["d"]
    val = value["value"]
    if isNull(d_ptr) and isNull(val):
        d.putValue("(null)")
        d.putNumChild(0)
        return
    if isNull(d_ptr) or isNull(val):
        d.putValue("<invalid>")
        d.putNumChild(0)
        return
    weakref = int(d_ptr["weakref"]["_q_value"])
    strongref = int(d_ptr["strongref"]["_q_value"])
    check(strongref >= -1)
    check(strongref <= weakref)
    check(weakref <= 10*1000*1000)

    if isSimpleType(val.dereference().type):
        d.putNumChild(3)
        d.putItem(val.dereference())
    else:
        d.putEmptyValue()

    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            innerType = d.templateArgument(value.type, 0)
            d.putSubItem("data", val.dereference().cast(innerType))
            d.putIntItem("weakref", weakref)
            d.putIntItem("strongref", strongref)


def qdump__QxXmlAttributes(d, value):
    pass


#######################################################################
#
# Standard Library dumper
#
#######################################################################

def qdump____c_style_array__(d, value):
    type = value.type.unqualified()
    targetType = value[0].type
    #d.putAddress(value.address)
    d.putType(type)
    d.putNumChild(1)
    format = d.currentItemFormat()
    isDefault = format == None and str(targetType.unqualified()) == "char"
    if isDefault or (format >= 0 and format <= 2):
        blob = d.readRawMemory(value.address, type.sizeof)

    if isDefault:
        # Use Latin1 as default for char [].
        d.putValue(blob, Hex2EncodedLatin1)
    elif format == 0:
        # Explicitly requested Latin1 formatting.
        d.putValue(blob, Hex2EncodedLatin1)
    elif format == 1:
        # Explicitly requested UTF-8 formatting.
        d.putValue(blob, Hex2EncodedUtf8)
    elif format == 2:
        # Explicitly requested Local 8-bit formatting.
        d.putValue(blob, Hex2EncodedLocal8Bit)
    else:
        d.putValue("@0x%x" % pointerValue(value.cast(targetType.pointer())))

    if d.currentIName in d.expandedINames:
        p = value.address
        ts = targetType.sizeof
        if not d.tryPutArrayContents(targetType, p, type.sizeof / ts):
            with Children(d, childType=targetType,
                    addrBase=p, addrStep=ts):
                d.putFields(value)


def qdump__std__array(d, value):
    size = d.numericTemplateArgument(value.type, 1)
    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        innerType = d.templateArgument(value.type, 0)
        d.putArrayData(innerType, value.address, size)


def qdump__std__complex(d, value):
    innerType = d.templateArgument(value.type, 0)
    base = value.address.cast(innerType.pointer())
    real = base.dereference()
    imag = (base + 1).dereference()
    d.putValue("(%f, %f)" % (real, imag));
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d, 2, childType=innerType):
            d.putSubItem("real", real)
            d.putSubItem("imag", imag)


def qdump__std__deque(d, value):
    innerType = d.templateArgument(value.type, 0)
    innerSize = innerType.sizeof
    bufsize = 1
    if innerSize < 512:
        bufsize = 512 / innerSize

    impl = value["_M_impl"]
    start = impl["_M_start"]
    finish = impl["_M_finish"]
    size = (bufsize * (finish["_M_node"] - start["_M_node"] - 1)
        + (finish["_M_cur"] - finish["_M_first"])
        + (start["_M_last"] - start["_M_cur"]))

    check(0 <= size and size <= 1000 * 1000 * 1000)
    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        with Children(d, size, maxNumChild=2000, childType=innerType):
            pcur = start["_M_cur"]
            pfirst = start["_M_first"]
            plast = start["_M_last"]
            pnode = start["_M_node"]
            for i in d.childRange():
                d.putSubItem(i, pcur.dereference())
                pcur += 1
                if pcur == plast:
                    newnode = pnode + 1
                    pnode = newnode
                    pfirst = newnode.dereference()
                    plast = pfirst + bufsize
                    pcur = pfirst

def qdump__std____debug__deque(d, value):
    qdump__std__deque(d, value)


def qdump__std__list(d, value):
    impl = value["_M_impl"]
    node = impl["_M_node"]
    head = node.address
    size = 0
    p = node["_M_next"]
    while p != head and size <= 1001:
        size += 1
        p = p["_M_next"]

    d.putItemCount(size, 1000)
    d.putNumChild(size)

    if d.isExpanded():
        p = node["_M_next"]
        innerType = d.templateArgument(value.type, 0)
        with Children(d, size, maxNumChild=1000, childType=innerType):
            for i in d.childRange():
                innerPointer = innerType.pointer()
                d.putSubItem(i, (p + 1).cast(innerPointer).dereference())
                p = p["_M_next"]

def qdump__std____debug__list(d, value):
    qdump__std__list(d, value)

def qform__std__map():
    return mapForms()

def qdump__std__map(d, value):
    impl = value["_M_t"]["_M_impl"]
    size = int(impl["_M_node_count"])
    check(0 <= size and size <= 100*1000*1000)
    d.putItemCount(size)
    d.putNumChild(size)

    if d.isExpanded():
        keyType = d.templateArgument(value.type, 0)
        valueType = d.templateArgument(value.type, 1)
        try:
            # Does not work on gcc 4.4, the allocator type (fourth template
            # argument) seems not to be available.
            pairType = d.templateArgument(d.templateArgument(value.type, 3), 0)
            pairPointer = pairType.pointer()
        except:
            # So use this as workaround:
            pairType = d.templateArgument(impl.type, 1)
            pairPointer = pairType.pointer()
        isCompact = d.isMapCompact(keyType, valueType)
        innerType = pairType
        if isCompact:
            innerType = valueType
        node = impl["_M_header"]["_M_left"]
        childType = innerType
        if size == 0:
            childType = pairType
        childNumChild = 2
        if isCompact:
            childNumChild = None
        with Children(d, size, maxNumChild=1000,
                childType=childType, childNumChild=childNumChild):
            for i in d.childRange():
                with SubItem(d, i):
                    pair = (node + 1).cast(pairPointer).dereference()
                    if isCompact:
                        d.putMapName(pair["first"])
                        d.putItem(pair["second"])
                    else:
                        d.putEmptyValue()
                        if d.isExpanded():
                            with Children(d, 2):
                                d.putSubItem("first", pair["first"])
                                d.putSubItem("second", pair["second"])
                if isNull(node["_M_right"]):
                    parent = node["_M_parent"]
                    while node == parent["_M_right"]:
                        node = parent
                        parent = parent["_M_parent"]
                    if node["_M_right"] != parent:
                        node = parent
                else:
                    node = node["_M_right"]
                    while not isNull(node["_M_left"]):
                        node = node["_M_left"]

def qdump__std____debug__map(d, value):
    qdump__std__map(d, value)

def stdTreeIteratorHelper(d, value):
    pnode = value["_M_node"]
    node = pnode.dereference()
    d.putNumChild(1)
    d.putEmptyValue()
    if d.isExpanded():
        dataType = d.templateArgument(value.type, 0)
        nodeType = d.lookupType("std::_Rb_tree_node<%s>" % dataType)
        data = pnode.cast(nodeType.pointer()).dereference()["_M_value_field"]
        with Children(d):
            try:
                d.putSubItem("first", data["first"])
                d.putSubItem("second", data["second"])
            except:
                d.putSubItem("value", data)
            with SubItem(d, "node"):
                d.putNumChild(1)
                d.putEmptyValue()
                d.putType(" ")
                if d.isExpanded():
                    with Children(d):
                        d.putSubItem("color", node["_M_color"])
                        d.putSubItem("left", node["_M_left"])
                        d.putSubItem("right", node["_M_right"])
                        d.putSubItem("parent", node["_M_parent"])


def qdump__std___Rb_tree_iterator(d, value):
    stdTreeIteratorHelper(d, value)

def qdump__std___Rb_tree_const_iterator(d, value):
    stdTreeIteratorHelper(d, value)

def qdump__std__map__iterator(d, value):
    stdTreeIteratorHelper(d, value)

def qdump__std__map__const_iterator(d, value):
    stdTreeIteratorHelper(d, value)

def qdump__std__set__iterator(d, value):
    stdTreeIteratorHelper(d, value)

def qdump__std__set__const_iterator(d, value):
    stdTreeIteratorHelper(d, value)



def qdump__std__set(d, value):
    impl = value["_M_t"]["_M_impl"]
    size = int(impl["_M_node_count"])
    check(0 <= size and size <= 100*1000*1000)
    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        valueType = d.templateArgument(value.type, 0)
        node = impl["_M_header"]["_M_left"]
        with Children(d, size, maxNumChild=1000, childType=valueType):
            for i in d.childRange():
                d.putSubItem(i, (node + 1).cast(valueType.pointer()).dereference())
                if isNull(node["_M_right"]):
                    parent = node["_M_parent"]
                    while node == parent["_M_right"]:
                        node = parent
                        parent = parent["_M_parent"]
                    if node["_M_right"] != parent:
                        node = parent
                else:
                    node = node["_M_right"]
                    while not isNull(node["_M_left"]):
                        node = node["_M_left"]


def qdump__std__stack(d, value):
    qdump__std__deque(d, value["c"])

def qdump__std____debug__stack(d, value):
    qdump__std__stack(d, value)

def qform__std__string():
    return "Inline,In Separate Window"

def qdump__std__string(d, value):
    data = value["_M_dataplus"]["_M_p"]
    baseType = value.type.strip_typedefs()
    charSize = d.templateArgument(baseType, 0).sizeof
    # We can't lookup the std::string::_Rep type without crashing LLDB,
    # so hard-code assumption on member position
    # struct { size_type _M_length, size_type _M_capacity, int _M_refcount; }
    sizePtr = data.cast(d.sizetType().pointer())
    size = int(sizePtr[-3])
    alloc = int(sizePtr[-2])
    refcount = int(sizePtr[-1])
    check(refcount >= -1) # Can be -1 accoring to docs.
    check(0 <= size and size <= alloc and alloc <= 100*1000*1000)

    n = min(size, qqStringCutOff)
    mem = d.readRawMemory(data, n * charSize)
    if charSize == 1:
        encodingType = Hex2EncodedLatin1
        displayType = DisplayLatin1String
    elif charSize == 2:
        encodingType = Hex4EncodedLittleEndian
        displayType = DisplayUtf16String
    else:
        encodingType = Hex8EncodedLittleEndian
        displayType = DisplayUtf16String

    d.putNumChild(0)
    d.putValue(mem, encodingType)

    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        d.putField("editformat", displayType)
        if n != size:
            mem = d.readRawMemory(p, size * charType.sizeof)
        d.putField("editvalue", mem)

#def qdump__std__string(d, value):
#    data = value["__r_"]
#    d.putValue("SSSS")
#    d.putType("std::string")
#    d.putNumChild(1)
#    d.putPlainChildren(value)


def qdump__std__shared_ptr(d, value):
    i = value["_M_ptr"]
    if isNull(i):
        d.putValue("(null)")
        d.putNumChild(0)
        return

    if isSimpleType(d.templateArgument(value.type, 0)):
        d.putValue("%s @0x%x" % (i.dereference(), pointerValue(i)))
    else:
        i = expensiveDowncast(i)
        d.putValue("@0x%x" % pointerValue(i))

    d.putNumChild(3)
    with Children(d, 3):
        d.putSubItem("data", i)
        refcount = value["_M_refcount"]["_M_pi"]
        d.putIntItem("usecount", refcount["_M_use_count"])
        d.putIntItem("weakcount", refcount["_M_weak_count"])


def qdump__std__unique_ptr(d, value):
    i = value["_M_t"]["_M_head_impl"]
    if isNull(i):
        d.putValue("(null)")
        d.putNumChild(0)
        return

    if isSimpleType(d.templateArgument(value.type, 0)):
        d.putValue("%s @0x%x" % (i.dereference(), pointerValue(i)))
    else:
        i = expensiveDowncast(i)
        d.putValue("@0x%x" % pointerValue(i))

    d.putNumChild(1)
    with Children(d, 1):
        d.putSubItem("data", i)


def qedit__std__vector(expr, value):
    values = value.split(',')
    n = len(values)
    ob = gdb.parse_and_eval(expr)
    innerType = d.templateArgument(ob.type, 0)
    cmd = "set $d = (%s*)calloc(sizeof(%s)*%s,1)" % (innerType, innerType, n)
    gdb.execute(cmd)
    cmd = "set {void*[3]}%s = {$d, $d+%s, $d+%s}" % (ob.address, n, n)
    gdb.execute(cmd)
    cmd = "set (%s[%d])*$d={%s}" % (innerType, n, value)
    gdb.execute(cmd)

def qdump__std__vector(d, value):
    impl = value["_M_impl"]
    type = d.templateArgument(value.type, 0)
    alloc = impl["_M_end_of_storage"]
    isBool = str(type) == 'bool'
    if isBool:
        start = impl["_M_start"]["_M_p"]
        finish = impl["_M_finish"]["_M_p"]
        # FIXME: 8 is CHAR_BIT
        storage = d.lookupType("unsigned long")
        storagesize = storage.sizeof * 8
        size = (finish - start) * storagesize
        size += impl["_M_finish"]["_M_offset"]
        size -= impl["_M_start"]["_M_offset"]
    else:
        start = impl["_M_start"]
        finish = impl["_M_finish"]
        size = finish - start

    check(0 <= size and size <= 1000 * 1000 * 1000)
    check(finish <= alloc)
    checkPointer(start)
    checkPointer(finish)
    checkPointer(alloc)

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        if isBool:
            with Children(d, size, maxNumChild=10000, childType=type):
                for i in d.childRange():
                    q = start + i / storagesize
                    d.putBoolItem(str(i), (q.dereference() >> (i % storagesize)) & 1)
        else:
            d.putArrayData(type, start, size)

def qdump__std____debug__vector(d, value):
    qdump__std__vector(d, value)

def qedit__std__string(expr, value):
    cmd = "print (%s).assign(\"%s\")" % (expr, value)
    gdb.execute(cmd)

def qedit__string(expr, value):
    qedit__std__string(expr, value)

def qdump__string(d, value):
    qdump__std__string(d, value)

def qdump__std__wstring(d, value):
    qdump__std__string(d, value)

def qdump__std__basic_string(d, value):
    qdump__std__string(d, value)

def qdump__wstring(d, value):
    qdump__std__string(d, value)


def qdump____gnu_cxx__hash_set(d, value):
    ht = value["_M_ht"]
    size = int(ht["_M_num_elements"])
    check(0 <= size and size <= 1000 * 1000 * 1000)
    d.putItemCount(size)
    d.putNumChild(size)
    type = d.templateArgument(value.type, 0)
    d.putType("__gnu__cxx::hash_set<%s>" % type)
    if d.isExpanded():
        with Children(d, size, maxNumChild=1000, childType=type):
            buckets = ht["_M_buckets"]["_M_impl"]
            bucketStart = buckets["_M_start"]
            bucketFinish = buckets["_M_finish"]
            p = bucketStart
            itemCount = 0
            for i in xrange(bucketFinish - bucketStart):
                if not isNull(p.dereference()):
                    cur = p.dereference()
                    while not isNull(cur):
                        with SubItem(d, itemCount):
                            d.putValue(cur["_M_val"])
                            cur = cur["_M_next"]
                            itemCount += 1
                p = p + 1


#######################################################################
#
# Boost dumper
#
#######################################################################

def qdump__boost__bimaps__bimap(d, value):
    leftType = d.templateArgument(value.type, 0)
    rightType = d.templateArgument(value.type, 1)
    size = value["core"]["node_count"]
    d.putItemCount(size)
    d.putNumChild(size)
    #if d.isExpanded():
    d.putPlainChildren(value)


def qdump__boost__optional(d, value):
    if int(value["m_initialized"]) == 0:
        d.putValue("<uninitialized>")
        d.putNumChild(0)
    else:
        type = d.templateArgument(value.type, 0)
        storage = value["m_storage"]
        if type.code == ReferenceCode:
            d.putItem(storage.cast(type.target().pointer()).dereference())
        else:
            d.putItem(storage.cast(type))
        d.putBetterType(value.type)


def qdump__boost__shared_ptr(d, value):
    # s                  boost::shared_ptr<int>
    #    pn              boost::detail::shared_count
    #        pi_ 0x0     boost::detail::sp_counted_base *
    #    px      0x0     int *
    if isNull(value["pn"]["pi_"]):
        d.putValue("(null)")
        d.putNumChild(0)
        return

    if isNull(value["px"]):
        d.putValue("(null)")
        d.putNumChild(0)
        return

    countedbase = value["pn"]["pi_"].dereference()
    weakcount = int(countedbase["weak_count_"])
    usecount = int(countedbase["use_count_"])
    check(weakcount >= 0)
    check(weakcount <= int(usecount))
    check(usecount <= 10*1000*1000)

    val = value["px"].dereference()
    if isSimpleType(val.type):
        d.putNumChild(3)
        d.putItem(val)
        d.putBetterType(value.type)
    else:
        d.putEmptyValue()

    d.putNumChild(3)
    if d.isExpanded():
        with Children(d, 3):
            d.putSubItem("data", val)
            d.putIntItem("weakcount", weakcount)
            d.putIntItem("usecount", usecount)


def qdump__boost__gregorian__date(d, value):
    d.putValue(int(value["days_"]), JulianDate)
    d.putNumChild(0)


def qdump__boost__posix_time__ptime(d, item):
    ms = int(item["time_"]["time_count_"]["value_"]) / 1000
    d.putValue("%s/%s" % divmod(ms, 86400000), JulianDateAndMillisecondsSinceMidnight)
    d.putNumChild(0)


def qdump__boost__posix_time__time_duration(d, item):
    d.putValue(int(item["ticks_"]["value_"]) / 1000, MillisecondsSinceMidnight)
    d.putNumChild(0)


#######################################################################
#
# SSE
#
#######################################################################

def qform____m128():
    return "As Floats,As Doubles"

def qdump____m128(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        format = d.currentItemFormat()
        if format == 2: # As Double
            d.putArrayData(d.lookupType("double"), value.address, 2)
        else: # Default, As float
            d.putArrayData(d.lookupType("float"), value.address, 4)


#######################################################################
#
# Webkit
#
#######################################################################


def jstagAsString(tag):
    # enum { Int32Tag =        0xffffffff };
    # enum { CellTag =         0xfffffffe };
    # enum { TrueTag =         0xfffffffd };
    # enum { FalseTag =        0xfffffffc };
    # enum { NullTag =         0xfffffffb };
    # enum { UndefinedTag =    0xfffffffa };
    # enum { EmptyValueTag =   0xfffffff9 };
    # enum { DeletedValueTag = 0xfffffff8 };
    if tag == -1:
        return "Int32"
    if tag == -2:
        return "Cell"
    if tag == -3:
        return "True"
    if tag == -4:
        return "Null"
    if tag == -5:
        return "Undefined"
    if tag == -6:
        return "Empty"
    if tag == -7:
        return "Deleted"
    return "Unknown"



def qdump__QTJSC__JSValue(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            tag = value["u"]["asBits"]["tag"]
            payload = value["u"]["asBits"]["payload"]
            #d.putIntItem("tag", tag)
            with SubItem(d, "tag"):
                d.putValue(jstagAsString(int(tag)))
                d.putNoType()
                d.putNumChild(0)

            d.putIntItem("payload", int(payload))
            d.putFields(value["u"])

            if tag == -2:
                cellType = d.lookupType("QTJSC::JSCell").pointer()
                d.putSubItem("cell", payload.cast(cellType))

            try:
                # FIXME: This might not always be a variant.
                delegateType = d.lookupType(d.ns + "QScript::QVariantDelegate").pointer()
                delegate = scriptObject["d"]["delegate"].cast(delegateType)
                #d.putSubItem("delegate", delegate)
                variant = delegate["m_value"]
                d.putSubItem("variant", variant)
            except:
                pass


def qdump__QScriptValue(d, value):
    # structure:
    #  engine        QScriptEnginePrivate
    #  jscValue      QTJSC::JSValue
    #  next          QScriptValuePrivate *
    #  numberValue   5.5987310416280426e-270 myns::qsreal
    #  prev          QScriptValuePrivate *
    #  ref           QBasicAtomicInt
    #  stringValue   QString
    #  type          QScriptValuePrivate::Type: { JavaScriptCore, Number, String }
    #d.putEmptyValue()
    dd = value["d_ptr"]["d"]
    if isNull(dd):
        d.putValue("(invalid)")
        d.putNumChild(0)
        return
    if int(dd["type"]) == 1: # Number
        d.putValue(dd["numberValue"])
        d.putType("%sQScriptValue (Number)" % d.ns)
        d.putNumChild(0)
        return
    if int(dd["type"]) == 2: # String
        d.putStringValue(dd["stringValue"])
        d.putType("%sQScriptValue (String)" % d.ns)
        return

    d.putType("%sQScriptValue (JSCoreValue)" % d.ns)
    x = dd["jscValue"]["u"]
    tag = x["asBits"]["tag"]
    payload = x["asBits"]["payload"]
    #isValid = int(x["asBits"]["tag"]) != -6   # Empty
    #isCell = int(x["asBits"]["tag"]) == -2
    #warn("IS CELL: %s " % isCell)
    #isObject = False
    #className = "UNKNOWN NAME"
    #if isCell:
    #    # isCell() && asCell()->isObject();
    #    # in cell: m_structure->typeInfo().type() == ObjectType;
    #    cellType = d.lookupType("QTJSC::JSCell").pointer()
    #    cell = payload.cast(cellType).dereference()
    #    dtype = "NO DYNAMIC TYPE"
    #    try:
    #        dtype = cell.dynamic_type
    #    except:
    #        pass
    #    warn("DYNAMIC TYPE: %s" % dtype)
    #    warn("STATUC  %s" % cell.type)
    #    type = cell["m_structure"]["m_typeInfo"]["m_type"]
    #    isObject = int(type) == 7 # ObjectType;
    #    className = "UNKNOWN NAME"
    #warn("IS OBJECT: %s " % isObject)

    #inline bool JSCell::inherits(const ClassInfo* info) const
    #for (const ClassInfo* ci = classInfo(); ci; ci = ci->parentClass) {
    #    if (ci == info)
    #        return true;
    #return false;

    try:
        # This might already fail for "native" payloads.
        scriptObjectType = d.lookupType(d.ns + "QScriptObject").pointer()
        scriptObject = payload.cast(scriptObjectType)

        # FIXME: This might not always be a variant.
        delegateType = d.lookupType(d.ns + "QScript::QVariantDelegate").pointer()
        delegate = scriptObject["d"]["delegate"].cast(delegateType)
        #d.putSubItem("delegate", delegate)

        variant = delegate["m_value"]
        #d.putSubItem("variant", variant)
        t = qdump__QVariant(d, variant)
        # Override the "QVariant (foo)" output
        d.putBetterType("%sQScriptValue (%s)" % (d.ns, t))
        if t != "JSCoreValue":
            return
    except:
        pass

    # This is a "native" JSCore type for e.g. QDateTime.
    d.putValue("<native>")
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
           d.putSubItem("jscValue", dd["jscValue"])


#######################################################################
#
# Qt Creator
#
#######################################################################

def qdump__Core__Id(d, value):
    try:
        name = parseAndEvaluate("Core::nameForId(%d)" % value["m_id"])
        d.putValue(encodeCharArray(name), Hex2EncodedLatin1)
        d.putNumChild(1)
        if d.isExpanded():
            with Children(d):
                d.putFields(value)
    except:
        d.putValue(value["m_id"])
        d.putNumChild(0)

def qdump__Debugger__Internal__GdbMi(d, value):
    str = d.encodeByteArray(value["m_name"]) + "3a20" \
        + d.encodeByteArray(value["m_data"])
    d.putValue(str, Hex2EncodedLatin1)
    d.putPlainChildren(value)

def qdump__Debugger__Internal__WatchData(d, value):
    d.putByteArrayValue(value["iname"])
    d.putPlainChildren(value)

def qdump__Debugger__Internal__WatchItem(d, value):
    d.putByteArrayValue(value["iname"])
    d.putPlainChildren(value)

def qdump__Debugger__DebuggerEngine__BreakpointModelId(d, value):
    d.putValue("%s.%s" % (value["m_majorPart"], value["m_minorPart"]))
    d.putPlainChildren(value)

def qdump__CPlusPlus__ByteArrayRef(d, value):
    d.putValue(encodeCharArray(value["m_start"], 100, value["m_length"]),
        Hex2EncodedLatin1)
    d.putPlainChildren(value)

def qdump__CPlusPlus__Identifier(d, value):
    d.putValue(encodeCharArray(value["_chars"]), Hex2EncodedLatin1)
    d.putPlainChildren(value)

def qdump__CPlusPlus__IntegerType(d, value):
    d.putValue(value["_kind"])
    d.putPlainChildren(value)

def qdump__CPlusPlus__NamedType(d, value):
    literal = downcast(value["_name"])
    d.putValue(encodeCharArray(literal["_chars"]), Hex2EncodedLatin1)
    d.putPlainChildren(value)

def qdump__CPlusPlus__TemplateNameId(d, value):
    s = encodeCharArray(value["_identifier"]["_chars"])
    d.putValue(s + "3c2e2e2e3e", Hex2EncodedLatin1)
    d.putPlainChildren(value)

def qdump__CPlusPlus__Literal(d, value):
    d.putValue(encodeCharArray(value["_chars"]), Hex2EncodedLatin1)
    d.putPlainChildren(value)

def qdump__CPlusPlus__Internal__Value(d, value):
    d.putValue(value["l"])
    d.putPlainChildren(value)

def qdump__Utils__FileName(d, value):
    d.putStringValue(value)
    d.putPlainChildren(value)

def qdump__Utils__ElfSection(d, value):
    d.putByteArrayValue(value["name"])
    d.putPlainChildren(value)

def qdump__CPlusPlus__Token(d, value):
    k = int(value["f"]["kind"])
    if int(k) == 6:
        d.putValue("T_IDENTIFIER. offset: %d, len: %d"
            % (value["offset"], value["f"]["length"]))
    elif int(k) == 7:
        d.putValue("T_NUMERIC_LITERAL. offset: %d, value: %d"
            % (value["offset"], value["f"]["length"]))
    elif int(k) == 60:
        d.putValue("T_RPAREN")
    else:
        d.putValue("Type: %s" % k)
    d.putPlainChildren(value)

def qdump__CPlusPlus__Internal__PPToken(d, value):
    k = value["f"]["kind"];
    data, size, alloc = d.byteArrayData(value["m_src"])
    length = int(value["f"]["length"])
    offset = int(value["offset"])
    #warn("size: %s, alloc: %s, offset: %s, length: %s, data: %s"
    #    % (size, alloc, offset, length, data))
    d.putValue(encodeCharArray(data + offset, 100, length),
        Hex2EncodedLatin1)
    d.putPlainChildren(value)


#######################################################################
#
# Eigen
#
#######################################################################

#def qform__Eigen__Matrix():
#    return "Transposed"

def qdump__Eigen__Matrix(d, value):
    innerType = d.templateArgument(value.type, 0)
    storage = value["m_storage"]
    options = d.numericTemplateArgument(value.type, 3)
    rowMajor = (int(options) & 0x1)
    argRow = d.numericTemplateArgument(value.type, 1)
    argCol = d.numericTemplateArgument(value.type, 2)
    nrows = value["m_storage"]["m_rows"] if argRow == -1 else int(argRow)
    ncols = value["m_storage"]["m_cols"] if argCol == -1 else int(argCol)
    p = storage["m_data"]
    if p.type.code == StructCode: # Static
        p = p["array"].cast(innerType.pointer())
    d.putValue("(%s x %s), %s" % (nrows, ncols, ["ColumnMajor", "RowMajor"][rowMajor]))
    d.putField("keeporder", "1")
    d.putNumChild(nrows * ncols)

    limit = 10000
    nncols = min(ncols, limit)
    nnrows = min(nrows, limit * limit / nncols)
    if d.isExpanded():
        #format = d.currentItemFormat() # format == 1 is "Transposed"
        with Children(d, nrows * ncols, childType=innerType):
            if ncols == 1 or nrows == 1:
                for i in range(0, min(nrows * ncols, 10000)):
                    d.putSubItem(i, (p + i).dereference())
            elif rowMajor == 1:
                s = 0
                for i in range(0, nnrows):
                    for j in range(0, nncols):
                        v = (p + i * ncols + j).dereference()
                        d.putNamedSubItem(s, v, "[%d,%d]" % (i, j))
                        s = s + 1
            else:
                s = 0
                for j in range(0, nncols):
                    for i in range(0, nnrows):
                        v = (p + i + j * nrows).dereference()
                        d.putNamedSubItem(s, v, "[%d,%d]" % (i, j))
                        s = s + 1


#######################################################################
#
# D
#
#######################################################################

def cleanDType(type):
    return stripClassTag(str(type)).replace("uns long long", "string")

def qdump_Array(d, value):
    n = value["length"]
    p = value["ptr"]
    t = cleanDType(value.type)[7:]
    d.putType("%s[%d]" % (t, n))
    if t == "char":
        d.putValue(encodeCharArray(p, 100), Hex2EncodedLocal8Bit)
        d.putNumChild(0)
    else:
        d.putEmptyValue()
        d.putNumChild(n)
        innerType = p.type
        if d.isExpanded():
            with Children(d, n, childType=innerType):
                for i in range(0, n):
                    d.putSubItem(i, p.dereference())
                    p = p + 1


def qdump_AArray(d, value):
    #n = value["length"]
    # This ends up as _AArray_<key>_<value> with a single .ptr
    # member of type void *. Not much that can be done here.
    p = value["ptr"]
    t = cleanDType(value.type)[8:]
    d.putType("%s]" % t.replace("_", "["))
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d, 1):
                d.putSubItem("ptr", p)


#######################################################################
#
# Display Test
#
#######################################################################

if False:

    # FIXME: Make that work
    def qdump__Color(d, value):
        v = value
        d.putValue("(%s, %s, %s; %s)" % (v["r"], v["g"], v["b"], v["a"]))
        if d.isExpanded():
            with Children(d):
                d.putFields(value)

    def qdump__Color_(d, value):
        v = value
        d.putValue("(%s, %s, %s; %s)" % (v["r"], v["g"], v["b"], v["a"]))
        if d.isExpanded():
            with Children(d):
                with SubItem(d, "0"):
                    d.putItem(v["r"])
                with SubItem(d, "1"):
                    d.putItem(v["g"])
                with SubItem(d, "2"):
                    d.putItem(v["b"])
                with SubItem(d, "3"):
                    d.putItem(v["a"])


if False:

    def qform__basic__Function():
        return "Normal,Displayed"

    def qdump__basic__Function(d, value):
        min = value["min"]
        max = value["max"]
        data, size, alloc = d.byteArrayData(value["var"])
        var = extractCString(data, 0)
        data, size, alloc = d.byteArrayData(value["f"])
        f = extractCString(data, 0)
        d.putValue("%s, %s=%f..%f" % (f, var, min, max))
        d.putNumChild(0)
        format = d.currentItemFormat()
        if format == 1:
            d.putDisplay(StopDisplay)
        elif format == 2:
            input = "plot [%s=%f:%f] %s" % (var, min, max, f)
            d.putDisplay(DisplayProcess, input, "gnuplot")


if False:

    def qdump__tree_entry(d, value):
        d.putValue("len: %s, offset: %s, type: %s" %
            (value["blocklength"], value["offset"], value["type"]))
        d.putNumChild(0)

    def qdump__tree(d, value):
        count = value["count"]
        entries = value["entries"]
        base = value["base"].cast(d.charPtrType())
        d.putItemCount(count)
        d.putNumChild(count)
        if d.isExpanded():
          with Children(d):
            with SubItem(d, "tree"):
              d.putEmptyValue()
              d.putNoType()
              d.putNumChild(1)
              if d.isExpanded():
                with Children(d):
                  for i in xrange(count):
                      d.putSubItem(Item(entries[i], iname))
            with SubItem(d, "data"):
              d.putEmptyValue()
              d.putNoType()
              d.putNumChild(1)
              if d.isExpanded():
                 with Children(d):
                    for i in xrange(count):
                      with SubItem(d, i):
                        entry = entries[i]
                        mpitype = str(entry["type"])
                        d.putType(mpitype)
                        length = int(entry["blocklength"])
                        offset = int(entry["offset"])
                        d.putValue("%s items at %s" % (length, offset))
                        if mpitype == "MPI_INT":
                          innerType = "int"
                        elif mpitype == "MPI_CHAR":
                          innerType = "char"
                        elif mpitype == "MPI_DOUBLE":
                          innerType = "double"
                        else:
                          length = 0
                        d.putNumChild(length)
                        if d.isExpanded():
                           with Children(d):
                              t = d.lookupType(innerType).pointer()
                              p = (base + offset).cast(t)
                              for j in range(length):
                                d.putSubItem(j, p.dereference())

    #struct KRBase
    #{
    #    enum Type { TYPE_A, TYPE_B } type;
    #    KRBase(Type _type) : type(_type) {}
    #};
    #
    #struct KRA : KRBase { int x; int y; KRA():KRBase(TYPE_A),x(1),y(32) {} };
    #struct KRB : KRBase { KRB():KRBase(TYPE_B) {}  };
    #
    #void testKR()
    #{
    #    KRBase *ptr1 = new KRA;
    #    KRBase *ptr2 = new KRB;
    #    ptr2 = new KRB;
    #}

    def qdump__KRBase(d, value):
        if getattr(value, "__nested__", None) is None:
            base = ["KRA", "KRB"][int(value["type"])]
            nest = value.cast(d.lookupType(base))
            nest.__nested__ = True
            warn("NEST %s " % dir(nest))
            d.putItem(nest)
        else:
            d.putName("type")
            d.putValue(value["type"])
            d.putNoType()



if False:
    def qdump__bug5106__A5106(d, value):
        d.putName("a")
        d.putValue("This is the value: %s" % value["m_a"])
        d.putNoType()
        d.putNumChild(0)


if False:
    def qdump__bug6933__Base(d, value):
        d.putValue("foo")
        d.putPlainChildren(value)

if False:
    def qdump__gdb13393__Base(d, value):
        d.putValue("Base (%s)" % value["a"])
        d.putType(value.type)
        d.putNumChild(1)
        if d.isExpanded():
            with Children(d):
                d.putFields(value)

    def qdump__gdb13393__Derived(d, value):
        d.putValue("Derived (%s, %s)" % (value["a"], value["b"]))
        d.putType(value.type)
        d.putNumChild(1)
        if d.isExpanded():
            with Children(d):
                d.putFields(value)


