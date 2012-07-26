/****************************************************************************
 *
 * Copyright (C) 2012 Olivier Goffart <ogoffart@woboq.com>
 * http://woboq.com
 *
 * This is an experiment to process UTF-8 using SSE4 intrinscis.
 * Read: http://woboq.com/blog/utf-8-processing-using-simd.html
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * For any question, please contact contact@woboq.com
 *
 ****************************************************************************/

#include <QtCore/QtCore>
#include <iostream>

#ifdef USE_ICONV
#include <iconv.h>
#endif

void fromUtf8_sse(const char *&src, int &len, ushort * &dst);

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QString filename = app.arguments().value(1);
    if (filename.isEmpty()) {
        std::cout << "Usage: " << argv[0] << " <utf-8 file>. " << std::endl;
        exit(-1);
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        printf("Unable to open %s for reading\n", qPrintable(filename));
        exit(-1);
    }

    QByteArray source = file.readAll();

    QString str;
    str.resize(source.size()*3);

#ifdef USE_ICONV
    iconv_t iconv_cd = iconv_open("UTF-8", "UTF-16");
#endif

    const int Iterations = 10000;
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < Iterations; ++i) {
        size_t len = source.size();
        char *dst = reinterpret_cast<char *>(str.begin());
        size_t outlen = str.length()*2;
#ifdef USE_ICONV
        char *src = source.begin();
        iconv(iconv_cd, &src, &len, &dst, &outlen);
#elif defined USE_U8U16
        extern size_t u8u16(char**inbuf,size_t*inbytesleft,char**outbuf,size_t*outbytesleft);
        char *src = source.begin();
        u8u16(&src, &len, &dst, &outlen);
#else
        const char *src = source.constBegin();
        fromUtf8(&src, &len, &dst, &outlen);
#endif
    }

    std::cout << "Done in " <<  (timer.nsecsElapsed() / Iterations) << "ns"<< std::endl;

#ifdef USE_ICONV
    iconv_close(iconv_cd);
#endif
}
