//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Spin box with auto fix to nearest raw value base functionaliy (implementation)

   Spin box with auto fix to nearest raw value base functionaliy

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OgeSpxAutoFixBase.hpp"
#include "C_SdNdeDpContentUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeSpxAutoFixBase::C_OgeSpxAutoFixBase(void) :
   mu64_NumberOfStepsAvailable(0),
   mf64_Factor(1.0),
   mf64_Offset(0.0),
   mu32_Index(0)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeSpxAutoFixBase::~C_OgeSpxAutoFixBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize widget with valid values

   \param[in] orc_Min     Unscaled minimum value
   \param[in] orc_Max     Unscaled maximum value
   \param[in] of64_Factor Scaling factor
   \param[in] of64_Offset Scaling offset
   \param[in] ou32_Index  Optional data index (used if array)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeSpxAutoFixBase::Init(const C_OscNodeDataPoolContent & orc_Min, const C_OscNodeDataPoolContent & orc_Max,
                               const float64_t of64_Factor, const float64_t of64_Offset, const uint32_t ou32_Index)
{
   this->mc_UnscaledMin = orc_Min;
   this->mc_UnscaledMax = orc_Max;
   if (C_SdNdeDpContentUtil::h_GetNumberOfAvailableSteps(this->mc_UnscaledMin, this->mc_UnscaledMax,
                                                         this->mu64_NumberOfStepsAvailable,
                                                         ou32_Index) != C_NO_ERR)
   {
      this->mu64_NumberOfStepsAvailable = 0;
   }
   this->mf64_Factor = of64_Factor;
   this->mf64_Offset = of64_Offset;
   this->mu32_Index = ou32_Index;
   m_Init();
}
