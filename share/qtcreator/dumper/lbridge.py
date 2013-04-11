import json


#warn("LOADING LLDB")

# Data members
SimpleValueCode, \
StructCode, \
PointerCode \
    = range(3)

# Breakpoints. Keep synchronized with BreakpointType in breakpoint.h
UnknownType = 0
BreakpointByFileAndLine = 1
BreakpointByFunction = 2
BreakpointByAddress = 3
BreakpointAtThrow = 4
BreakpointAtCatch = 5
BreakpointAtMain = 6
BreakpointAtFork = 7
BreakpointAtExec = 8
BreakpointAtSysCall = 10
WatchpointAtAddress = 11
WatchpointAtExpression = 12
BreakpointOnQmlSignalEmit = 13
BreakpointAtJavaScriptThrow = 14


def dumpJson(stuff):
    warn("%s" % json.dumps(stuff, sort_keys=True, indent=4, separators=(',', ': ')))

def registerCommand(name, func):
    pass

class Type:
    def __init__(self, var):
        self.raw = var
        if var.num_children == 0:
            self.code = SimpleValueCode
        else:
            self.code = StructCode
        self.value_type = var.value_type

    def __str__(self):
        #try:
            return self.raw.type.name
        #except:
        #    return "<illegal type>"

    def fieldCount(self):
        return self.raw.num_children

    def unqualified(self):
        return self

class Value:
    def __init__(self, var):
        self.raw = var
        self.is_optimized_out = False
        self.address = var.addr
        self.type = Type(var)
        self.name = var.name

    def __str__(self):
        return str(self.raw.value)

    def fields(self):
        return [Value(self.raw.GetChildAtIndex(i)) for i in range(self.raw.num_children)]

currentThread = None
currentFrame = None

def listOfLocals(varList):
    global currentThread
    global currentFrame

    items = []
    currentThread = lldb.process.GetThreadAtIndex(0)
    currentFrame = currentThread.GetFrameAtIndex(0)
    for var in currentFrame.variables:
        item = LocalItem()
        item.iname = "local." + var.name
        item.name = var.name
        item.value = Value(var)
        items.append(item)
    return items

def extractFields(value):
    return value.fields()

def fieldCount(type):
    return type.fieldCount();

def threadsData(options):
    result = "threads={threads=["
    for thread in lldb.process.threads:
        result += "{id=\"%d\"" % thread.id
        result += ",target-id=\"%s\"" % thread.id
        result += ",index=\"%s\"" % thread.idx
        result += ",stop-reason=\"%s\"" % thread.stop_reason

        if thread.IsSuspended():
            result += ",state=\"stopped\""
        else:
            result += ",state=\"running\""

        if not thread.name is None:
            result += ",name=\"%s\"" % thread.name

        result += ",frame={"
        frame = thread.GetFrameAtIndex(0)
        result += "pc=\"%s\"" % frame.pc
        result += ",addr=\"%s\"" % frame.pc
        result += ",fp=\"%s\"" % frame.fp
        result += ",func=\"%s\"" % frame.function.name
        result += ",line=\"%s\"" % frame.line_entry.line
        result += ",fullname=\"%s\"" % frame.line_entry.file
        result += ",file=\"%s\"" % frame.line_entry.file
        result += "}},"

    result += "],current-thread-id=\"%s\"}" % lldb.process.selected_thread.id
    return result

# See lldb.StateType
stateNames = ["invalid", "unloaded", "connected", "attaching", "launching", "stopped",
    "running", "stepping", "crashed", "detached", "exited", "suspended" ]

def stateData(options):
    state = lldb.process.GetState()
    return "state=\"%s\"," % stateNames[state]

def locationData(options):
    thread = lldb.process.GetSelectedThread()
    frame = thread.GetFrameAtIndex(0)
    return "location={file=\"%s\",line=\"%s\",addr=\"%s\"}," \
        % (frame.line_entry.file, frame.line_entry.line, frame.pc)

def stackData(options):
    try:
        thread = lldb.process.GetThreadById(options["threadid"])
    except:
        thread = lldb.process.GetThreadAtIndex(0)
    result = "stack={frames=["
    for frame in thread.frames:
        result += "{pc=\"%s\"" % frame.pc
        result += ",level=\"%d\"" % frame.idx
        result += ",addr=\"%s\"" % frame.pc
        result += ",fp=\"%s\"" % frame.fp
        result += ",func=\"%s\"" % frame.function.name
        result += ",line=\"%s\"" % frame.line_entry.line
        result += ",fullname=\"%s\"" % frame.line_entry.file
        result += ",usable=\"1\""
        result += ",file=\"%s\"}," % frame.line_entry.file

    hasmore = "0"
    result += "],hasmore=\"%s\"}, " % hasmore
    return result

