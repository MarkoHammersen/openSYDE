//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Complete line with all functionality (header)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_GILILINEGROUP_HPP
#define C_GILILINEGROUP_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QGraphicsItemGroup>
#include <QTimer>

#include "stwtypes.hpp"

#include "C_GiLiLine.hpp"
#include "C_GiBiBase.hpp"
#include "C_PuiBsLineBase.hpp"
#include "C_GiBiCustomMouseItem.hpp"
#include "C_GiLiInteractionPoint.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_GiLiLineGroup :
   public C_GiBiConnectableItem,
   public C_GiBiCustomMouseItem,
   public C_GiBiBase,
   public QGraphicsItemGroup
{
   Q_OBJECT

public:
   C_GiLiLineGroup(const std::vector<QPointF> * const opc_Points = NULL, const bool & orq_MiddleLine = false,
                   QGraphicsItem * const opc_Parent = NULL);
   ~C_GiLiLineGroup(void) override;

   int32_t BendLine(const QPointF & orc_ScenePos, const int32_t * const ops32_Index = NULL);
   int32_t RemoveBend(const QPointF & orc_ScenePos, const int32_t * const ops32_Index = NULL);
   virtual void OnPointChange(const int32_t & ors32_PointIndex);
   QRectF boundingRect() const override;
   void RestoreDefaultCursor(void) override;
   void SetTemporaryCursor(const QCursor & orc_TemporaryCursor) override;
   void SetDefaultCursor(const QCursor & orc_Value) override;
   virtual void CopyStyle(const QGraphicsItem * const opc_GuidelineItem);

   //GI base
   void SetZetValueCustom(const float64_t of64_ZetValue) override;

   // Wrapper calls for C_GiLiLine
   virtual void SetWidth(const int32_t & ors32_Width);
   void SetColor(const QColor & orc_Color);
   void SetMiddleLineColor(const QColor & orc_Color);
   void SetMiddleLine(const bool & orq_MiddleLine);
   int32_t GetWidth(void) const;
   QColor GetColor(void) const;
   QColor GetMiddleLineColor(void) const;
   QVector<C_GiLiLineConnection *> GetLines(void) const;
   bool GetMiddleLine(void) const;
   int32_t GetNumberPoints(void) const;
   virtual QPointF GetPos(void) const;
   virtual void SetDisabledLook(const bool oq_Disabled);
   //lint -e{1735} Suppression, because default parameters are identical
   virtual void SetAnimated(const bool oq_Active, const bool oq_Inverse = false, const bool oq_SpeedUp = false,
                            const QPolygonF oc_Polygon = QPolygonF(), const bool oq_ShowOrignalLine = true);

   virtual void SetResizing(const bool & orq_ResizeActive);
   void FindClosestPoint(const QPointF & orc_ScenePoint, QPointF & orc_Closest) const override;
   void FindClosestConnection(const QPointF & orc_ScenePoint, int32_t & ors32_Index) const;
   void UpdatePoint(const int32_t & ors32_Index, const QPointF & orc_Pos,
                    const bool oq_BlockTriggerOfChangedSignal = false);
   QPointF GetPointScenePos(const int32_t os32_Index) const;
   void GetPointPos(const int32_t & ors32_Index, QPointF & orc_Pos) const;
   bool CheckBendPointAt(const QPointF & orc_ScenePoint) const;

   void TriggerSigChangedGraphic(void) override;

   void paint(QPainter * const opc_Painter, const QStyleOptionGraphicsItem * const opc_Option,
              QWidget * const opc_Widget) override;
   QPainterPath shape(void) const override;
   void UpdateTransform(const QTransform & orc_Transform);

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736

Q_SIGNALS:
   //lint -restore
   void SigItemWasMoved(const QPointF & orc_PositionDifference);
   void SigItemWasResized(const int32_t & ors32_InteractionPointIndex, const QPointF & orc_PositionDifference);
   void SigSubItemMoves(const int32_t & ors32_LineIndex, const QPointF & orc_PositionDifference);
   void SigSubItemWasMoved(const int32_t & ors32_LineIndex, const QPointF & orc_PositionDifference);

protected:
   QVector<C_GiLiInteractionPoint *> mc_Points; ///< Interaction points of segmented line
   C_GiLiLine * mpc_LinePath;

   virtual void m_InitPoint(const int32_t & ors32_Index, const QPointF & orc_Pos);
   virtual void m_AddPointAt(const int32_t & ors32_Index, const QPointF & orc_Pos);
   virtual void m_RemovePointAt(const int32_t & ors32_Index);

   void m_LoadBasicData(const stw::opensyde_gui_logic::C_PuiBsLineBase & orc_Data);
   void m_UpdateBasicData(stw::opensyde_gui_logic::C_PuiBsLineBase & orc_Data) const;
   void m_Init(const std::vector<QPointF> & orc_Points);

   QVariant itemChange(const GraphicsItemChange oe_Change, const QVariant & orc_Value) override;
   void mousePressEvent(QGraphicsSceneMouseEvent * const opc_Event) override;
   void mouseMoveEvent(QGraphicsSceneMouseEvent * const opc_Event) override;
   void mouseReleaseEvent(QGraphicsSceneMouseEvent * const opc_Event) override;

   enum E_MoveMode
   {
      eNO_ELEMENT,
      ePOINT,
      eLINE,
      eALL
   };

   E_MoveMode me_ActiveResizeMode; ///< Currently active resize mode
   int32_t ms32_ActiveItemIndex;   ///< Currently active item index (Mainly for resize mode)

   static const float64_t mhf64_MAX_DIST_TO_ALIGN;

private:
   //Avoid call
   C_GiLiLineGroup(const C_GiLiLineGroup &);
   C_GiLiLineGroup & operator =(const C_GiLiLineGroup &) &; //lint !e1511 //we want to hide the base func.

   void m_SetInteractionVisibility(const bool & orq_Visible);
   void m_HideInteraction(const bool & orq_Invisible);
   void m_CheckLineGrid(const QPointF & orc_MouseScenePos);
   static bool mh_Near(const float64_t of64_Exact, const float64_t of64_Eval);

   QPointF mc_LastKnownPosition;
   bool mq_BlockChangeSignal;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif // C_LINEGRAPHICSITEMGROUP_HPP
