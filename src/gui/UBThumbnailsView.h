#ifndef UBTHUMBNAILSVIEW_H
#define UBTHUMBNAILSVIEW_H


#include <QGraphicsView>

#include "document/UBDocumentContainer.h"
#include "gui/UBThumbnailWidget.h"

class UBThumbnailsView : public QGraphicsView
{
    Q_OBJECT

public:
     UBThumbnailsView(QWidget* parent = nullptr, const char* name = "UBBoardThumbnailsView");

public slots:
    virtual void ensureVisibleThumbnail(int index);
    virtual void centerOnThumbnail(int index);

    virtual void clearThumbnails();
    virtual void initThumbnails(UBDocumentContainer* source);
    virtual void addThumbnail(UBDocumentContainer* source, int i);
    virtual void moveThumbnail(int from, int to);
    virtual void removeThumbnail(int i);
    virtual void updateThumbnails();

    virtual void longPressTimeout();
    virtual void mousePressAndHoldEvent(QPoint pos);

protected:
    virtual void resizeEvent(QResizeEvent *event);

    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual UBDraggableThumbnailView* createThumbnail(UBDocumentContainer* source, int i);
    virtual void updateThumbnailsPos();

    QList<UBDraggableThumbnailView*> mThumbnails;

    int mThumbnailWidth;
    const int mThumbnailMinWidth;
    const int mMargin;

    UBDraggableThumbnailView* mDropSource;
    UBDraggableThumbnailView* mDropTarget;
    QGraphicsRectItem *mDropBar;

    int mLongPressInterval;
    QTimer mLongPressTimer;
    QPoint mLastPressedMousePos;

signals:
    void mousePressAndHoldEventRequired(QPoint pos);
    void moveThumbnailRequired(int from, int to);

};

#endif // UBTHUMBNAILSVIEW_H
