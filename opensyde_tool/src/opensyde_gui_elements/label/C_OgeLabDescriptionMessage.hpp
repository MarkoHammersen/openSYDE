//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label for description of custom message box.

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABDESCRIPTIONMESSAGE_HPP
#define C_OGELABDESCRIPTIONMESSAGE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OgeLabContextMenuBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabDescriptionMessage :
   public C_OgeLabContextMenuBase
{
   Q_OBJECT

public:
   explicit C_OgeLabDescriptionMessage(QWidget * const opc_Parent = NULL);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
