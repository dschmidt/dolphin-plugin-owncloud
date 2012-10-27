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
    m_dummyAction->setText(i18nc("@item:inmenu", "Do something not very useful..."));
    connect(m_dummyAction, SIGNAL(triggered()),
            this, SLOT(showStupidBox()));

}

DolphinPluginOwnCloud::~DolphinPluginOwnCloud()
{

}

bool DolphinPluginOwnCloud::beginRetrieval(const QString& directory)
{
    qDebug() << Q_FUNC_INFO;

    sendCommand("RETRIEVE_STATUS:"+directory.toUtf8());

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
    return NormalVersion;
}

QString DolphinPluginOwnCloud::fileName() const
{
    const QLatin1String fileName(".csync_journal.db");
    qDebug() << Q_FUNC_INFO << fileName;
    return QLatin1String(fileName);
}

void DolphinPluginOwnCloud::sendCommand(const QString& command) const
{
    if ( QThread::currentThread() != thread() )
    {
        QMetaObject::invokeMethod(this, "sendCommand", Qt::QueuedConnection, Q_ARG(QString, command));
        return;
    }


    qDebug() << Q_FUNC_INFO << command;

    if(!m_owncloudSocket)
        m_owncloudSocket
    if(!m_owncloudSocket->isOpen())
    {
        m_owncloudSocket->connectToServer("ownCloud");
        m_owncloudSocket->waitForConnected();
    }

    QString localCommand = command;
    m_owncloudSocket->write(localCommand.append("\n").toUtf8());
}

void DolphinPluginOwnCloud::showStupidBox() const
{
    QMessageBox::information(0, "Stupid Alert", "Doing an api request for " + m_currentUrls.first().toLocalFile());

    sendCommand(QString(QLatin1String("ONLINELINK:%1")).arg(m_currentUrls.first().toLocalFile()));
}