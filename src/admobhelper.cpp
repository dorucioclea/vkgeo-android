#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "admobhelper.h"

const QString AdMobHelper::ADMOB_APP_ID              ("ca-app-pub-2455088855015693~9304224395");
const QString AdMobHelper::ADMOB_BANNERVIEW_UNIT_ID  ("ca-app-pub-3940256099942544/6300978111");
const QString AdMobHelper::ADMOB_INTERSTITIAL_UNIT_ID("ca-app-pub-3940256099942544/1033173712");

AdMobHelper::AdMobHelper(QObject *parent) : QObject(parent)
{
    Initialized      = false;
    BannerViewHeight = 0;
}

AdMobHelper::~AdMobHelper()
{
}

int AdMobHelper::bannerViewHeight() const
{
    return BannerViewHeight;
}

void AdMobHelper::initialize()
{
    if (!Initialized) {
        QAndroidJniObject j_app_id               = QAndroidJniObject::fromString(ADMOB_APP_ID);
        QAndroidJniObject j_interstitial_unit_id = QAndroidJniObject::fromString(ADMOB_INTERSTITIAL_UNIT_ID);

        QtAndroid::androidActivity().callMethod<void>("initAds", "(Ljava/lang/String;Ljava/lang/String;)V", j_app_id.object<jstring>(),
                                                                                                            j_interstitial_unit_id.object<jstring>());

        Initialized = true;
    }
}

void AdMobHelper::showBannerView()
{
    if (Initialized) {
        QAndroidJniObject j_unit_id = QAndroidJniObject::fromString(ADMOB_BANNERVIEW_UNIT_ID);

        QtAndroid::androidActivity().callMethod<void>("showBannerView", "(Ljava/lang/String;)V", j_unit_id.object<jstring>());
    }
}

void AdMobHelper::hideBannerView()
{
    if (Initialized) {
        QtAndroid::androidActivity().callMethod<void>("hideBannerView");
    }
}

void AdMobHelper::showInterstitial()
{
    if (Initialized) {
        QtAndroid::androidActivity().callMethod<void>("showInterstitial");
    }
}

void AdMobHelper::setBannerViewHeight(int height)
{
    BannerViewHeight = height;

    emit bannerViewHeightChanged(BannerViewHeight);
}
