#include "UBThumbnailsView.h"

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




#include <QList>
#include <QPointF>
#include <QPixmap>
#include <QTransform>
#include <QScrollBar>
#include <QFontMetrics>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>

#include "core/UBApplication.h"
#include "UBThumbnailsView.h"
#include "board/UBBoardController.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardPaletteManager.h"
#include "core/UBApplicationController.h"
#include "core/UBPersistenceManager.h"
#include "UBThumbnailView.h"

UBThumbnailsView::UBThumbnailsView(QWidget *parent, const char *name)
    : QGraphicsView(parent)
    , mThumbnailWidth(0)
    , mThumbnailMinWidth(100)
    , mMargin(20)
    , mDropSource(nullptr)
    , mDropTarget(nullptr)
    , mDropBar(new QGraphicsRectItem(0))
    , mLongPressInterval(350)
{
    setScene(new QGraphicsScene(this));

    mDropBar->setPen(QPen(Qt::darkGray));
    mDropBar->setBrush(QBrush(Qt::lightGray));
    scene()->addItem(mDropBar);
    mDropBar->hide();

    setObjectName(name);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShadow(QFrame::Plain);

    mLongPressTimer.setInterval(mLongPressInterval);
    mLongPressTimer.setSingleShot(true);
}

void UBThumbnailsView::moveThumbnail(int from, int to)
{
    mThumbnails.move(from, to);

    updateThumbnailsPos();
}

void UBThumbnailsView::updateThumbnails()
{
    updateThumbnailsPos();
}

void UBThumbnailsView::removeThumbnail(int i)
{
    UBDraggableThumbnailView* item = mThumbnails.at(i);

    scene()->removeItem(item->pageNumber());
    scene()->removeItem(item);

    mThumbnails.removeAt(i);

    updateThumbnailsPos();
}

UBDraggableThumbnailView* UBThumbnailsView::createThumbnail(UBDocumentContainer* source, int i)
{
    UBApplication::showMessage(tr("Loading page (%1/%2)").arg(i+1).arg(source->selectedDocument()->pageCount()));

    UBGraphicsScene* pageScene = UBPersistenceManager::persistenceManager()->loadDocumentScene(source->selectedDocument(), i);
    UBThumbnailView* pageView = new UBThumbnailView(pageScene);

    return new UBDraggableThumbnailView(pageView, source->selectedDocument(), i);
}

void UBThumbnailsView::addThumbnail(UBDocumentContainer* source, int i)
{
    UBDraggableThumbnailView* item = createThumbnail(source, i);
    mThumbnails.insert(i, item);

    scene()->addItem(item);
    scene()->addItem(item->pageNumber());

    updateThumbnailsPos();
}

void UBThumbnailsView::clearThumbnails()
{
    for(int i = 0; i < mThumbnails.size(); i++)
    {
        scene()->removeItem(mThumbnails.at(i)->pageNumber());
        scene()->removeItem(mThumbnails.at(i));
        mThumbnails.at(i)->deleteLater();
    }

    mThumbnails.clear();
}

void UBThumbnailsView::initThumbnails(UBDocumentContainer* source)
{
    clearThumbnails();

    for(int i = 0; i < source->selectedDocument()->pageCount(); i++)
    {
        mThumbnails.append(createThumbnail(source, i));

        scene()->addItem(mThumbnails.last());
        scene()->addItem(mThumbnails.last()->pageNumber());
    }

    updateThumbnailsPos();
}

void UBThumbnailsView::centerOnThumbnail(int index)
{
    centerOn(mThumbnails.at(index));
}

void UBThumbnailsView::ensureVisibleThumbnail(int index)
{
    ensureVisible(mThumbnails.at(index));
}

void UBThumbnailsView::updateThumbnailsPos()
{
    qreal thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;

    for (int i=0; i < mThumbnails.length(); i++)
    {
        mThumbnails.at(i)->setSceneIndex(i);
        mThumbnails.at(i)->setPageNumber(i);
        mThumbnails.at(i)->updatePos(mThumbnailWidth, thumbnailHeight);
    }

    scene()->setSceneRect(0, 0, scene()->itemsBoundingRect().size().width() - verticalScrollBar()->width(), scene()->itemsBoundingRect().size().height());

    update();
}

