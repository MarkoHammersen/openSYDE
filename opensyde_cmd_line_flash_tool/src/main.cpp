//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE simple command line flash tool main file

   Windows console application for flashing one device with one HEX file.

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <iostream>

#include "stwtypes.hpp"

#include "C_BasicFlashTool.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   main

   \param[in]   os32_Argc    Number of command line arguments
   \param[in]   oppcn_Argv   Command line arguments

   \retval   0
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t main(const int32_t os32_Argc, char_t * const oppcn_Argv[])
{
   C_BasicFlashTool c_TheFlasher;
   C_BasicFlashTool::E_Result e_ResultCode;

   const int32_t argc = 9;
   char_t* const argvAppCore[argc] =
   {
     "some placeholder for exe...",
     "-f",
     "C:\\Projekte\\grast_v3\\ESX_4CT\\appl_boot_core_release\\appl_boot_core.hex",
     "-i",
     "C:\\stw\\openSYDE\\tool\\STW_dlls\\stwpeak2\\stwpeak2.dll",
     "-n",
     "0",
     "-b",
     "250"
   };

   char_t* const argvSafetyCore[argc] =
   {
     "some placeholder for exe...",
     "-f",
     "C:\\Projekte\\grast_v3\\ESX_4CT\\safety_core_release\\safety_core.hex",
     "-i",
     "C:\\stw\\openSYDE\\tool\\STW_dlls\\stwpeak2\\stwpeak2.dll",
     "-n",
     "0",
     "-b",
     "250"
   };

   c_TheFlasher.Init();

   // application core
   e_ResultCode = c_TheFlasher.ParseCommandLine(argc, argvAppCore);
   if (e_ResultCode == C_BasicFlashTool::eRESULT_OK)
   {
      e_ResultCode = c_TheFlasher.Flash(false);
   }

   if (e_ResultCode == C_BasicFlashTool::eRESULT_OK)
   {
     // safety core
     e_ResultCode = c_TheFlasher.ParseCommandLine(argc, argvSafetyCore);
     if (e_ResultCode == C_BasicFlashTool::eRESULT_OK)
     {
       e_ResultCode = c_TheFlasher.Flash(true);
     }
   }

   return static_cast<uint8_t>(e_ResultCode);
}
