/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBBOARDTHUMBNAILSVIEW_H
#define UBBOARDTHUMBNAILSVIEW_H

#include <QResizeEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>

#include "document/UBDocumentContainer.h"
#include "UBThumbnailWidget.h"
#include "UBThumbnailsView.h"

class UBBoardThumbnailsView : public UBThumbnailsView
{
    Q_OBJECT
public:
    UBBoardThumbnailsView(QWidget* parent=0, const char* name="UBBoardThumbnailsView");

public slots:
    void ensureVisibleThumbnail(int index) override;
    void centerOnThumbnail(int index) override;

    void clearThumbnails() override;
    void initThumbnails(UBDocumentContainer* source) override;
    void addThumbnail(UBDocumentContainer* source, int i) override;
    void moveThumbnail(int from, int to) override;
    void removeThumbnail(int i) override;
    void updateThumbnails() override;

    void longPressTimeout() override;
    void mousePressAndHoldEvent(QPoint pos) override;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void updateThumbnailsPos() override;
};

#endif // UBBOARDTHUMBNAILSVIEW_H
