//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Bold heading for group box (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABTOOLBOXHEADINGGROUPBIG_HPP
#define C_OGELABTOOLBOXHEADINGGROUPBIG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QLabel>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabToolboxHeadingGroupBig :
   public QLabel
{
   Q_OBJECT

public:
   explicit C_OgeLabToolboxHeadingGroupBig(QWidget * const opc_Parent = NULL);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
