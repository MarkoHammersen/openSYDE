﻿//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       short description (implementation)

   detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SdUtil.hpp"
#include "TglUtils.hpp"
#include "constants.hpp"
#include "stwerrors.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_SdBueUnoMessageAddDeleteBaseCommand.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::tgl;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in]      orc_MessageId             Message identification indices
   \param[in,out]  opc_MessageSyncManager    Message sync manager to perform actions on
   \param[in,out]  opc_MessageTreeWidget     Message tree widget to perform actions on
   \param[in]      orc_Text                  Optional command text for informational display
   \param[in,out]  opc_Parent                Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdBueUnoMessageAddDeleteBaseCommand::C_SdBueUnoMessageAddDeleteBaseCommand(
   const std::vector<C_OscCanMessageIdentificationIndices> & orc_MessageId,
   C_PuiSdNodeCanMessageSyncManager * const opc_MessageSyncManager,
   C_SdBueMessageSelectorTreeWidget * const opc_MessageTreeWidget, const QString & orc_Text,
   QUndoCommand * const opc_Parent) :
   C_SdBueUnoMessageBaseCommand(orc_MessageId, opc_MessageSyncManager, opc_MessageTreeWidget, orc_Text, opc_Parent),
   mc_LastMessageId(orc_MessageId)
{
   this->mc_MatchingIds.resize(this->mc_UniqueId.size(),
                               std::vector<stw::opensyde_core::C_OscCanMessageIdentificationIndices>());
   this->mc_LastMessageId.resize(this->mc_UniqueId.size(), C_OscCanMessageIdentificationIndices());
   this->mc_Message.resize(this->mc_UniqueId.size(), C_OscCanMessage());
   this->mc_OscSignalCommons.resize(this->mc_UniqueId.size(),
                                    std::vector<stw::opensyde_core::C_OscNodeDataPoolListElement>());
   this->mc_UiSignalCommons.resize(this->mc_UniqueId.size(), std::vector<C_PuiSdNodeDataPoolListElement>());
   this->mc_UiMessage.resize(this->mc_UniqueId.size(), C_PuiSdNodeCanMessage());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Generic add
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdBueUnoMessageAddDeleteBaseCommand::m_Add(void)
{
   if (this->mpc_MessageSyncManager != NULL)
   {
      for (uint32_t u32_ItStep = 0UL; u32_ItStep < this->mc_UniqueId.size(); ++u32_ItStep)
      {
         //Adapt default values for protocol
         C_SdUtil::h_AdaptMessageToProtocolType(this->mc_Message[u32_ItStep], this->mc_UiMessage[u32_ItStep],
                                                this->mc_OscSignalCommons[u32_ItStep],
                                                this->mc_UiSignalCommons[u32_ItStep],
                                                this->mc_LastMessageId[u32_ItStep].e_ComProtocol, NULL, false);
         C_SdBueUnoMessageAddDeleteBaseCommand::mh_UpdateSignalsToProtocol(this->mc_Message[u32_ItStep],
                                                                           this->mc_OscSignalCommons[u32_ItStep],
                                                                           this->mc_UiSignalCommons[u32_ItStep],
                                                                           this->mc_LastMessageId[u32_ItStep].e_ComProtocol);
         //Cycle time
         if (this->mc_Message[u32_ItStep].u32_CycleTimeMs == 0U)
         {
            this->mc_Message[u32_ItStep].u32_CycleTimeMs = 1U;
         }
         tgl_assert(this->mpc_MessageSyncManager->AddCanMessage(this->mc_LastMessageId[u32_ItStep].u32_NodeIndex,
                                                                this->mc_LastMessageId[u32_ItStep].e_ComProtocol,
                                                                this->mc_LastMessageId[u32_ItStep].u32_InterfaceIndex,
                                                                this->mc_LastMessageId[u32_ItStep].u32_DatapoolIndex,
                                                                this->mc_LastMessageId[u32_ItStep].q_MessageIsTx,
                                                                this->mc_Message[u32_ItStep],
                                                                this->mc_OscSignalCommons[u32_ItStep],
                                                                this->mc_UiSignalCommons[u32_ItStep],
                                                                this->mc_UiMessage[u32_ItStep],
                                                                this->mc_LastMessageId[u32_ItStep].u32_MessageIndex) ==
                    C_NO_ERR);
         for (uint32_t u32_ItMessage = 0U; u32_ItMessage < this->mc_MatchingIds[u32_ItStep].size(); ++u32_ItMessage)
         {
            const C_OscCanMessageIdentificationIndices & rc_CurMessageId =
               this->mc_MatchingIds[u32_ItStep][u32_ItMessage];
            //Skip already added one
            if ((rc_CurMessageId.u32_NodeIndex != this->mc_LastMessageId[u32_ItStep].u32_NodeIndex) ||
                (rc_CurMessageId.u32_InterfaceIndex != this->mc_LastMessageId[u32_ItStep].u32_InterfaceIndex) ||
                (rc_CurMessageId.u32_DatapoolIndex != this->mc_LastMessageId[u32_ItStep].u32_DatapoolIndex))
            {
               //Always add as Rx
               tgl_assert(this->mpc_MessageSyncManager->AddCanMessageRx(this->mc_LastMessageId[u32_ItStep],
                                                                        rc_CurMessageId.u32_NodeIndex,
                                                                        rc_CurMessageId.u32_InterfaceIndex,
                                                                        rc_CurMessageId.u32_DatapoolIndex) ==
                          C_NO_ERR);
               //Change to Tx
               if (rc_CurMessageId.q_MessageIsTx == true)
               {
                  const C_OscCanMessageContainer * const pc_Container =
                     C_PuiSdHandler::h_GetInstance()->GetCanProtocolMessageContainer(
                        rc_CurMessageId.u32_NodeIndex,
                        this->mc_LastMessageId[u32_ItStep].e_ComProtocol,
                        rc_CurMessageId.u32_InterfaceIndex,
                        rc_CurMessageId.u32_DatapoolIndex);

                  if ((pc_Container != NULL) && (pc_Container->c_RxMessages.size() > 0UL))
                  {
                     //Should be the newest Rx message
                     const C_OscCanMessageIdentificationIndices c_Tmp(rc_CurMessageId.u32_NodeIndex,
                                                                      this->mc_LastMessageId[u32_ItStep].e_ComProtocol,
                                                                      rc_CurMessageId.u32_InterfaceIndex,
                                                                      rc_CurMessageId.u32_DatapoolIndex,
                                                                      false,
                                                                      pc_Container->c_RxMessages.size() - 1UL);
                     tgl_assert(this->mpc_MessageSyncManager->SetCanMessageDirection(c_Tmp, true) ==
                                C_NO_ERR);
                  }
               }
            }
         }
         //Update unique ID
         this->mc_UniqueId[u32_ItStep] = this->mpc_MessageSyncManager->GetUniqueId(this->mc_LastMessageId[u32_ItStep]);
         //UI
         if (this->mpc_MessageTreeWidget != NULL)
         {
            this->mpc_MessageTreeWidget->InternalAddMessage(this->mc_LastMessageId[u32_ItStep]);
         }
      }
      //UI
      if (this->mpc_MessageTreeWidget != NULL)
      {
         this->mpc_MessageTreeWidget->InternalAddMessageCommit();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Generic delete
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdBueUnoMessageAddDeleteBaseCommand::m_Delete(void)
{
   m_Store();
   m_Remove();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Store current values
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdBueUnoMessageAddDeleteBaseCommand::m_Store(void)
{
   if (this->mpc_MessageSyncManager != NULL)
   {
      for (uint32_t u32_ItStep = 0UL; u32_ItStep < this->mc_UniqueId.size(); ++u32_ItStep)
      {
         {
            const C_OscCanMessageIdentificationIndices c_LastMessageId =
               this->mpc_MessageSyncManager->GetMessageIdForUniqueId(this->mc_UniqueId[u32_ItStep]);
            if (u32_ItStep < this->mc_LastMessageId.size())
            {
               this->mc_LastMessageId[u32_ItStep] = c_LastMessageId;
            }
            else
            {
               this->mc_LastMessageId.push_back(c_LastMessageId);
            }
         }
         {
            C_OscCanMessage c_Message;
            std::vector<C_OscNodeDataPoolListElement> c_OscSignalCommons;
            std::vector<C_PuiSdNodeDataPoolListElement> c_UiSignalCommons;
            C_PuiSdNodeCanMessage c_UiMessage;
            tgl_assert(C_PuiSdHandler::h_GetInstance()->GetCanMessageComplete(this->mpc_MessageSyncManager->
                                                                              GetMessageIdForUniqueId(this->
                                                                                                      mc_UniqueId[
                                                                                                         u32_ItStep]),
                                                                              c_Message,
                                                                              c_OscSignalCommons, c_UiSignalCommons,
                                                                              c_UiMessage) ==
                       C_NO_ERR);
            this->mc_Message[u32_ItStep] = c_Message;
            this->mc_OscSignalCommons[u32_ItStep] = c_OscSignalCommons;
            this->mc_UiSignalCommons[u32_ItStep] = c_UiSignalCommons;
            this->mc_UiMessage[u32_ItStep] = c_UiMessage;
         }
         {
            const std::vector<C_OscCanMessageIdentificationIndices> c_MatchingIds =
               this->mpc_MessageSyncManager->GetMatchingMessageVector(this->mpc_MessageSyncManager->
                                                                      GetMessageIdForUniqueId(
                                                                         this->
                                                                         mc_UniqueId
                                                                         [u32_ItStep]));
            this->mc_MatchingIds[u32_ItStep] = c_MatchingIds;
         }
      }
      tgl_assert(this->m_CheckMessagesSortedAscending());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Remove current message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdBueUnoMessageAddDeleteBaseCommand::m_Remove(void)
{
   if (this->mpc_MessageSyncManager != NULL)
   {
      uint32_t u32_InternalMessageIndex = 0UL;
      //Avoid selection change triggers during uncertain tree state
      if (this->mpc_MessageTreeWidget != NULL)
      {
         this->mpc_MessageTreeWidget->DisconnectSelectionHandling();
      }
      for (uint32_t u32_ItStep = this->mc_UniqueId.size(); u32_ItStep > 0; --u32_ItStep)
      {
         tgl_assert(this->mpc_MessageSyncManager->DeleteCanMessage(this->mpc_MessageSyncManager->GetMessageIdForUniqueId(
                                                                      this->
                                                                      mc_UniqueId[static_cast<std::vector<uint64_t>
                                                                                              ::size_type>(u32_ItStep -
                                                                                                           1UL)])) ==
                    C_NO_ERR);
         if (this->mpc_MessageTreeWidget != NULL)
         {
            //At this point we can't get the message ID by unique ID because it was already deleted
            // but this should be no problem as we do always remember the message ID anyways
            u32_InternalMessageIndex =
               this->mpc_MessageTreeWidget->InternalDeleteMessage(this->mc_LastMessageId[static_cast<std::vector<uint64_t>
                                                                                                     ::size_type>(
                                                                                            u32_ItStep - 1UL)]);
         }
      }
      if (this->mpc_MessageTreeWidget != NULL)
      {
         //Reactivate selection handling
         this->mpc_MessageTreeWidget->ReconnectSelectionHandling();
         //At this point we can't get the message ID by unique ID because it was already deleted
         // but this should be no problem as we do always remember the message ID anyways
         this->mpc_MessageTreeWidget->InternalDeleteMessageCommit(u32_InternalMessageIndex);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Update signals to protocol

   \param[in,out]  orc_Message      Message
   \param[in,out]  orc_OscSignals   Core signals
   \param[in,out]  orc_UiSignals    UI signals
   \param[in]      oe_ProtocolType  Protocol type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdBueUnoMessageAddDeleteBaseCommand::mh_UpdateSignalsToProtocol(
   stw::opensyde_core::C_OscCanMessage & orc_Message, std::vector<C_OscNodeDataPoolListElement> & orc_OscSignals,
   const std::vector<C_PuiSdNodeDataPoolListElement> & orc_UiSignals,
   const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType)
{
   tgl_assert(orc_Message.c_Signals.size() == orc_OscSignals.size());
   tgl_assert(orc_UiSignals.size() == orc_OscSignals.size());
   if ((orc_Message.c_Signals.size() == orc_OscSignals.size()) && (orc_UiSignals.size() == orc_OscSignals.size()))
   {
      for (uint32_t u32_ItSig = 0UL; u32_ItSig < orc_OscSignals.size(); ++u32_ItSig)
      {
         C_SdUtil::h_AdaptSignalToProtocolType(orc_Message.c_Signals[u32_ItSig], orc_OscSignals[u32_ItSig],
                                               orc_UiSignals[u32_ItSig],
                                               oe_ProtocolType, NULL);
      }
   }
}
