#ifndef ANDROIDAPPLISTPROVIDER_H
#define ANDROIDAPPLISTPROVIDER_H

#include <applistprovider.h>
#include <QObject>

class AndroidAppListProvider : public AppListProvider {
  Q_OBJECT
 public:
  AndroidAppListProvider(QObject* parent);
  void getApplicationList() override;
};

#endif  // ANDROIDAPPLISTPROVIDER_H
