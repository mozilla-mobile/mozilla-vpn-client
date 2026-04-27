//
//  ToggleWidgetBundle.swift
//  ToggleWidget
//
//  Created by Matt Cleinman on 4/17/26.
//

import WidgetKit
import SwiftUI

@main
struct ToggleWidgetBundle: WidgetBundle {
    var body: some Widget {
      if #available(iOS 18.0, *) {
        ToggleWidgetControl()
      }
    }
}
