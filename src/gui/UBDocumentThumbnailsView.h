#ifndef UBDOCUMENTTHUMBNAILSVIEW_H
#define UBDOCUMENTTHUMBNAILSVIEW_H

#include "UBThumbnailsView.h"

class UBDocumentThumbnailsView : public UBThumbnailsView
{
    Q_OBJECT

public:
    UBDocumentThumbnailsView(QWidget* parent=0, const char* name="UBDocumentThumbnailsView");

    QList<QGraphicsItem*> selectedItems();
    void selectItemAt(int pIndex, bool extend = false);
    void unselectItemAt(int pIndex);

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
    static bool thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2);
};

#endif // UBDOCUMENTTHUMBNAILSVIEW_H
