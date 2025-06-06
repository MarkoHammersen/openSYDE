//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief        Class for utilities with dependencies to Pui-classes.
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUIUTIL_HPP
#define C_PUIUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_Uti.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiUtil
{
public:
   C_PuiUtil(void);

   static QString h_GetAbsolutePathFromProject(const QString & orc_Path);
   static QString h_MakeIndependentOfDbProjectPath(const QString & orc_DbProjectPath, const QString & orc_Path);
   static QString h_ResolvePlaceholderVariables(const QString & orc_Path, const QString & orc_DbProjectPath = "");
   static QString h_GetResolvedAbsPathFromProject(const QString & orc_Path);
   static QString h_GetResolvedAbsPathFromExe(const QString & orc_Path, const QString & orc_DbProjectPath = "");
   static QString h_GetResolvedAbsPathFromDbProject(const QString & orc_DbProjectPath, const QString & orc_Path);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
