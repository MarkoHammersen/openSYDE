//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Buffer for max performance model additions (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMMETTREEGUIBUFFER_HPP
#define C_CAMMETTREEGUIBUFFER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <list>
#include <mutex>
#include <QTimer>
#include <QObject>
#include "C_CamMetTreeLoggerData.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamMetTreeGuiBuffer :
   public QObject
{
   Q_OBJECT

public:
   C_CamMetTreeGuiBuffer(QObject * const opc_Parent = NULL);

   void HandleData(const C_CamMetTreeLoggerData & orc_NewData);
   void ClearBuffer(void);

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736

Q_SIGNALS:
   //lint -restore
   void SigInternalTrigger(void);
   void SigUpdateUi(const std::list<C_CamMetTreeLoggerData> & orc_Buffer);

private:
   bool mq_Connected;
   QTimer mc_Timer;
   std::mutex mc_BufferMutex;
   std::list<C_CamMetTreeLoggerData> mc_Buffer;

   void m_HandleUpdateUi(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
