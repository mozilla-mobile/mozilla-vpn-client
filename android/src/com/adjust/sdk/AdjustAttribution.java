package com.adjust.sdk;

import org.json.JSONObject;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.ObjectStreamField;
import java.io.Serializable;

/**
 * Created by pfms on 07/11/14.
 */
public class AdjustAttribution implements Serializable {
  private static final long serialVersionUID = 1L;
  private static final ObjectStreamField[] serialPersistentFields = {
      new ObjectStreamField("trackerToken", String.class),
      new ObjectStreamField("trackerName", String.class),
      new ObjectStreamField("network", String.class),
      new ObjectStreamField("campaign", String.class),
      new ObjectStreamField("adgroup", String.class),
      new ObjectStreamField("creative", String.class),
      new ObjectStreamField("clickLabel", String.class),
      new ObjectStreamField("adid", String.class),
      new ObjectStreamField("costType", String.class),
      new ObjectStreamField("costAmount", Double.class),
      new ObjectStreamField("costCurrency", String.class),
  };

  public String trackerToken;
  public String trackerName;
  public String network;
  public String campaign;
  public String adgroup;
  public String creative;
  public String clickLabel;
  public String adid;
  public String costType;
  public Double costAmount;
  public String costCurrency;

  public static AdjustAttribution fromJson(JSONObject jsonObject, String adid, String sdkPlatform) {
    if (jsonObject == null)
      return null;

    AdjustAttribution attribution = new AdjustAttribution();

    if ("unity".equals(sdkPlatform)) {
      // Unity platform.
      attribution.trackerToken = jsonObject.optString("tracker_token", "");
      attribution.trackerName = jsonObject.optString("tracker_name", "");
      attribution.network = jsonObject.optString("network", "");
      attribution.campaign = jsonObject.optString("campaign", "");
      attribution.adgroup = jsonObject.optString("adgroup", "");
      attribution.creative = jsonObject.optString("creative", "");
      attribution.clickLabel = jsonObject.optString("click_label", "");
      attribution.adid = adid != null ? adid : "";
      attribution.costType = jsonObject.optString("cost_type", "");
      attribution.costAmount = jsonObject.optDouble("cost_amount", 0);
      attribution.costCurrency = jsonObject.optString("cost_currency", "");
    } else {
      // Rest of all platforms.
      attribution.trackerToken = jsonObject.optString("tracker_token");
      attribution.trackerName = jsonObject.optString("tracker_name");
      attribution.network = jsonObject.optString("network");
      attribution.campaign = jsonObject.optString("campaign");
      attribution.adgroup = jsonObject.optString("adgroup");
      attribution.creative = jsonObject.optString("creative");
      attribution.clickLabel = jsonObject.optString("click_label");
      attribution.adid = adid;
      attribution.costType = jsonObject.optString("cost_type");
      attribution.costAmount = jsonObject.optDouble("cost_amount");
      attribution.costCurrency = jsonObject.optString("cost_currency");
    }

    return attribution;
  }

  @Override
  public boolean equals(Object other) {
    if (other == this)
      return true;
    if (other == null)
      return false;
    if (getClass() != other.getClass())
      return false;
    AdjustAttribution otherAttribution = (AdjustAttribution) other;

    if (!Util.equalString(trackerToken, otherAttribution.trackerToken))
      return false;
    if (!Util.equalString(trackerName, otherAttribution.trackerName))
      return false;
    if (!Util.equalString(network, otherAttribution.network))
      return false;
    if (!Util.equalString(campaign, otherAttribution.campaign))
      return false;
    if (!Util.equalString(adgroup, otherAttribution.adgroup))
      return false;
    if (!Util.equalString(creative, otherAttribution.creative))
      return false;
    if (!Util.equalString(clickLabel, otherAttribution.clickLabel))
      return false;
    if (!Util.equalString(adid, otherAttribution.adid))
      return false;
    if (!Util.equalString(costType, otherAttribution.costType))
      return false;
    if (!Util.equalsDouble(costAmount, otherAttribution.costAmount))
      return false;
    if (!Util.equalString(costCurrency, otherAttribution.costCurrency))
      return false;

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 17;
    hashCode = 37 * hashCode + Util.hashString(trackerToken);
    hashCode = 37 * hashCode + Util.hashString(trackerName);
    hashCode = 37 * hashCode + Util.hashString(network);
    hashCode = 37 * hashCode + Util.hashString(campaign);
    hashCode = 37 * hashCode + Util.hashString(adgroup);
    hashCode = 37 * hashCode + Util.hashString(creative);
    hashCode = 37 * hashCode + Util.hashString(clickLabel);
    hashCode = 37 * hashCode + Util.hashString(adid);
    hashCode = 37 * hashCode + Util.hashString(costType);
    hashCode = 37 * hashCode + Util.hashDouble(costAmount);
    hashCode = 37 * hashCode + Util.hashString(costCurrency);

    return hashCode;
  }

  @Override
  public String toString() {
    return Util.formatString(
        "tt:%s tn:%s net:%s cam:%s adg:%s cre:%s cl:%s adid:%s ct:%s ca:%.2f cc:%s", trackerToken,
        trackerName, network, campaign, adgroup, creative, clickLabel, adid, costType, costAmount,
        costCurrency);
  }

  private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
  }

  private void readObject(ObjectInputStream stream) throws ClassNotFoundException, IOException {
    stream.defaultReadObject();
  }
}
