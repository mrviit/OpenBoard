#include "UBDocumentThumbnailsView.h"

#include "core/UBApplication.h"
#include "UBBoardThumbnailsView.h"
#include "document/UBDocumentController.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardPaletteManager.h"
#include "core/UBApplicationController.h"
#include "core/UBPersistenceManager.h"
#include "UBThumbnailView.h"
#include "domain/UBGraphicsWidgetItem.h"

UBDocumentThumbnailsView::UBDocumentThumbnailsView(QWidget* parent, const char* name) :
    UBThumbnailsView(parent)
{
    setScene(new QGraphicsScene(this));

    setOptimizationFlag(IndirectPainting);

    mDropBar->setPen(QPen(Qt::darkGray));
    mDropBar->setBrush(QBrush(Qt::lightGray));
    scene()->addItem(mDropBar);
    mDropBar->hide();

    setObjectName(name);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShadow(QFrame::Plain);

    //setRenderingContext(UBGraphicsScene::NonScreen);

    mThumbnailWidth = 100 - 2*mMargin;

    mLongPressTimer.setInterval(mLongPressInterval);
    mLongPressTimer.setSingleShot(true);

    connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressTimeout()), Qt::UniqueConnection);

    connect(this, SIGNAL(moveThumbnailRequired(int, int)), this, SLOT(moveThumbnail(int, int)), Qt::UniqueConnection);
    connect(this, SIGNAL(mousePressAndHoldEventRequired(QPoint)), this, SLOT(mousePressAndHoldEvent(QPoint)), Qt::UniqueConnection);
}


void UBDocumentThumbnailsView::moveThumbnail(int from, int to)
{
    mThumbnails.move(from, to);

    updateThumbnailsPos();
}

void UBDocumentThumbnailsView::updateThumbnails()
{
    updateThumbnailsPos();
}

void UBDocumentThumbnailsView::removeThumbnail(int i)
{
    UBDraggableThumbnailView* item = mThumbnails.at(i);

    scene()->removeItem(item->pageNumber());
    scene()->removeItem(item);

    mThumbnails.removeAt(i);

    updateThumbnailsPos();
}

void UBDocumentThumbnailsView::addThumbnail(UBDocumentContainer* source, int i)
{
    UBDraggableThumbnailView* item = createThumbnail(source, i);
    mThumbnails.insert(i, item);

    scene()->addItem(item);
    scene()->addItem(item->pageNumber());

    updateThumbnailsPos();
}

void UBDocumentThumbnailsView::clearThumbnails()
{
    for(int i = 0; i < mThumbnails.size(); i++)
    {
        scene()->removeItem(mThumbnails.at(i)->pageNumber());
        scene()->removeItem(mThumbnails.at(i));
        mThumbnails.at(i)->deleteLater();
    }

    mThumbnails.clear();
}

void UBDocumentThumbnailsView::initThumbnails(UBDocumentContainer* source)
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

void UBDocumentThumbnailsView::centerOnThumbnail(int index)
{
    centerOn(mThumbnails.at(index));
}

void UBDocumentThumbnailsView::ensureVisibleThumbnail(int index)
{
    ensureVisible(mThumbnails.at(index));
}

void UBDocumentThumbnailsView::updateThumbnailsPos()
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

void UBDocumentThumbnailsView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Update the thumbnails width
    mThumbnailWidth = (100 > mThumbnailMinWidth) ? 100 - verticalScrollBar()->width() - 2*mMargin : mThumbnailMinWidth;

    // Refresh the scene
    updateThumbnailsPos();

    emit UBApplication::boardController->centerOnThumbnailRequired(UBApplication::boardController->activeSceneIndex());
}

void UBDocumentThumbnailsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if (!event->isAccepted())
    {
        mLastPressedMousePos = event->pos();

        mLongPressTimer.start();
    }
}

void UBDocumentThumbnailsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void UBDocumentThumbnailsView::longPressTimeout()
{
    if (QApplication::mouseButtons() != Qt::NoButton)
        emit mousePressAndHoldEventRequired(mLastPressedMousePos);

    mLongPressTimer.stop();
}

void UBDocumentThumbnailsView::mousePressAndHoldEvent(QPoint pos)
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

void UBDocumentThumbnailsView::mouseReleaseEvent(QMouseEvent *event)
{
    mLongPressTimer.stop();

    QGraphicsView::mouseReleaseEvent(event);
}

void UBDocumentThumbnailsView::dragEnterEvent(QDragEnterEvent *event)
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

void UBDocumentThumbnailsView::dragMoveEvent(QDragMoveEvent *event)
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

void UBDocumentThumbnailsView::dropEvent(QDropEvent *event)
{
    Q_UNUSED(event);

    if (mDropSource->sceneIndex() != mDropTarget->sceneIndex())
        UBApplication::boardController->moveSceneToIndex(mDropSource->sceneIndex(), mDropTarget->sceneIndex());

    mDropSource = NULL;
    mDropTarget = NULL;

    mDropBar->setRect(QRectF());
    mDropBar->hide();
}

QList<QGraphicsItem*> UBDocumentThumbnailsView::selectedItems()
{
    QList<QGraphicsItem*> sortedSelectedItems = scene()->selectedItems();
    qSort(sortedSelectedItems.begin(), sortedSelectedItems.end(), thumbnailLessThan);
    return sortedSelectedItems;
}

void UBDocumentThumbnailsView::selectItemAt(int pIndex, bool extend)
{
    QGraphicsItem* itemToSelect = 0;

    /*if (pIndex >= 0 && pIndex < mGraphicItems.size())
        itemToSelect = mGraphicItems.at(pIndex);
*/
    foreach (QGraphicsItem* item, items())
    {
        if (item == itemToSelect)
        {
  //          mLastSelectedThumbnail = dynamic_cast<UBThumbnail*>(item);
            item->setSelected(true);
            ensureVisible(item);
        }
        else if (!extend)
        {
            item->setSelected(false);
        }
    }
}

void UBDocumentThumbnailsView::unselectItemAt(int pIndex)
{
    /*if (pIndex >= 0 && pIndex < mGraphicItems.size())
    {
        QGraphicsItem *itemToUnselect = mGraphicItems.at(pIndex);
        itemToUnselect->setSelected(false);
    }*/
}

bool UBDocumentThumbnailsView::thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2)
{
    UBThumbnail *thumbnail1 = dynamic_cast<UBThumbnail*>(item1);
    UBThumbnail *thumbnail2 = dynamic_cast<UBThumbnail*>(item2);
    if (thumbnail1 && thumbnail2)
    {
        if (thumbnail1->row() != thumbnail2->row())
            return thumbnail1->row() < thumbnail2->row();
        else
            return thumbnail1->column() < thumbnail2->column();
    }
    return false;
}

