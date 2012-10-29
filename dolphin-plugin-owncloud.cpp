#include "dolphin-plugin-owncloud.h"

#include <KDE/KAction>
#include <KDE/KFileItem>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KLocalizedString>

#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QLocalSocket>
#include <QThread>
#include <QFileInfo>

K_PLUGIN_FACTORY(DolphinPluginOwnCloudFactory, registerPlugin<DolphinPluginOwnCloud>();)
K_EXPORT_PLUGIN(DolphinPluginOwnCloudFactory("dolphin-plugin-owncloud"))


DolphinPluginOwnCloud::DolphinPluginOwnCloud(QObject* parent, const QList< QVariant >& args)
    : KVersionControlPlugin2(parent)
    , m_owncloudSocket(new QLocalSocket(this))
{
    Q_UNUSED(args);
    qDebug() << Q_FUNC_INFO;

    m_dummyAction = new KAction(this);
    m_dummyAction->setIcon(KIcon("internet-web-browser"));
    m_dummyAction->setText(i18nc("@item:inmenu", "Copy public link to clipboard"));
    connect(m_dummyAction, SIGNAL(triggered()),
            this, SLOT(showStupidBox()));

    connect(m_owncloudSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
    m_owncloudSocket->connectToServer("ownCloud");
}

DolphinPluginOwnCloud::~DolphinPluginOwnCloud()
{

}

bool DolphinPluginOwnCloud::beginRetrieval(const QString& directory)
{
    qDebug() << Q_FUNC_INFO;

    QLocalSocket* socket = new QLocalSocket();

    sendCommand("RETRIEVE_STATUS:"+directory.toUtf8(), socket);
    socket->waitForReadyRead();

    while(socket->canReadLine())
    {
        QString line = socket->readLine().trimmed();
        QStringList splittedLine = line.split(":");

        if(splittedLine.count() != 3) continue;

        QString filePath = splittedLine.at(2);
        QString status = splittedLine.at(1);

        if(splittedLine.first().startsWith("STATUS"))
        {
            QFileInfo fileInfo( filePath );
            if(fileInfo.exists())
            {
//                 qDebug() << "storing: " << filePath << status;
                m_status[filePath] = itemVersionForString(status);
            }
            else
                qDebug() << "received invalid file path";
        }
        else
        {
            qDebug() << "Hit race condition :\"";
            Q_ASSERT(false);
        }

//         qDebug() << "FILESTATUS RECEIVED: " << line;

    }

    socket->close();
    socket->deleteLater();

    return true;
}

void DolphinPluginOwnCloud::endRetrieval()
{
    qDebug() << Q_FUNC_INFO;
}

QList< QAction* > DolphinPluginOwnCloud::actions(const KFileItemList& items) const
{
    qDebug() << Q_FUNC_INFO;
    m_currentUrls.clear();
    foreach(const KFileItem& item, items)
    {
        m_currentUrls.append(item.mostLocalUrl());
    }

    return (QList<QAction*>() << m_dummyAction);
}

KVersionControlPlugin2::ItemVersion DolphinPluginOwnCloud::itemVersion(const KFileItem& item) const
{
    qDebug() << Q_FUNC_INFO << item.localPath();

    // pretend everything is synced until we have an api for this
    return m_status.value(item.localPath(), UnversionedVersion);
}

QString DolphinPluginOwnCloud::fileName() const
{
    const QLatin1String fileName(".csync_journal.db");
    qDebug() << Q_FUNC_INFO << fileName;
    return QLatin1String(fileName);
}

void DolphinPluginOwnCloud::sendCommand(const QString& command, QLocalSocket* socket) const
{
    if(!socket)
        socket = m_owncloudSocket;


    qDebug() << Q_FUNC_INFO << command;

    if(!socket->isOpen())
    {
        socket->connectToServer("ownCloud");
        socket->waitForConnected();
    }

    QString localCommand = command;
    socket->write(localCommand.append("\n").toUtf8());
}

void DolphinPluginOwnCloud::showStupidBox() const
{
    sendCommand(QString(QLatin1String("PUBLIC_SHARE_LINK:%1")).arg(m_currentUrls.first().toLocalFile()));
}


void DolphinPluginOwnCloud::onReadyRead()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    Q_ASSERT(socket);

    qDebug() << "******************************* JAAAAAAAAAAAAAAAAAAAAAAA";
    while(socket->canReadLine())
    {
        QString line = socket->readLine();
        qDebug() << Q_FUNC_INFO << line;
        if(line.startsWith("UPDATE_VIEW"))
            emit itemVersionsChanged();
    }
}


KVersionControlPlugin2::ItemVersion DolphinPluginOwnCloud::itemVersionForString(const QString& version) const
{
    qDebug() << Q_FUNC_INFO << version;

    if(version == QLatin1String("STATUS_NONE"))
    {
        return NormalVersion;
    }
    if(version == QLatin1String("STATUS_NEW"))
    {
        return AddedVersion;
    }
    if(version == QLatin1String("STATUS_EVAL"))
    {
        return LocallyModifiedVersion;
    }
    else if( version == QLatin1String("STATUS_STAT_ERROR") || version == QLatin1String("STATUS_ERROR"))
    {
        return UnversionedVersion;
    }
    else if(version == QLatin1String("STATUS_REMOVE") || version == QLatin1String("STATUS_UPDATED"))
    {
        qDebug() << Q_FUNC_INFO << version;
        return UnversionedVersion;
    }
    else if(version == QLatin1String("STATUS_CONFLICT"))
    {
        return ConflictingVersion;
    }
    else if(version == QLatin1String("STATUS_REMOVE"))
    {
        return RemovedVersion;
    }
    else if(version == QLatin1String("STATUS_IGNORE"))
    {
        return IgnoredVersion;
    }

    qWarning() << Q_FUNC_INFO << "unknown status: " << version;
    Q_ASSERT(false);
    return UnversionedVersion;
}
