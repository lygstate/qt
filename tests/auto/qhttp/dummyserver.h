/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
// Use if you need

class DummyHttpServer : public QTcpServer
{
    Q_OBJECT
public:
    DummyHttpServer() : phase(Header)
        { listen(); }

protected:
    enum {
        Header,
        Data1,
        Data2,
        Close
    } phase;
    void incomingConnection(int socketDescriptor)
    {
        QSslSocket *socket = new QSslSocket(this);
        socket->setSocketDescriptor(socketDescriptor, QAbstractSocket::ConnectedState);
        socket->ignoreSslErrors();
        socket->startServerEncryption();
        connect(socket, SIGNAL(readyRead()), SLOT(handleReadyRead()));
    }

public slots:
    void handleReadyRead()
    {
        QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
        socket->readAll();
        if (phase != Header)
            return;

        phase = Data1;
        static const char header[] =
            "HTTP/1.0 200 OK\r\n"
            "Date: Fri, 07 Sep 2007 12:33:18 GMT\r\n"
            "Server: Apache\r\n"
            "Expires:\r\n"
            "Cache-Control:\r\n"
            "Pragma:\r\n"
            "Last-Modified: Thu, 06 Sep 2007 08:52:06 +0000\r\n"
            "Etag: a700f59a6ccb1ad39af68d998aa36fb1\r\n"
            "Vary: Accept-Encoding\r\n"
            "Content-Length: 6560\r\n"
            "Connection: close\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "\r\n";


        socket->write(header, sizeof header - 1);
        connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(handleBytesWritten()), Qt::QueuedConnection);
    }

    void handleBytesWritten()
    {
        QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
        if (socket->bytesToWrite() != 0)
            return;

        if (phase == Data1) {
            QByteArray data(4096, 'a');
            socket->write(data);
            phase = Data2;
        } else if (phase == Data2) {
            QByteArray data(2464, 'a');
            socket->write(data);
            phase = Close;
        } else {
            //socket->disconnectFromHost();
            //socket->deleteLater();
        }
    }
};
