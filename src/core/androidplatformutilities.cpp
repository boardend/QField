/***************************************************************************
                            androidplatformutilities.cpp  -  utilities for qfield

                              -------------------
              begin                : February 2016
              copyright            : (C) 2016 by Matthias Kuhn
              email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "androidplatformutilities.h"
#include "androidpicturesource.h"
#include "androidprojectsource.h"
#include "androidviewstatus.h"
#include "feedback.h"
#include "fileutils.h"

#include <QMap>
#include <QString>
#include <QtAndroid>
#include <QDebug>
#include <QAndroidJniEnvironment>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QFile>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>

AndroidPlatformUtilities::AndroidPlatformUtilities()
  : mActivity( QtAndroid::androidActivity() )
{
}

class FileCopyThread : public QThread
{
    Q_OBJECT

public:
    FileCopyThread( const QString &source, const QString &destination, Feedback *feedback )
        : QThread()
        , mSource( source )
        , mDestination( destination )
        , mFeedback( feedback )
    {
    }

private:

    void run() override {
        FileUtils::copyRecursively( mSource, mDestination, mFeedback );
    }

    QString mSource;
    QString mDestination;
    Feedback *mFeedback;
};


void AndroidPlatformUtilities::initSystem()
{
  // Copy data away from the virtual path `assets:/` to a path accessible also for non-qt-based libs
  QString appDataLocation = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
  mSystemGenericDataLocation = appDataLocation + QStringLiteral( "/share" );
  QFile gitRevFile( appDataLocation + QStringLiteral( "/gitRev" ) );
  gitRevFile.open( QIODevice::ReadWrite );
  QByteArray gitRev = getIntentExtra( "GIT_REV" ).toLocal8Bit();
  if ( gitRevFile.readAll() != gitRev )
  {
    int argc = 0;
    QApplication app( argc, nullptr );
    QQmlApplicationEngine engine;
    Feedback feedback;
    qmlRegisterType<Feedback>( "org.qfield", 1, 0, "Feedback" );
    engine.rootContext()->setContextProperty( "feedback", &feedback );
    engine.load(QUrl(QStringLiteral("qrc:/qml/SystemLoader.qml")));

    QMetaObject::invokeMethod(&app, [this, &app, &feedback]{
      FileCopyThread *thread = new FileCopyThread( QStringLiteral( "assets:/share" ), mSystemGenericDataLocation, &feedback );
      app.connect( thread, &QThread::finished, &app, QApplication::quit );
      app.connect( thread, &QThread::finished, thread, &QThread::deleteLater );
      feedback.moveToThread( thread );
      thread->start();
    });
    app.exec();

    gitRevFile.write( gitRev );
  }
}

QString AndroidPlatformUtilities::systemGenericDataLocation() const
{
  return mSystemGenericDataLocation;
}

QString AndroidPlatformUtilities::qgsProject() const
{
  return getIntentExtra( "QGS_PROJECT" );
}

QString AndroidPlatformUtilities::qfieldDataDir() const
{
  return getIntentExtra( "QFIELD_DATA_DIR" );
}

QString AndroidPlatformUtilities::getIntentExtra( const QString &extra, QAndroidJniObject extras ) const
{
  if ( extras == nullptr )
  {
    extras = getNativeExtras();
  }
  if ( extras.isValid() )
  {
    QAndroidJniObject extraJni = QAndroidJniObject::fromString( extra );
    extraJni = extras.callObjectMethod( "getString", "(Ljava/lang/String;)Ljava/lang/String;", extraJni.object<jstring>() );
    if ( extraJni.isValid() )
    {
      return extraJni.toString();
    }
  }
  return QString();
}

QAndroidJniObject AndroidPlatformUtilities::getNativeIntent() const
{
  if ( mActivity.isValid() )
  {
    QAndroidJniObject intent = mActivity.callObjectMethod( "getIntent", "()Landroid/content/Intent;" );
    return intent;
  }
  return nullptr;
}

QAndroidJniObject AndroidPlatformUtilities::getNativeExtras() const
{
  QAndroidJniObject intent = getNativeIntent();
  if ( intent.isValid() )
  {
    QAndroidJniObject extras = intent.callObjectMethod( "getExtras", "()Landroid/os/Bundle;" );

    return extras;
  }
  return nullptr;
}

PictureSource *AndroidPlatformUtilities::getCameraPicture( const QString &prefix, const QString &pictureFilePath, const QString &suffix )
{
  if ( !checkCameraPermissions() )
    return nullptr;

  QAndroidJniObject activity = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldCameraPictureActivity" ) );
  QAndroidJniObject intent = QAndroidJniObject( "android/content/Intent", "(Ljava/lang/String;)V", activity.object<jstring>() );
  QAndroidJniObject packageName = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield" ) );

  intent.callObjectMethod( "setClassName", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", packageName.object<jstring>(), activity.object<jstring>() );

  QAndroidJniObject pictureFilePath_label = QAndroidJniObject::fromString( "pictureFilePath" );
  QAndroidJniObject pictureFilePath_value = QAndroidJniObject::fromString( pictureFilePath );

  intent.callObjectMethod( "putExtra",
                           "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                           pictureFilePath_label.object<jstring>(),
                           pictureFilePath_value.object<jstring>() );

  QAndroidJniObject prefix_label = QAndroidJniObject::fromString( "prefix" );
  QAndroidJniObject prefix_value = QAndroidJniObject::fromString( prefix );
  intent.callObjectMethod( "putExtra",
                           "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                           prefix_label.object<jstring>(),
                           prefix_value.object<jstring>() );

  QAndroidJniObject suffix_label = QAndroidJniObject::fromString( "suffix" );
  QAndroidJniObject suffix_value = QAndroidJniObject::fromString( suffix );
  intent.callObjectMethod( "putExtra",
                           "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                           suffix_label.object<jstring>(),
                           suffix_value.object<jstring>() );

  AndroidPictureSource *pictureSource = new AndroidPictureSource( prefix );

  QtAndroid::startActivity( intent.object<jobject>(), 171, pictureSource );

  return pictureSource;
}

PictureSource *AndroidPlatformUtilities::getGalleryPicture( const QString &prefix, const QString &pictureFilePath )
{
  QAndroidJniObject activity = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldGalleryPictureActivity" ) );
  QAndroidJniObject intent = QAndroidJniObject( "android/content/Intent", "(Ljava/lang/String;)V", activity.object<jstring>() );
  QAndroidJniObject packageName = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield" ) );

  intent.callObjectMethod( "setClassName", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", packageName.object<jstring>(), activity.object<jstring>() );

  QAndroidJniObject pictureFilePath_label = QAndroidJniObject::fromString( "pictureFilePath" );
  QAndroidJniObject pictureFilePath_value = QAndroidJniObject::fromString( pictureFilePath );

  intent.callObjectMethod( "putExtra",
                           "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                           pictureFilePath_label.object<jstring>(),
                           pictureFilePath_value.object<jstring>() );

  QAndroidJniObject prefix_label = QAndroidJniObject::fromString( "prefix" );
  QAndroidJniObject prefix_value = QAndroidJniObject::fromString( prefix );

  intent.callObjectMethod( "putExtra",
                           "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                           prefix_label.object<jstring>(),
                           prefix_value.object<jstring>() );

  AndroidPictureSource *pictureSource = new AndroidPictureSource( prefix );

  QtAndroid::startActivity( intent.object<jobject>(), 171, pictureSource );

  return pictureSource;
}

ViewStatus *AndroidPlatformUtilities::open( const QString &uri )
{
  checkWriteExternalStoragePermissions();

  QAndroidJniObject activity = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldOpenExternallyActivity" ) );
  QAndroidJniObject intent = QAndroidJniObject( "android/content/Intent", "(Ljava/lang/String;)V", activity.object<jstring>() );
  QAndroidJniObject packageName = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield" ) );

  intent.callObjectMethod( "setClassName", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", packageName.object<jstring>(), activity.object<jstring>() );

  QMimeDatabase db;
  QAndroidJniObject filepath_label = QAndroidJniObject::fromString( "filepath" );
  QAndroidJniObject filepath = QAndroidJniObject::fromString( uri );
  QAndroidJniObject filetype_label = QAndroidJniObject::fromString( "filetype" );
  QAndroidJniObject filetype = QAndroidJniObject::fromString( db.mimeTypeForFile( uri ).name() );

  intent.callObjectMethod( "putExtra", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", filepath_label.object<jstring>(), filepath.object<jstring>() );
  intent.callObjectMethod( "putExtra", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", filetype_label.object<jstring>(), filetype.object<jstring>() );

  AndroidViewStatus *viewStatus = new AndroidViewStatus();
  QtAndroid::startActivity( intent.object<jobject>(), 102, viewStatus );

  return viewStatus;
}

ProjectSource *AndroidPlatformUtilities::openProject()
{
  checkWriteExternalStoragePermissions();

  QAndroidJniObject activity = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldProjectActivity" ) );
  QAndroidJniObject intent = QAndroidJniObject( "android/content/Intent", "(Ljava/lang/String;)V", activity.object<jstring>() );

  QAndroidJniObject packageName = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield" ) );
  QAndroidJniObject className = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldProjectActivity" ) );

  intent.callObjectMethod( "setClassName", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", packageName.object<jstring>(), className.object<jstring>() );

  AndroidProjectSource *projectSource = nullptr;

  if ( intent.isValid() )
  {
    projectSource = new AndroidProjectSource();
    QtAndroid::startActivity( intent.object<jobject>(), 103, projectSource );
  }
  else
  {
    qDebug() << "Something went wrong creating the project intent";
  }

  return projectSource;
}

bool AndroidPlatformUtilities::checkPositioningPermissions() const
{
  // First check for coarse permissions. If the user configured QField to only get coarse permissions
  // it's his wish and we just let it be.
  QtAndroid::PermissionResult r = QtAndroid::checkPermission( "android.permission.ACCESS_COARSE_LOCATION" );
  if ( r == QtAndroid::PermissionResult::Denied )
  {
    return checkAndAcquirePermissions( "android.permission.ACCESS_FINE_LOCATION" );
  }
  return true;
}

bool AndroidPlatformUtilities::checkCameraPermissions() const
{
  return checkAndAcquirePermissions( "android.permission.CAMERA" );
}

bool AndroidPlatformUtilities::checkWriteExternalStoragePermissions() const
{
  return checkAndAcquirePermissions( "android.permission.WRITE_EXTERNAL_STORAGE" );
}

bool AndroidPlatformUtilities::checkAndAcquirePermissions( const QString &permissionString ) const
{
  QtAndroid::PermissionResult r = QtAndroid::checkPermission( permissionString );
  if ( r == QtAndroid::PermissionResult::Denied )
  {
    QtAndroid::requestPermissionsSync( QStringList() << permissionString );
    r = QtAndroid::checkPermission( permissionString );
    if ( r == QtAndroid::PermissionResult::Denied )
    {
      return false;
    }
  }
  return true;
}

void AndroidPlatformUtilities::setScreenLockPermission( const bool allowLock )
{
  if ( mActivity.isValid() )
  {
    QtAndroid::runOnAndroidThread( [allowLock]
    {
      QAndroidJniObject activity = QtAndroid::androidActivity();
      if ( activity.isValid() )
      {
        QAndroidJniObject window =
        activity.callObjectMethod( "getWindow", "()Landroid/view/Window;" );

        if ( window.isValid() )
        {
          const int FLAG_KEEP_SCREEN_ON = 128;
          if ( !allowLock )
          {
            window.callMethod<void>( "addFlags", "(I)V", FLAG_KEEP_SCREEN_ON );
          }
          else
          {
            window.callMethod<void>( "clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON );
          }
        }
      }

      QAndroidJniEnvironment env;
      if ( env->ExceptionCheck() )
      {
        env->ExceptionClear();
      }
    } );
  }
}

void AndroidPlatformUtilities::showRateThisApp() const
{

  QAndroidJniObject activity = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldAppRaterActivity" ) );
  QAndroidJniObject intent = QAndroidJniObject( "android/content/Intent", "(Ljava/lang/String;)V", activity.object<jstring>() );

  QAndroidJniObject packageName = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield" ) );
  QAndroidJniObject className = QAndroidJniObject::fromString( QStringLiteral( "ch.opengis.qfield.QFieldAppRaterActivity" ) );

  intent.callObjectMethod( "setClassName", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", packageName.object<jstring>(), className.object<jstring>() );

  QtAndroid::startActivity( intent.object<jobject>(), 104 );
}

#include "androidplatformutilities.moc"
