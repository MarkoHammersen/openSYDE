//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for system view dashboard progress bar properties (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDAPEPROGRESSBAR_HPP
#define C_SYVDAPEPROGRESSBAR_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QWidget>

#include "C_SyvDaPeBase.hpp"

#include "C_PuiSvDbProgressBar.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace Ui
{
class C_SyvDaPeProgressBar;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_SyvDaPeProgressBar :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_SyvDaPeProgressBar(C_SyvDaPeBase & orc_Parent, const bool oq_DarkMode);
   ~C_SyvDaPeProgressBar(void) override;

   void InitStaticNames(void) const;

   bool GetShowMinMax(void) const;
   stw::opensyde_gui_logic::C_PuiSvDbProgressBar::E_Type GetType(void) const;
   stw::opensyde_gui_logic::C_PuiSvDbProgressBar::E_Alignment GetAlignment(void) const;
   void SetShowMinMax(const bool oq_Value) const;
   void SetType(const stw::opensyde_gui_logic::C_PuiSvDbProgressBar::E_Type oe_Type);
   void SetAlignment(const stw::opensyde_gui_logic::C_PuiSvDbProgressBar::E_Alignment oe_Alignment) const;

private:
   //Avoid call
   C_SyvDaPeProgressBar(const C_SyvDaPeProgressBar &);
   C_SyvDaPeProgressBar & operator =(const C_SyvDaPeProgressBar &) &;

   //----Functions------------------------------------------------------------------------------------------------------
   void m_TypeChanged(void);
   void m_FillAlignmentComboBox(const int32_t os32_Type) const;
   void m_UpdatePreview(void);
   void m_GetProgressBarPreviewSizeLocation(const QSizeF & orc_SceneSize, QSizeF * const opc_ProgressBarRecSize,
                                            QPointF * const opc_ProgressBarPosition);

   //----Pointers-------------------------------------------------------------------------------------------------------
   Ui::C_SyvDaPeProgressBar * mpc_Ui;

   //----Variables------------------------------------------------------------------------------------------------------
   //lint -e{1725} Only problematic if copy or assignment is allowed
   C_SyvDaPeBase & mrc_ParentDialog;
   const bool mq_DarkMode;

   //----Constants------------------------------------------------------------------------------------------------------
   static const int32_t mhs32_INDEX_STYLE_TYPE1;
   static const int32_t mhs32_INDEX_STYLE_TYPE2;
   static const int32_t mhs32_INDEX_STYLE_TYPE3;

   static const int32_t mhs32_INDEX_ALIGNMENT_LEFT;
   static const int32_t mhs32_INDEX_ALIGNMENT_RIGHT;
   static const int32_t mhs32_INDEX_ALIGNMENT_TOP;
   static const int32_t mhs32_INDEX_ALIGNMENT_BOTTOM;

   const float32_t mf32_HalfModifier = 2.0F;
   const float32_t mf32_MinimumFontSize = 15.0F;
};
}
}

#endif
