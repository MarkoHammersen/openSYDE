//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for configuring and showing the CAN message generation (implementation)

   Widget for configuring and showing the CAN message generation

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_CamGenWidget.hpp"
#include "ui_C_CamGenWidget.h"

#include "C_CamOgeWiSectionHeader.hpp"
#include "C_GtGetText.hpp"
#include "C_UsHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor

   Set up GUI with all elements.

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamGenWidget::C_CamGenWidget(QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mpc_Ui(new Ui::C_CamGenWidget)
{
   this->mpc_Ui->setupUi(this);

   this->mpc_Ui->pc_WiTitle->SetTitle(C_GtGetText::h_GetText("Message Generator"));
   this->mpc_Ui->pc_WiTitle->SetIconType(C_CamOgeWiSectionHeader::E_ButtonType::eUPDOWN);

   // distribute remaining space equal over messages and signals widgets
   this->mpc_Ui->pc_Splitter->setStretchFactor(1, 1);

   //Connections
   // expand collapse
   connect(this->mpc_Ui->pc_WiTitle, &C_CamOgeWiSectionHeader::SigExpandSection,
           this, &C_CamGenWidget::m_OnExpandMessageGen);

   // data information
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigSelected, this->mpc_Ui->pc_WidgetSignals,
           &C_CamGenSigWidget::UpdateSelection);
   connect(this->mpc_Ui->pc_WidgetSignals, &C_CamGenSigWidget::SigUpdateMessageData, this->mpc_Ui->pc_WidgetMessages,
           &C_CamGenMessagesWidget::UpdateMessageData);
   connect(this->mpc_Ui->pc_WidgetSignals, &C_CamGenSigWidget::SigTriggerModelUpdateCyclicMessage,
           this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::TriggerModelUpdateCyclicMessage);
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigUpdateMessageDlc,
           this->mpc_Ui->pc_WidgetSignals, &C_CamGenSigWidget::UpdateMessageDlc);
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigSendMessage,
           this, &C_CamGenWidget::SigSendMessage);
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigRegisterCyclicMessage,
           this, &C_CamGenWidget::SigRegisterCyclicMessage);
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigRemoveAllCyclicMessages,
           this, &C_CamGenWidget::SigRemoveAllCyclicMessages);
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigAutoProtocolSupport,
           this, &C_CamGenWidget::SigAutoProtocolSupport);
   connect(this->mpc_Ui->pc_WidgetMessages, &C_CamGenMessagesWidget::SigAutoProtocolSupport,
           this->mpc_Ui->pc_WidgetSignals, &C_CamGenSigWidget::UpdateAutoProtocolSupport);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamGenWidget::~C_CamGenWidget()
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load all user settings.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::LoadUserSettings()
{
   this->mpc_Ui->pc_WidgetMessages->LoadUserSettings();
   this->mpc_Ui->pc_WidgetSignals->LoadUserSettings();

   // splitter between messages and signals
   this->mpc_Ui->pc_Splitter->SetFirstSegment(C_UsHandler::h_GetInstance()->GetSplitterMesSigHorizontal());

   // splitter between trace and message generator
   this->ExpandMessageGen(C_UsHandler::h_GetInstance()->GetMessageGenIsExpanded());
   // adapt GUI by hand (signal gets not received)
   this->m_OnExpandMessageGen(C_UsHandler::h_GetInstance()->GetMessageGenIsExpanded());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save all user settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::SaveUserSettings(void) const
{
   // splitter
   const QList<int32_t> c_Sizes = this->mpc_Ui->pc_Splitter->sizes();

   if (c_Sizes.size() > 0)
   {
      C_UsHandler::h_GetInstance()->SetSplitterMesSigHorizontal(c_Sizes.at(0));
   }

   // child widgets
   this->mpc_Ui->pc_WidgetMessages->SaveUserSettings();
   this->mpc_Ui->pc_WidgetSignals->SaveUserSettings();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle signal reload on external trigger
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::TriggerSignalReload(void) const
{
   this->mpc_Ui->pc_WidgetSignals->TriggerSignalReload();
   this->mpc_Ui->pc_WidgetMessages->TriggerMessageReload();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Remove all messages for the specified file

   \param[in]  orc_File       Database to remove all messages for
   \param[in]  opc_Indices    Optional indices to specify which messages should be deleted instead of all
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::RemoveMessagesForFile(const QString & orc_File,
                                           const std::vector<uint32_t> * const opc_Indices) const
{
   this->mpc_Ui->pc_WidgetMessages->RemoveMessagesForFile(orc_File, opc_Indices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Signal communication start

   \param[in]  oq_Online   Online/offline flag of trace
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::SetCommunicationStarted(const bool oq_Online) const
{
   this->mpc_Ui->pc_WidgetMessages->SetCommunicationStarted(oq_Online);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if the key needs to be handled by this widget

   \param[in]  orc_Input   Key input interpreted as text

   \return
   True  Handled by this widget
   False No handling
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamGenWidget::CheckAndHandleKey(const QString & orc_Input) const
{
   return this->mpc_Ui->pc_WidgetMessages->CheckAndHandleKey(orc_Input);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Expand or collapse messages widget.

   \param[in]  oq_Expand   true: expand messages widget subsections
                           false: collapse messages widget subsections
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::ExpandMessageGen(const bool oq_Expand) const
{
   this->mpc_Ui->pc_WiTitle->SetOpen(oq_Expand);
   // this toggles the "vv" button and therefore emits a signal; on this signal we connect and adapt the GUI
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle expand or collapse message generator sub widgets.

   \param[in]  oq_Expand   true: expand settings subsections
                           false: collapse settings subsections
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenWidget::m_OnExpandMessageGen(const bool oq_Expand)
{
   this->mpc_Ui->pc_WidgetMessages->setVisible(oq_Expand);
   this->mpc_Ui->pc_WidgetSignals->setVisible(oq_Expand);

   Q_EMIT (this->SigExpandMessageGen(oq_Expand));
}
