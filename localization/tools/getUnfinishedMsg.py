import os
import sys
import traceback
import PoRW

def hasMsgStrPlural(msg):
    for line in msg.lines:
        lineTrimed = line.strip()
        if lineTrimed.startswith("msgstr[1]"):
            return True
    return False

def getUnfinishedMsg(msgList):
    dstList = []
    for msg in msgList:
        if msg.msgStr.count('"') == len(msg.msgStr):
            dstList.append(msg)
        elif hasMsgStrPlural(msg) and msg.msgStrPlural.count('"') == len(msg.msgStrPlural):
            dstList.append(msg)
    return dstList

if __name__ == "__main__":
    try:
        appDir = os.path.dirname(os.path.abspath(__file__))
        msgList, invalidMsgList = PoRW.readMsgList(sys.argv[1], False)
        dstMsgList = getUnfinishedMsg(msgList)
        saveNameInvalid = os.path.join(appDir, "invalid.po")
        PoRW.saveMsgList(invalidMsgList, saveNameInvalid)
        saveNameDst = os.path.join(appDir, "unfinished.po")
        PoRW.saveMsgList(dstMsgList, saveNameDst)
    except:
        traceback.print_exc()
        os.system('pause')
