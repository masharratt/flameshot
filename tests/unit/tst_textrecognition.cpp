// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for macOS Vision text recognition (Copy Text tool).

#include "platform/macos/textrecognition.h"

#include <QFont>
#include <QImage>
#include <QPainter>
#include <QTest>

namespace {

QImage renderLines(const QStringList& lines)
{
    QImage img(1400, 200 * lines.size() + 100, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setPen(Qt::black);
    QFont f(QStringLiteral("Helvetica"));
    f.setPixelSize(90);
    p.setFont(f);
    int y = 150;
    for (const QString& line : lines) {
        p.drawText(60, y, line);
        y += 200;
    }
    return img;
}

} // namespace

class TestTextRecognition : public QObject
{
    Q_OBJECT
private slots:
    void readsRenderedText()
    {
        const QString out =
          recognizeText(renderLines({ QStringLiteral("Hello Flameshot 42") }));
        QVERIFY2(out.contains(QStringLiteral("Hello Flameshot 42"),
                              Qt::CaseInsensitive),
                 qPrintable(out));
    }

    void emptyImageReturnsEmpty()
    {
        QImage img(600, 300, QImage::Format_RGB32);
        img.fill(Qt::white);
        QVERIFY(recognizeText(img).isEmpty());
        QVERIFY(recognizeText(QImage()).isEmpty());
    }

    void linesComeOutTopToBottom()
    {
        const QString out = recognizeText(
          renderLines({ QStringLiteral("First line"),
                        QStringLiteral("Second line") }));
        const qsizetype a = out.indexOf(QStringLiteral("First"));
        const qsizetype b = out.indexOf(QStringLiteral("Second"));
        QVERIFY2(a >= 0 && b > a, qPrintable(out));
        QVERIFY2(out.contains(QLatin1Char('\n')), qPrintable(out));
    }
};

QTEST_MAIN(TestTextRecognition)
#include "tst_textrecognition.moc"
