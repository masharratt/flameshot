// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the after-edit thumbnail's click, drag and drop-payload logic.

#include "widgets/thumbnailgesture.h"

#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QTest>
#include <QUrl>

class TestThumbnailGesture : public QObject
{
    Q_OBJECT
private slots:
    void leftClickOpens()
    {
        QCOMPARE(thumbnailReleaseAction(Qt::LeftButton),
                 ThumbnailAction::Open);
    }

    void rightClickCloses()
    {
        QCOMPARE(thumbnailReleaseAction(Qt::RightButton),
                 ThumbnailAction::Close);
    }

    void otherButtonDoesNothing()
    {
        QCOMPARE(thumbnailReleaseAction(Qt::MiddleButton),
                 ThumbnailAction::None);
    }

    void leftMoveBeyondDistanceDrags()
    {
        QVERIFY(thumbnailShouldStartDrag(Qt::LeftButton, QPoint(10, 10), QPoint(25, 10), 10));
    }

    void leftMoveWithinDistanceDoesNotDrag()
    {
        QVERIFY(!thumbnailShouldStartDrag(Qt::LeftButton, QPoint(10, 10), QPoint(14, 13), 10));
    }

    void rightMoveNeverDrags()
    {
        QVERIFY(!thumbnailShouldStartDrag(Qt::RightButton, QPoint(10, 10), QPoint(100, 100), 10));
    }

    void dropPayloadCarriesFileAndImage()
    {
        QImage image(4, 3, QImage::Format_ARGB32);
        image.fill(Qt::red);
        const QString path = QStringLiteral("/tmp/some dir/shot.png");
        QMimeData* data = thumbnailMimeData(path, image);
        QVERIFY(data->hasUrls());
        QCOMPARE(data->urls().size(), 1);
        QCOMPARE(data->urls().first(), QUrl::fromLocalFile(path));
        QVERIFY(data->hasImage());
        QCOMPARE(qvariant_cast<QImage>(data->imageData()).size(), QSize(4, 3));
        delete data;
    }
};

QTEST_GUILESS_MAIN(TestThumbnailGesture)
#include "tst_thumbnailgesture.moc"