def parseOptions(optionstring):
    options = {}
    for opt in optionstring.split(","):
        try:
            key, value = opt.split(":")
            options[key] = value
        except:
            pass
    return options

def updateData(parts, localsOptions, stackOptions, threadOptions):
    result = "result={";
    if parts & 1:
        result += bb(localsOptions) + ","
    if parts & 2:
        result += stackData(parseOptions(stackOptions))
    if parts & 4:
        result += threadsData(parseOptions(threadOptions))
    result += stateData({})
    result += locationData({})
    result += "}"
    return result

def listModules():
    result = "result={modules=["
    for module in lldb.target.modules:
        result += "{file=\"%s\"" % module.file.fullpath
        result += ",name=\"%s\"" % module.file.basename
        #result += ",addrsize=\"%s\"" % module.addr_size
        #result += ",triple=\"%s\"" % module.triple
        #result += ",sections={"
        #for section in module.sections:
        #    result += "[name=\"%s\"" % section.name
        #    result += ",addr=\"%s\"" % section.addr
        #    result += ",size=\"%s\"]," % section.size
        #result += "}"
        result += "},"
    result += "]}"
    return result

def breakpoint_function_wrapper(baton, process, frame, bp_loc):
    result = "*stopped"
    result += ",line=\"%s\"" % frame.line_entry.line
    result += ",file=\"%s\"" % frame.line_entry.file
    warn("WRAPPER: %s " %result)
    return result

def initLldb():
    pass

def dumpBreakpoint(bp, modelId):
    cond = bp.GetCondition()
    result  = "{lldbid=\"%s\"" % bp.GetID()
    result += ",modelid=\"%s\"" % modelId
    result += ",hitcount=\"%s\"" % bp.GetHitCount()
    result += ",threadid=\"%s\"" % bp.GetThreadID()
    result += ",oneshot=\"%s\"" % (1 if bp.IsOneShot() else 0)
    result += ",enabled=\"%s\"" % (1 if bp.IsEnabled() else 0)
    result += ",valid=\"%s\"" % (1 if bp.IsValid() else 0)
    result += ",condition=\"%s\"" % ("" if cond is None else cond)
    result += ",ignorecount=\"%s\"" % bp.GetIgnoreCount()
    result += ",locations=["
    for i in range(bp.GetNumLocations()):
        loc = bp.GetLocationAtIndex(i)
        addr = loc.GetAddress()
        result += "{locid=\"%s\"" % loc.GetID()
        result += ",func=\"%s\"" % addr.GetFunction().GetName()
        result += ",enabled=\"%s\"" % (1 if loc.IsEnabled() else 0)
        result += ",resolved=\"%s\"" % (1 if loc.IsResolved() else 0)
        result += ",valid=\"%s\"" % (1 if loc.IsValid() else 0)
        result += ",ignorecount=\"%s\"" % loc.GetIgnoreCount()
        result += ",addr=\"%s\"}," % loc.GetLoadAddress()
    result += "]},"
    return result

def onBreak():
    lldb.debugger.HandleCommand("settings set frame-format ''")
    lldb.debugger.HandleCommand("settings set thread-format ''")
    result = "*stopped,frame={....}"
    print result

def handleBreakpoints(stuff):
    todo = json.loads(stuff)
    #dumpJson(todo)
    #target = lldb.debugger.CreateTargetWithFileAndArch (exe, lldb.LLDB_ARCH_DEFAULT)
    target = lldb.debugger.GetTargetAtIndex(0)
    #target = lldb.target

    result = "result={bkpts={added=["

    for bp in todo["add"]:
        bpType = bp["type"]
        if bpType == BreakpointByFileAndLine:
            bpNew = target.BreakpointCreateByLocation(str(bp["file"]), int(bp["line"]))
        elif bpType == BreakpointByFunction:
            bpNew = target.BreakpointCreateByName(bp["function"])
        elif bpType == BreakpointAtMain:
            bpNew = target.BreakpointCreateByName("main", target.GetExecutable().GetFilename())
        bpNew.SetIgnoreCount(int(bp["ignorecount"]))
        bpNew.SetCondition(str(bp["condition"]))
        bpNew.SetEnabled(int(bp["enabled"]))
        bpNew.SetOneShot(int(bp["oneshot"]))
        #bpNew.SetCallback(breakpoint_function_wrapper, None)
        #bpNew.SetCallback(breakpoint_function_wrapper, None)
        #"breakpoint command add 1 -o \"import time; print time.asctime()\"
        #cmd = "script print(11111111)"
        cmd = "continue"
        lldb.debugger.HandleCommand(
            "breakpoint command add -o 'script onBreak()' %s" % bpNew.GetID())

        result += dumpBreakpoint(bpNew, bp["modelid"])

    result += "],changed=["

    for bp in todo["change"]:
        bpChange = target.FindBreakpointByID(int(bp["lldbid"]))
        bpChange.SetIgnoreCount(int(bp["ignorecount"]))
        bpChange.SetCondition(str(bp["condition"]))
        bpChange.SetEnabled(int(bp["enabled"]))
        bpChange.SetOneShot(int(bp["oneshot"]))
        result += dumpBreakpoint(bpChange, bp["modelid"])

    result += "],removed=["

    for bp in todo["remove"]:
        bpDead = target.BreakpointDelete(int(bp["lldbid"]))
        result += "{modelid=\"%s\"}" % bp["modelid"]

    result += "]}}"
    return result

