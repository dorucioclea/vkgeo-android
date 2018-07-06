#include <QtPositioning/QGeoCoordinate>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "vkhelper.h"
#include "vkservice.h"

VKService::VKService(QObject *parent) : QObject(parent)
{
    connect(&UpdateFriendsTimer, &QTimer::timeout, this, &VKService::updateFriends);

    UpdateFriendsTimer.setInterval(UPDATE_FRIENDS_TIMER_INTERVAL);
    UpdateFriendsTimer.start();
}

VKService::~VKService()
{
}

void VKService::authStateChanged(int authState)
{
    if (authState == VKAuthState::StateAuthorized) {
        emit updateFriends();
    }
}

void VKService::locationReported()
{
    emit updateTrackedFriendsLocations(true);
}

void VKService::friendsUpdated()
{
    VKHelper *vk_helper = qobject_cast<VKHelper *>(QObject::sender());

    if (vk_helper != NULL) {
        QVariantMap friends_data = vk_helper->getFriends();

        foreach (QString key, friends_data.keys()) {
            QVariantMap frnd = friends_data[key].toMap();

            if (FriendsData.contains(key)) {
                if (FriendsData[key].toMap().contains("nearby")) {
                    frnd["nearby"] = FriendsData[key].toMap()["nearby"].toBool();
                } else {
                    frnd["nearby"] = false;
                }
            } else {
                frnd["nearby"] = false;
            }

            friends_data[key] = frnd;
        }

        FriendsData = friends_data;
    }

    emit updateTrackedFriendsLocations(true);
}

void VKService::trackedFriendLocationUpdated(QString id, qint64 updateTime, qreal latitude, qreal longitude)
{
    Q_UNUSED(updateTime)

    VKHelper *vk_helper = qobject_cast<VKHelper *>(QObject::sender());

    if (vk_helper != NULL) {
        if (FriendsData.contains(id)) {
            QVariantMap frnd = FriendsData[id].toMap();

            if (vk_helper->locationValid()) {
                QGeoCoordinate my_coordinate(vk_helper->locationLatitude(), vk_helper->locationLongitude());
                QGeoCoordinate frnd_coordinate(latitude, longitude);

                if (my_coordinate.distanceTo(frnd_coordinate) < NEARBY_DISTANCE) {
                    if (!frnd.contains("nearby") || !frnd["nearby"].toBool()) {
                        frnd["nearby"] = true;

                        if (frnd.contains("firstName") && frnd.contains("lastName")) {
                            QAndroidJniObject j_title = QAndroidJniObject::fromString(tr("New friends nearby"));
                            QAndroidJniObject j_text  = QAndroidJniObject::fromString(tr("%1 is nearby")
                                                                                        .arg(QString("%1 %2").arg(frnd["firstName"].toString())
                                                                                                             .arg(frnd["lastName"].toString())));

                            QtAndroid::androidService().callMethod<void>("showFriendsNearbyNotification", "(Ljava/lang/String;Ljava/lang/String;)V", j_title.object<jstring>(),
                                                                                                                                                     j_text.object<jstring>());
                        }
                    }
                } else {
                    frnd["nearby"] = false;
                }
            }

            FriendsData[id] = frnd;
        }
    }
}
