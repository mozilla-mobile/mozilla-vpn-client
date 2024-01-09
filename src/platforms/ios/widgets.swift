//
//  widgets.swift
//  widgets
//
//  Created by Matt Lichtenstein on 1/5/24.
//

import WidgetKit
import SwiftUI

struct Provider: TimelineProvider {
        
    func placeholder(in context: Context) -> SimpleEntry {
        SimpleEntry(date: Date(), emoji: "😀")
    }

    func getSnapshot(in context: Context, completion: @escaping (SimpleEntry) -> ()) {
        let entry = SimpleEntry(date: Date(), emoji: "😀")
        completion(entry)
    }

    func getTimeline(in context: Context, completion: @escaping (Timeline<Entry>) -> ()) {
        var entries: [SimpleEntry] = []

        // Generate a timeline consisting of five entries an hour apart, starting from the current date.
        let currentDate = Date()
        for hourOffset in 0 ..< 5 {
            let entryDate = Calendar.current.date(byAdding: .hour, value: hourOffset, to: currentDate)!
            let entry = SimpleEntry(date: entryDate, emoji: "😀")
            entries.append(entry)
        }

        let timeline = Timeline(entries: entries, policy: .atEnd)
        completion(timeline)
    }
}

struct SimpleEntry: TimelineEntry {
    let date: Date
    let emoji: String
}

struct widgetsEntryView : View {
    
    var impl = IOSWidgetsImpl()
    
    var entry: Provider.Entry

    var body: some View {
        VStack {
            HStack {
                Spacer()
                Text(Date.now, style: .timer)
//                Spacer()  
            }
            Text(impl.retrieveFromUserDefaults())
        }
    }
}

struct widgets: Widget {
    let kind: String = "widgets"

    var body: some WidgetConfiguration {
        StaticConfiguration(kind: kind, provider: Provider()) { entry in
                widgetsEntryView(entry: entry)
                    .containerBackground(.fill.tertiary, for: .widget)
        }
        .configurationDisplayName("My Widget")
        .description("This is an example widget.")
    }
}

#Preview(as: .systemSmall) {
    widgets()
} timeline: {
    SimpleEntry(date: .now, emoji: "🤩")
    SimpleEntry(date: .now, emoji: "xxx")
}

#Preview(as: .systemMedium) {
    widgets()
} timeline: {
    SimpleEntry(date: .now, emoji: "🤩")
    SimpleEntry(date: .now, emoji: "xxx")
}

#Preview(as: .systemLarge) {
    widgets()
} timeline: {
    SimpleEntry(date: .now, emoji: "🤩")
    SimpleEntry(date: .now, emoji: "xxx")
}