def doSync(func):
    lldb.debugger.SetAsync(False)
    func()
    lldb.debugger.SetAsync(True)

def createReport():
    result = "result={"
    result += stackData({'threadid': lldb.process.selected_thread.id})
    result += threadsData({})
    result += stateData({})
    result += locationData({})
    result += "}"
    return result

def executeNext():
    doSync(lldb.thread.StepOver)
    return createReport()

def executeNextI():
    doSync(lldb.thread.StepOver)
    return createReport()

def executeStep():
    doSync(lldb.thread.Step)
    return createReport()

def executeStepI():
    doSync(lldb.thread.StepInstOver)
    return createReport()

def executeStepOut():
    doSync(lldb.thread.StepOut)
    return createReport()

def executeRunToLine():
    return "result={error={msg='Not implemented'}}"

def executeJumpToLine():
    return "result={error={msg='Not implemented'}}"

def continueInferior():
    #lldb.debugger.HandleCommand("process continue")
    lldb.process.Continue()
    return "result={state='running'}"

def interruptInferior():
    lldb.debugger.SetAsync(False)
    lldb.process.Stop()
    #lldb.process.SendAsyncInterrupt()
    lldb.debugger.SetAsync(True)
    return createReport()

def setupInferior(fileName):
    lldb.debugger.HandleCommand("target create '%s'" % fileName)

def runEngine():
    lldb.debugger.HandleCommand("process launch")

def activateFrame(frame):
    lldb.debugger.HandleCommand("frame select " + frame)

def selectThread(thread):
    lldb.debugger.HandleCommand("thread select " + thread)

def requestModuleSymbols(frame):
    lldb.debugger.HandleCommand("target module list " + frame)



#SBEvent data;
#while (!stop) {
#if (self->m_listener.WaitForEvent(UINT32_MAX, data)) {
#   if (data.getType() == SBProcess::eBroadcastBitStateChanged &&
#m_process.GetStateFromEvent (data) == eStateStopped) {
#     SBThread th = m_process.GetSelectedThread();
#     if (th.GetStopReason() == eStopReasonBreakpoint) {
#       // th.GetStopReasonDataAtIndex(0) should have the breakpoint id
#     }
#   }

lldb.debugger.HandleCommand("setting set auto-confirm on")
lldb.debugger.HandleCommand("setting set interpreter.prompt-on-quit off")

if False:
    # default:
    # frame-format (string) = "frame #${frame.index}: ${frame.pc}
    # { ${module.file.basename}{`${function.name-with-args}${function.pc-offset}}}
    # { at ${line.file.basename}:${line.number}}\n"
    lldb.debugger.HandleCommand("settings set frame-format frame=\{"
                + "index='${frame.index}',"
                + "pc='${frame.pc}',"
                + "module='${module.file.basename}',"
                #+ "function='${function.name-with-args}',"
                + "function='${function.name}',"
                + "pcoffset='${function.pc-offset}',"
                + "file='${line.file.basename}',"
                + "line='${line.number}'"
                + "\},")


    # default:
    # "thread #${thread.index}: tid = ${thread.id}{, ${frame.pc}}
    # { ${module.file.basename}{`${function.name-with-args}${function.pc-offset}}}
    # { at ${line.file.basename}:${line.number}}
    # {, stop reason = ${thread.stop-reason}}{\nReturn value: ${thread.return-value}}\n"
    lldb.debugger.HandleCommand("settings set thread-format thread=\{"
                + "index='${thread.index}',"
                + "tid='${thread.id}',"
                + "framepc='${frame.pc}',"
                + "module='${module.file.basename}',"
                + "function='${function.name}',"
                + "pcoffset='${function.pc-offset}',"
                + "stopreason='${thread.stop-reason}'"
                #+ "returnvalue='${thread.return-value}'"
                + "\},")
