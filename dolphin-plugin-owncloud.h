#ifndef DOLPHIN_PLUGIN_OWNCLOUD_H
#define DOLPHIN_PLUGIN_OWNCLOUD_H


#include <kversioncontrolplugin2.h>
#include <KDE/KFileItem>

class QTimer;
class KAction;
class QLocalSocket;

class DolphinPluginOwnCloud : public KVersionControlPlugin2
{
    Q_OBJECT

public:
    DolphinPluginOwnCloud(QObject* parent, const QList<QVariant>& args);
    virtual ~DolphinPluginOwnCloud();

    bool beginRetrieval(const QString& directory);
    void endRetrieval();
    QList<QAction*> actions(const KFileItemList& items) const;
    ItemVersion itemVersion(const KFileItem& item) const;
    QString fileName() const;

private slots:
    void getPublicShareLink() const;
    void sendCommand(const QString& command, QLocalSocket* socket = 0) const;

    void onReadyRead();

private:
    ItemVersion itemVersionForString(const QString& version) const;

private:
    mutable QList<KUrl> m_currentUrls;

    KAction* m_publicShareLinkAction;
    QLocalSocket* m_owncloudSocket;
    QMap<QString, ItemVersion> m_status;

    mutable int m_count;
    QString m_socketPath;
};

#endif