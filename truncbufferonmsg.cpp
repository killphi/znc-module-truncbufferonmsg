#include <znc/IRCNetwork.h>
#include <znc/Chan.h>
#include <znc/Buffer.h>

using std::vector;


const unsigned int uDefaultKeepLines = 10;
const CString sDefaultValue = CString(uDefaultKeepLines);

class CTruncBufferOnMsgMod : public CModule {
    public:

        MODCONSTRUCTOR(CTruncBufferOnMsgMod) {
            AddHelpCommand();
            AddCommand("Set", static_cast<CModCommand::ModCmdFunc>(&CTruncBufferOnMsgMod::SetCommand),
                    "NumLines | <nothing>",
                    "Number of lines to keep (default: " + sDefaultValue + "), no parameter to unset");
            AddCommand("Get", static_cast<CModCommand::ModCmdFunc>(&CTruncBufferOnMsgMod::GetCommand),
                    "", "show current setting");
        }

        void TruncAllBuffers() {
            CIRCNetwork* pNetwork = GetNetwork();

            if (pNetwork) {
                const vector<CChan*>& vChans = pNetwork->GetChans();

                for (vector<CChan*>::const_iterator it = vChans.begin(); it != vChans.end(); ++it) {
                    // Skip detached channels, they weren't read yet
                    if ((*it)->IsDetached())
                        continue;

                    // set buffer to smaller line count and then reset it
                    CString sTmp;
                    const unsigned int uKeepLines = (sTmp = GetNV("KeepLines")).empty() ? uDefaultKeepLines : sTmp.ToUInt();
                    const unsigned int uLineCount = (*it)->GetBufferCount();
                    (*it)->SetBufferCount(uKeepLines, true);
                    (*it)->SetBufferCount(uLineCount, true);

                    // We deny AutoClearChanBuffer on all channels since this module
                    // doesn't make any sense with it
                    (*it)->SetAutoClearChanBuffer(false);
                }
            }
        }

        virtual EModRet OnUserMsg(CString& sTarget, CString& sMessage) {
            TruncAllBuffers();
            return CONTINUE;
        }

        virtual EModRet OnUserCTCP(CString& sTarget, CString& sMessage) {
            TruncAllBuffers();
            return CONTINUE;
        }

        virtual EModRet OnUserAction(CString& sTarget, CString& sMessage) {
            TruncAllBuffers();
            return CONTINUE;
        }

        virtual EModRet OnUserNotice(CString& sTarget, CString& sMessage) {
            TruncAllBuffers();
            return CONTINUE;
        }

        virtual EModRet OnUserPart(CString& sChannel, CString& sMessage) {
            TruncAllBuffers();
            return CONTINUE;
        }

        virtual EModRet OnUserTopic(CString& sChannel, CString& sTopic) {
            TruncAllBuffers();
            return CONTINUE;
        }


    private:

        void SetCommand(const CString& sLine) {
            CString sToken = sLine.Token(1, true);

            if (sToken.empty()) {
                DelNV("KeepLines");
                PutModule("value reset to default: " + sDefaultValue);
            } else {
                CString sInt = CString(sToken.ToUInt());
                SetNV("KeepLines", sInt);
                PutModule("value set to: " + sInt);
            }
        }

        void GetCommand(const CString& sLine) {
            CString sKeepLines = GetNV("KeepLines");
            if (sKeepLines.empty()) {
                PutModule("value is not set");
            } else {
                PutModule("value is set to: " + sKeepLines);
            }
        }
};

USERMODULEDEFS(CTruncBufferOnMsgMod, "Truncate all channel and query buffers whenever the user does something")
