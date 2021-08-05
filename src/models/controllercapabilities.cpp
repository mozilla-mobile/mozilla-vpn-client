#include "controllercapabilities.h"

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMetaObject>
#include <QMetaProperty>

void ControllerCapabilities::read(const QJsonObject & obj){
   auto mo = metaObject();
   for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i){
       QMetaProperty prop =  mo->property(i);
       QString name = prop.name();
       if(!obj.contains(name)){
           continue;
       }
       int jsonValue = obj[name].toInt();
      prop.write(this,jsonValue);
   }
}

QJsonObject ControllerCapabilities::serialize(){
    QJsonObject obj;
    auto mo = metaObject();
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i){
        QMetaProperty prop =  mo->property(i);
        QString name = prop.name();
        obj[name] = prop.read(this).toInt();
    }
    return obj;
}
ControllerCapabilities::ControllerCapabilities(const ControllerCapabilities& other) {
  *this = other;
}

ControllerCapabilities& ControllerCapabilities::operator=(const ControllerCapabilities& other) {
  if (this == &other) return *this;
  m_SplitTunnel = other.m_SplitTunnel;
  m_MultiHop = other.m_MultiHop;
  return *this;
}
