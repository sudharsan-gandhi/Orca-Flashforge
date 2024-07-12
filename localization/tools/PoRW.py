class Msg(object):
    def __init__(self):
        self.msgCtxt = ""
        self.msgId = ""
        self.msgIdPlural = ""
        self.msgStr = ""
        self.msgStrPlural = ""
        self.lines = []

def procLineWithKey(lineVal, msg, attr):
    val = getattr(msg, attr)
    if len(val) != 0:
        return False
    if lineVal[0] != '\"' or lineVal[-1] != '\"':
        return False
    setattr(msg, attr, val + lineVal)
    return True

def parseMsg(msg):
    attr = ""
    for line in msg.lines:
        lineTrimed = line.strip()
        if lineTrimed.startswith("msgctxt") and lineTrimed[7].isspace():
            if procLineWithKey(lineTrimed[7:].strip(), msg, "msgCtxt"):
                attr = "msgCtxt"
            else:
                return False
        elif lineTrimed.startswith("msgid") and lineTrimed[5].isspace():
            if procLineWithKey(lineTrimed[5:].strip(), msg, "msgId"):
                attr = "msgId"
            else:
                return False
        elif lineTrimed.startswith("msgid_plural") and lineTrimed[12].isspace():
            if procLineWithKey(lineTrimed[12:].strip(), msg, "msgIdPlural"):
                attr = "msgIdPlural"
            else:
                return False
        elif lineTrimed.startswith("msgstr") and lineTrimed[6].isspace():
            if procLineWithKey(lineTrimed[6:].strip(), msg, "msgStr"):
                attr = "msgStr"
            else:
                return False
        elif lineTrimed.startswith("msgstr[0]") and lineTrimed[9].isspace():
            if procLineWithKey(lineTrimed[9:].strip(), msg, "msgStr"):
                attr = "msgStr"
            else:
                return False
        elif lineTrimed.startswith("msgstr[1]") and lineTrimed[9].isspace():
            if procLineWithKey(lineTrimed[9:].strip(), msg, "msgStrPlural"):
                attr = "msgStrPlural"
            else:
                return False
        elif lineTrimed[0] == '\"' and lineTrimed[-1] == '\"':
            if len(attr) == 0:
                return False
            else:
                setattr(msg, attr, getattr(msg, attr) + lineTrimed)
        elif lineTrimed.startswith("#"):
            attr = ""
        else:
            return False
    return True

def readMsgList(fileName):
    msg = Msg()
    msgList = []
    InvalidMsgList = []
    for line in open(fileName, encoding='utf-8').readlines():
        if len(line.strip()) == 0:
            if len(msg.lines) != 0:
                if not parseMsg(msg):
                    print("invalid message %d, %s" % (len(msgList), fileName))
                    for line in msg.lines:
                        print(line, end='')
                    print()
                    InvalidMsgList.append(msg)
                elif len(msg.msgId) != 0 and len(msg.msgStr) != 0:
                    msgList.append(msg)
                else:
                    InvalidMsgList.append(msg)
            msg = Msg()
        else:
            msg.lines.append(line)
    return msgList, InvalidMsgList

def saveMsgList(msgList, fileName):
    file = open(fileName, "w", encoding='utf-8')
    for msg in msgList:
        file.writelines(msg.lines)
        file.write("\n")
