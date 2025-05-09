//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Custom line edit for table (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELETABLE_HPP
#define C_OGELETABLE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "C_OgeLeContextMenuBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLeTable :
   public C_OgeLeContextMenuBase
{
   Q_OBJECT

public:
   C_OgeLeTable(QWidget * const opc_Parent = NULL);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
