//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for showing a dashboard with a graphics view and scene with different dashboard items

   Widget for showing a dashboard with a graphics view and scene with different dashboard items

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SyvDaDashboardSceneWidget.hpp"
#include "ui_C_SyvDaDashboardSceneWidget.h"

#include "C_UsHandler.hpp"
#include "C_PuiSvHandler.hpp"
#include "C_PuiSvData.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in]     ou32_ViewIndex        Index of view
   \param[in]     ou32_DashboardIndex   Index of dashboard
   \param[in,out] opc_Parent            Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaDashboardSceneWidget::C_SyvDaDashboardSceneWidget(const uint32_t ou32_ViewIndex,
                                                         const uint32_t ou32_DashboardIndex,
                                                         QWidget * const opc_Parent) :
   C_SyvDaDashboardContentBaseWidget(opc_Parent),
   mpc_Ui(new Ui::C_SyvDaDashboardSceneWidget),
   mu32_ViewIndex(ou32_ViewIndex),
   mu32_DashboardIndex(ou32_DashboardIndex)
{
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);
   QString c_ViewName = "";
   QString c_DashboardName = "";

   this->mpc_Ui->setupUi(this);

   // create scene for graphics view
   this->mpc_Scene = new C_SyvDaDashboardScene(this->mu32_ViewIndex, this->mu32_DashboardIndex, true);

   // configure scene
   this->mpc_Scene->setSceneRect(0.0, 0.0,
                                 static_cast<float64_t>(this->mpc_Ui->pc_GraphicsView->width()),
                                 static_cast<float64_t>(this->mpc_Ui->pc_GraphicsView->height()));
   this->mpc_Ui->pc_GraphicsView->SetSceneAndConnect(this->mpc_Scene);

   // make all generic connects
   //Connect for tool tip
   connect(this->mpc_Scene, &C_SebScene::SigShowToolTip, this->mpc_Ui->pc_GraphicsView,
           &C_SebGraphicsView::ShowToolTip);
   connect(this->mpc_Scene, &C_SebScene::SigHideToolTip, this->mpc_Ui->pc_GraphicsView,
           &C_SebGraphicsView::HideToolTip);
   connect(this->mpc_Ui->pc_GraphicsView, &C_SebGraphicsView::SigShowToolTip, this->mpc_Scene,
           &C_SebScene::DisplayToolTip);
   //Interaction point scaling
   connect(this->mpc_Scene, &C_SebScene::SigTriggerUpdateTransform, this->mpc_Ui->pc_GraphicsView,
           &C_SebGraphicsView::UpdateTransform);
   //Error handling
   connect(this->mpc_Scene, &C_SyvDaDashboardScene::SigErrorChange, this, &C_SyvDaDashboardSceneWidget::SigErrorChange);

   connect(this->mpc_Scene, &C_SyvDaDashboardScene::SigTriggerUpdateTransmissionConfiguration, this,
           &C_SyvDaDashboardSceneWidget::SigTriggerUpdateTransmissionConfiguration);

   // Delayed update
   // Necessary because of issue #49525
   this->mc_UpdateTimer.setSingleShot(true);
   this->mc_UpdateTimer.setInterval(300);
   connect(&this->mc_UpdateTimer, &QTimer::timeout, this->mpc_Scene, &C_SyvDaDashboardScene::UpdateBoundaries);

   // Manual datapool element handling
   connect(this->mpc_Scene, &C_SyvDaDashboardScene::SigDataPoolWrite, this,
           &C_SyvDaDashboardSceneWidget::SigDataPoolWrite);
   connect(this->mpc_Scene, &C_SyvDaDashboardScene::SigDataPoolRead, this,
           &C_SyvDaDashboardSceneWidget::SigDataPoolRead);
   connect(this->mpc_Scene, &C_SyvDaDashboardScene::SigNvmReadList, this, &C_SyvDaDashboardSceneWidget::SigNvmReadList);

   //Update all items with initial zoom & pos value
   if (pc_View != NULL)
   {
      const C_PuiSvDashboard * const pc_Dashboard = pc_View->GetDashboard(this->mu32_DashboardIndex);
      c_ViewName = pc_View->GetName().c_str();
      if (pc_Dashboard != NULL)
      {
         c_DashboardName = pc_Dashboard->GetName();
      }
   }
   this->mpc_Ui->pc_GraphicsView->SetZoomValue(C_UsHandler::h_GetInstance()->GetProjSvSetupView(
                                                  c_ViewName).GetDashboardSettings(
                                                  c_DashboardName).s32_SceneZoom, true);
   this->mpc_Ui->pc_GraphicsView->SetViewPos(C_UsHandler::h_GetInstance()->GetProjSvSetupView(
                                                c_ViewName).GetDashboardSettings(
                                                c_DashboardName).c_ScenePos);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaDashboardSceneWidget::~C_SyvDaDashboardSceneWidget()
{
   delete this->mpc_Ui;
   delete this->mpc_Scene;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set data index

   \param[in]  ou32_Value  New data index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::SetDashboardIndex(const uint32_t ou32_Value)
{
   this->mpc_Scene->SetDashboardIndex(ou32_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the edit mode

   \param[in]  oq_Active   Flag for edit mode
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::SetEditMode(const bool oq_Active)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->SetDrawingBackground(oq_Active);
      this->mpc_Scene->SetEditMode(oq_Active);
   }
   this->mpc_Ui->pc_GraphicsView->SetDrawingBackground(oq_Active);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the dark mode

   \param[in]  oq_Active   Dark mode active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::SetDarkMode(const bool oq_Active)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->SetDarkModeActive(oq_Active);
      this->mpc_Scene->SetDarkModeInitialized();
   }
   this->mpc_Ui->pc_GraphicsView->SetDarkMode(oq_Active);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Function to activate or deactivate drawing of performance heavy widgets

   \param[in]  oq_Active   Flag if widgets should currently be drawn
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::SetDrawingActive(const bool oq_Active)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->SetDrawingActive(oq_Active);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::Save(void)
{
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);

   // store configuration of the view
   if (pc_View != NULL)
   {
      const C_PuiSvDashboard * const pc_Dashboard = pc_View->GetDashboard(this->mu32_DashboardIndex);
      if (pc_Dashboard != NULL)
      {
         C_UsHandler::h_GetInstance()->SetProjSvDashboardScenePositionAndZoom(pc_View->GetName().c_str(),
                                                                              pc_Dashboard->GetName(),
                                                                              this->mpc_Ui->pc_GraphicsView->GetViewPos(),
                                                                              this->mpc_Ui->pc_GraphicsView->GetZoomValue());
      }
   }
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->Save();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Registers all relevant dashboard widgets at the associated data dealer

   \param[in]  orc_ComDriver  Com driver containing information about all data dealer
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::RegisterWidgets(C_SyvComDriverDiag & orc_ComDriver)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->RegisterWidgets(orc_ComDriver);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Information about the start or stop of a connection

   \param[in]  oq_Active   Flag if connection is active or not active now
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::ConnectionActiveChanged(const bool oq_Active)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->ConnectionActiveChanged(oq_Active);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Updates all values of all dashboard widgets
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::UpdateShowValues(void)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->UpdateShowValues();
      this->mpc_Ui->pc_GraphicsView->repaint();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle changes of transmission mode for any data element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::UpdateTransmissionConfiguration(void)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->UpdateTransmissionConfiguration();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle manual user operation finished event

   \param[in]  os32_Result    Operation result
   \param[in]  ou8_Nrc        Negative response code, if any
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::HandleManualOperationFinished(const int32_t os32_Result, const uint8_t ou8_Nrc)
{
   if (this->mpc_Scene != NULL)
   {
      this->mpc_Scene->HandleManualOperationFinished(os32_Result, ou8_Nrc);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set focus to scene
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::SetFocus(void) const
{
   this->mpc_Ui->pc_GraphicsView->setFocus();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets current dashboard tab name

 *     \param[in]  orc_CurrentDashboardTabName   current chart tab name
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{9175} intentionally no functionality in this implementation
void C_SyvDaDashboardSceneWidget::SetCurrentDashboardName(const QString & orc_CurrentDashboardTabName)
{
   Q_UNUSED(orc_CurrentDashboardTabName) //Base class have  as pure virtual function which we don't need here in this
                                         // child class expecting for future use (but so far using in
                                         // "C_SyvDaChaWidget")
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Event

   \param[in,out]  opc_Event  Event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSceneWidget::showEvent(QShowEvent * const opc_Event)
{
   QWidget::showEvent(opc_Event);
   // Trigger delayed update
   // Necessary because of issue #49525
   this->mc_UpdateTimer.start();
}