void UBThumbnailsView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Update the thumbnails width
    mThumbnailWidth = (width() > mThumbnailMinWidth) ? width() - verticalScrollBar()->width() - 2*mMargin : mThumbnailMinWidth;

    // Refresh the scene
    updateThumbnailsPos();

    emit UBApplication::boardController->centerOnThumbnailRequired(UBApplication::boardController->activeSceneIndex());
}

void UBThumbnailsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if (!event->isAccepted())
    {
        mLastPressedMousePos = event->pos();

        UBDraggableThumbnailView* item = dynamic_cast<UBDraggableThumbnailView*>(itemAt(event->pos()));

        if (item)
        {
            UBApplication::boardController->persistViewPositionOnCurrentScene();
            UBApplication::boardController->persistCurrentScene();
            UBApplication::boardController->setActiveDocumentScene(item->sceneIndex());
            UBApplication::boardController->centerOn(UBApplication::boardController->activeScene()->lastCenter());
        }

        mLongPressTimer.start();
    }
}

void UBThumbnailsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void UBThumbnailsView::longPressTimeout()
{
    if (QApplication::mouseButtons() != Qt::NoButton)
        emit mousePressAndHoldEventRequired(mLastPressedMousePos);

    mLongPressTimer.stop();
}

void UBThumbnailsView::mousePressAndHoldEvent(QPoint pos)
{
    UBDraggableThumbnailView* item = dynamic_cast<UBDraggableThumbnailView*>(itemAt(pos));
    if (item)
    {
        mDropSource = item;
        mDropTarget = item;

        QPixmap pixmap = item->widget()->grab().scaledToWidth(mThumbnailWidth/2);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(new QMimeData());
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));

        drag->exec();
    }
}

void UBThumbnailsView::mouseReleaseEvent(QMouseEvent *event)
{
    mLongPressTimer.stop();

    QGraphicsView::mouseReleaseEvent(event);
}

void UBThumbnailsView::dragEnterEvent(QDragEnterEvent *event)
{
    mDropBar->show();

    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->acceptProposedAction();
    }
}

void UBThumbnailsView::dragMoveEvent(QDragMoveEvent *event)
{
    QPointF position = event->pos();

    //autoscroll during drag'n'drop
    QPointF scenePos = mapToScene(position.toPoint());
    int thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;
    QRectF thumbnailArea(0, scenePos.y() - thumbnailHeight/2, mThumbnailWidth, thumbnailHeight);

    ensureVisible(thumbnailArea);

    UBDraggableThumbnailView* item = dynamic_cast<UBDraggableThumbnailView*>(itemAt(position.toPoint()));
    if (item)
    {
        if (item != mDropTarget)
            mDropTarget = item;

        qreal scale = item->transform().m11();

        QPointF itemCenter(item->pos().x() + (item->boundingRect().width()-verticalScrollBar()->width()) * scale,
                           item->pos().y() + item->boundingRect().height() * scale / 2);

        bool dropAbove = mapToScene(position.toPoint()).y() < itemCenter.y();
        bool movingUp = mDropSource->sceneIndex() > item->sceneIndex();
        qreal y = 0;

        if (movingUp)
        {
            if (dropAbove)
            {
                y = item->pos().y() - UBSettings::thumbnailSpacing / 2;
                if (mDropBar->y() != y)
                    mDropBar->setRect(QRectF(item->pos().x(), y, (item->boundingRect().width()-verticalScrollBar()->width())*scale, 3));
            }
        }
        else
        {
            if (!dropAbove)
            {
                y = item->pos().y() + item->boundingRect().height() * scale + UBSettings::thumbnailSpacing / 2;
                if (mDropBar->y() != y)
                    mDropBar->setRect(QRectF(item->pos().x(), y, (item->boundingRect().width()-verticalScrollBar()->width())*scale, 3));
            }
        }
    }

    event->acceptProposedAction();
}

void UBThumbnailsView::dropEvent(QDropEvent *event)
{
    Q_UNUSED(event);

    if (mDropSource->sceneIndex() != mDropTarget->sceneIndex())
        UBApplication::boardController->moveSceneToIndex(mDropSource->sceneIndex(), mDropTarget->sceneIndex());

    mDropSource = NULL;
    mDropTarget = NULL;

    mDropBar->setRect(QRectF());
    mDropBar->hide();
}
