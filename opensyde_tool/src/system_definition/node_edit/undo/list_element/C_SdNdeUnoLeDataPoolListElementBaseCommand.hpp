//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data pool list element base command (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEUNOLEDATAPOOLLISTELEMENTBASECOMMAND_HPP
#define C_SDNDEUNOLEDATAPOOLLISTELEMENTBASECOMMAND_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QString>
#include <QUndoCommand>
#include "C_SdNdeDpListModelViewManager.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeUnoLeDataPoolListElementBaseCommand :
   public QUndoCommand
{
public:
   C_SdNdeUnoLeDataPoolListElementBaseCommand(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
                                              const uint32_t & oru32_DataPoolListIndex,
                                              C_SdNdeDpListModelViewManager * const opc_DataPoolListModelViewManager,
                                              const QString & orc_Text = "", QUndoCommand * const opc_Parent = NULL);

protected:
   const uint32_t mu32_NodeIndex;
   const uint32_t mu32_DataPoolIndex;
   const uint32_t mu32_DataPoolListIndex;
   C_SdNdeDpListModelViewManager * const mpc_DataPoolListModelViewManager;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
