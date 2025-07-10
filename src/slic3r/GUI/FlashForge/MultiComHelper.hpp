#ifndef slic3r_GUI_MultiComHelper_hpp_
#define slic3r_GUI_MultiComHelper_hpp_

#include "FlashNetworkIntfc.h"
#include "MultiComDef.hpp"
#include "Singleton.hpp"

namespace Slic3r { namespace GUI {

class MultiComHelper : public Singleton<MultiComHelper>
{
public:
    void setUid(const std::string &uid) { m_uid = uid; }

    ComErrno getUserAiPointsInfo(com_user_ai_points_info_t &userAiPointsInfo, int msTimeout);

private:
    std::string m_uid;
};

}} // namespace Slic3r::GUI

#endif
