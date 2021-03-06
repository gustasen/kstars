/***************************************************************************
                          pvplotwidget.h
                             -------------------
    begin                : Sat 17 Dec 2005
    copyright            : (C) 2005 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include "kplotwidget.h"

class PlanetViewer;

class PVPlotWidget : public KPlotWidget
{
    Q_OBJECT
  public:
    explicit PVPlotWidget(QWidget *parent = nullptr);

  public slots:
    void slotZoomIn();
    void slotZoomOut();

  signals:
    void doubleClicked(double, double);

  protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

  private:
    void updateFactor(const int modifier);

    bool mouseButtonDown { false };
    int oldx { 0 };
    int oldy { 0 };
    double factor { 2 };
    PlanetViewer *pv { nullptr };
};
