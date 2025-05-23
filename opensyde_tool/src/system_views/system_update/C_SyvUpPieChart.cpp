//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for update PieChart (implementation)

   Widget for update PieChart

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QPainter>
#include <QFileInfo>
#include "constants.hpp"
#include "C_SclString.hpp"
#include "C_SyvUpPieChart.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const QString C_SyvUpPieChart::mhc_FILE_PATH = "../../src/images/custom.gif";

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvUpPieChart::C_SyvUpPieChart(QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mu16_ValueInPercent(0),
   mc_ActiveColor(mc_STYLE_GUIDE_COLOR_13),
   mq_FinishedAnimation(false)
{
   //lint -e{1938}  static const is guaranteed preinitialized before main
   const QFileInfo c_FileInfo(C_SyvUpPieChart::mhc_FILE_PATH);

   if (c_FileInfo.exists() == true)
   {
      //lint -e{1938}  static const is guaranteed preinitialized before main
      this->mpc_Movie = new QMovie(C_SyvUpPieChart::mhc_FILE_PATH);
   }
   else
   {
      this->mpc_Movie = new QMovie("://images/system_views/UpdateFinishedAnimation.gif");
   }
   connect(this->mpc_Movie, &QMovie::frameChanged, this, &C_SyvUpPieChart::m_OnFrameChanged);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvUpPieChart::~C_SyvUpPieChart(void)
{
   delete (mpc_Movie);
   mpc_Movie = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update progress value

   \param[in]  ou16_Progress100  Progress in percent
   \param[in]  oq_Finished       Flag if finished
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPieChart::SetProgress100(const uint16_t ou16_Progress100, const bool oq_Finished)
{
   this->mu16_ValueInPercent = ou16_Progress100;
   this->mq_FinishedAnimation = oq_Finished;
   if ((this->mq_FinishedAnimation == true) && (this->mpc_Movie != NULL))
   {
      this->mpc_Movie->start();
   }
   this->update();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set active color

   \param[in] orc_Value New active color
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPieChart::SetActiveColor(const QColor & orc_Value)
{
   this->mc_ActiveColor = orc_Value;
   this->update();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overrided paint event

   Draws the element

   \param[in,out] opc_Event  Pointer to paint event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPieChart::paintEvent(QPaintEvent * const opc_Event)
{
   const QRect c_CompleteRect = this->rect();
   const int32_t s32_Height = std::min(c_CompleteRect.width(), c_CompleteRect.height());
   const int32_t s32_X = static_cast<int32_t>(c_CompleteRect.x()) +
                         ((static_cast<int32_t>(c_CompleteRect.width()) - s32_Height) / 2);
   const int32_t s32_Y = static_cast<int32_t>(c_CompleteRect.y()) +
                         ((static_cast<int32_t>(c_CompleteRect.height()) - s32_Height) / 2);
   QPainter c_Painter(this);

   if ((this->mq_FinishedAnimation == true) && (this->mpc_Movie != NULL))
   {
      c_Painter.drawPixmap(QRect(s32_X, s32_Y, s32_Height, s32_Height),
                           this->mpc_Movie->currentPixmap().scaled(s32_Height, s32_Height, Qt::KeepAspectRatio,
                                                                   Qt::SmoothTransformation));
   }
   else
   {
      const float32_t f32_Height = static_cast<float32_t>(s32_Height);
      //const QColor c_Invalid(mc_STYLE_GUIDE_COLOR_24);
      const QBrush c_BackgroundBrush(mc_STYLE_GUIDE_COLOR_10);
      const QBrush c_ForegroundBrush(mc_STYLE_GUIDE_COLOR_0);
      const QBrush c_PieBrush(mc_ActiveColor);
      const QPen c_Pen(mc_STYLE_GUIDE_COLOR_1);
      QFont c_Font;
      QRect c_Rect;
      int16_t s16_StartAngle;
      float32_t f32_SpanAngle;
      const float32_t f32_InnerCircleOffset = ((f32_Height * 12.0F) / 180.0F) / 2.0F;
      const int32_t s32_InnerCircleOffset = static_cast<int32_t>(f32_InnerCircleOffset);
      stw::scl::C_SclString c_String;

      c_Painter.save();

      c_Font.setFamily("Segoe UI");
      c_Font.setPointSize(static_cast<int32_t>((s32_Height - (6 * s32_InnerCircleOffset)) / 3));
      //convert to pixel size important for high dpi support
      c_Font.setPixelSize(c_Font.pointSize());

      c_Painter.setPen(Qt::NoPen);
      c_Painter.setRenderHint(QPainter::Antialiasing);

      //background cycle
      c_Painter.setBrush(c_BackgroundBrush);
      c_Painter.drawEllipse(s32_X, s32_Y, s32_Height, s32_Height);

      //pie
      c_Rect.setRect(s32_X, s32_Y, s32_Height, s32_Height);
      s16_StartAngle = 90 * 16;
      f32_SpanAngle = (-1.0F * (360.0F / 100.0F) * static_cast<float32_t>(mu16_ValueInPercent)  * 16.0F);

      c_Painter.setBrush(c_PieBrush);
      c_Painter.drawPie(c_Rect, s16_StartAngle, static_cast<int32_t>(f32_SpanAngle));

      //foreground cycle
      c_Painter.setBrush(c_ForegroundBrush);
      c_Painter.drawEllipse(static_cast<int32_t>(s32_X + s32_InnerCircleOffset),
                            static_cast<int32_t>(s32_Y + s32_InnerCircleOffset),
                            static_cast<int32_t>(s32_Height - (2 * s32_InnerCircleOffset)),
                            static_cast<int32_t>(s32_Height - (2 * s32_InnerCircleOffset)));

      //text
      c_Painter.setPen(c_Pen);
      c_Painter.setFont(c_Font);
      c_String = stw::scl::C_SclString::IntToStr(mu16_ValueInPercent);
      c_String += "%";
      c_Painter.drawText(c_Rect, static_cast<int32_t>(Qt::AlignCenter), c_String.c_str());

      c_Painter.restore();
   }
   QWidget::paintEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle frame changed
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPieChart::m_OnFrameChanged(void)
{
   this->update();
}
